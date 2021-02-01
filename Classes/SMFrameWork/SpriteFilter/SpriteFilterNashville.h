//
//  SpriteFilterNashville.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterNashville_h
#define SpriteFilterNashville_h

#include <2d/CCSprite.h>

class SpriteFilterNashville : public cocos2d::Sprite
{
public:
    static SpriteFilterNashville * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterNashville();
    virtual ~SpriteFilterNashville();
    
    virtual bool initWithTexture(cocos2d::Texture2D * texture) override;
    
protected:
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
protected:
    cocos2d::Texture2D * _ext;
    
private:
    GLint _uniformAddedTexture;
    cocos2d::CustomCommand _customCommand;
    
};


#endif /* SpriteFilterNashville_h */
