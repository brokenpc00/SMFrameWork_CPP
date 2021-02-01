//
//  SMSlider.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 10..
//
//

#include "SMSlider.h"
#include "SMButton.h"
#include "ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "ViewAction.h"
#include <2d/CCTweenFunction.h>
#include <math.h>
#include <cmath>
#include <cocos2d.h>

#define FLAG_SLIDE_VALUE (1<<0)
#define FLAG_CONTENT_SIZE (1<<1)

#define CENTER_RADIUS (10)

const SMSlider::InnerColor SMSlider::InnerColor::LIGHT(MAKE_COLOR4F(0xdbdcdf, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xeeeff1, 1));
const SMSlider::InnerColor SMSlider::InnerColor::DARK(MAKE_COLOR4F(0x5e5e5e, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xeeeff1, 1));

class SMSlider::KnobButton : public SMButton
{
public:
    static KnobButton* create() {
        auto knob = new (std::nothrow) KnobButton();
        if (knob && knob->initWithStyle(SMButton::Style::SOLID_CIRCLE)) {
            knob->autorelease();
        } else {
            CC_SAFE_DELETE(knob);
        }
        return knob;
    }
    
    virtual void onSmoothUpdate(const uint32_t flags, float dt) override {
        if (flags | VIEWFLAG_POSITION) {
            ((SMSlider*)getParent())->updateKnob();
        }
    }
};

SMSlider* SMSlider::create(const Type type, const InnerColor& initColor)
{
    auto slider = new (std::nothrow) SMSlider();
    
    if (slider && slider->initWithType(type, initColor)) {
        slider->autorelease();
    } else {
        CC_SAFE_DELETE(slider);
    }
    
    return slider;
}

SMSlider::SMSlider() :
_listener(nullptr),
_sliderValue(0),
_minValue(0),
_maxValue(1),
_knobFocused(false),
_minFocused(false),
_maxFocused(false),
_sliderWidth(1),
_leftLine(nullptr),
_rightLine(nullptr),
_bgLine(nullptr),
_circle(nullptr)
{
    
}

SMSlider::~SMSlider()
{
    
}

bool SMSlider::initWithType(const SMSlider::Type type, const SMSlider::InnerColor &initColor)
{
    if (!SMView::init()) {
        return false;
    }
    
    _type = type;
    
    switch (_type) {
        case Type::MINUS_ONE_TO_ONE:
        {
            _leftLine = ShapeSolidRect::create();
            _leftLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
            _leftLine->setColor4F(initColor.bgLine);
            addChild(_leftLine);
            
            _rightLine = ShapeSolidRect::create();
            _rightLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
            _rightLine->setColor4F(initColor.bgLine);
            addChild(_rightLine);
            
            _circle = ShapeCircle::create();
            _circle->setContentSize(cocos2d::Size(CENTER_RADIUS*2, CENTER_RADIUS*2));
            _circle->setLineWidth(4);
            _circle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            _circle->setColor4F(initColor.fgLine);
            addChild(_circle);
        }
            break;
        case Type::ZERO_TO_ONE:
        {
            _rightLine = ShapeSolidRect::create();
            _rightLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
            _rightLine->setColor4F(initColor.bgLine);
            addChild(_rightLine);
            
            _circle = ShapeCircle::create();
            _circle->setContentSize(cocos2d::Size(CENTER_RADIUS*2, CENTER_RADIUS*2));
            _circle->setLineWidth(4);
            _circle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            _circle->setColor4F(initColor.fgLine);
            addChild(_circle);
        }
            break;
        case Type::MIN_TO_MAX:
        {
            _rightLine = ShapeSolidRect::create();
            _rightLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
            _rightLine->setColor4F(initColor.bgLine);
            addChild(_rightLine);
        }
            break;
    }

    _bgLine = ShapeSolidRect::create();
    _bgLine->setColor4F(initColor.fgLine);
    addChild(_bgLine);
    
    
    if (_type==Type::MIN_TO_MAX) {
        _minButton = KnobButton::create();
        _minButton->setContentSize(cocos2d::Size(120, 120));
        _minButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _minButton->setPadding(29);
        _minButton->setButtonColor(SMButton::State::NORMAL, initColor.knob);
        _minButton->setButtonColor(SMButton::State::PRESSED, initColor.knobPress);
        
        auto minShadow = ShapeSolidCircle::create();
        _minButton->setBackgroundNode(minShadow);
        minShadow->setContentSize(cocos2d::Size(75, 75));
        minShadow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        minShadow->setAntiAliasWidth(20);
        minShadow->setPosition(60, 60-2);
        _minButton->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.2));
        _minButton->setOnTouchListener(this);
        addChild(_minButton);
        
        _maxButton = KnobButton::create();
        _maxButton->setContentSize(cocos2d::Size(120, 120));
        _maxButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _maxButton->setPadding(29);
        _maxButton->setButtonColor(SMButton::State::NORMAL, initColor.knob);
        _maxButton->setButtonColor(SMButton::State::PRESSED, initColor.knobPress);
        
        auto maxShadow = ShapeSolidCircle::create();
        _maxButton->setBackgroundNode(maxShadow);
        maxShadow->setContentSize(cocos2d::Size(75, 75));
        maxShadow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        maxShadow->setAntiAliasWidth(20);
        maxShadow->setPosition(60, 60-2);
        _maxButton->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.2));
        _maxButton->setOnTouchListener(this);
        addChild(_maxButton);
        setSliderValue(_minValue, _maxValue);
    } else {
        _knobButton = KnobButton::create();
        _knobButton->setContentSize(cocos2d::Size(120, 120));
        _knobButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _knobButton->setPadding(29);
        _knobButton->setButtonColor(SMButton::State::NORMAL, initColor.knob);
        _knobButton->setButtonColor(SMButton::State::PRESSED, initColor.knobPress);
        
        auto shadow = ShapeSolidCircle::create();
        _knobButton->setBackgroundNode(shadow);
        shadow->setContentSize(cocos2d::Size(75, 75));
        shadow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        shadow->setAntiAliasWidth(20);
        shadow->setPosition(60, 60-2);
        _knobButton->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.2));
        _knobButton->setOnTouchListener(this);
        addChild(_knobButton);
        setSliderValue(0);
    }
    
    return true;
}

void SMSlider::setContentSize(const cocos2d::Size &size)
{
    SMView::setContentSize(size);
    
    registerUpdate(FLAG_CONTENT_SIZE);
}

void SMSlider::setSliderValue(const float sliderValue, const bool immediate)
{
    CCASSERT(_type!=Type::MIN_TO_MAX, "type must be minusToOne or zeroToOne");
    float value;
    if (_type==Type::MINUS_ONE_TO_ONE) {
        value = std::min(std::max(sliderValue, -1.0f), 1.0f);
    } else {
        value = std::min(std::max(sliderValue, 0.0f), 1.0f);
    }
    
    _sliderValue = value;
    
    if (_listener) {
        _listener->onSliderValueChanged(this, _sliderValue);
    }
    if (onSliderValueChange) {
        onSliderValueChange(this, _sliderValue);
    }
    
    setKnobPosition(_sliderValue, immediate);
}

void SMSlider::setSliderValue(const float minValue, const float maxValue, const bool immediate)
{
    CCASSERT(_type==Type::MIN_TO_MAX, "type must be minToMax");
    _minValue = std::min(std::max(minValue, 0.0f), 1.0f);
    _maxValue = std::min(std::max(maxValue, 0.0f), 1.0f);
    
    if (_listener) {
        _listener->onSliderValueChanged(this, _minValue, _maxValue);
    }
    if (onSliderMinMaxValueChange) {
        onSliderMinMaxValueChange(this, _minValue, _maxValue);
    }
    
    setKnobPosition(_minValue, _maxValue, immediate);
}

void SMSlider::updateKnob()
{
    registerUpdate(FLAG_SLIDE_VALUE);
}

void SMSlider::updateLayout()
{
    _sliderWidth = _contentSize.width - 100;
    
    switch (_type) {
        case Type::MINUS_ONE_TO_ONE:
        {
            _leftLine->setContentSize(cocos2d::Size(_sliderWidth/2-(CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5), ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
            _leftLine->setPosition(cocos2d::Vec2(50, _contentSize.height/2));
            
            _rightLine->setContentSize(cocos2d::Size(_sliderWidth/2-(CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5), ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
            _rightLine->setPosition(_contentSize.width-50, _contentSize.height/2);
            
            _circle->setPosition(_contentSize/2);
        }
            break;
        case Type::ZERO_TO_ONE:
        {
            _rightLine->setContentSize(cocos2d::Size(_sliderWidth-(CENTER_RADIUS-1.5), ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
            _rightLine->setPosition(_contentSize.width-50, _contentSize.height/2);
            
            _circle->setPosition(50, _contentSize.height/2);
        }
            break;
        case Type::MIN_TO_MAX:
        {
            _rightLine->setContentSize(cocos2d::Size(_sliderWidth, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
            _rightLine->setPosition(_contentSize.width-50, _contentSize.height/2);
        }
            break;
    }
    
    if (_type==Type::MIN_TO_MAX) {
        setKnobPosition(_minValue, _maxValue, true);
    } else {
        setKnobPosition(_sliderValue, true);
    }
}

void SMSlider::setKnobPosition(const float sliderValue, const bool immediate)
{
    CCASSERT(_type!=Type::MIN_TO_MAX, "type must be minusToOne or zeroToOne");
    
    float x;
    if (_type==Type::MINUS_ONE_TO_ONE) {
        x = sliderValue * _sliderWidth / 2;
        _knobButton->setPosition(_contentSize.width/2 + x, _contentSize.height/2, sliderValue==0?true:immediate);
    } else {
        x = sliderValue * _sliderWidth;
        _knobButton->setPosition(50+x, _contentSize.height/2, sliderValue==0?true:immediate);
    }
    
    registerUpdate(FLAG_SLIDE_VALUE);
}

void SMSlider::setKnobPosition(const float minValue, const float maxValue, const bool immediate)
{
    CCASSERT(_type==Type::MIN_TO_MAX, "type must be minToMax");
    
    float minX = minValue * _sliderWidth;
    float maxX = maxValue * _sliderWidth;
    
    _minButton->setPosition(50+minX, _contentSize.height/2, minValue==0?true:immediate);
    _maxButton->setPosition(50+maxX, _contentSize.height/2, maxValue==0?true:immediate);
    
    registerUpdate(FLAG_SLIDE_VALUE);
}

void SMSlider::onUpdateOnVisit()
{
    if (isUpdate(FLAG_CONTENT_SIZE)) {
        unregisterUpdate(FLAG_CONTENT_SIZE);
        updateLayout();
    }
    
    if (isUpdate(FLAG_SLIDE_VALUE)) {
        unregisterUpdate(FLAG_SLIDE_VALUE);

        switch (_type) {
            case Type::MINUS_ONE_TO_ONE:
            {
                float x = (_knobButton->getPosition().x) - _contentSize.width/2;
                if (x>0) {
                    float len = x - (CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5);
                    if (len<0) {
                        len = 0;
                    }
                    
                    _bgLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
                    _bgLine->setContentSize(cocos2d::Size(len, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
                    _bgLine->setPosition(_contentSize.width/2 + (CENTER_RADIUS-1.5), _contentSize.height/2);
                } else {
                    float len = -x - (CENTER_RADIUS-1.5);
                    if (len<0) {
                        len = 0;
                    }
                    
                    _bgLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
                    _bgLine->setContentSize(cocos2d::Size(len, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
                    _bgLine->setPosition(_contentSize.width/2 - (CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5), _contentSize.height/2);
                }
            }
                break;
            case Type::ZERO_TO_ONE:
            {
                float x =(_knobButton->getPosition().x - 50);
                
                float len = x - (CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5);
                if (len<0) {
                    len = 0;
                }
                
                _bgLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
                _bgLine->setContentSize(cocos2d::Size(len, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
                _bgLine->setPosition(50 + (CENTER_RADIUS-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*1.5), _contentSize.height/2);
            }
                break;
            case Type::MIN_TO_MAX:
            {
                float x = _minButton->getPosition().x - 50;
                float len = _maxButton->getPosition().x - x - 50 - (CENTER_RADIUS-1.5)*ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2;
                
                if (len<0) {
                    len = 0;
                }
                
                _bgLine->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
                _bgLine->setContentSize(cocos2d::Size(len, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
                _bgLine->setPosition(_minButton->getPosition().x, _contentSize.height/2);
            }
                break;
        }
    }
}

int SMSlider::onTouch(SMView *view, const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            if (!getActionByTag(SMViewConstValue::Tag::USER+2)) {
                if (_type==Type::MIN_TO_MAX) {
                    if (view==_minButton) {
                        _minPoint = *point;
                        _minFocused = true;
                    } else {
                        _maxPoint = *point;
                        _maxFocused = true;
                    }
                } else {
                    _knobPoint = *point;
                    _knobFocused = true;
                }
            }
        }
            break;
        case MotionEvent::ACTION_MOVE:
        {
            if (view==_knobButton) {
                if (_knobFocused) {
                    float value;
                    if (_type==Type::MINUS_ONE_TO_ONE) {
                        auto pt = view->getPosition()-_contentSize/2 + (*point) - _knobPoint;
                        float dist = pt.x;
                        
                        if (dist>_sliderWidth/2) {
                            dist = _sliderWidth/2;
                        } else if (dist<-_sliderWidth/2) {
                            dist = -_sliderWidth/2;
                        }
                        
                        value = dist / (_sliderWidth/2);
                    } else {
                        auto pt = view->getPosition() + (*point) - _knobPoint;
                        float dist = pt.x - 50;
                        
                        if (dist<0) {
                            dist = 0;
                        } else if (dist>_sliderWidth) {
                            dist = _sliderWidth;
                        }
                        
                        value = dist / _sliderWidth;
                    }
                    
                    if (std::abs(value) < 0.015) {
                        value = 0;
                    }
                    
                    setSliderValue(value, false);
                }
                return TOUCH_INTERCEPT;
            } else if (view==_minButton) {
                if (_minFocused) {
                    float value;
                    auto pt = view->getPosition() + (*point) - _minPoint;
                    float dist = pt.x - 50;
                    if (dist<0) {
                        dist = 0;
                    } else if (dist>_maxButton->getPosition().x-110) {
                        dist = _maxButton->getPosition().x-110;
                    }
                    
                    value = dist / _sliderWidth;
                    
                    if (std::abs(value) < 0.015) {
                        value = 0;
                    }
                    
                    setSliderValue(value, _maxValue, false);
                }
                return TOUCH_INTERCEPT;
            } else if (view==_maxButton) {
                if (_maxFocused) {
                    float value;
                    auto pt = view->getPosition() + (*point) - _maxPoint;
                    float dist = pt.x - 50;
                    if (dist<_minButton->getPosition().x+10) {
                        dist = _minButton->getPosition().x+10;
                    } else if (dist>_sliderWidth) {
                        dist = _sliderWidth;
                    }
                    
                    value = dist / _sliderWidth;
                    
                    setSliderValue(_minValue, value, false);
                }
                return TOUCH_INTERCEPT;
            }
        }
            break;
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            if (_type==Type::MIN_TO_MAX) {
                _minFocused = false;
                _maxFocused = false;
            } else {
                _knobFocused = false;
            }
            break;
    }
    
    return TOUCH_FALSE;
}

int SMSlider::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    return TOUCH_TRUE;
}
