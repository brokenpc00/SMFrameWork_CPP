//
//  VideoSprite.h
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#ifndef VideoSprite_h
#define VideoSprite_h

#include "VideoCodecProtocol.h"
#include "../Base/SMImageView.h"
#include <2d/CCSprite.h>
#include <string>

class VideoSprite : public cocos2d::Sprite, protected VideoCodecProtocol
{
public:
    static VideoSprite* createWithImageView(const std::string& videoSource, SMImageView * imageView); // url
    
    void play();
    void pause();
    void resume();
    void stop();

protected:
    virtual bool initWithVideo(const std::string& videoSource, SMImageView * imageView);
    virtual void onEnter() override;
    virtual void onExit() override;
    void onFrameUpdate(float dt);
    
    virtual void onVideoInitResult(bool success, int width, int height, double frameRate, double durationTime) override;
    virtual void onAudioInitResult(bool success, int samples, int rate, int fmt, double durationTime) override {};

private:
    VideoSprite();
    virtual ~VideoSprite();
    
    void initShader();
    cocos2d::Texture2D* _texturePlanarU;
    cocos2d::Texture2D* _texturePlanarV;
    
    float _updateDeltaTime;
    SMImageView* _displayView;
    
    int _videoWidth;
    int _videoHeight;
    bool _initialized;
};

#endif /* VideoSprite_h */
