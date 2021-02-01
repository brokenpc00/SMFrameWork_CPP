//
//  SpriteFilterLomofi.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterLomofi_h
#define SpriteFilterLomofi_h

#include <2d/CCSprite.h>

class SpriteFilterLomofi : public cocos2d::Sprite
{
public:
    static SpriteFilterLomofi * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterLomofi();
    virtual ~SpriteFilterLomofi();
    
    virtual bool initWithTexture(cocos2d::Texture2D * texture) override;
    
protected:
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
protected:
    cocos2d::Texture2D * _ext1;
    cocos2d::Texture2D * _ext2;
    
private:
    GLint _uniformAddedTexture1;
    GLint _uniformAddedTexture2;
    cocos2d::CustomCommand _customCommand;
    
};


#endif /* SpriteFilterLomofi_h */
