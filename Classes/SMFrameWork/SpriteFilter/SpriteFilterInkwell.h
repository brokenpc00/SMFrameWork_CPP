//
//  SpriteFilterInkwell.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterInkwell_h
#define SpriteFilterInkwell_h

#include <2d/CCSprite.h>

class SpriteFilterInkwell : public cocos2d::Sprite
{
public:
    static SpriteFilterInkwell * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterInkwell();
    virtual ~SpriteFilterInkwell();
    
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


#endif /* SpriteFilterInkwell_h */
