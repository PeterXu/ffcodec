#ifndef _FFAUDIOPARAM_H_
#define _FFAUDIOPARAM_H_

#include "FFHeader.h"


///
/// @brief  The audio parameter class for FFEncoder initializing
///
class FF_EXPORT FFAudioParam
{
public:
    ///
    /// @brief  Constructor for initializing an object of FFAudioParam
    ///
    /// @param  [in] sampleRate     The sample rate of the audio, must be greater than 0
    /// @param  [in] channels       The number of channels in the audio, must be greater than 0
    /// @param  [in] bitRate        The target bit rate of the target audio stream, must be greater than 0
    /// @param  [in] audioCodecName The name of the audio codec which is going to be used in encoding/decoding
    ///
    FFAudioParam(int sampleRate, int channels, int bitRate, std::string codecName = "");

    ///
    /// @brief  Constructor for initializing an empty FFAudioParam object
    ///
    FFAudioParam();

    ///
    /// @brief  Destructor
    ///
    virtual ~FFAudioParam();

    ///
    /// @brief  Judge whether a FFAudioParam object is empty
    ///
    bool empty();

    ///
    /// @brief  Judge whether a FFAudioParam object's parameters are right
    ///
    bool isValid();

public:
    int sampleRate;             ///< The sample rate of the audio
    int channels;               ///< The number of audio channels
    int bitRate;                ///< The bit rate of the audio
    std::string codecName;      ///< The name of the audio codec
};

#endif
