//
//  SpriteFilterBrannan.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SpriteFilterBrannan_h
#define SpriteFilterBrannan_h


#include <2d/CCSprite.h>

class SpriteFilterBrannan : public cocos2d::Sprite
{
public:
    static SpriteFilterBrannan * createWithTexture(cocos2d::Texture2D * texture);
    
protected:
    SpriteFilterBrannan();
    virtual ~SpriteFilterBrannan();
    
    virtual bool initWithTexture(cocos2d::Texture2D * texture) override;
    
protected:
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
protected:
    cocos2d::Texture2D * _ext1;
    cocos2d::Texture2D * _ext2;
//    cocos2d::Texture2D * _ext3;
    cocos2d::Texture2D * _ext4;
    cocos2d::Texture2D * _ext5;
    
private:
    GLint _uniformAddedTexture1;
    GLint _uniformAddedTexture2;
//    GLint _uniformAddedTexture3;
    GLint _uniformAddedTexture4;
    GLint _uniformAddedTexture5;
    cocos2d::CustomCommand _customCommand;
    
};



#endif /* SpriteFilterBrannan_h */