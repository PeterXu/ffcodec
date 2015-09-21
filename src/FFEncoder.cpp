#include "FFEncoder.h"
#include "LogTrace.h"

// define the max audio packet size as 128 KB
#define MAX_AUDIO_PACKET_SIZE (128 * 1024)

#ifndef snprintf
#define snprintf _snprintf
#endif


FFEncoder::FFEncoder(const FFVideoParam &vp, const FFAudioParam &ap) : 
    videoParam(vp), audioParam(ap)
{
    init();
}

FFEncoder::FFEncoder(const FFVideoParam &vp) : videoParam(vp)
{
    init();
}

FFEncoder::FFEncoder(const FFAudioParam &ap) : audioParam(ap)
{
    init();
}

void FFEncoder::init()
{
    // initialize the private fields
    this->outputContext = NULL;
    this->videoStream = NULL;
    this->audioStream = NULL;

    this->videoFrame = NULL;
    this->videoBuffer = NULL;
    this->videoBufferSize = 0;

    this->audioBuffer = NULL;
    this->audioBufferSize = 0;

    this->opened = false;
    this->encodeVideo = !this->videoParam.empty();
    this->encodeAudio = !this->audioParam.empty();

    // initialize libavcodec, and register all codecs and formats
    av_register_all();
}

FFEncoder::~FFEncoder()
{
    this->close();
}


//////////////////////////////////////////////////////////////////////////
//
//  Methods For Video
//
//////////////////////////////////////////////////////////////////////////

const uint8_t *FFEncoder::getVideoEncodedBuffer() const
{
    return this->videoBuffer;
}

double FFEncoder::getVideoTimeStamp() const
{
    if (!this->opened || !this->encodeVideo)
    {
        return 0;
    }
    return (double)this->videoStream->pts.val * this->videoStream->time_base.num / this->videoStream->time_base.den;
}

const FFVideoParam &FFEncoder::getVideoParam() const
{
    return this->videoParam;
}

int FFEncoder::getVideoFrameSize() const
{
    if (!this->opened || !this->encodeVideo)
    {
        return 0;
    }
    return avpicture_get_size(this->videoParam.pixelFormat, this->videoParam.width, this->videoParam.height);
}

int FFEncoder::encodeVideoFrame(const uint8_t *frameData, PixelFormat format, int width, int height)
{
    if (!this->opened)
    {
        return -1;
    }

    if (!this->encodeVideo)
    {
        return -1;
    }

    // set input video param
    FFVideoParam inParam(width, height, format, 0, 0, "");

    // encode the image frame
    AVPicture picture;
    if(avpicture_fill(&picture, (uint8_t *)frameData, inParam.pixelFormat, inParam.width, inParam.height) == -1) 
    {
        return -1;
    }

    return this->encodeVideoData(&picture, inParam);
}

// private method 
int FFEncoder::encodeVideoData(AVPicture *picture, FFVideoParam &picParam)
{
    AVCodecContext *videoCodecContext = this->videoStream->codec;

    AVFrame *frame = avcodec_alloc_frame();
    if (!frame)
    {
        return -1;
    }

    // convert the pixel format if needed
    if (picParam.pixelFormat != videoCodecContext->pix_fmt ||
            picParam.width != videoCodecContext->width ||
            picParam.height != videoCodecContext->height)
    {
        LOGE("FFEncoder::encodeVideoData, VideoParam match error");
        return -1;
    }
    else
    {
        // fill the frame
        *(AVPicture *)frame = *picture;
    }

    frame->pts = AV_NOPTS_VALUE;

    // encode the frame
    int encodedSize = avcodec_encode_video(videoCodecContext, this->videoBuffer, this->videoBufferSize, frame);

    av_free(frame);

    if (encodedSize < 0)
    {
        return -1;
    }
    else
    {
        return encodedSize;
    }
}

int FFEncoder::convertPixFmt(const uint8_t *src, int srclen, int srcw, int srch, PixelFormat srcfmt, 
        uint8_t *dst, int dstlen, int dstw, int dsth, PixelFormat dstfmt)
{
    if (!src || !dst) 
    {
        LOGE("[%s] src or dst is NULL", __FUNCTION__);
        return -1;
    }

    // src input frame
    AVPicture srcPic;
    FFVideoParam srcParam(srcw, srch, srcfmt, 0, 0, "");
    if(avpicture_fill(&srcPic, (uint8_t *)src, srcParam.pixelFormat, srcParam.width, srcParam.height) == -1) 
    {
        LOGE("[%s] fail to avpicture_fill for src picture", __FUNCTION__);
        return -1;
    }

    // dst output frame
    AVPicture dstPic;
    FFVideoParam dstParam(dstw, dsth, dstfmt, 0, 0, "");
    if(avpicture_alloc(&dstPic, dstParam.pixelFormat, dstParam.width, dstParam.height) == -1) 
    {
        LOGE("[%s] fail to avpicture_alloc for dst picture", __FUNCTION__);
        return -1;
    }

    if (convertPixFmt(&srcPic, &dstPic, &srcParam, &dstParam) < 0)
    {
        LOGE("[%s] fail to convertPixFmt", __FUNCTION__);
        return -1;
    }

    return avpicture_layout(&dstPic, dstParam.pixelFormat, dstParam.width, dstParam.height, dst, dstlen);
}

// private method
int FFEncoder::convertPixFmt(AVPicture *srcPic, AVPicture *dstPic, const FFVideoParam *srcParam, const FFVideoParam *dstParam)
{
    SwsContext *img_convert_ctx = NULL;
    img_convert_ctx = sws_getContext(
            srcParam->width, srcParam->height, srcParam->pixelFormat,
            dstParam->width, dstParam->height, dstParam->pixelFormat,
            SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (img_convert_ctx == NULL)
    {
        LOGE("[%s] fail to sws_getContext", __FUNCTION__);
        return -1;
    }

    int dstheight = sws_scale(img_convert_ctx, srcPic->data, srcPic->linesize, 0, srcParam->height, dstPic->data, dstPic->linesize);
    sws_freeContext(img_convert_ctx);
    return (dstheight == dstParam->height) ? 0 : -1;
}


//////////////////////////////////////////////////////////////////////////
//
//  Methods For Audio
//
//////////////////////////////////////////////////////////////////////////

const uint8_t *FFEncoder::getAudioEncodedBuffer() const
{
    return this->audioBuffer;
}

double FFEncoder::getAudioTimeStamp() const
{
    if (!this->opened || !this->encodeAudio)
    {
        return 0;
    }
    return (double)this->audioStream->pts.val * this->audioStream->time_base.num / this->audioStream->time_base.den;
}

const FFAudioParam &FFEncoder::getAudioParam() const
{
    return this->audioParam;
}

int FFEncoder::getAudioFrameSize() const
{
    if (!this->opened || !this->encodeAudio)
    {
        return 0;
    }

    int frameSize = 0;
    if (this->audioStream->codec && this->audioStream->codec->frame_size > 1)
    {
        frameSize  = this->audioStream->codec->frame_size;
        frameSize *= this->audioStream->codec->channels;    // multiply the channels
        frameSize *= sizeof(short); // convert to bytes
    }
    else
    {
        // hack for PCM audio codec
        //frameSize = this->audioBufferSize / this->audioParam.channels;
        //switch (this->audioStream->codec->codec_id)
        //{
        //    case CODEC_ID_PCM_S16LE:
        //    case CODEC_ID_PCM_S16BE:
        //    case CODEC_ID_PCM_U16LE:
        //    case CODEC_ID_PCM_U16BE:
        //        frameSize >>= 1;
        //        break;
        //    default:
        //        break;
        //}
        frameSize = this->audioBufferSize;  // including all channels, return bytes directly
    }
    return frameSize;
}

int FFEncoder::encodeAudioFrame(const uint8_t *frameData, int dataSize)
{
    if (!this->opened)
    {
        return -1;
    }

    if (!this->encodeAudio)
    {
        return -1;
    }

    if (this->audioStream->codec->frame_size <= 1 && dataSize < 1)
    {
        return -1;
    }

    return this->encodeAudioData((short*)frameData, dataSize/sizeof(short));
}

// private method
int FFEncoder::encodeAudioData(short *frameData, int dataSize)
{
    // the output size of the buffer which stores the encoded data
    int audioSize = this->audioBufferSize;

    if (this->audioStream->codec->frame_size <=1 && dataSize > 0)
    {
        // For PCM related codecs, the output size of the encoded data is
        // calculated from the size of the input audio frame.
        audioSize = dataSize;

        // The following codes are used for calculating "short" size from original "sample" size.
        // The codes are not needed any more because now the input size is already in "short" unit.

        // calculated the PCM size from input data size
        //switch(this->audioStream->codec->codec_id)
        //{
        //    case CODEC_ID_PCM_S32LE:
        //    case CODEC_ID_PCM_S32BE:
        //    case CODEC_ID_PCM_U32LE:
        //    case CODEC_ID_PCM_U32BE:
        //        audioSize <<= 1;
        //        break;
        //    case CODEC_ID_PCM_S24LE:
        //    case CODEC_ID_PCM_S24BE:
        //    case CODEC_ID_PCM_U24LE:
        //    case CODEC_ID_PCM_U24BE:
        //    case CODEC_ID_PCM_S24DAUD:
        //        audioSize = audioSize / 2 * 3;
        //        break;
        //    case CODEC_ID_PCM_S16LE:
        //    case CODEC_ID_PCM_S16BE:
        //    case CODEC_ID_PCM_U16LE:
        //    case CODEC_ID_PCM_U16BE:
        //        break;
        //    default:
        //        audioSize >>= 1;
        //        break;
        //}
    }

    // encode the frame
    int encodedSize = avcodec_encode_audio(this->audioStream->codec, this->audioBuffer, audioSize, frameData);

    if (encodedSize < 0)
    {
        return -1;
    }
    else
    {
        return encodedSize;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//  Other Methods
//
//////////////////////////////////////////////////////////////////////////

int FFEncoder::open()
{
    LOGI("FFEncoder.open, begin!");
    if (this->opened)
    {
        LOGW("FFEncoder.open, try to reopen!");
        return -1;
    }

    if (this->videoParam.codecName.empty() && 
            this->audioParam.codecName.empty())
    {
        LOGE("FFEncoder.open, no output or codec name");
        return -1;
    }

    // allocate the output media context
    this->outputContext = avformat_alloc_context();
    if (!this->outputContext)
    {
        LOGE("FFEncoder.open, failed to alloc context!");
        return -1;
    }

    // video related initialization if necessary
    if (this->encodeVideo)
    {
        // validate the video codec
        if (this->videoParam.codecName.empty())
        {
            LOGE("FFEncoder.open, no video codec name!");
            return -1;
        }

        // find the video encoder
        AVCodec *videoCodec = NULL;

        // use the codec name preferentially if it is specified in the input param
        videoCodec = avcodec_find_encoder_by_name(this->videoParam.codecName.c_str());

        if (!videoCodec)
        {
            LOGE("FFEncoder.open, find no video codec!");
            return -1;
        }

        // add the video stream with stream id 0
        this->videoStream = av_new_stream(this->outputContext, 0);
        if (!this->videoStream)
        {
            LOGE("FFEncoder.open, failed to new video stream!");
            return -1;
        }

        // set the parameters for video codec context
        AVCodecContext *videoCodecContext = this->videoStream->codec;
        videoCodecContext->codec_id       = videoCodec->id;
        videoCodecContext->codec_type     = CODEC_TYPE_VIDEO;
        videoCodecContext->bit_rate       = this->videoParam.bitRate;
        videoCodecContext->width          = this->videoParam.width;
        videoCodecContext->height         = this->videoParam.height;
        videoCodecContext->time_base.den  = this->videoParam.frameRate;
        videoCodecContext->time_base.num  = 1;

        // tune for video encoding
        videoCodecContext->gop_size = 24;
        videoCodecContext->qmin = 3;
        videoCodecContext->qmax = 33;
        videoCodecContext->max_qdiff = 4;
        videoCodecContext->qcompress = 0.6f;

        videoCodecContext->me_range = 64;		
        videoCodecContext->me_method = ME_FULL;
        videoCodecContext->me_range = 64;
        videoCodecContext->partitions = X264_PART_I4X4 | X264_PART_I8X8 | X264_PART_P8X8 | X264_PART_P4X4 | X264_PART_B8X8;
        videoCodecContext->coder_type = FF_CODER_TYPE_AC;
        videoCodecContext->max_b_frames = 1;

        // set the PixelFormat of the target encoded video
        if (videoCodec->pix_fmts)
        {
            // try to find the PixelFormat required by the input param,
            // use the default PixelFormat directly if required format not found
            const enum PixelFormat *p= videoCodec->pix_fmts;
            for ( ; *p != PIX_FMT_NONE; p ++)
            {
                if (*p == this->videoParam.pixelFormat)
                    break;
            }
            if (*p == PIX_FMT_NONE)
                videoCodecContext->pix_fmt = videoCodec->pix_fmts[0];
            else
                videoCodecContext->pix_fmt = *p;
        }

        // open the video codec
        if (avcodec_open(videoCodecContext, videoCodec) < 0)
        {
            LOGE("FFEncoder.open, find but failed to open video codec!");
            return -1;
        }

        // allocate the output buffer
        // the maximum possible buffer size could be the raw bmp format with R/G/B/A
        this->videoBufferSize = 4 * this->videoParam.width * this->videoParam.height;
        this->videoBuffer     = (uint8_t*)(av_malloc(this->videoBufferSize));

        // allocate the temporal video frame buffer for pixel format conversion if needed
        // FIXME: always allocate it when format or size is different
        if (this->videoParam.pixelFormat != videoCodecContext->pix_fmt)
        {
            this->videoFrame = (AVPicture *)av_malloc(sizeof(AVPicture));
            if (   this->videoFrame == NULL
                    || avpicture_alloc(this->videoFrame, videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height) < 0 )
            {
                LOGE("FFEncoder.open, failed to alloc video frame!");
                return -1;
            }
        }
    }

    // audio related initialization if necessary
    if (this->encodeAudio)
    {
        // validate the audio codec
        if (this->audioParam.codecName.empty())
        {
            LOGE("FFEncoder.open, no outputformat or no audio codec name!");
            return -1;
        }

        // find the audio encoder
        AVCodec *audioCodec = NULL;

        // use the codec name preferentially if it is specified in the input param
        audioCodec = avcodec_find_encoder_by_name(this->audioParam.codecName.c_str());

        if (!audioCodec)
        {
            LOGE("FFEncoder.open, invalid audio codec!");
            return -1;
        }

        // add the audio stream with stream id 1
        this->audioStream = av_new_stream(this->outputContext, 1);
        if (!this->audioStream)
        {
            LOGE("FFEncoder.open, failed to new audio stream!");
            return -1;
        }

        // set the parameters for audio codec context
        AVCodecContext *audioCodecContext = this->audioStream->codec;
        audioCodecContext->codec_id       = audioCodec->id;
        audioCodecContext->codec_type     = CODEC_TYPE_AUDIO;
        audioCodecContext->bit_rate       = this->audioParam.bitRate;
        audioCodecContext->sample_rate    = this->audioParam.sampleRate;
        audioCodecContext->channels       = this->audioParam.channels;

        // open the audio codec
        if (avcodec_open(audioCodecContext, audioCodec) < 0)
        {
            LOGE("FFEncoder.open, failed to open audio codec!");
            return -1;
        }

        // TODO: how to determine the buffer size?
        // allocate the output buffer
        this->audioBufferSize = 4 * MAX_AUDIO_PACKET_SIZE;
        this->audioBuffer     = (uint8_t*)(av_malloc(this->audioBufferSize));
    }

    this->opened = true;
    LOGI("FFEncoder.open, end!");

    return 0;
}

void FFEncoder::close()
{
    if (!this->opened)
    {
        return;
    }

    if (this->encodeVideo)
    {
        // close the video stream and codec
        avcodec_close(this->videoStream->codec);
        av_freep(&this->videoStream->codec);
        av_freep(&this->videoStream);
        av_freep(&this->videoBuffer);
        this->videoBufferSize = 0;
        if (this->videoFrame != NULL)
        {
            avpicture_free(this->videoFrame);
            av_freep(&this->videoFrame);
        }
    }

    if (this->encodeAudio)
    {
        // close the audio stream and codec
        avcodec_close(this->audioStream->codec);
        av_freep(&this->audioStream->codec);
        av_freep(&this->audioStream);
        av_freep(&this->audioBuffer);
        this->audioBufferSize = 0;
    }

    av_freep(&this->outputContext);

    this->opened = false;
    this->encodeVideo = false;
    this->encodeAudio = false;
}

