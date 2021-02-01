//
//  SMButton.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 4..
//
//

#include "SMButton.h"
#include "ShaderNode.h"
#include <ui/CocosGUI.h>
#include "../Util/ViewUtil.h"
#include <2d/CCActionInterval.h>
#include <2d/CCActionInstant.h>
#include <2d/CCSprite.h>
#include <2d/CCSpriteFrame.h>
#include <cocos2d.h>

#define FLAG_CONTENT_SIZE   (1<<0)
#define FLAG_BUTTON_COLOR   (1<<1)
#define FLAG_ICON_COLOR     (1<<2)
#define FLAG_TEXT_COLOR     (1<<3)
#define FLAG_OUTLINE_COLOR  (1<<4)
#define FLAG_TEXT_ICON_POSITION (1<<5)
#define FLAG_SHAPE_STYLE   (1<<6)


// button state change action class

class SMButton::_StateTransitionAction : public cocos2d::ActionInterval
{
public:
    static _StateTransitionAction * create(SMView::State toState) {
        auto action = new (std::nothrow)_StateTransitionAction();
        if (action!=nullptr && action->initWithDuration(0)) {
            action->_toState = toState;
        }
        return action;
    }
    
    virtual void startWithTarget(cocos2d::Node * target) override {
        ActionInterval::startWithTarget(target);
        int tag;
        if (_toState == SMView::State::PRESSED) {
            tag = SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_PRESS_TO_NORMAL;
        } else {
            tag = SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS;
        }
        cocos2d::ActionInterval * action = (cocos2d::ActionInterval*)target->getActionByTag(tag);
        
        if (action!=nullptr) {
            target->stopAction(action);
            float run = action->getElapsed() / action->getDuration();
            if (run<1) {
                _firstTick = false;
                _elapsed = getDuration() * (1-run);
            }
        }
    }
    
    virtual void update(float t) override {
        ((SMButton*)_target)->onUpdateStateTransition(_toState, _toState==SMView::State::PRESSED?t:1-t);
    }
    
private:
    _StateTransitionAction(){};
    virtual ~_StateTransitionAction(){};
    SMView::State _toState;
    
    CC_DISALLOW_COPY_AND_ASSIGN(_StateTransitionAction);
};

// release action starter

class SMButton::_ReleaseActionStarter : public cocos2d::ActionInstant {
public:
    static _ReleaseActionStarter * create() {
        _ReleaseActionStarter * action = new (std::nothrow)_ReleaseActionStarter();
        if (action) {
            action->autorelease();
        }
        
        return action;
    }
    
    virtual void update(float t) override {
        SMButton * btn =(SMButton*)_target;
        btn->runAction(btn->_buttonReleaseAction);
    }
    
private:
    _ReleaseActionStarter(){};
    virtual ~_ReleaseActionStarter(){};
    
    CC_DISALLOW_COPY_AND_ASSIGN(_ReleaseActionStarter);
};


SMButton::SMButton() :

_align(Align::CENTER),

_iconScale(1.0f),
_textScale(1.0f),

_iconPadding(0.0f),
_iconOffset(cocos2d::Vec2::ZERO),
_textOffset(cocos2d::Vec2::ZERO),

_pushDownOffset(cocos2d::Vec2::ZERO),
_pushDownScale(1.0f),

_buttonNode(nullptr),
_iconNode(nullptr),
_textLabel(nullptr),

_buttonColor(nullptr),
_iconColor(nullptr),
_textColor(nullptr),
_outlineColor(nullptr),
_textUnderline(nullptr),

_buttonPressAction(nullptr),
_buttonReleaseAction(nullptr),
_buttonPressActionTime(SMViewConstValue::Config::BUTTON_STATE_CHANGE_NORMAL_TO_PRESS_TIME),
_buttonReleaseActionTime(SMViewConstValue::Config::BUTTON_STATE_CHANGE_PRESS_TO_NORMAL_TIME),

_buttonStateValue(0.0f),
_shapeOutlineWidth(0.0f)
{
    
}

SMButton::~SMButton()
{
    // release action
    if (_buttonPressAction != nullptr) {
        _buttonPressAction->release();
    }
    
    if (_buttonReleaseAction != nullptr) {
        _buttonReleaseAction->release();
    }
    
    
    // release state obj (textcolor, iconcolor, buttoncolor, buttonnode, iconnode)
    int numState = stateToInt(State::MAX_STATE);
    
    if (_textColor!=nullptr) {
        for (int i=0; i<numState; i++) {
            if (_textColor[i]!=nullptr) {
                delete _textColor[i];
            }
        }
        CC_SAFE_DELETE_ARRAY(_textColor);
    }
    
    if (_iconColor!=nullptr) {
        for (int i=0; i<numState; i++) {
            if (_iconColor[i]!=nullptr) {
                delete _iconColor[i];
            }
        }
        CC_SAFE_DELETE_ARRAY(_iconColor);
    }
    
    if (_buttonColor!=nullptr) {
        for (int i=0; i<numState; i++) {
            if (_buttonColor[i]!=nullptr) {
                delete _buttonColor[i];
            }
        }
        CC_SAFE_DELETE_ARRAY(_buttonColor);
    }
    
    if (_outlineColor != nullptr) {
        for (int i = 0; i < numState; i++) {
            if (_outlineColor[i] != nullptr) {
                delete _outlineColor[i];
            }
        }
        CC_SAFE_DELETE_ARRAY(_outlineColor);
    }
    
    if (_buttonNode!=nullptr) {
        CC_SAFE_DELETE_ARRAY(_buttonNode);
    }
    
    if (_iconNode!=nullptr) {
        CC_SAFE_DELETE_ARRAY(_iconNode);
    }
}

bool SMButton::initWithStyle(const SMButton::Style style)
{
    _style = style;
    
    ShapeNode * buttonNode = nullptr;
    
    switch (_style) {
        case Style::RECT :
        {
            buttonNode = ShapeRect::create();
        }
            break;
        case Style::ROUNDEDRECT :
        {
            buttonNode = ShapeRoundedRect::create();
        }
            break;
        case Style::CIRCLE :
        {
            buttonNode = ShapeCircle::create();
        }
            break;
        case Style::SOLID_RECT :
        {
            buttonNode = ShapeSolidRect::create();
        }
            break;
        case Style::SOLID_ROUNDEDRECT :
        {
            buttonNode = ShapeSolidRoundRect::create();
        }
            break;
        case Style::SOLID_CIRCLE :
        {
            buttonNode = ShapeSolidCircle::create();
        }
            break;
        default:    //Style::DEFAULT
        {
            
        }
            break;
    }
    
    if (buttonNode!=nullptr) {  // not Style::DEFAULT
        _shapeRadius = buttonNode->getCornerRadius();
        _shapeLineWidth = buttonNode->getLineWidth();
        _shapeAntiAliasWidth = buttonNode->getAntiAliasWidth();
        _shapeQuadrant = ShapeNode::Quadrant::ALL;
        
        // default 1 node
        setButton(State::NORMAL, buttonNode);
        // default white background color
        setButtonColor(State::NORMAL, cocos2d::Color4F::WHITE);
    }
    return true;
}

void SMButton::onUpdateOnVisit()
{
    if (isUpdate(FLAG_CONTENT_SIZE)) {
        registerUpdate(FLAG_TEXT_ICON_POSITION);
        
        if (_buttonNode!=nullptr) {
            cocos2d::Size size = _uiContainer->getContentSize();
            cocos2d::Vec2 center = size / 2.0f;
            
            for (int i=0; i<2; i++) {   // normal, press 2 state
                cocos2d::Node* node = _buttonNode[i];
                if (node!=nullptr) {
                    node->setPosition(center);
                    if (_style==Style::DEFAULT) {
                        cocos2d::ui::Scale9Sprite * sprite = dynamic_cast<cocos2d::ui::Scale9Sprite*>(node);
                        if (sprite!=nullptr && sprite->isScale9Enabled()) {
                            sprite->setContentSize(size);
                        }
                    } else {    // Shape button
                        node->setContentSize(size);
                    }
                }
            }
        }
        
        unregisterUpdate(FLAG_CONTENT_SIZE);
    }
    
    if (isUpdate(FLAG_TEXT_ICON_POSITION)) {
        bool isContainedText = false;
        cocos2d::Size textSize = cocos2d::Size::ZERO;
        if (_textLabel!=nullptr) {
            textSize = _textLabel->getContentSize() * _textScale;
            isContainedText = true;
        }
        
        bool isContainedIcon = false;
        cocos2d::Size iconSize = cocos2d::Size::ZERO;
        if (_iconNode!=nullptr) {
            for (int i=0; i<2; i++) {
                if (_iconNode[i]!=nullptr) {
                    cocos2d::Size s = _iconNode[i]->getContentSize() * _iconScale;
                    iconSize = cocos2d::Size(std::max(iconSize.width, s.width), std::max(iconSize.height, s.height));
                    isContainedIcon = true;
                }
            }
        }
        
        cocos2d::Size size = _uiContainer->getContentSize();
        cocos2d::Vec2 center = size/2.0f;
        cocos2d::Vec2 textPosition = center;
        cocos2d::Vec2 iconPosition = center;
        float width=0.0f, height=0.0f;
        
        if (isContainedText && isContainedIcon) {
            // align은 text와 icon이 둘다 있을때만 의미가 있다.
            // icon의 상대적 align을 setting한다.
            switch (_align) {
                case Align::LEFT:
                {
                    width = textSize.width + _iconPadding + iconSize.width;
                    iconPosition.x= (size.width - width + iconSize.width)/2;
                    textPosition.x = (size.width + width - textSize.width)/2;
                }
                    break;
                case Align::RIGHT:
                {
                    width = textSize.width + _iconPadding + iconSize.width;
                    iconPosition.x = (size.width + width - iconSize.width)/2;
                    textPosition.x = (size.width - width + textSize.width)/2;
                }
                    break;
                case Align::TOP:
                {
                    height = textSize.height + _iconPadding + iconSize.height;
                    iconPosition.y = (size.height + height - iconSize.height)/2;
                    textPosition.y = (size.height - height + textSize.height)/2;
                }
                    break;
                case Align::BOTTOM:
                {
                    height = textSize.height + _iconPadding + iconSize.height;
                    iconPosition.y = (size.height - height + iconSize.height)/2;
                    textPosition.y = (size.height + height - textSize.height)/2;
                }
                    break;
                default:
                {
                    // center... nothing to do.
                }
                    break;
            }
        }
        
        if (isContainedText) {
            _textLabel->setPosition(textPosition+_textOffset);
            _textLabel->setScale(_textScale);
            if (_textUnderline) {
                _textUnderline->setPosition(textPosition+_textOffset-cocos2d::Size(0, textSize.height/2));
                _textUnderline->setContentSize(cocos2d::Size(textSize.width, ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2));
            }
        }
        
        if (isContainedIcon) {
            for (int i=0; i<2; i++) {
                if (_iconNode[i]!=nullptr) {
                    _iconNode[i]->setPosition(iconPosition+_iconOffset);
                    _iconNode[i]->setScale(_iconScale);
                }
            }
        }
        
        unregisterUpdate(FLAG_TEXT_ICON_POSITION);
    }
    
    if (isUpdate(FLAG_BUTTON_COLOR | FLAG_ICON_COLOR | FLAG_TEXT_COLOR | FLAG_OUTLINE_COLOR)) {
        unregisterUpdate(FLAG_BUTTON_COLOR | FLAG_ICON_COLOR | FLAG_TEXT_COLOR | FLAG_OUTLINE_COLOR);
    }
    
    if (isUpdate(FLAG_SHAPE_STYLE)) {
        if (_style!=Style::DEFAULT && _buttonNode!=nullptr) {
            for (int i=0; i<2; i++) {
                cocos2d::Node * node = _buttonNode[i];
                if (node != nullptr) {
                    ShapeNode * shape = dynamic_cast<ShapeNode*>(node);
                    if (shape!=nullptr) {
                        shape->setCornerRadius(_shapeRadius);
                        shape->setAntiAliasWidth(_shapeAntiAliasWidth);
                        if (i==0) {
                            shape->setLineWidth(_shapeLineWidth);
                        } else {
                            shape->setLineWidth(_shapeOutlineWidth);
                        }
                        shape->setCornerQuadrant((ShapeNode::Quadrant)_shapeQuadrant);
                    }
                }
            }
        }
        
        unregisterUpdate(FLAG_SHAPE_STYLE);
    }
    
    onUpdateStateTransition(State::NORMAL, _buttonStateValue);
}

bool SMButton::isTouchEnable() const
{
    return true;
}

cocos2d::Color4F* SMButton::getButtonColor(const State state)
{
    return _buttonColor[stateToInt(state)];
}

cocos2d::Color4F* SMButton::getIconColor(const State state)
{
    return _iconColor[stateToInt(state)];
}

cocos2d::Color4F* SMButton::getTextColor(const State state)
{
    return _textColor[stateToInt(state)];
}

cocos2d::Color4F* SMButton::getOutlineColor(const State state)
{
    return _outlineColor[stateToInt(state)];
}

cocos2d::Node* SMButton::getButtonNode(const State state)
{
    if (_buttonNode==nullptr) {
        return nullptr;
    }
    return _buttonNode[stateToInt(state)];
}

cocos2d::Node* SMButton::getIconNode(const State state)
{
    if (_iconNode==nullptr) {
        return nullptr;
    }
    return _iconNode[stateToInt(state)];
}

cocos2d::Label * SMButton::getTextLabel()
{
    return _textLabel;
}

void SMButton::setPushDownOffset(const cocos2d::Vec2 &offset)
{
    _pushDownOffset = offset;
}

void SMButton::setPushDownScale(const float scale)
{
    _pushDownScale = scale;
}

void SMButton::setStateColor(cocos2d::Color4F ***target, State state, const cocos2d::Color4F &color)
{
    if (*target==nullptr) {
        // new create
        *target = new cocos2d::Color4F*[stateToInt(State::MAX_STATE)]{nullptr, };
    }
    
    if ((*target)[stateToInt(state)]!=nullptr) {
        *((*target)[stateToInt(state)]) = color;
    } else {
        (*target)[stateToInt(state)] = new cocos2d::Color4F(color);
    }
}

void SMButton::setStateNode(cocos2d::Node ***target, State state, cocos2d::Node *node, const int localZOrder, cocos2d::Color4F ***targetColor)
{
    if (*target==nullptr && node!=nullptr) {
        // new create node
        *target = new cocos2d::Node*[stateToInt(State::MAX_STATE)] {nullptr, };
        
        if (*targetColor==nullptr) {
            // new create color
            *targetColor = new cocos2d::Color4F*[stateToInt(State::MAX_STATE)]{nullptr, };
        }
    }
    
    cocos2d::Node * currentStateNode = (*target)[stateToInt(state)];
    
    if (currentStateNode!=node) {
        if (currentStateNode!=nullptr) {
            // remove previous state node
            _uiContainer->removeChild(currentStateNode);
        }
        if (node!=nullptr) {
            // change new state node
            _uiContainer->addChild(node, localZOrder);
        }
    }
    
    (*target)[stateToInt(state)] = node;
}

void SMButton::setContentSize(const cocos2d::Size &size)
{
    _UIContainerView::setContentSize(size);
    registerUpdate(FLAG_CONTENT_SIZE);
}


// set color & update
void SMButton::setButtonColor(const State state, const cocos2d::Color4F &color)
{
    setStateColor(&_buttonColor, state, color);
    registerUpdate(FLAG_BUTTON_COLOR);
}

void SMButton::setIconColor(const State state, const cocos2d::Color4F &color)
{
    setStateColor(&_iconColor, state, color);
    registerUpdate(FLAG_ICON_COLOR);
}

void SMButton::setTextColor(const State state, const cocos2d::Color4F &color)
{
    setStateColor(&_textColor, state, color);
    registerUpdate(FLAG_TEXT_COLOR);
}

void SMButton::setOutlineColor(const State state, const cocos2d::Color4F &color)
{
    setStateColor(&_outlineColor, state, color);
    registerUpdate(FLAG_OUTLINE_COLOR);
}


void SMButton::setButton(const State state, cocos2d::Node *node)
{
    setStateNode(&_buttonNode, state, node, state==State::NORMAL?SMViewConstValue::ZOrder::BUTTON_NORMAL:SMViewConstValue::ZOrder::BUTTON_PRESSED, &_buttonColor);
    
    if (node!=nullptr) {
        node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        registerUpdate(FLAG_CONTENT_SIZE);
        registerUpdate(FLAG_BUTTON_COLOR);
    }
}

void SMButton::setButton(const State state, cocos2d::SpriteFrame *frame, bool enableScale9)
{
    CC_ASSERT(_style==Style::DEFAULT);  // only default, do not shapes button
    
    cocos2d::ui::Scale9Sprite * sprite = cocos2d::ui::Scale9Sprite::create();
    if (sprite==nullptr) {
        return;
    }
    
    sprite->setScale9Enabled(enableScale9);
    sprite->initWithSpriteFrame(frame);
    
    setButton(state, sprite);
}

void SMButton::setButton(const State state, const std::string &imageFileName, bool enableScale9)
{
    CC_ASSERT(_style==Style::DEFAULT); // only default, do not shapes button
    
    if (imageFileName.empty()) {
        return;
    }
    
    cocos2d::ui::Scale9Sprite * sprite = cocos2d::ui::Scale9Sprite::create();
    if (sprite==nullptr) {
        return;
    }
    
    sprite->setScale9Enabled(enableScale9);
    sprite->initWithFile(imageFileName);
    
    setButton(state, sprite);
}

void SMButton::setOutlineWidth(float lineWidth)
{
    CCASSERT(_style!=Style::DEFAULT, "Shapes button is not Default style button");  // only shapes button
    
    if (lineWidth == _shapeOutlineWidth) {  // already setting
        return;
    }
    
    if (lineWidth>0) {
        if (_buttonNode!=nullptr && _buttonNode[1]==nullptr) {
            // press state not setting
            ShapeNode * outlineNode = nullptr;
            
            switch (_style) {
                case Style::RECT:
                case Style::SOLID_RECT:
                {
                    outlineNode = ShapeRect::create();
                }
                    break;
                case Style::CIRCLE:
                case Style::SOLID_CIRCLE:
                {
                    outlineNode = ShapeCircle::create();
                }
                    break;
                case Style::ROUNDEDRECT:
                case Style::SOLID_ROUNDEDRECT:
                {
                    outlineNode = ShapeRoundedRect::create();
                }
                    break;
                    
                default:
                    break;
            }
            
            if (outlineNode!=nullptr) {
                // set outline node on button pressed state
                setStateNode(&_buttonNode, State::PRESSED, outlineNode, SMViewConstValue::ZOrder::BUTTON_PRESSED, &_outlineColor);
                outlineNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                registerUpdate(FLAG_CONTENT_SIZE);
                
                if (_outlineColor==nullptr || _outlineColor[0]==nullptr) {
                    // outline default color
                    setOutlineColor(State::NORMAL, cocos2d::Color4F::BLACK);
                }
            }
        }
    } else {
        setButton(State::PRESSED, nullptr);
    }
    
    _shapeOutlineWidth = lineWidth;
    registerUpdate(FLAG_SHAPE_STYLE);
}


// icon
void SMButton::setIconAlign(Align align)
{
    _align = align;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setIcon(State state, cocos2d::Node *node)
{
    setStateNode(&_iconNode, state, node, state==State::NORMAL?SMViewConstValue::ZOrder::BUTTON_ICON_NORMAL:SMViewConstValue::ZOrder::BUTTON_ICON_PRESSED, &_iconColor);
    if (node!=nullptr) {
        node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    }
    
    registerUpdate(FLAG_TEXT_ICON_POSITION);
    registerUpdate(FLAG_ICON_COLOR);
}

void SMButton::setIcon(State state, cocos2d::SpriteFrame *frame)
{
    cocos2d::Sprite * sprite = cocos2d::Sprite::create();
    if (sprite==nullptr) {
        return;
    }
    
    sprite->initWithSpriteFrame(frame);
    setIcon(state, sprite);
    
//    registerUpdate(FLAG_TEXT_ICON_POSITION);
//    registerUpdate(FLAG_ICON_COLOR);
}

void SMButton::setIcon(State state, const std::string &imageFileName)
{
    if (imageFileName.empty()) {
        return;
    }
    
    cocos2d::Sprite* sprite = cocos2d::Sprite::create();
    if (sprite==nullptr) {
        return;
    }
    
    sprite->initWithFile(imageFileName);
    setIcon(state, sprite);
}

// text
void SMButton::setText(const std::string &text)
{
    if (_textLabel==nullptr) {
        setTextSystemFont(text, SMViewConstValue::Config::DEFAULT_FONT_SIZE);
    } else {
        if (_textLabel->getString().compare(text)!=0) {
            _textLabel->setString(text);
        }
    }
    
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setTextTTF(const std::string &text, const std::string &fontFileName, float fontSize)
{
    cocos2d::TTFConfig ttfConfig(fontFileName.c_str(), fontSize);
    setTextTTF(text, ttfConfig);
}

void SMButton::setTextTTF(const std::string &text, const cocos2d::TTFConfig &ttfConfig)
{
    if (_textLabel==nullptr) {
        _textLabel = cocos2d::Label::createWithTTF(ttfConfig, text, cocos2d::TextHAlignment::CENTER);
        _textLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        _uiContainer->addChild(_textLabel, SMViewConstValue::ZOrder::BUTTON_TEXT);
        if (_textColor==nullptr || _textColor[0]==nullptr) {
            // default color black
            setTextColor(State::NORMAL, cocos2d::Color4F::BLACK);
        }
        
        registerUpdate(FLAG_TEXT_COLOR);
    } else {
        _textLabel->setTTFConfig(ttfConfig);
        _textLabel->setString(text);
    }
    
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setTextSystemFont(const std::string &text, float fontSize)
{
    setTextSystemFont(text, "", fontSize);
}

void SMButton::setTextSystemFont(const std::string &text, const std::string &fontName, float fontSize)
{
    if (_textLabel==nullptr) {
        _textLabel = cocos2d::Label::createWithSystemFont(text, fontName, fontSize, cocos2d::Size::ZERO, cocos2d::TextHAlignment::CENTER, cocos2d::TextVAlignment::CENTER);
        _textLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        _uiContainer->addChild(_textLabel, SMViewConstValue::ZOrder::BUTTON_TEXT);
        if (_textColor==nullptr || _textColor[0]==nullptr) {
            // default color black
            setTextColor(State::NORMAL, cocos2d::Color4F::BLACK);
        }
        
        registerUpdate(FLAG_TEXT_COLOR);
    } else {
        _textLabel->setSystemFontName(fontName);
        _textLabel->setSystemFontSize(fontSize);
        _textLabel->setString(text);
    }
    
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}


// underline
void SMButton::setUnderline()
{
    _textUnderline = ShapeSolidRect::create();
    _textUnderline->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_textUnderline);
}



// button state (icon, text)
void SMButton::setIconPadding(float padding)
{
    _iconPadding = padding;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setIconScale(float scale)
{
    _iconScale = scale;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setTextScale(float scale)
{
    _textScale = scale;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setIconOffset(cocos2d::Vec2 &offset)
{
    _iconOffset = offset;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}

void SMButton::setTextOffset(cocos2d::Vec2 &offset)
{
    _textOffset = offset;
    registerUpdate(FLAG_TEXT_ICON_POSITION);
}


// button shapes style
void SMButton::setShapeCornerRadius(float radius)
{
    if (_style==Style::DEFAULT) {
        return;
    }
    _shapeRadius = radius;
    
    registerUpdate(FLAG_SHAPE_STYLE);
}

void SMButton::setShapeCornerQuadrant(int quadrant)
{
    if (_style==Style::DEFAULT) {
        return;
    }
    
    _shapeQuadrant = quadrant;
    
    registerUpdate(FLAG_SHAPE_STYLE);
}

void SMButton::setShapeAntiAliasWidth(float width)
{
    if (_style==Style::DEFAULT) {
        return;
    }
    
    _shapeAntiAliasWidth = width;
    
    registerUpdate(FLAG_SHAPE_STYLE);
}

void SMButton::setShapeLineWidth(float width)
{
    if (_style==Style::DEFAULT) {
        return;
    }
    
    _shapeLineWidth = width;
    
    registerUpdate(FLAG_SHAPE_STYLE);
}


// state change
void SMButton::onUpdateStateTransition(State toState, float t)
{
    _buttonStateValue = t;
    
    if (_buttonNode!=nullptr) {
        if (_style==Style::DEFAULT) {
            // there is no color and no outline color
            colorChangeSrcNodeToDstNode(_buttonNode[0], _buttonNode[1], _buttonColor[0], _buttonColor[1], t);
        } else {
            if (_buttonColor!=nullptr) {
                // button color
                colorChangeSrcNodeToDstNode(_buttonNode[0], nullptr, _buttonColor[0], _buttonColor[1], t);
            }
            if (_outlineColor!=nullptr) {
                // outline color
                colorChangeSrcNodeToDstNode(_buttonNode[1], nullptr, _outlineColor[0], _outlineColor[1], t);
            }
        }
    }
    
    if (_iconNode!=nullptr) {
        colorChangeSrcNodeToDstNode(_iconNode[0], _iconNode[1], _iconColor[0], _iconColor[1], t);
    }
    
    if (_textLabel!=nullptr) {
        colorChangeSrcNodeToDstNode(_textLabel, nullptr, _textColor[0], _textColor[1], t);
        if (_textUnderline) {
            // need to underline color??????
            colorChangeSrcNodeToDstNode(_textUnderline, nullptr, _textColor[0], _textColor[1], t);
        }
    }
}


void SMButton::onStateChangeNormalToPress()
{
    if (_pushDownOffset != cocos2d::Vec2::ZERO) {
        _uiContainer->setAnimOffset(_pushDownOffset);
    }
    
    if (_pushDownScale!=1.0f) {
        _uiContainer->setAnimScale(_pushDownScale);
    }
    
    if (_buttonPressAction==nullptr) {
        // not exist press state... new state
        _buttonPressAction = _StateTransitionAction::create(State::PRESSED);
        _buttonPressAction->setTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS);
    }
    
    // exist delay action state??
    if (getActionByTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_DELAY)) {
        // stop delay action state
        stopAction(getActionByTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_DELAY));
    }
    
    if (getActionByTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS)==nullptr) {
        // new press action duration time
        _buttonPressAction->setDuration(_buttonPressActionTime);
        runAction(_buttonPressAction);
    }
}

void SMButton::onStateChangePressToNormal()
{
    // recover button state
    _uiContainer->setAnimOffset(cocos2d::Vec2::ZERO);
    _uiContainer->setAnimScale(1.0f);
    
    if (_buttonReleaseAction==nullptr) {
        // not exist normal state... new state
        _buttonReleaseAction = _StateTransitionAction::create(State::NORMAL);
        _buttonReleaseAction->setTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_PRESS_TO_NORMAL);
    }
    
    _StateTransitionAction *action = (_StateTransitionAction*)getActionByTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS);
    if (action!=nullptr) {
        float minTime = action->getDuration() * 0.3f - action->getElapsed();
        if (minTime>0) {
            _buttonReleaseAction->setDuration(_buttonReleaseActionTime);
            // create delay
            auto * sequence = cocos2d::Sequence::createWithTwoActions(cocos2d::DelayTime::create(minTime), _ReleaseActionStarter::create());
            sequence->setTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_DELAY);
            runAction(sequence);
            return;
        }
    }
    
    if (getActionByTag(SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_PRESS_TO_NORMAL)==nullptr) {
        // new normal action duration time
        _buttonReleaseAction->setDuration(_buttonReleaseActionTime);
        runAction(_buttonReleaseAction);
    }
}

void SMButton::colorChangeSrcNodeToDstNode(cocos2d::Node *srcNode, cocos2d::Node *dstNode, cocos2d::Color4F *srcColor, cocos2d::Color4F *dstColor, float t)
{
    float srcAlpha = 1.0f - t;
    float dstAlpha = t;
    
    if (dstNode!=nullptr) {
        if (srcColor) {
            srcNode->setColor(cocos2d::Color3B(*srcColor));
            srcAlpha *= srcColor->a;
        }
        
        if (dstColor) {
            dstNode->setColor(cocos2d::Color3B(*dstColor));
            dstAlpha *= dstColor->a;
        }
        
        if (srcNode) {
            GLubyte opacity = (GLubyte)(255.0f*srcAlpha);
            srcNode->setVisible(opacity>0);
            srcNode->setOpacity(opacity);
        }
        GLubyte opacity = (GLubyte)(255.0f*dstAlpha);
        dstNode->setVisible(opacity>0);
        dstNode->setOpacity(opacity);
    } else if (srcNode!=nullptr) { // dstNode==nullptr
        cocos2d::Vec4 sc = srcColor != nullptr ? ViewUtil::colorToVec4(*srcColor):cocos2d::Vec4::ONE;
        cocos2d::Vec4 ds = dstColor != nullptr ? ViewUtil::colorToVec4(*dstColor):(_style==Style::DEFAULT?cocos2d::Vec4::ONE:sc);
        cocos2d::Vec4 rc = sc*srcAlpha + ds*dstAlpha;
        srcNode->setColor(ViewUtil::vec4ToColor3B(rc));
        srcNode->setOpacity((GLubyte)(255.0f*rc.w));
    }
}
