#ifndef __FFPARAM_H_
#define __FFPARAM_H_

#include "ffheader.h"

typedef void * ff_codec_t;

enum FFMediaType {
    FF_MEDIA_VIDEO,
    FF_MEDIA_AUDIO,
};

enum FFPixelFormat {
    FF_PIX_FMT_NONE = -1,
    FF_PIX_FMT_I420,
    FF_PIX_FMT_RGB24,
    FF_PIX_FMT_BGR24,
    FF_PIX_FMT_NV21,

    FF_PIX_FMT_NB
};

enum FFSampleFormat {
    FF_SAMPLE_FMT_NONE = -1,
    FF_SAMPLE_FMT_U8,   // pcm u8
    FF_SAMPLE_FMT_S16,  // pcm s16
    FF_SAMPLE_FMT_S32,  // pcm s32
    FF_SAMPLE_FMT_FLT,  // pcm float
    FF_SAMPLE_FMT_DBL,  // pcm double

    // planar
    FF_SAMPLE_FMT_U8P,
    FF_SAMPLE_FMT_S16P,
    FF_SAMPLE_FMT_S32P,
    FF_SAMPLE_FMT_FLTP,
    FF_SAMPLE_FMT_DBLP,

    FF_SAMPLE_FMT_NB    // Number of sample formats
};

enum FFCodecID {
    FF_CODEC_ID_NONE,

    FF_CODEC_ID_OPUS,
    FF_CODEC_ID_MP2,

    FF_CODEC_ID_MJPG,
    FF_CODEC_ID_VP8,
    FF_CODEC_ID_H264,

    FF_CODEC_ID_NB
};


class FFAudioFormat {
public:
    FFAudioFormat() {
        reset();
    }
    FFAudioFormat(int sample_rate, FFSampleFormat sample_fmt, int channels, int bitrate) {
        set(sample_rate, sample_fmt, channels, bitrate);
    }
    void reset() {
        set(0, FF_SAMPLE_FMT_NONE, 0, 0);
    }
    void set(int sample_rate, FFSampleFormat sample_fmt, int channels, int bitrate) {
        this->sample_rate = sample_rate;
        this->sample_fmt = sample_fmt;
        this->channels = channels;
        this->bitrate = bitrate;
    }

public:
    int sample_rate;
    int channels;
    int bitrate;
    FFSampleFormat sample_fmt;
};

class FFVideoFormat {
public:
    FFVideoFormat() {
        reset();
    }
    FFVideoFormat(int width, int height, FFPixelFormat pix_fmt, int bitrate, int fps) {
        set(width, height, pix_fmt, bitrate, fps);
    }
    void reset() {
        set(0, 0, FF_PIX_FMT_NONE, 0, 0);
    }
    void set(int width, int height, FFPixelFormat pix_fmt, int bitrate, int fps) {
        this->width = width;
        this->height = height;
        this->pix_fmt = pix_fmt;
        this->bitrate = bitrate;
        this->fps = fps;
    }

public:
    struct CodecData {
        CodecData() {
            gop_size = 0;
            max_b_frames = 0;
        }
        int gop_size;
        int max_b_frames;
    };

public:
    int width;
    int height;
    FFPixelFormat pix_fmt;
    int bitrate;
    int fps;
    CodecData data;
};

#endif // __FFPARAM_H_

