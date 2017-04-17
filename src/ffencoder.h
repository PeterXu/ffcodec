#ifndef __FFENCODER_H_
#define __FFENCODER_H_

#include "ffparam.h"

class FF_EXPORT FFEncoder
{
public:
    FFEncoder();
    virtual ~FFEncoder();

    long openVideo(FFCodecID codec_id, const FFVideoFormat &format);
    void closeVideo();
    long encodeVideo(const uint8_t *in_data, const int in_size, const FFVideoFormat &in_fmt,
            uint8_t *out_data, int &out_size);

    long openAudio(FFCodecID codec_id, const FFAudioFormat &format);
    void closeAudio();
    long encodeAudio(const uint8_t *in_data, const int in_size, uint8_t *out_data, int &out_size);

protected:
    long openContext(ff_codec_t codec, FFCodecID codec_id);
    long openCodec(ff_codec_t codec, const FFVideoFormat &format);
    long openCodec(ff_codec_t codec, const FFAudioFormat &format);

private:
    ff_codec_t m_video;
    ff_codec_t m_audio;
    FFVideoFormat m_vfmt;
};

#endif //__FFENCODER_H_

