//
//  CircleCropSprite.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#ifndef CircleCropSprite_h
#define CircleCropSprite_h
#include <2d/CCSprite.h>

class CircleCropSprite : public cocos2d::Sprite
{
public:
    static CircleCropSprite* createWithTexture(cocos2d::Texture2D* texture, bool crop=true);
    
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    
    void setAntiAliasWidth(float aaWidth);

    void setRadius(float radius);
    
protected:
    CircleCropSprite();
    virtual ~CircleCropSprite();
    
    bool initWithCropTexture(cocos2d::Texture2D* texture, bool crop);

    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
private:
    GLint _uniformDimension;
    GLint _uniformRadius;
    GLint _uniformAAWidth;
    GLint _uniformColor;
    GLint _uniformTexture;
    
    float _aaWidth;
    float _radius;
    cocos2d::Vec2 _size;
    
    cocos2d::CustomCommand _customCommand;
};


#endif /* CircleCropSprite_h */
