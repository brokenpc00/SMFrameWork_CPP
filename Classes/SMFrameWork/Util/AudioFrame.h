//
//  AudioFrame.h
//  IPCT
//
//  Created by SteveKim on 2020/02/19.
//

#ifndef AudioFrame_h
#define AudioFrame_h

#include <stdio.h>
#include <cocos2d.h>

extern "C" {
    struct AVFrame;
    struct AVPacket;
    struct AVFormatContext;
    struct AVCodecContext;
    struct SwrContext;
    struct SwsContext;
}

class AudioFrame
{
public:
    AudioFrame(AVFrame* avFrame);
    virtual ~AudioFrame();
    
public:
    void setData(AVFrame* framem, SwrContext* swrContext=nullptr);

    // for video
    uint8_t* getAudioBuffer() {return _audio_buf;}
    int getAudioSize() {return _size;}
    int getSampleRate() {return _sample_rate;}
    int getFormat() {return _fmt;}
    int getChannels() {return _channels;}
    int getSamples() {return _samples;}
    int getBitPerSample() {return _bit_per_sample;}
    int getBitsPerChannel() {return _bit_per_channel;}
private:
    double _timestamp;
    int _channels;
    int _samples;
    int _sample_rate;
    int _fmt;
    int _size;
    uint8_t * _audio_buf;
    int _bit_per_sample;
    int _bit_per_channel;
};


#endif /* AudioFrame_h */
