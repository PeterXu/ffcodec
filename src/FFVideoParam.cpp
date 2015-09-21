#include "FFVideoParam.h"


FFVideoParam::FFVideoParam(int width, int height, PixelFormat pixelFormat, int bitRate, int frameRate, std::string codecName) :
    width(width), height(height), pixelFormat(pixelFormat), bitRate(bitRate), frameRate(frameRate), codecName(codecName)
{
}

FFVideoParam::FFVideoParam() :
width(0), height(0), pixelFormat(PIX_FMT_NONE), bitRate(0), frameRate(0), codecName("")
{
}

void FFVideoParam::setVideoParam(int w, int h, PixelFormat fmt, int br, int fr, std::string codec)
{
	width = w;
	height = h;
	pixelFormat = fmt;
	bitRate = br;
	frameRate = fr;
	codecName = codec;
}

FFVideoParam::~FFVideoParam()
{
}

bool FFVideoParam::empty()
{
	return width < 1 && height < 1 && pixelFormat == PIX_FMT_NONE  && 
		bitRate < 1 && frameRate < 1 && codecName == "";
}

bool FFVideoParam::isValid()
{
	// To validate the arguments
	if (width < 1 || height < 1 || pixelFormat == PIX_FMT_NONE || bitRate < 1 || frameRate < 1)
	{
		return false;
	}

	return true;
}

