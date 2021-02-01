//
//  SMAudioDecoder.h
//  IPCT
//
//  Created by SteveKim on 2020/02/19.
//

#ifndef SMAudioDecoder_h
#define SMAudioDecoder_h

#include "AudioCodecProtocol.h"
#include <memory>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <unordered_map>

#define USING_AUDIO

extern "C" {
    struct AVFrame;
    struct AVPacket;
    struct AVFormatContext;
    struct AVCodecContext;
    struct AVStream;
    struct SwrContext;
}

class AudioFrame;
class SMAudioDecoder;

class AudioDecodeThread;

class SMAudioDecoder : public std::enable_shared_from_this<SMAudioDecoder> {
public:
    static std::shared_ptr<SMAudioDecoder> create(AudioCodecProtocol* client, const std::string& mediaName);
    
    SMAudioDecoder();
    
    virtual ~SMAudioDecoder();
    
    void playAudio();
    
    void stopAudio();
    
    void destroy();
    
public:
    AudioFrame* dequeueDecodedAudioFrame();
    void queueIdleAudioFrame(AudioFrame* frame);
    
private:
    bool init(AudioCodecProtocol* client, const std::string& audioName);
    
    void release();
    
    AudioFrame* dequeueIdleAudioFrame();
    void queueDecodedAudioFrame(AudioFrame* frame);
    std::queue<AudioFrame*> _idleAudioQueue;
    std::queue<AudioFrame*> _decodedAudioQueue; // 디코딩이 끝난 사운드 프레임
    
    int decodeAudioFrame(AudioFrame** audioFrame=nullptr);

private:
    
    void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime);
    
    void threadForInit();
    
    void threadForAudioPlayback();
    
    void waitForPlay();
    
    void decodeAudio(AudioFrame ** audioFrame);
    
private:
    
    std::mutex _audioQueueMutex;
    std::mutex _audioMutex;
    std::condition_variable _playCond;
    
private:
    AudioCodecProtocol* _client;
    
    std::string _mediaUrl;
    
    std::shared_ptr<SMAudioDecoder> _sharedThis;
    
    std::shared_ptr<AudioDecodeThread> _audioDecodeThread;
    
    bool _interrupted;
    
    bool _playing;
    
    bool _initialized;
    
private:
    std::string _mediaName;
    
    AVFormatContext* _formatContext;
    AVCodecContext* _audioCodecContext;
    AVPacket* _avPacket;
    AVFrame* _avFrame;
    SwrContext * _swrContext;

    double _audioDurationTime;
    int _channels;
    int _samples;
    int _bit_rate;
    int _fmt;

    int _audioStreamIndex;

    double _playTime;
    
    int _audioMaxFrameCount;
    
    friend class AudioDecodeThread;

};


#endif /* SMAudioDecoder_h */
