//
//  SpriteFilterXproii.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 26..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterXproii_h
#define SpriteFilterXproii_h

#include <2d/CCSprite.h>

class SpriteFilterXproii : public cocos2d::Sprite
{
public:
    static SpriteFilterXproii * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterXproii();
    virtual ~SpriteFilterXproii();
    
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



#endif /* SpriteFilterXproii_h */
