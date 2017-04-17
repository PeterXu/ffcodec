#ifndef __FFCODEC_H_
#define __FFCODEC_H_

#include "ffparam.h"

class FFCodec {
public:
    explicit FFCodec(FFMediaType type) {
        mtype = type;
        codec = NULL;
        avctx = NULL;
        frame = NULL;
        frame2 = NULL;
        swsctx = NULL;
    }

    virtual ~FFCodec() {
        codec = NULL;
        if (avctx) {
            avcodec_close(avctx);
            av_free(avctx);
            avctx = NULL;
        }
        if (frame) {
            av_frame_free(&frame);
            frame = NULL;
        }
        if (frame2) {
            av_frame_free(&frame2);
            frame2 = NULL;
        }
        if (swsctx) {
            sws_freeContext(swsctx);
            swsctx = NULL;
        }
    }

public:
    FFMediaType mtype;

    AVCodec *codec;
    AVCodecContext *avctx;
    AVFrame  *frame;    // for pre-allocated buffer
    AVFrame  *frame2;   // for self-allocated buffer
    AVPacket avpkt;
    SwsContext *swsctx;
};


#define safe_delete_codec(p) do {if(p) delete (FFCodec *)p; p = NULL;}while(0)

// for const variables
AVPixelFormat GetAVPixelFormat(FFPixelFormat pix_fmt);
FFPixelFormat GetFFPixelFormat(AVPixelFormat pix_fmt);

AVCodecID GetAVCodecID(FFCodecID codec_id);
FFCodecID GetFFCodecID(AVCodecID codec_id);

AVSampleFormat GetAVSampleFormat(FFSampleFormat fmt);
FFSampleFormat GetFFSampleFormat(AVSampleFormat fmt);

// for audio codec
int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt);
AVSampleFormat select_sample_fmt(AVCodec *codec);
int select_channel_layout(AVCodec *codec);
int check_sample_rate(AVCodec *codec, int sample_rate);
int select_sample_rate(AVCodec *codec);

// for video codec
int check_pix_fmt(AVCodec *codec, enum AVPixelFormat pix_fmt);
AVPixelFormat select_pix_fmt(AVCodec *codec);


#endif // __FFCODEC_H_

