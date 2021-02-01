//
//  VideoCodecProtocol.h
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#ifndef VideoCodecProtocol_h
#define VideoCodecProtocol_h

#include <stdlib.h>
#include <memory>
#include <string>

class VideoFrame;
class VideoDecoder;

class VideoCodecProtocol
{
public:
    VideoCodecProtocol();
    virtual ~VideoCodecProtocol();
    
    bool initVideoDecoder(const std::string& videoName);
    void playVideo();
    void stopVideo();

    VideoFrame* getVideoFrame();
    

    // for video
    virtual void onVideoInitResult(bool success, int width, int height, double frameRate, double durationTime) = 0;
    void recycleVideoFrame(VideoFrame* frame);

//#ifdef USING_AUDIO
// for audio
    VideoFrame* getAudioFrame();
    void recycleAudioFrame(VideoFrame* frame);
    virtual void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime) = 0;
    //#endif
    
private:
    std::shared_ptr<VideoDecoder> _decoder;
};

#endif /* VideoCodecProtocol_h */
