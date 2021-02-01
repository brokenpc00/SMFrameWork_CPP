//
//  SpriteFilterLordKelvin.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterLordKelvin_h
#define SpriteFilterLordKelvin_h

#include <2d/CCSprite.h>

class SpriteFilterLordKelvin : public cocos2d::Sprite
{
public:
    static SpriteFilterLordKelvin * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterLordKelvin();
    virtual ~SpriteFilterLordKelvin();
    
    virtual bool initWithTexture(cocos2d::Texture2D * texture) override;
    
protected:
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);

protected:
    cocos2d::Texture2D * _ext1;
    
private:
    GLint _uniformAddedTexture1;
    cocos2d::CustomCommand _customCommand;
    
};

#endif /* SpriteFilterLordKelvin_h */
