#include "ffdecoder.h"
#include "fflog.h"
#include "ffcodec.h"

FFDecoder::FFDecoder() {
    m_video = NULL;
    m_audio = NULL;
    m_vfmt.reset();
}

FFDecoder::~FFDecoder() {
    closeVideo();
    closeAudio();
}

long FFDecoder::openCodec(ff_codec_t codec, FFCodecID codec_id) {
    FFCodec *pCodec = (FFCodec *)codec;
    returnv_if_fail(pCodec, -1);

    AVCodecID av_codec_id = GetAVCodecID(codec_id);
    return_if_fail(av_codec_id != AV_CODEC_ID_NONE, -1);

    pCodec->codec = avcodec_find_decoder(av_codec_id);
    returnv_if_fail(pCodec->codec, -1);

    pCodec->avctx = avcodec_alloc_context3(pCodec->codec);
    returnv_if_fail(pCodec->avctx, -1);

    int iret = avcodec_open2(pCodec->avctx, pCodec->codec, NULL);
    returnv_if_fail(iret == 0, -1);

    pCodec->frame = av_frame_alloc();
    av_init_packet(&pCodec->avpkt);
    return 0;
}

long FFDecoder::openVideo(FFCodecID codec_id) {
    returnv_if_fail(!m_video, 1); // has been opened
    m_video = (ff_codec_t)new FFCodec(FF_MEDIA_VIDEO);
    returnv_if_fail(m_video, -1);
    m_vfmt.reset();

    long lret = openCodec(m_video, codec_id);
    if (lret != 0) {
        safe_delete(m_video);
        LOGE("fail to open ff_codec_id="<<codec_id<<", return=" << lret);
    }
    return lret;
}
void FFDecoder::closeVideo() {
    safe_delete(m_video);
    m_vfmt.reset();
}

long FFDecoder::openAudio(FFCodecID codec_id) {
    returnv_if_fail(!m_audio, 1); // had been opened
    m_audio = (ff_codec_t)new FFCodec(FF_MEDIA_AUDIO);
    returnv_if_fail(m_audio, -1);
    long lret = openCodec(m_audio, codec_id);
    if (lret != 0) {
        safe_delete(m_audio);
        LOGE("fail to open ff_codec_id="<<codec_id<<", return=" << lret);
    }
    return lret;
}
void FFDecoder::closeAudio() {
    safe_delete(m_audio);
}

// return consumed bytes(>0) if success, else < 0
long FFDecoder::decodeVideo(const uint8_t *in_data, int in_size, uint8_t *out_data, int &out_size, 
        const FFVideoFormat &out_fmt) {
    returnv_if_fail(m_video, -1);
    returnv_if_fail(out_data, -1);

    // required output format
    int out_width = out_fmt.width;
    int out_height = out_fmt.heght;
    AVPixelFormat out_pix_fmt = GetAVPixelFormat(out_fmt.pix_fmt);
    if (out_pix_fmt == AV_PIX_FMT_NONE) {
        LOGE("unsupported output ff_pix_fmt="<<out_fmt.pix_fmt);
        return -1;
    }

    FFCodec *pCodec = (FFCodec *)m_video;
    if (!sws_isSupportedOutput(out_pix_fmt)) {
        LOGE("(sws) unsupported output ff_pix_fmt="<<out_fmt.pix_fmt<<", pix_fmt="<<out_pix_fmt);
        return -1;
    }

    // prepare output(linesize and buffer)
    int dst_linesize[4] = { 0 };
    int iret = av_image_fill_linesizes(dst_linesize, out_pix_fmt, out_width);
    returnv_if_fail(iret >= 0, -1);

    uint8_t *dst_data[4] = { 0 };
    iret = av_image_fill_pointers(dst_data, out_pix_fmt, out_height, out_data, dst_linesize);
    returnv_if_fail(iret > 0 && iret <= out_size, -1);
    out_size = iret; // actual output size if success

    // prepare input, flush decoder if input is null & 0.
    pCodec->avpkt.data = (uint8_t *)in_data;
    pCodec->avpkt.size = in_size;

    // decode frame
    int got_frame = 0;
    int consumed_bytes = avcodec_decode_video2(pCodec->avctx, pCodec->frame, &got_frame, &pCodec->avpkt);
    if (consumed_bytes < 0 || got_frame <= 0) {
        LOGE("decode failure or no output, return="<<consumed_bytes);
        return consumed_bytes;
    }

    if (!sws_isSupportedInput(pCodec->avctx->pix_fmt)) {
        LOGE("(sws) unsupported decoded pix_fmt="<<pCodec->avctx->pix_fmt);
        return -1;
    }

    // prepare sws convert
    FFPixelFormat avctx_pix_fmt = GetFFPixelFormat(pCodec->avctx->pix_fmt);
    if (m_vfmt.width != pCodec->avctx->width ||
        m_vfmt.height != pCodec->avctx->height ||
        m_vfmt.pix_fmt != avctx_pix_fmt) 
    {
        m_vfmt.width = pCodec->avctx->width;
        m_vfmt.height = pCodec->avctx->height;
        m_vfmt.pix_fmt = avctx_pix_fmt;

        // reset sws context when format changes
        sws_freeContext(pCodec->swsctx);
        pCodec->swsctx = sws_getContext(pCodec->avctx->width, pCodec->avctx->height, pCodec->avctx->pix_fmt,
                out_width, out_height, out_pix_fmt, SWS_FAST_BILINEAR,
                NULL, NULL, NULL);
    }
    returnv_if_fail(pCodeoc->swsctx, -1);

    // sws convert
    iret = sws_scale(pCodec->swsctx, pCodec->frame->data, pCodec->frame->linesize, 0, pCodec->frame->height,
            dst_data, dst_linesize);
    returnv_if_fail(iret == pCodec->avctx->height, -1);

    return consumed_bytes;
}

// return consumed bytes(>0) if success, else < 0
long FFDecoder::decodeAudio(const uint8_t *in_data, const int in_size, uint8_t *out_data, int &out_size) {
    returnv_if_fail(m_audio, -1);
    returnv_if_fail(in_data && out_data, -1);

    FFCodec *pCodec = (FFCodec *)m_audio;

    // prepare input
    pCodec->avpkt.data = (uint8_t *)in_data;
    pCodec->avpkt.size = in_size;

    // decode frame
    int got_frame = 0;
    int consumed_bytes = avcodec_decode_audio4(pCodec->avctx, pCodec->frame, &got_frame, &pCodec->avpkt);
    if (consumed_bytes < 0 || got_frame <= 0) {
        LOGE("decode failure or no output, return="<<consumed_bytes);
        return consumed_bytes;
    }

    // prepare output
    int data_size = av_get_bytes_per_sample(pCodec->avctx->sample_fmt);
    returnv_if_fail(data_size > 0, -1);
    for (int i=0; i < pCodec->frame->nb_samples; i++) {
        for (int ch=0; ch < pCodec->avctx->channels; ch++) {
            memcpy(out_data+data_size*i, pCodec->frame->data[ch] + data_size*i, data_size);
        }
    }
    out_size = pCodec->frame->nb_samples * pCodec->avctx->channels * data_size;

    return consumed_bytes;
}

