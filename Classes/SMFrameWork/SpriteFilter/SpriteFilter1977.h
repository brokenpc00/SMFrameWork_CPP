//
//  SpriteFilter1977.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilter1977_h
#define SpriteFilter1977_h

#include <2d/CCSprite.h>

class SpriteFilter1977 : public cocos2d::Sprite
{
public:
    static SpriteFilter1977 * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilter1977();
    virtual ~SpriteFilter1977();
    
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



#endif /* SpriteFilter1977_h */
