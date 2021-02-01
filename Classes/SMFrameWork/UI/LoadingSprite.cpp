//
//  LoadingSprite.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#include "LoadingSprite.h"
#include "../Const/SMViewConstValue.h"
#include <cocos2d.h>

#define NUM_TICK (12)
#define TICK_TIME (0.1)
#define DELAY_TIME (0.2)
#define FADE_TIME (0.1)


LoadingSprite* LoadingSprite::createWithTexture(cocos2d::Texture2D* texture)
{
    auto sprite = new (std::nothrow)LoadingSprite();
    if (sprite!=nullptr) {
        bool bInit = false;
        if (texture!=nullptr) {
            bInit = sprite->initWithTexture(texture);
        } else {
            bInit = sprite->initWithFile("images/loading_spinner_white.png");
        }
        if (!bInit) {
            CC_SAFE_DELETE(sprite);
        } else {
            sprite->autorelease();
            sprite->_start = NUM_TICK*cocos2d::rand_0_1();
            sprite->setColor(SMViewConstValue::Const::LOADING_SPRITE_COLOR);
        }
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

LoadingSprite* LoadingSprite::createWithFileName(std::string spriteFileName)
{
    auto sprite = new (std::nothrow)LoadingSprite();
    if (sprite!=nullptr) {
        if (spriteFileName=="") {
            spriteFileName = "images/loading_spinner_white.png";
        }
        if (sprite->initWithFile(spriteFileName)) {
            sprite->autorelease();
            sprite->_start = NUM_TICK*cocos2d::rand_0_1();
            sprite->setColor(SMViewConstValue::Const::LOADING_SPRITE_COLOR);
        } else {
            CC_SAFE_DELETE(sprite);
        }
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

LoadingSprite::LoadingSprite()
{
    
}

LoadingSprite::~LoadingSprite()
{
    
}

void LoadingSprite::setVisible(bool visible)
{
    if (visible!=_visible) {
        _visible = visible;
        if (_visible) {
            _transformUpdated = _transformDirty = _inverseDirty = true;
            _visibleTime = _director->getGlobalTime();
        }
    }
}

void LoadingSprite::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    // calc spend time
    float t = _director->getGlobalTime() - _visibleTime;
    
    if (t<DELAY_TIME) {
        // 대기 시간 보다 이르면 걍 넘어감
        return;
    }
    
    t = (t - DELAY_TIME) / FADE_TIME;
    GLubyte opacity = 0xff;
    if (t<1) {
        opacity *= t;
    }
    
    if (getOpacity()!=opacity) {
        setOpacity(opacity);
    }
    
    float time = _director->getGlobalTime() + _start;
    
    // 흐른 시간만큼 돌린다.
    int tick = ((int)(time/TICK_TIME))%NUM_TICK;
    int angle = tick*360/NUM_TICK;
    
    if (getRotation()!=angle) {
        setRotation(angle);
    }
    
    cocos2d::Sprite::draw(renderer, transform, flags);
}
