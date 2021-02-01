//
//  FocusingView.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 3..
//  Copyright © 2017년 KimSteve. All rights reserved.
//  화면에 포커싱되는 효과를 주는 view

#ifndef FocusingView_h
#define FocusingView_h

#include "../Base/SMView.h"

class ShapeCircle;
class ShapeSolidRect;
class ShapeArcRing;
class OnFocusingListener;


class FocusLayer : public SMView
{
public:
    static FocusLayer* create();
    
    void startAutoFocus();
    void finishAutoFocus();
    
protected:
    FocusLayer();
    virtual ~FocusLayer();
    
    virtual bool init() override;
    
    void setRadius(float radius);
    float getRadius() {return _radius;}
    
    void onFinishUpdate(float t);
    void onTimeOut(float dt);
    
protected:
    float _showValue;
    
private:
    class FocusingAction;
    class FinishAction;
    
    // shape
    ShapeCircle* _mainRing;
    ShapeCircle* _subRing;
    ShapeArcRing* _arcRing;
    ShapeSolidRect* _crosshairV;
    ShapeSolidRect* _crosshairH;

    float _radius;
    
    FinishAction* _finishAction;
};

class FocusingView : public SMView
{
public:
    static FocusingView * create(OnFocusingListener* l);
    
    void setFocusLayer(float x, float y, float scale, bool isMirror, const cocos2d::Size& size);;
    
    void setEnableFocus(bool enabled);
    
protected:
    FocusingView();
    virtual ~FocusingView();
    
    bool initWithListener(OnFocusingListener* l);
    
    virtual void performClick(const cocos2d::Vec2& worldPoint) override;
    
private:
    float _baseScale;
    bool _isMirror;
    FocusLayer * _focusLayer;
    OnFocusingListener * _listener;
};


#endif /* FocusingView_h */
