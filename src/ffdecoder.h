#ifndef __FFDECODER_H_
#define __FFDECODER_H_

#include "ffparam.h"

class FF_EXPORT FFDecoder
{
public:
    FFDecoder();
    virtual ~FFDecoder();

    long openVideo(FFCodecID codec_id);
    void closeVideo();
    long decodeVideo(const uint8_t *in_data, const int in_size, uint8_t *out_data, int &out_size, 
        const FFVideoFormat &out_fmt);

    long openAudio(FFCodecID codec_id);
    void closeAudio();
    long decodeAudio(const uint8_t *in_data, const int in_size, uint8_t *out_data, int &out_size);

protected:
    long openCodec(ff_codec_t codec, FFCodecID codec_id);

private:
    ff_codec_t *m_video;
    ff_codec_t *m_audio;
    FFVideoFormat m_vfmt;
};


#endif // __FFDECODER_H_

