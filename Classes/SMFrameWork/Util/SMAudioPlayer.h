//
//  SMAudioPlayer.h
//  IPCT
//
//  Created by SteveKim on 2020/02/21.
//

#ifndef SMAudioPlayer_h
#define SMAudioPlayer_h

#include <stdio.h>
#include <cocos2d.h>
#include <OpenAL/OpenAL.h>
#include "AudioCodecProtocol.h"
#include "../Base/SMView.h"
#include <SimpleAudioEngine.h>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <unordered_map>

#define QUEUE_BUFFER_NUM 3

class SMAudioPlayer : public cocos2d::Node, public AudioCodecProtocol
{
public:
    static SMAudioPlayer * getInstance() {
        static SMAudioPlayer * _instance = nullptr;
        if (_instance==nullptr) {
            _instance = new SMAudioPlayer();
            _instance->setContentSize(cocos2d::Size(0, 0));
            _instance->setPosition(0, 0);
            _instance->retain();
            cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::DIM)->addChild(_instance);
        }
        
        return _instance;
    }
    
    bool initAudio(std::string audioUrl);
    void startAudioFn();
    void stopAudioFn();
//    void setData(uint8_t* buf, int size);
    void clearAudio();
    
    int getSamples() {return _samples;}
    int getBitrate() {return _bit_rate;}
    int getChannels() {return _channels;}
//    int getFormat() {return _fmt;}
    
    
protected:
    SMAudioPlayer();
    ~SMAudioPlayer();
    
    virtual void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime) override;
    
    void audioBufferPeek(float dt);
    
    void destroy();
    
    void listingAudioDevice(const ALCchar* devices);
    
    ALenum getFormat(short channels, short bitsPerChannel);
    
    std::string openALErrorString(int error);
    bool CheckError(const std::string &pFile, int pLine, const std::string &pfunc);
    
    void EnqueueBuffer(const uint8_t * pData, int pFrames, short fmt);
    int QueuedUpBuffers();
    int ProcessedBuffers();
    
    void openAudioFromQueue(uint8_t * buf, int size);
    bool updateQueueBuffer();
    
    
//    int getDecodedAudioFrames(uint32_t p_numBuffers, std::vector<uint8_t*>& p_audioBuffers, std::vector<unsigned int>& p_audioBufferSizes);
private:
    int _samples;
    int _bit_rate;
    int _channels;
    int _fmt;
    float _updateDeltaTime;
    
    
    // open al
    bool _isDestroyed;
    bool _removeByAudioEngine;
    bool _ready;

    unsigned int _id;
    float _volume;
    bool _loop;
    std::function<void (int, const std::string &)> _finishCallbak;
    float _currTime;
    bool _streamingSource;

    ALuint _alSourceID;
    ALuint _bufferIds[QUEUE_BUFFER_NUM];
    ALCdevice * _device;
    ALboolean _enumeration;
    ALCcontext * _context;
    ALuint _bufferID;
    ALint _source_state;
    ALenum _format;
    
    std::mutex _audioQueueMutex;
    std::mutex _audioMutex;
    std::condition_variable _playCond;

    
//    CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic();
};


#endif /* SMAudioPlayer_h */
