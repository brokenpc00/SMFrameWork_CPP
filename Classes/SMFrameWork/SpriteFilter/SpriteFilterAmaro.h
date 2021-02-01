//
//  SpriteFilterAmaro.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterAmaro_h
#define SpriteFilterAmaro_h

#include <2d/CCSprite.h>

class SpriteFilterAmaro : public cocos2d::Sprite
{
public:
    static SpriteFilterAmaro * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterAmaro();
    virtual ~SpriteFilterAmaro();
    
    virtual bool initWithTexture(cocos2d::Texture2D * texture) override;
    
protected:
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
protected:
    cocos2d::Texture2D * _ext1;
    cocos2d::Texture2D * _ext2;
    cocos2d::Texture2D * _ext3;
    
private:
    GLint _uniformAddedTexture1;
    GLint _uniformAddedTexture2;
    GLint _uniformAddedTexture3;
    cocos2d::CustomCommand _customCommand;
    
};



#endif /* SpriteFilterAmaro_h */
