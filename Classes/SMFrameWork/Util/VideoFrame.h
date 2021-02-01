//
//  VideoFrame.h
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#ifndef VideoFrame_h
#define VideoFrame_h

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

class VideoFrame
{
public:
    VideoFrame(AVFrame* avFrame, bool isVideo=true);
    virtual ~VideoFrame();
    
public:
    void setData(AVFrame* framem, SwrContext* swrContext=nullptr);

    // for video
    int getWidth() { return _width; }
    int getHeight() { return _height; }
    double getTimestamp() { return _timestamp; }
    uint8_t* getData()  { return _data; }
    uint8_t* getDataU() { return _dataU; }
    uint8_t* getDataV() { return _dataV; }

private:
    double _timestamp;
    int _width;
    int _height;
    uint8_t* _data;
    uint8_t* _dataU;
    uint8_t* _dataV;
    
    bool _videoFrame;
    
    // for video
    
public:
    uint8_t* getAudioBuffer() {return _audio_buf;}
    int getAudioSize() {return _size;}
    int getSampleRate() {return _sample_rate;}
    int getFormat() {return _fmt;}
    int getChannels() {return _channels;}
    int getSamples() {return _samples;}
    int getBitPerSample() {return _bit_per_sample;}
    int getBitsPerChannel() {return _bit_per_channel;}
private:
    int _channels;
    int _samples;
    int _sample_rate;
    int _fmt;
    int _size;
    uint8_t * _audio_buf;
    int _bit_per_sample;
    int _bit_per_channel;
};

#endif /* VideoFrame_hpp */
