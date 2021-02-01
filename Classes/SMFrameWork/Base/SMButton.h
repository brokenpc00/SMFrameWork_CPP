//
//  SMButton.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 4..
//
//

#ifndef SMButton_h
#define SMButton_h

#include "SMView.h"
#include <2d/CCLabel.h>

class ShapeSolidRect;   // for under line

class SMButton : public _UIContainerView
{
public:
    SMButton();
    virtual ~SMButton();

    // button style
    enum class Style {
        DEFAULT, // image and text without solid-color and out-line
        RECT, // only out-line Rectangle
        ROUNDEDRECT, // only out-line Rounded Rect
        CIRCLE, // only out-line Circle
        SOLID_RECT, // has solid-color and out-line color
        SOLID_ROUNDEDRECT,   // has solid-color and out-line color
        SOLID_CIRCLE,   // has solid-color and out-line color
    };
    
    // icon align
    enum class Align {
        CENTER,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };
    
public:
    // creator
    // basic
    static SMButton * create(int tag=0, Style style=Style::SOLID_RECT) {
        SMButton * buton = new (std::nothrow)SMButton();
        if (buton && buton->initWithStyle(style)) {
            buton->setTag(tag);
            buton->autorelease();
        } else {
            CC_SAFE_DELETE(buton);
        }
        return buton;
    };
    
    // specific
    static SMButton * create(int tag, Style style, float x, float y, float width, float height, float anchorX=0.0f, float anchorY=0.0f) {
        SMButton * button = SMButton::create(tag, style);
        if (button != nullptr) {
            button->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
            button->setPosition(cocos2d::Vec2(x, y));
            button->setContentSize(cocos2d::Size(width, height));
        }
        return button;
    }
    
public:
    // public method
    virtual bool isTouchEnable() const override;
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    cocos2d::Label * getTextLabel();
    
    // push state added effect
    void setPushDownOffset(const cocos2d::Vec2& offset);
    void setPushDownScale(const float scale);

    // set color
    void setButtonColor(const State state, const cocos2d::Color4F& color);
    void setTextColor(const State state, const cocos2d::Color4F& color);
    void setIconColor(const State state, const cocos2d::Color4F& color);

    void setOutlineWidth(float lineWidth);
    void setOutlineColor(const State state, const cocos2d::Color4F& color);
    
    void setButton(const State state, cocos2d::Node * node);
    void setButton(const State state, cocos2d::SpriteFrame * frame, bool enableScale9);
    void setButton(const State state, const std::string& imageFileName, bool enableScale9);
    
    void setIcon(const State state, cocos2d::Node * node);
    void setIcon(const State state, cocos2d::SpriteFrame * frame);
    void setIcon(const State state, const std::string& imageFileName);
    
    void setText(const std::string& text);
    void setTextTTF(const std::string& text, const std::string& fontFileName, float fontSize);
    void setTextTTF(const std::string& text, const cocos2d::TTFConfig& ttfConfig);
    void setTextSystemFont(const std::string& text, float fontSize);
    void setTextSystemFont(const std::string& text, const std::string& fontName, float fontSize);
    void setUnderline();
    
    void setIconPadding(float padding);
    void setIconScale(float scale);
    void setTextScale(float scale);
    void setIconOffset(cocos2d::Vec2& offset);
    void setTextOffset(cocos2d::Vec2& offset);
    
    
    // for circle, rounded-rect
    void setShapeCornerRadius(float radius);
    void setShapeCornerQuadrant(int quadrant);
    void setShapeAntiAliasWidth(float width);
    void setShapeLineWidth(float width);
    
    void setIconAlign(Align align=Align::CENTER);
    
    cocos2d::Color4F* getButtonColor(const State state);
    cocos2d::Color4F* getIconColor(const State state);
    cocos2d::Color4F* getTextColor(const State state);
    cocos2d::Color4F* getOutlineColor(const State state);
    
    cocos2d::Node* getButtonNode(const State state);
    cocos2d::Node* getIconNode(const State state);
    
protected:
    // initialize
    virtual bool initWithStyle(const Style style);
    virtual void onStateChangePressToNormal() override;
    virtual void onStateChangeNormalToPress() override;
    virtual void onUpdateStateTransition(State toState, float t);
    virtual void onUpdateOnVisit() override;
    
private:
    class _StateTransitionAction;
    class _ReleaseActionStarter;
    
    Style _style;
    Align _align;
    cocos2d::Label * _textLabel;
    cocos2d::Color4F** _textColor;
    cocos2d::Color4F** _iconColor;
    cocos2d::Color4F** _buttonColor;
    cocos2d::Color4F** _outlineColor;
    cocos2d::Node** _buttonNode;
    cocos2d::Node** _iconNode;
    cocos2d::Vec2 _pushDownOffset;
    ShapeSolidRect * _textUnderline;
    float _pushDownScale;
    float _iconScale;
    float _textScale;
    float _iconPadding;
    cocos2d::Vec2 _iconOffset;
    cocos2d::Vec2 _textOffset;
    float _shapeRadius;
    int _shapeQuadrant;
    float _shapeLineWidth;
    float _shapeAntiAliasWidth;
    float _shapeOutlineWidth;
    
    _StateTransitionAction* _buttonPressAction;
    _StateTransitionAction* _buttonReleaseAction;
    
    float _buttonPressActionTime;
    float _buttonReleaseActionTime;
    float _buttonStateValue;
    
    void setStateColor(cocos2d::Color4F*** target, State state, const cocos2d::Color4F& color);
    void setStateNode(cocos2d::Node*** target, State state, cocos2d::Node* node, const int localZOrder, cocos2d::Color4F*** targetColor);
    void colorChangeSrcNodeToDstNode(cocos2d::Node* srcNode, cocos2d::Node* dstNode, cocos2d::Color4F* srcColor, cocos2d::Color4F* dstColor, float t);

private:
    DISALLOW_COPY_AND_ASSIGN(SMButton);
};


#endif /* SMButton_h */
