#include "ffencoder.h"
#include "fflog.h"
#include "ffcodec.h"

FFEncoder::FFEncoder() {
    m_video = NULL;
    m_audio = NULL;
    m_vfmt.reset();
}

FFEncoder::~FFEncoder() {
    closeVideo();
    closeAudio();
}

long FFEncoder::openContext(ff_codec_t codec, FFCodecID codec_id) {
    FFCodec *pCodec = (FFCodec *)codec;
    returnv_if_fail(pCodec, -1);

    AVCodecID av_codec_id = GetAVCodecID(codec_id);
    returnv_if_fail(av_codec_id != AV_CODEC_ID_NONE, -1);

    pCodec->codec = avcodec_find_encoder(av_codec_id);
    returnv_if_fail(pCodec->codec, -1);

    pCodec->avctx = avcodec_alloc_context3(pCodec->codec);
    returnv_if_fail(pCodec->avctx, -1);

    return 0;
}
long FFEncoder::openCodec(ff_codec_t codec, const FFVideoFormat &fmt) {
    FFCodec *pCodec = (FFCodec *)codec;
    returnv_if_fail(pCodec, -1);
    returnv_if_fail(pCodec->avctx, -1);

    pCodec->avctx->bit_rate = fmt.bitrate;
    pCodec->avctx->width = fmt.width;
    pCodec->avctx->height = fmt.height;
    pCodec->avctx->time_base = (AVRational){1,fmt.fps};
    pCodec->avctx->gop_size = fmt.data.gop_size;
    pCodec->avctx->max_b_frames = fmt.data.max_b_frames;

    AVPixelFormat pix_fmt = GetAVPixelFormat(fmt.pix_fmt);
    if (!check_pix_fmt(pCodec->codec, pix_fmt)) {
        pCodec->avctx->pix_fmt = select_pix_fmt(pCodec->codec);
        LOGW("unsupported (ff_pix_fmt="<<fmt.pix_fmt<<", pix_fmt="<<pix_fmt<<")"
                <<", and select from codec="<<pCodec->avctx->pix_fmt);
    }else {
        pCodec->avctx->pix_fmt = pix_fmt;
    }

    // for codec private data
    if (pCodec->avctx->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(pCodec->avctx->priv_data, "preset", "fast", 0);
    }

    int iret = avcodec_open2(pCodec->avctx, pCodec->codec, NULL);
    returnv_if_fail(iret == 0, -1);

    av_init_packet(&pCodec->avpkt);
    pCodec->avpkt.data = NULL;
    pCodec->avpkt.size = 0;
    return 0;
}
long FFEncoder::openCodec(ff_codec_t codec, const FFAudioFormat &fmt) {
    FFCodec *pCodec = (FFCodec *)codec;
    returnv_if_fail(pCodec, -1);
    returnv_if_fail(pCodec->avctx, -1);

    pCodec->avctx->bit_rate = fmt.bitrate;

    AVSampleFormat sample_fmt = GetAVSampleFormat(fmt.sample_fmt);
    if (!check_sample_fmt(pCodec->codec, pCodec->avctx->sample_fmt)) {
        pCodec->avctx->sample_fmt = select_sample_fmt(pCodec->codec);
        LOGW("unsupported (ff_sample_fmt="<<fmt.sample_fmt<<", sample_fmt="<<sample_fmt<<")"
                <<", and select from codec="<<pCodec->avctx->sample_fmt);
    }else {
        pCodec->avctx->sample_fmt = sample_fmt;
    }

    if (!check_sample_rate(pCodec->codec, fmt.sample_rate)) {
        pCodec->avctx->sample_rate = select_sample_rate(pCodec->codec);
        LOGW("unsupported sample_rate="<<fmt.sample_rate<<", and select from codec="<<pCodec->avctx->sample_rate);
    }else{
        pCodec->avctx->sample_rate = fmt.sample_rate;
    }

    pCodec->avctx->channel_layout = select_channel_layout(pCodec->codec);
    pCodec->avctx->channels = av_get_channel_layout_nb_channels(pCodec->avctx->channel_layout);
    if (fmt.channels != pCodec->avctx->channels) {
        LOGW("require channels="<<fmt.channels<<", but select from codec="<<pCodec->avctx->channels);
    }

    int iret = avcodec_open2(pCodec->avctx, pCodec->codec, NULL);
    returnv_if_fail(iret == 0, -1);

    av_init_packet(&pCodec->avpkt);
    pCodec->avpkt.data = NULL;
    pCodec->avpkt.size = 0;
    return 0;
}

long FFEncoder::openVideo(FFCodecID codec_id, const FFVideoFormat &format) {
    returnv_if_fail(!m_video, 1); // opened
    m_video = (ff_codec_t)new FFCodec(FF_MEDIA_VIDEO);
    returnv_if_fail(m_video, -1);
    m_vfmt.reset();

    long lret = openContext(m_video, codec_id);
    if (lret == 0) {
        lret = openCodec(m_video, format);
        if (lret == 0)
            return 0;
    }
    safe_delete_codec(m_video);
    return lret;
}
void FFEncoder::closeVideo() {
    safe_delete_codec(m_video);
    m_vfmt.reset();
}

long FFEncoder::openAudio(FFCodecID codec_id, const FFAudioFormat &format) {
    returnv_if_fail(!m_audio, 1); // opened
    m_audio = (ff_codec_t)new FFCodec(FF_MEDIA_AUDIO);
    returnv_if_fail(m_audio, -1);

    long lret = openContext(m_audio, codec_id);
    if (lret == 0) {
        lret = openCodec(m_audio, format);
        if (lret == 0)
            return 0;
    }
    safe_delete_codec(m_audio);
    return lret;
}
void FFEncoder::closeAudio() {
    safe_delete_codec(m_audio);
}

// return 0 if success, else < 0
long FFEncoder::encodeVideo(const uint8_t *in_data, int in_size, const FFVideoFormat &in_fmt, 
        uint8_t *out_data, int &out_size) {
    returnv_if_fail(m_video, -1);
    returnv_if_fail(out_data, -1);

    FFCodec *pCodec = (FFCodec *)m_video;

    // prepare output
    av_init_packet(&pCodec->avpkt);
    pCodec->avpkt.data = out_data;
    pCodec->avpkt.size = out_size;
    
    // flush delayed frames
    if (!in_data) {
        int got_output = 0;
        int iret = avcodec_encode_video2(pCodec->avctx, &pCodec->avpkt, NULL, &got_output);
        if (iret < 0 || got_output <= 0) {
            LOGW("no delayed frames and donot flush again, return="<<iret);
            return -1;
        }
        out_size = pCodec->avpkt.size;
        return 0;
    }

    // check input format
    AVPixelFormat in_pix_fmt = GetAVPixelFormat(in_fmt.pix_fmt);
    if (in_pix_fmt == AV_PIX_FMT_NONE) {
        LOGE("unsupported format ff_pix_fmt="<<in_fmt.pix_fmt);
        return -1;
    }

    // prepare input frame
    AVFrame *input_frame = NULL;
    if (in_pix_fmt != pCodec->avctx->pix_fmt || 
        in_fmt.width != pCodec->avctx->width || 
        in_fmt.height != pCodec->avctx->height) {
        // convert pix_fmt
        if (!sws_isSupportedInput(in_pix_fmt) || !sws_isSupportedOutput(pCodec->avctx->pix_fmt)) {
            LOGE("(sws) unsupported from av_pix_fmt="<<in_pix_fmt<<" to av_pix_fmt="<<pCodec->avctx->pix_fmt);
            return -1;
        }

        if (m_vfmt.width != in_fmt.width || 
            m_vfmt.height != in_fmt.height || 
            m_vfmt.pix_fmt != in_fmt.pix_fmt) {
            m_vfmt = in_fmt;
            sws_freeContext(pCodec->swsctx);
            pCodec->swsctx = sws_getContext(in_fmt.width, in_fmt.height, in_pix_fmt,
                pCodec->avctx->width, pCodec->avctx->height, pCodec->avctx->pix_fmt, SWS_FAST_BILINEAR,
                NULL, NULL, NULL);
        }
        returnv_if_fail(pCodec->swsctx, -1);

        // prepare sws output frame
        if (!pCodec->frame2) {
            pCodec->frame2 = av_frame_alloc();
            pCodec->frame2->width = pCodec->avctx->width;
            pCodec->frame2->height = pCodec->avctx->height;
            pCodec->frame2->format = pCodec->avctx->pix_fmt;
            int iret = av_frame_get_buffer(pCodec->frame2, 0);
            returnv_if_fail(iret==0, -1);
        }

        // prepare sws input data
        int sws_in_linesize[4] = { 0 };
        uint8_t *sws_in_data[4] = { 0 };
        int iret = av_image_fill_arrays(sws_in_data, sws_in_linesize, 
                in_data, in_pix_fmt, in_fmt.width, in_fmt.height, 0);
        returnv_if_fail(iret > 0 && iret <= in_size, -1);

        // sws convert (for input frame)
        iret = sws_scale(pCodec->swsctx, sws_in_data, sws_in_linesize, 0, in_fmt.height,
                pCodec->frame2->data, pCodec->frame2->linesize);
        returnv_if_fail(iret == in_fmt.height, -1);

        input_frame = pCodec->frame2;
    }else {
        if (!pCodec->frame) {
            pCodec->frame = av_frame_alloc();
            pCodec->frame->format = in_pix_fmt;
            pCodec->frame->width  = in_fmt.width;
            pCodec->frame->height = in_fmt.height;
        }

        // prepare raw input data
        int iret = av_image_fill_arrays(pCodec->frame->data, pCodec->frame->linesize,
                in_data, in_pix_fmt, in_fmt.width, in_fmt.height, 0);
        returnv_if_fail(iret > 0 && iret <= in_size, -1);

        input_frame = pCodec->frame;
    }

    // encode frame
    int got_output = 0;
    int iret = avcodec_encode_video2(pCodec->avctx, &pCodec->avpkt, input_frame, &got_output);
    if (iret < 0 || got_output <= 0) {
        LOGE("encode failure or no output, return="<<iret);
        return -1;
    }
    out_size = pCodec->avpkt.size;

    return 0;
}

// return 0 if success, else < 0
long FFEncoder::encodeAudio(const uint8_t *in_data, const int in_size, uint8_t *out_data, int &out_size) {
    returnv_if_fail(m_video, -1);
    returnv_if_fail(out_data, -1);

    FFCodec *pCodec = (FFCodec *)m_audio;

    // prepare output
    av_init_packet(&pCodec->avpkt);
    pCodec->avpkt.data = out_data;
    pCodec->avpkt.size = out_size;

    // flush delayed frames
    if (!in_data) {
        int got_output = 0;
        int iret = avcodec_encode_audio2(pCodec->avctx, &pCodec->avpkt, NULL, &got_output);
        if (iret < 0 || got_output <= 0) {
            LOGW("no delayed frames and donot flush next time, return="<<iret);
            return -1;
        }
        out_size = pCodec->avpkt.size;
        return 0;
    }

    // prepare input
    if (!pCodec->frame) {
        pCodec->frame = av_frame_alloc();
    }
    int iret = avcodec_fill_audio_frame(pCodec->frame, pCodec->avctx->channels, pCodec->avctx->sample_fmt,
            in_data, in_size, 0);
    returnv_if_fail(iret >= 0, -1);

    // encode frame
    int got_output = 0;
    iret = avcodec_encode_audio2(pCodec->avctx, &pCodec->avpkt, pCodec->frame, &got_output);
    if (iret < 0 || got_output <= 0) {
        LOGE("encode failure or no output, return="<<iret);
        return -1;
    }
    out_size = pCodec->avpkt.size;

    return 0;
}

