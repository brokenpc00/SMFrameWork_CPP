//
//  SMSlider.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 10..
//
//

#ifndef SMSlider_h
#define SMSlider_h

#include "SMView.h"

class SMButton;
class SMSlider;
class ShapeSolidRect;
class ShapeCircle;

class OnSliderListener {
public:
    virtual void onSliderValueChanged(SMSlider* slider, float value) = 0;
    virtual void onSliderValueChanged(SMSlider* slider, float minValue, float maxValue) = 0;
};

class SMSlider : public SMView, public OnTouchListener {
public:
    
    struct InnerColor {
        cocos2d::Color4F bgLine;
        cocos2d::Color4F fgLine;
        cocos2d::Color4F knob;
        cocos2d::Color4F knobPress;
        
        InnerColor(const cocos2d::Color4F& bgLine, const cocos2d::Color4F& fgLine, const cocos2d::Color4F& knob, const cocos2d::Color4F& knobPress) {
            this->bgLine = bgLine;
            this->fgLine = fgLine;
            this->knob = knob;
            this->knobPress = knobPress;
        }
        
        static const InnerColor LIGHT;
        static const InnerColor DARK;
    };
    
    enum Type {
        MINUS_ONE_TO_ONE=0,
        ZERO_TO_ONE,
        MIN_TO_MAX
    };
    
    static SMSlider * create(const Type type = ZERO_TO_ONE, const InnerColor& initColor=InnerColor::LIGHT);

    void setSliderValue(const float sliderValue, const bool immediate=true);
    void setSliderValue(const float minValue, const float maxValue, const bool immeidate=true);
    
    float getSliderValue() {return _sliderValue;}
    float getMinValue() {return _minValue;}
    float getMaxValue() {return _maxValue;}
    
    void setOnSliderListener(OnSliderListener*l) {_listener = l;}
    
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    std::function<void(SMSlider* slider, float value)> onSliderValueChange;
    std::function<void(SMSlider* slider, float minValue, float maxValue)> onSliderMinMaxValueChange;
    
protected:
    SMSlider();
    virtual ~SMSlider();
    
    bool initWithType(const Type type, const InnerColor& initColor);
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    virtual void onUpdateOnVisit() override;
    void updateKnob();
    
private:
    void setKnobPosition(const float sliderValue, const bool immediate=true);
    void setKnobPosition(const float minValue, const float maxValue, const bool immeidate=true);
    
    void updateLayout();
    
private:
    class KnobButton;
    OnSliderListener* _listener;
    KnobButton* _knobButton;
    KnobButton* _minButton;
    KnobButton* _maxButton;
    
    cocos2d::Vec2 _knobPoint;
    cocos2d::Vec2 _minPoint;
    cocos2d::Vec2 _maxPoint;
    
    float _sliderValue;
    float _minValue;
    float _maxValue;
    
    float _sliderWidth;
    bool _knobFocused;
    bool _minFocused;
    bool _maxFocused;
    
    Type _type;
    
    ShapeSolidRect* _bgLine;
    ShapeSolidRect* _leftLine;
    ShapeSolidRect* _rightLine;
    ShapeCircle* _circle;
};


#endif /* SMSlider_h */
