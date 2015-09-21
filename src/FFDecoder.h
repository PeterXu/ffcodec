#ifndef _FFDECODER_H_
#define _FFDECODER_H_

#include "FFVideoParam.h"
#include "FFAudioParam.h"


///
/// @brief  The video decoder class for wrapping FFmpeg decoding API.
///
/// @todo   There may be several audio/video streams in the input file. However, only one stream is used here.
///
class FF_EXPORT FFDecoder
{
public:
    //////////////////////////////////////////////////////////////////////////
    //
    //  Initialization and finalization
    //
    //////////////////////////////////////////////////////////////////////////

    ///
    /// @brief  Constructor for initializing an object of FFDecoder.
    ///
    FFDecoder(const FFVideoParam &videoParam, const FFAudioParam &audioParam);

    // only video
    FFDecoder(const FFVideoParam &videoParam);

    // only audio
    FFDecoder(const FFAudioParam &audioParam);

    ///
    /// @brief  Destructor
    ///
    virtual ~FFDecoder();


public:
    //////////////////////////////////////////////////////////////////////////
    //
    //  Public properties
    //
    //////////////////////////////////////////////////////////////////////////

    ///
    /// @brief  Get the video parameters which are available after open() is called
    ///
    const FFVideoParam &getVideoParam() const;

    ///
    /// @brief  Get the audio parameters which are available after open() is called
    ///
    const FFAudioParam &getAudioParam() const;

    ///
    /// @brief  Get the decoded video frame data.
    ///
    /// If the return value of decodeFrame() is 0 (i.e. video stream),
    /// use this method to get decoded video frame.
    ///
    /// Use getVideoFrameSize() to get the returned size of the decoded video frame data (unit: in bytes/uint8_t).
    ///
    /// @return A uint8_t pointer representing the decoded frame.
    ///
    const uint8_t *getVideoFrame() const;

    ///
    /// @brief  Get the size of the decoded video frame (unit: in bytes).
    ///
    /// If the return value of decodeFrame() is 0 (i.e. video stream),
    /// use this method to get the size of the decoded video frame.
    ///
    /// @return An int representing the bytes of decoded video frame (unit: in bytes/uint8_t).
    ///
    int getVideoFrameSize() const;

    ///
    /// @brief  Get the decoded audio frame data containing all channels.
    ///
    /// If the return value of decodeFrame() is 1 (i.e. audio stream),
    /// use this method to get decoded audio frame.
    ///
    /// Use getAudioFrameSize() to get the returned size of decoded audio frame data (unit: in bytes/uint8_t).
    ///
    /// @return A uint8_t pointer representing the decoded frame.
    ///
    const uint8_t *getAudioFrame() const;

    ///
    /// @brief  Get the size of the decoded audio frame containing all channels (unit: in bytes/uint8_t).
    ///
    /// If the return value of decodeFrame() is 1 (i.e. audio stream),
    /// use this method to get the size of the decoded audio frame.
    ///
    /// @return An int representing the bytes of the decoded audio frame containing all channels (unit: in bytes/uint8_t).
    ///
    int getAudioFrameSize() const;

    ///
    /// @brief  Get the presentation time stamp of current packet being decoded
    ///
    /// @return A double representing the presentation time stamp (in seconds).
    ///
    double getPresentTimeStamp() const;

    ///
    /// @brief  Get the decoding time stamp of current packet being decoded
    ///
    /// @return A double representing the decoding time stamp (in seconds).
    ///
    double getDecodeTimeStamp() const;


public:
    //////////////////////////////////////////////////////////////////////////
    //
    //  Public Methods
    //
    //////////////////////////////////////////////////////////////////////////

    ///
    /// @brief  Open the input file, codecs, and allocate the necessary structures and memories.
    ///
    /// @param  [in] fileName   The name of the input media file (including the extension).
    ///
    int open();

    ///
    /// @brief  Close the input file, codecs, and release the memories.
    ///
    /// Must be called after decoding process is finished.
    ///
    void close();

    ///
    /// @brief  Read a frame from the input file and decode it.
    ///
    /// The frame can be a video frame or an audio frame.
    /// After decodeFrame() is called, you can call getVideoFrame()/getAudioFrame()
    /// or getVideoFrameSize()/getAudioFrameSize() to get what you want according to
    ///  the returned value of this function.
    ///
    /// @return An int representing the stream identity of the decoded frame.
    /// @retval 0  Video frame
    /// @retval 1  Audio frame
    /// @retval -1 Error occurred or end of file
    ///
    int decodeVideoFrame(const uint8_t *frameData, int dataSize, int64_t pts=0, int64_t dts=0);
    int decodeAudioFrame(const uint8_t *frameData, int dataSize, int64_t pts=0, int64_t dts=0);


private:

    //////////////////////////////////////////////////////////////////////////
    //
    //  Private Definitions
    //
    //////////////////////////////////////////////////////////////////////////

    bool decodeVideo;           ///< Whether video decoding is needed
    bool decodeAudio;           ///< Whether audio decoding is needed
    bool opened;                ///< Whether the FFDecoder is opened yet

    FFVideoParam videoParam;        ///< The video parameters of the video to be decoded
    FFAudioParam audioParam;        ///< The audio parameters of the audio to be decoded

    AVFormatContext *inputContext;      ///< The input format context
    AVStream *videoStream;              ///< The video input stream
    AVStream *audioStream;              ///< The audio input stream

    uint8_t *videoFrameBuffer;  ///< The buffer storing one output video frame data
    int      videoFrameSize;    ///< The size of the output video frame
    int      videoBufferSize;   ///< The total size of the video output buffer

    uint8_t *audioFrameBuffer;  ///< The buffer storing one output audio frame data
    int      audioFrameSize;    ///< The size of the output audio frame
    int      audioBufferSize;   ///< The total size of the audio output buffer

    double currentPacketPts;    ///< The presentation time stamp of the current packet
    double currentPacketDts;    ///< The decompression time stamp of the current packet


private:
    //////////////////////////////////////////////////////////////////////////
    //
    //  Private Methods
    //
    //////////////////////////////////////////////////////////////////////////

    // init parameters for decode
    void init();

    ///
    /// @brief  Decode a video frame from current packet, and store it in the video frame buffer
    ///
    int decodeVideoFrame(AVPacket &avpkt);

    ///
    /// @brief  Decode an audio frame from current packet, and store it in the audio frame buffer
    ///
    int decodeAudioFrame(AVPacket &avpkt);
};

#endif

