//
//  AudioCodecProtocol.h
//  IPCT
//
//  Created by SteveKim on 2020/02/19.
//

#ifndef AudioCodecProtocol_h
#define AudioCodecProtocol_h

#include <stdlib.h>
#include <memory>
#include <string>

class AudioFrame;
class SMAudioDecoder;

class AudioCodecProtocol
{
public:
    AudioCodecProtocol();
    virtual ~AudioCodecProtocol();
    
    bool initAudioDecoder(const std::string& mediaName);
    void playAudio();
    void stopAudio();

    AudioFrame* getAudioFrame();

    // for video
    virtual void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime) = 0;
    void recycleAudioFrame(AudioFrame* frame);
    
private:
    std::shared_ptr<SMAudioDecoder> _decoder;
};


#endif /* AudioCodecProtocol_h */
