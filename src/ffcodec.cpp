#include "ffcodec.h"

// for video pixel format
typedef struct pix_fmt_entry_t {
    enum FFPixelFormat pix_fmt;
    enum AVPixelFormat av_pix_fmt;
    int bpp;
    const char *name;
}pix_fmt_entry_t;
const pix_fmt_entry_t k_pix_fmt_entries[] = {
    { FF_PIX_FMT_NONE, AV_PIX_FMT_NONE, 0, "" },

    { FF_PIX_FMT_I420,  AV_PIX_FMT_YUV420P,  12, "I420"  },
    { FF_PIX_FMT_RGB24, AV_PIX_FMT_RGB24,    24, "RGB24" },
    { FF_PIX_FMT_BGR24, AV_PIX_FMT_BGR24,    24, "BGR24" },
    { FF_PIX_FMT_NV21,  AV_PIX_FMT_NV21,     12, "NV21"  },
};

// for video codec id
typedef struct codec_id_entry_t {
    enum FFCodecID codec_id;
    enum AVCodecID av_codec_id;
    const char *fourcc;
}codec_id_entry_t;
const codec_id_entry_t k_codec_id_entries[] = {
    { FF_CODEC_ID_MP2,  AV_CODEC_ID_MP2,  "mp2 " },
    { FF_CODEC_ID_OPUS, AV_CODEC_ID_OPUS, "opus" },
    { FF_CODEC_ID_H264, AV_CODEC_ID_H264, "h264" },
    { FF_CODEC_ID_VP8,  AV_CODEC_ID_VP8,  "vp8" },
};

// for audio sample format
typedef struct sample_fmt_entry_t {
    enum FFSampleFormat sample_fmt;
    enum AVSampleFormat av_sample_fmt; 
    const char *fmt_be, *fmt_le;
} sample_fmt_entry_t;
const sample_fmt_entry_t k_sample_fmt_entries[] = {
    { FF_SAMPLE_FMT_NONE, AV_SAMPLE_FMT_NONE, "", ""},

    { FF_SAMPLE_FMT_U8,  AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
    { FF_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16, "s16be", "s16le" },
    { FF_SAMPLE_FMT_S32, AV_SAMPLE_FMT_S32, "s32be", "s32le" },
    { FF_SAMPLE_FMT_FLT, AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
    { FF_SAMPLE_FMT_DBL, AV_SAMPLE_FMT_DBL, "f64be", "f64le" },

    { FF_SAMPLE_FMT_U8P,  AV_SAMPLE_FMT_U8P,  "u8p",    "u8p"    },
    { FF_SAMPLE_FMT_S16P, AV_SAMPLE_FMT_S16P, "s16pbe", "s16ple" },
    { FF_SAMPLE_FMT_S32P, AV_SAMPLE_FMT_S32P, "s32pbe", "s32ple" },
    { FF_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_FLTP, "f32pbe", "f32ple" },
    { FF_SAMPLE_FMT_DBLP, AV_SAMPLE_FMT_DBLP, "f64pbe", "f64ple" },
};


AVPixelFormat GetAVPixelFormat(FFPixelFormat pix_fmt) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_pix_fmt_entries); i++) {
        struct pix_fmt_entry_t *entry = &k_pix_fmt_entries[i];
        if (pix_fmt == entry->pix_fmt)
            return entry->av_pix_fmt;
    }
    return AV_PIX_FMT_NONE;
}

FFPixelFormat GetFFPixelFormat(AVPixelFormat pix_fmt) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_pix_fmt_entries); i++) {
        struct pix_fmt_entry_t *entry = &k_pix_fmt_entries[i];
        if (pix_fmt == entry->av_pix_fmt)
            return entry->pix_fmt;
    }
    return FF_PIX_FMT_NONE;
}

AVCodecID GetAVCodecID(FFCodecID codec_id) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_codec_id_entries); i++) {
        struct codec_id_entry_t *entry = &k_codec_id_entries[i];
        if (codec_id == entry->codec_id)
            return entry->av_codec_id;
    }
    return AV_CODEC_ID_NONE;
}

FFCodecID GetFFCodecID(AVCodecID codec_id) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_codec_id_entries); i++) {
        struct codec_id_entry_t *entry = &k_codec_id_entries[i];
        if (codec_id == entry->av_codec_id)
            return entry->codec_id;
    }
    return FF_CODEC_ID_NONE;
}

AVSampleFormat GetAVSampleFormat(FFSampleFormat sample_fmt) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_sample_fmt_entries); i++) {
        struct sample_fmt_entry_t *entry = &k_sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
            return entry->av_sample_fmt;
    }
    return AV_PIX_FMT_NONE;
}

FFSampleFormat GetFFSampleFormat(AVSampleFormat sample_fmt) {
    for (int i = 0; i < FF_ARRAY_ELEMS(k_sample_fmt_entries); i++) {
        struct sample_fmt_entry_t *entry = &k_sample_fmt_entries[i];
        if (sample_fmt == entry->av_sample_fmt)
            return entry->sample_fmt;
    }
    return FF_SAMPLE_FMT_NONE;
}



/* check that a given sample format is supported by the encoder */
int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }   
    return 0;
}

/* just pick the first supported sample_fmt */
AVSampleFormat select_sample_fmt(AVCodec *codec)
{
    const enum AVSampleFormat *p = codec->sample_fmts;
    return *p;
}

/* check that a given sample rate is supported by the encoder */
int check_sample_rate(AVCodec *codec, int sample_rate)
{
    const int *p;
    int best_samplerate = 0;
    if (!codec->supported_samplerates){
        if (sample_rate == 44100)
            return 1;
        return 0;
    }

    p = codec->supported_samplerates;
    while (*p) {
        if (*p == sample_rate)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
int select_sample_rate(AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;
    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
int select_channel_layout(AVCodec *codec)
{   
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;
    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);
        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}



/* check that a given pixel format is supported by the encoder */
int check_pix_fmt(AVCodec *codec, enum AVPixelFormat pix_fmt)
{
    const enum AVPixelFormat *p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pix_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the first supported pix_fmt */
AVPixelFormat select_pix_fmt(AVCodec *codec)
{
    const enum AVPixelFormat *p = codec->pix_fmts;
    return *p;
}

