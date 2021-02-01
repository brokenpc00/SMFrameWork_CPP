//
//  SlideButton.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
//

#include "SlideButton.h"
#include <2d/CCActionInterval.h>

#define SWIPE_MIN_DISTANCE 50.0f


class ChangeColorTo : public cocos2d::ActionInterval {
    
public:
    ChangeColorTo(){}
    virtual ~ChangeColorTo() {}
    
    static ChangeColorTo* create(float duration, const cocos2d::Color4F& color) {
        auto bgColorTo = new (std::nothrow) ChangeColorTo();
        
        if (bgColorTo != nullptr && bgColorTo->initWithDuration(duration)) {
            bgColorTo->autorelease();
            bgColorTo->_toColor = cocos2d::Vec4(color.r, color.g, color.b, color.a);
        }
        
        return bgColorTo;
    }
    
    virtual void startWithTarget(cocos2d::Node *target) override {
        auto view = (ShaderNode*)target;
        if (view) {
            cocos2d::ActionInterval::startWithTarget(target);
            cocos2d:: Color4F color = view->getColor4F();
            _startColor = cocos2d::Vec4(color.r, color.g, color.b, color.a);
            _deltaColor = _toColor - _startColor;
        }
    }
    
    virtual void update(float t)  override {
        auto view = (ShaderNode*)_target;
        if (view) {
            cocos2d::Vec4 color = _startColor + _deltaColor * t;
            view->setColor4F(cocos2d::Color4F(color.x, color.y, color.z, color.w));
        }
    }
    
protected:
    cocos2d::Vec4 _startColor;
    cocos2d::Vec4 _toColor;
    cocos2d::Vec4 _deltaColor;
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(ChangeColorTo);
};


SlideButton* SlideButton::create(float x, float y, float width, float height, cocos2d::Color4F onBgColor, cocos2d::Color4F offBgColor, cocos2d::Color4F knobColor, float knobPadding, bool bOn)
{
    SlideButton * button = new (std::nothrow) SlideButton();
    if (button && button->initButton(x, y, width, height, onBgColor, offBgColor, knobColor, knobPadding, bOn)) {
        button->autorelease();
    } else {
        CC_SAFE_DELETE(button);
    }
    
    return button;
}

SlideButton::SlideButton()
{
    
}

SlideButton::~SlideButton()
{
    
}

bool SlideButton::initButton(float x, float y, float width, float height, cocos2d::Color4F onBgColor, cocos2d::Color4F offBgColor, cocos2d::Color4F knobColor, float knobPadding, bool bOn)
{
    if (!SMView::init()) {
        return false;
    }
    setContentSize(cocos2d::Size(width, height));
    setPosition(cocos2d::Point(x, y));
    setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0));
    _bgRect = ShapeSolidRoundRect::create();
    _bgRect->setPosition(cocos2d::Point::ZERO);
    _bgRect->setContentSize(getContentSize());
    _bgRect->setColor4F(offBgColor);
    _bgRect->setCornerRadius(height/2);
    
    addChild(_bgRect);
    _bgOnColor = onBgColor;
    _bgOffColor = offBgColor;
    
    _knob = ShapeSolidCircle::create();
    _knob->setPosition(cocos2d::Point::ZERO);
    _knob->setContentSize(cocos2d::Size(height, height));
    _knob->setColor4F(offBgColor);
    
    auto circle = ShapeSolidCircle::create();
    float circleRadius = height-knobPadding*2;
    circle->setContentSize(cocos2d::Size(circleRadius,circleRadius));
    circle->setCornerRadius(circleRadius/2);
    circle->setColor4F(knobColor);
    auto circleBorder = ShapeCircle::create();
    circleBorder->setContentSize(circle->getContentSize());
    circleBorder->setLineWidth(2.0f);
    circleBorder->setCornerRadius(circle->getContentSize().height/2);
    circleBorder->setColor4F(cocos2d::Color4F(230/255.0f, 230/255.0f, 230/255.0f, 1.0f));
    circle->addChild(circleBorder);
    
    _knob->addChild(circle);
    circle->setPosition(cocos2d::Point(height/2-circleRadius/2, height/2-circleRadius/2));
    _bgRect->addChild(_knob);
    setOnClickListener(this);
    _isOn = bOn;
    _isChangeAnimation = false;
    _bCancelOut = false;
    if (bOn) {
        setCheck(bOn, false);
    }

    return true;
}

void SlideButton::setOnClickCallback(std::function<void (SMView *, bool)> callback)
{
    _onClickCallback = callback;
}

void SlideButton::setOnChangeCallback(std::function<void (SMView *, bool)> callback)
{
    _onChangeCallback =  callback;
}

int SlideButton::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event)
{
    static float downX, downY, upX, upY;
    
    SMView::dispatchTouchEvent(action, touch, point, event);
    
    if (action==MotionEvent::ACTION_DOWN) {
        downX = point[0].x;
        downY = point[0].y;
        return SMView::dispatchTouchEvent(action, touch, point, event);
    }
    
    if (action==MotionEvent::ACTION_MOVE) {
        upX = point[0].x;
        upY = point[0].y;
        
        float deltaX = downX-upX;
        
        if (fabsf(deltaX) > SWIPE_MIN_DISTANCE) {
            if (_touchMotionTarget) {
                cancelTouchEvent(_touchMotionTarget, touch, point, event);
                _touchMotionTarget = nullptr;
            }
            return TOUCH_INTERCEPT;
        }
        return SMView::dispatchTouchEvent(action, touch, point, event);
    }
    if (action==MotionEvent::ACTION_UP) {
        upX = point[0].x;
        upY = point[0].y;
        
        float deltaX = downX-upX;
        //        float deltaY = downY-upY;
        
        if (fabsf(deltaX) > SWIPE_MIN_DISTANCE) {
            if (deltaX < 0) {
                if (!_isOn) {
                    _isOn = true;
                    changeButtonState();
                }
            }
            if (deltaX > 0) {
                if (_isOn) {
                    _isOn = false;
                    changeButtonState();
                }
            }
        }
    }
    return SMView::dispatchTouchEvent(action, touch, point, event);
}

void SlideButton::setCancelOut()
{
    _bCancelOut = true;
}

void SlideButton::onClick(SMView* view)
{
    _isOn = !_isOn;
    if (_onClickCallback) {
        _onClickCallback(this, _isOn);
    }
    changeButtonState();
}

void SlideButton::setCheck(bool bCheck, bool bAnimation)
{
    if (_isChangeAnimation) {
        return;
    }
    _isOn = bCheck;
    if (bAnimation) {
        _isChangeAnimation = true;
        if (_isOn) {
            auto colorTo1 = ChangeColorTo::create(0.15f, _bgOnColor);
            colorTo1->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
            _knob->runAction(cocos2d::Sequence::create(colorTo1, cocos2d::CallFunc::create([&]{
            }), NULL));
            auto colorTo2 = ChangeColorTo::create(0.15f, _bgOnColor);
            colorTo2->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
            _bgRect->runAction(cocos2d::Sequence::create(colorTo2, cocos2d::CallFunc::create([&]{
            }), NULL));
            auto moveRight = cocos2d::MoveTo::create(0.15f, cocos2d::Point(_bgRect->getContentSize().width-_knob->getContentSize().width, 0));
            _knob->runAction(cocos2d::Sequence::create(moveRight, cocos2d::CallFunc::create([&]{
                _isChangeAnimation = false;
            }), NULL));
        } else {
            auto colorTo1 = ChangeColorTo::create(0.15f, _bgOffColor);
            colorTo1->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
            _knob->runAction(cocos2d::Sequence::create(colorTo1, cocos2d::CallFunc::create([&]{
            }), NULL));
            auto colorTo2 = ChangeColorTo::create(0.15f, _bgOffColor);
            colorTo2->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
            _bgRect->runAction(cocos2d::Sequence::create(colorTo2, cocos2d::CallFunc::create([&]{
            }), NULL));
            auto moveLeft = cocos2d::MoveTo::create(0.15f, cocos2d::Point(0, 0));
            _knob->runAction(cocos2d::Sequence::create(moveLeft, cocos2d::CallFunc::create([&]{
                _isChangeAnimation = false;
            }), NULL));
        }
    } else {
        if (_isOn) {
            _knob->setPosition(cocos2d::Point(_bgRect->getContentSize().width-_knob->getContentSize().width, 0));
            _knob->setColor4F(_bgOnColor);
            _bgRect->setColor4F(_bgOnColor);
        } else {
            _knob->setPosition(cocos2d::Point(0, 0));
            _knob->setColor4F(_bgOffColor);
            _bgRect->setColor4F(_bgOffColor);
        }
    }
}

void SlideButton::changeButtonState()
{
    if (_isChangeAnimation) {
        return;
    }
    if (_bCancelOut) {
        _bCancelOut = false;
        return;
    }
    
    _isChangeAnimation = true;
    if (_isOn) {
        auto colorTo1 = ChangeColorTo::create(0.15f, _bgOnColor);
        colorTo1->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
        _knob->runAction(cocos2d::Sequence::create(colorTo1, cocos2d::CallFunc::create([&]{
        }), NULL));
        auto colorTo2 = ChangeColorTo::create(0.15f, _bgOnColor);
        colorTo2->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
        _bgRect->runAction(cocos2d::Sequence::create(colorTo2, cocos2d::CallFunc::create([&]{
        }), NULL));
        auto moveRight = cocos2d::MoveTo::create(0.15f, cocos2d::Point(_bgRect->getContentSize().width-_knob->getContentSize().width, 0));
        _knob->runAction(cocos2d::Sequence::create(moveRight, cocos2d::CallFunc::create([&]{
            _isChangeAnimation = false;
            if (_onChangeCallback) {
                _onChangeCallback(this, _isOn);
            }
        }), NULL));
    } else {
        auto colorTo1 = ChangeColorTo::create(0.15f, _bgOffColor);
        colorTo1->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
        _knob->runAction(cocos2d::Sequence::create(colorTo1, cocos2d::CallFunc::create([&]{
        }), NULL));
        auto colorTo2 = ChangeColorTo::create(0.15f, _bgOffColor);
        colorTo2->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
        _bgRect->runAction(cocos2d::Sequence::create(colorTo2, cocos2d::CallFunc::create([&]{
        }), NULL));
        auto moveLeft = cocos2d::MoveTo::create(0.15f, cocos2d::Point(0, 0));
        _knob->runAction(cocos2d::Sequence::create(moveLeft, cocos2d::CallFunc::create([&]{
            _isChangeAnimation = false;
            if (_onChangeCallback) {
                _onChangeCallback(this, _isOn);
            }
        }), NULL));
    }
}
