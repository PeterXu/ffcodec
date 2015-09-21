#include "FFAudioParam.h"


FFAudioParam::FFAudioParam(int sampleRate, int channels, int bitRate, std::string codecName) : 
    sampleRate(sampleRate), channels(channels), bitRate(bitRate), codecName(codecName)
{
}

FFAudioParam::FFAudioParam() :
    sampleRate(0), channels(0), bitRate(0), codecName("")
{
}

FFAudioParam::~FFAudioParam()
{
}

bool FFAudioParam::empty()
{
    return bitRate < 1 && sampleRate < 1 && channels < 1 && codecName == "";
}

bool FFAudioParam::isValid()
{
    // To validate the arguments
    if (bitRate < 1 || sampleRate < 1 || channels < 1)
    {
        return false;
    }

    return true;
}

