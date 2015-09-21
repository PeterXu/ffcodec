#ifndef _FFVIDEOPARAM_H_
#define _FFVIDEOPARAM_H_

#include "FFHeader.h"


///
/// @brief  The video parameter class for FFEncoder initializing
///
class FF_EXPORT FFVideoParam
{
public:
    ///
    /// @brief  Constructor for initializing an object of FFVideoParam
    ///
    /// @param  [in] width          The width of the video frame, must be greater than 0
    /// @param  [in] height         The height of the video frame, must be greater than 0
    /// @param  [in] pixelFormat    PixelFormat enum representing the pixel format of the source video frame
    /// @param  [in] bitRate        The target bit rate of the target video stream, must be greater than 0
    /// @param  [in] frameRate      The frame rate of the target video, must be greater than 0
    /// @param  [in] videoCodecName The name of the video codec which is going to be used in encoding/decoding
    ///
    FFVideoParam(int width, int height, PixelFormat pixelFormat, int bitRate, int frameRate, std::string codecName = "");

    ///
    /// @brief  Constructor for initializing an empty FFVideoParam object
    ///
    FFVideoParam();

    void setVideoParam(int width, int height, PixelFormat pixelFormat, int bitRate, int frameRate, std::string codecName = "");
    ///
    /// @brief  Destructor
    ///
    virtual ~FFVideoParam();

    ///
    /// @brief  Judge whether a FFVideoParam object is empty
    /// 
    bool empty();

    ///
    /// @brief  Judge whether a FFVideoParam object's parameters are right
    /// 
    bool isValid();

public:
    int width;                  ///< The width of the video
    int height;                 ///< The height of the video
    PixelFormat pixelFormat;    ///< The pixel format of the video
    int bitRate;                ///< The bit rate of the video
    int frameRate;              ///< The frame rate of the video
    std::string codecName;      ///< The name of the video codec
};

#endif

