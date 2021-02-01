//
//  LoadingSprite.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#ifndef LoadingSprite_h
#define LoadingSprite_h

#include <2d/CCSprite.h>

class LoadingSprite : public cocos2d::Sprite
{
public:
    static LoadingSprite* createWithTexture(cocos2d::Texture2D* texture=nullptr);
    static LoadingSprite* createWithFileName(std::string spriteFileName="");
    
    virtual void setVisible(bool visible) override;
    
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    
protected:
    LoadingSprite();
    virtual ~LoadingSprite();
    
private:
    float _start;
    float _visibleTime;
};


#endif /* LoadingSprite_h */
