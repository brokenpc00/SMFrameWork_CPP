//
//  BaseCropView.hpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#ifndef BaseCropView_h
#define BaseCropView_h

#include "../Base/SMView.h"
#include "../Base/MaskSprite.h"

class ShapeSolidRect;

class BaseCropView : public SMView
{
public:
    float getSpriteScale();
    
    cocos2d::Rect getBoundRect();
    
    cocos2d::Sprite* cropSprite(cocos2d::Rect& outRect);
    
    bool cropData(uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect);
    
    void showUILayer(bool show);
    
protected:
    BaseCropView();
    virtual ~BaseCropView();
    virtual bool init(cocos2d::Sprite* sprite, const cocos2d::Rect& boundRect);
    
    void setDimOpacity(float opacity, float duration, float delay);
    
    virtual cocos2d::Sprite* cropSpriteInternal(cocos2d::Rect& outRect) = 0;
    
    virtual bool cropDataInternal(uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect) = 0;
    
protected:
    MaskSprite* _maskSprite;
    ShapeSolidRect* _dim;
    cocos2d::Rect _boundRect;
    cocos2d::Size _textureSize;
    cocos2d::Rect _textureRect;
    SMView* _uiLayer;
};



#endif /* BaseCropView_h */
