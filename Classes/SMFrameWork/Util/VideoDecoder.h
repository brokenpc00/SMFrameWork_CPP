//
//  VideoDecoder.h
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#ifndef VideoDecoder_h
#define VideoDecoder_h

#include "VideoCodecProtocol.h"
//#include "AudioCodecProtocol.h"
#include <memory>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <unordered_map>


//#define USING_AUDIO

extern "C" {
    struct AVFrame;
    struct AVPacket;
    struct AVFormatContext;
    struct AVCodecContext;
    struct AVStream;
    struct AVPacketList;
    struct SwrContext;
    struct SwsContext;
    struct AVCodecParserContext;
}

class VideoFrame;
class VideoDecoder;
class VideoDecodeThread;

class VideoDecoder : public std::enable_shared_from_this<VideoDecoder> {
public:
    static std::shared_ptr<VideoDecoder> create(VideoCodecProtocol* client, const std::string& videoName);
    
    VideoDecoder();
    
    virtual ~VideoDecoder();
    
    void playVideo();
    
    void stopVideo();
    
    void destroy();
    
public:
    VideoFrame* dequeueDecodedVideoFrame();
    void queueIdleVideoFrame(VideoFrame* frame);
    
private:
    bool init(VideoCodecProtocol* client, const std::string& videoName);
    
    void release();
    
    // for video
    VideoFrame* dequeueIdleVideoFrame();
    void queueDecodedVideoFrame(VideoFrame* frame);
    std::queue<VideoFrame*> _idleVideoQueue;    // 놀고 있는 프레임들
    std::queue<VideoFrame*> _decodedVideoQueue; // 디코딩이 끝난 이미지 프레임

#ifdef USING_AUDIO
    int decodeVideoFrame(VideoFrame** queueVideoFrame=nullptr, VideoFrame** queueAudioFrame=nullptr);
#else
    int decodeVideoFrame(VideoFrame** queueVideoFrame=nullptr);
#endif

private:
    
    void onVideoInitResult(bool success, int width, int height, double frameRate, double durationTime);
    
    void threadForInit();
    
    void threadForVideoPlayback();
    
    void waitForPlay();
    
    void decodeVideo(VideoFrame ** queueFrame);
    
private:
    
    std::mutex _videoQueueMutex;
    std::mutex _videoMutex;
    std::condition_variable _playCond;
    
private:
    VideoCodecProtocol* _videoClient;
    
    std::string _videoUrl;
    
    std::shared_ptr<VideoDecoder> _sharedThis;
    
    std::shared_ptr<VideoDecodeThread> _videoDecodeThread;
    
    bool _interrupted;
    
    bool _playing;
    
    bool _initialized;
    
private:
    std::string _videoName;
    
    int _videoWidth;
    int _videoHeight;
    int _videoStreamIndex;
    double _videoDurationTime;
    double _videoFrameRate;
    
    
    AVFormatContext* _formatContext;
    
    AVCodecContext* _videoCodecContext;
    AVPacket* _avPacket;
    AVFrame* _avFrame;

    double _playTime;
    
    int _videoMaxFrameCount;
    
    friend class VideoDecodeThread;
    
    SwsContext * _swsContext;
    
    int readVideoPacket(VideoFrame** queueVideoFrame);
    int readAudioPacket(VideoFrame** queueAudioFrame);
    
    // for audio
#ifdef USING_AUDIO
public:
    VideoFrame* dequeueDecodedAudioFrame();
    void queueIdleAudioFrame(VideoFrame* frame);
private:
    std::mutex _audioQueueMutex;
    std::mutex _audioMutex;
//    AudioCodecProtocol* _audioClient;
    void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime);

    VideoFrame* dequeueIdleAudioFrame();
    void queueDecodedAudioFrame(VideoFrame* frame);
    std::queue<VideoFrame*> _idleAudioQueue;
    std::queue<VideoFrame*> _decodedAudioQueue; // 디코딩이 끝난 사운드 프레임
    int decodeAudioFrame(VideoFrame** audioFrame=nullptr);

    AVCodecContext* _audioCodecContext;
    SwrContext * _swrContext;

    int _channels;
    int _samples;
    int _bit_rate;
    int _fmt;
    
    int _freq;
    int _format;
    

    int _audioStreamIndex;
    int _audioMaxFrameCount;

    int initDecodeFrame();
    int newVideoDecodeFrame(VideoFrame** queueFrame);
    int newAudioDecodeFrame(VideoFrame** queueFrame);
#endif
};


#endif /* VideoDecoder_h */
