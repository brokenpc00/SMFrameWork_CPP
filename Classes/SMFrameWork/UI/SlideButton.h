//
//  SlideButton.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
//

#ifndef SlideButton_h
#define SlideButton_h

#include "../Base/SMView.h"
#include "../Base/SMImageView.h"
#include "../Base/ShaderNode.h"
#include "../Const/SMViewConstValue.h"


class SlideButton : public SMView, public OnClickListener
{
public:
    static SlideButton* create(float x, float y, float width, float height, cocos2d::Color4F onBgColor=SMViewConstValue::Const::KNOB_ON_BGCOLOR, cocos2d::Color4F offBgColor=SMViewConstValue::Const::KNOB_OFF_BGCOLOR, cocos2d::Color4F knobColor=SMViewConstValue::Const::KNOB_COLOR, float knobPadding=7, bool bOn=false);
    
    void setOnClickCallback(std::function<void(SMView* view, bool isOn)> callback);
    void setOnChangeCallback(std::function<void(SMView* view, bool isOn)> callback);
    void setCheck(bool bCheck, bool bAnimation=false);
    void setCancelOut();
    
protected:
    SlideButton();
    virtual ~SlideButton();
    
    bool initButton(float x, float y, float width, float height, cocos2d::Color4F onBgColor, cocos2d::Color4F offBgColor, cocos2d::Color4F knobColor, float knobPadding, bool bOn);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    virtual void onClick(SMView* view) override;
    
private:
    DISALLOW_COPY_AND_ASSIGN(SlideButton);
    
    ShapeSolidRoundRect * _bgRect;
    ShapeSolidCircle * _knob;
    
    cocos2d::Color4F _bgOnColor;
    cocos2d::Color4F _bgOffColor;
    bool _isChangeAnimation;
    bool _isOn;
    
    
    bool _bCancelOut;
    
    void changeButtonState();
    std::function<void(SMView* view, bool isOn)> _onChangeCallback;
    std::function<void(SMView* view, bool isOn)> _onClickCallback;
    
};


#endif /* SlideButton_h */
