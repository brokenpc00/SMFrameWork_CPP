//
//  ActionBar.cpp
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 6..
//
//

#include "ActionBar.h"
#include "../Base/SMButton.h"
#include "../Base/SMImageView.h"
#include "../Base/ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "../Base/ViewAction.h"
#include "../Const/SMViewConstValue.h"
#include "../Const/SMFontColor.h"
#include "../Base/Intent.h"
#include <2d/CCTweenFunction.h>
#include <cmath>

#define DOT_DIAMETER            (20)
#define LINE_DIAMETER           (5)

#define ACTION_TAG_MENU         (SMViewConstValue::Tag::USER+1)
#define ACTION_TAG_COLOR        (SMViewConstValue::Tag::USER+2)
#define ACTION_TAG_TEXT         (SMViewConstValue::Tag::USER+3)
#define ACTION_TAG_BUTTON       (SMViewConstValue::Tag::USER+4)
#define ACTION_TAG_DROPDOWN     (SMViewConstValue::Tag::USER+5)

#define BUTTON_SIZE    (88)
static const cocos2d::Vec2 CENTER(BUTTON_SIZE/2, BUTTON_SIZE/2);

ActionBar::ColorSet::ColorSet() {
    bg      = cocos2d::Color4F(1, 1, 1, 1);
    text    = MAKE_COLOR4F(0x222222, 1);
    normal  = MAKE_COLOR4F(0x000000, 1);
    press   = MAKE_COLOR4F(0x222222, 1);
}

ActionBar::ColorSet::ColorSet(const cocos2d::Color4F& bg, const cocos2d::Color4F& text, const cocos2d::Color4F& normal, const cocos2d::Color4F& press) {
    this->bg = bg;
    this->text = text;
    this->normal = normal;
    this->press = press;
}

bool ActionBar::ColorSet::operator==(const ColorSet& rhs) const {
    return (bg == rhs.bg && text == rhs.text && normal == rhs.normal && press == rhs.press);
}

ActionBar::ColorSet& ActionBar::ColorSet::operator=(const ColorSet& rhs) {
    if (this == &rhs) {
        return *this;
    }
    
    this->bg = rhs.bg;
    this->text = rhs.text;
    this->normal = rhs.normal;
    this->press = rhs.press;
    
    return *this;
}

const ActionBar::ColorSet ActionBar::ColorSet::WHITE = ActionBar::ColorSet();
const ActionBar::ColorSet ActionBar::ColorSet::WHITE_TRANSLUCENT = ActionBar::ColorSet(cocos2d::Color4F(1, 1, 1, 0.7), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xADAFB3, 1));
const ActionBar::ColorSet ActionBar::ColorSet::BLACK = ActionBar::ColorSet(MAKE_COLOR4F(0x222222, 1), cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 1), MAKE_COLOR4F(0xADAFB3, 1));
const ActionBar::ColorSet ActionBar::ColorSet::NONE = ActionBar::ColorSet(cocos2d::Color4F::WHITE, cocos2d::Color4F::WHITE, cocos2d::Color4F::WHITE, cocos2d::Color4F::WHITE);
const ActionBar::ColorSet ActionBar::ColorSet::TRANSLUCENT = ActionBar::ColorSet(cocos2d::Color4F(1, 1, 1, 0), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xADAFB3, 1));

const ActionBar::ColorSet ActionBar::ColorSet::VIOLET = ActionBar::ColorSet(cocos2d::Color4F(1, 1, 1, 0), cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0x99/255.0f));

struct ActionBar::DotPosition {
    cocos2d::Vec2 from;
    cocos2d::Vec2 to;
    float diameter;
    
    DotPosition(const cocos2d::Vec2& from, const cocos2d::Vec2& to, float diameter) {
        this->from = from;
        this->to = to;
        this->diameter = diameter;
    }
};

// 메뉴 : degrees 0
static ActionBar::DotPosition sDotMenu[] {
    ActionBar::DotPosition(cocos2d::Vec2(-13, +13), cocos2d::Vec2(-13, +13), DOT_DIAMETER), // LT
    ActionBar::DotPosition(cocos2d::Vec2(+13, -13), cocos2d::Vec2(+13, -13), DOT_DIAMETER), // RB
    ActionBar::DotPosition(cocos2d::Vec2(-13, -13), cocos2d::Vec2(-13, -13), DOT_DIAMETER), // LB
    ActionBar::DotPosition(cocos2d::Vec2(+13, +13), cocos2d::Vec2(+13, +13), DOT_DIAMETER), // RT
};

// 닫기 X : degrees 180
static ActionBar::DotPosition sDotClose[] {
    ActionBar::DotPosition(cocos2d::Vec2(-20, +20), cocos2d::Vec2::ZERO, LINE_DIAMETER), // LT
    ActionBar::DotPosition(cocos2d::Vec2(+20, -20), cocos2d::Vec2::ZERO, LINE_DIAMETER), // RB
    ActionBar::DotPosition(cocos2d::Vec2(-20, -20), cocos2d::Vec2::ZERO, LINE_DIAMETER), // LB
    ActionBar::DotPosition(cocos2d::Vec2(+20, +20), cocos2d::Vec2::ZERO, LINE_DIAMETER), // RT
};

// 이전 : degrees 180 + 225
static ActionBar::DotPosition sDotBack[] {
    ActionBar::DotPosition(cocos2d::Vec2(-16, +16), cocos2d::Vec2::ZERO, LINE_DIAMETER), // LT
    ActionBar::DotPosition(cocos2d::Vec2(+16, -16), cocos2d::Vec2::ZERO, LINE_DIAMETER), // RB
    ActionBar::DotPosition(cocos2d::Vec2(-16, -12), cocos2d::Vec2(-16, +16), LINE_DIAMETER), // LB
    ActionBar::DotPosition(cocos2d::Vec2(+12, +16), cocos2d::Vec2(-16, +16), LINE_DIAMETER), // RT
};

// DOT : degrees 0
static ActionBar::DotPosition sDotDot[] {
    ActionBar::DotPosition(cocos2d::Vec2::ZERO, cocos2d::Vec2::ZERO, LINE_DIAMETER), // LT
    ActionBar::DotPosition(cocos2d::Vec2::ZERO, cocos2d::Vec2::ZERO, LINE_DIAMETER), // RB
    ActionBar::DotPosition(cocos2d::Vec2::ZERO, cocos2d::Vec2::ZERO, LINE_DIAMETER), // LB
    ActionBar::DotPosition(cocos2d::Vec2::ZERO, cocos2d::Vec2::ZERO, LINE_DIAMETER), // RT
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Text Container
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ActionBar::TextContainer : public SMView {
public:
    CREATE_VIEW(TextContainer);
    
    virtual bool init() override {
        
        stub[0] = cocos2d::Node::create();
        stub[0]->setCascadeOpacityEnabled(true);
        stub[1] = cocos2d::Node::create();
        stub[1]->setCascadeOpacityEnabled(true);
        addChild(stub[0]);
        addChild(stub[1]);
        
        return true;
    }
    
    virtual void onStateChangeNormalToPress() override {
        setAnimOffset(cocos2d::Vec2(0, -3));
    }
    
    virtual void onStateChangePressToNormal() override {
        setAnimOffset(cocos2d::Vec2(0, 0));
    }
    
    cocos2d::Node* stub[2];
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Menu Button Transform Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ActionBar::MenuTransform : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(MenuTransform);
    
    virtual void onStart() override {
        for (int i = 0; i < 4; i++) {
            auto line = _actionBar->_menuLine[i];
            _from[i] = line->getFromPosition();
            _to[i] = line->getToPosition();
            if (_toType==MenuType::MENU_MENU && _isFirstMenu) {
                _diameter[i] = _minLineWidth;
            } else {
            _diameter[i] = line->getLineWidth();
            }
            
            _actionBar->_menuCircle[i]->setVisible(false);
            _actionBar->_menuLine[i]->setVisible(true);
        }
        
        if (_toType==MenuType::MENU_MENU && _isFirstMenu) {
            _isFirstMenu = false;
        }
        
        _fromAngle = _actionBar->_buttonContainer->getRotation();

        switch (_toType) {
            case MenuType::MENU_CLOSE:
                _toAngle = 180;
                break;
            case MenuType::MENU_BACK:
                if (_fromType!=MENU_BACK) {
                _toAngle = 90 + 180 + 45;
                } else {
                    _toAngle = _fromAngle;
                }
                break;
            default:
                _toAngle = 0;
                break;
        }
        
        
        if (_toType == MenuType::MENU_BACK) {
            float diff = ::fmodf(_fromAngle, 90);
            _fromAngle = _toAngle - 45 - (90 + diff);
        }
        if (_fromType == MenuType::MENU_BACK) {
            if (_fromType!=_toType) {
            _fromAngle = SMView::getShortestAngle(0, _fromAngle);
            _toAngle = 90;
        }
        }
        if (_toAngle < _fromAngle) {
            _fromAngle -= 360;
        }
    }
    
    virtual void onUpdate(float t) override {
        auto a = (ActionBar*)_target;
        
        t = cocos2d::tweenfunc::cubicEaseOut(t);
        
        for (int i = 0; i < 4; i++) {
            auto line = _actionBar->_menuLine[i];
            
            float x1 = ViewUtil::interpolation(_from[i].x, _dst[i].from.x+BUTTON_SIZE/2, t);
            float y1 = ViewUtil::interpolation(_from[i].y, _dst[i].from.y+BUTTON_SIZE/2, t);
            float x2 = ViewUtil::interpolation(_to[i].x, _dst[i].to.x+BUTTON_SIZE/2, t);
            float y2 = ViewUtil::interpolation(_to[i].y, _dst[i].to.y+BUTTON_SIZE/2, t);
            float angle = ViewUtil::interpolation(_fromAngle, _toAngle, t);
            
            float diameter = 0.0f;
            if (_toType == MenuType::MENU_CLOSE) {
                float tt = t * 1.5;
                if (tt > 1) tt = 1;
                diameter = ViewUtil::interpolation(_diameter[i], _dst[i].diameter, tt);
            } else if (_toType == MenuType::MENU_MENU){
                float tt = t - 0.5;
                if (tt < 0) tt = 0;
                tt = tt / 0.5;
                diameter = ViewUtil::interpolation(_diameter[i], _dst[i].diameter, tt);
            } else {
                diameter = ViewUtil::interpolation(_diameter[i], _dst[i].diameter, t);
            }
            
            if (diameter<_minLineWidth) {
                _minLineWidth = diameter;
            }
            
            line->setLineWidth(diameter);
            line->line(x1, y1, x2, y2);
            _actionBar->_buttonContainer->setRotation(angle);
        }
    };
    
    virtual void onEnd() override {
        auto a = (ActionBar*)_target;
        
        for (int i = 0; i < 4; i++) {
            if (_toType == MenuType::MENU_MENU) {
                _actionBar->_menuLine[i]->setVisible(false);
                _actionBar->_menuCircle[i]->setVisible(true);
            } else {
                _actionBar->_menuLine[i]->setVisible(true);
                _actionBar->_menuCircle[i]->setVisible(false);
            }
        }
        
        switch (_toType) {
            default:
            case MenuType::MENU_MENU:
                _toAngle = 0;
                break;
            case MenuType::MENU_CLOSE:
                _toAngle = 180;
                break;
            case MenuType::MENU_BACK:
                _toAngle = 90+180+45;
                break;
            case MenuType::MENU_DOT:
                _toAngle = 0;
                break;
        }
        _actionBar->_buttonContainer->setRotation(_toAngle);
    }
    
    void onCancel() {
        for (int i = 0; i < 4; i++) {
            if (_fromType == MenuType::MENU_MENU) {
                _actionBar->_menuLine[i]->setVisible(false);
                _actionBar->_menuCircle[i]->setVisible(true);
            } else {
                _actionBar->_menuLine[i]->setVisible(true);
                _actionBar->_menuCircle[i]->setVisible(false);
            }
        }
        
        switch (_fromType) {
            default:
            case MenuType::MENU_MENU:
                _toAngle = 0;
                break;
            case MenuType::MENU_CLOSE:
                _toAngle = 180;
                break;
            case MenuType::MENU_BACK:
                _toAngle = 90+180+45;
                break;
            case MenuType::MENU_DOT:
                _toAngle = 0;
                break;
        }
        _actionBar->_buttonContainer->setRotation(_fromAngle);
        _actionBar->_menuButton->setTag(_fromType);
    }
    
    
    void setMenuType(const MenuType fromMenuType, const MenuType menuType, const float duration) {
        setTimeValue(duration, 0);
        
        _fromType = fromMenuType;
        _toType = menuType;
        
        switch (menuType) {
            default:
            case MenuType::MENU_MENU:
                _dst = sDotMenu;
                break;
            case MenuType::MENU_CLOSE:
                _dst = sDotClose;
                break;
            case MenuType::MENU_BACK:
                _dst = sDotBack;
                break;
            case MenuType::MENU_DOT:
                _dst = sDotDot;
                break;
        }
    }
    
    void setActionBar(ActionBar* actionBar) {
        _actionBar = actionBar;
    }

    bool _isFirstMenu = true;
    
    cocos2d::Vec2 _from[4];
    
    cocos2d::Vec2 _to[4];
    
    float _diameter[4];
    
    float _fromAngle;
    
    float _toAngle;
    
    float _minLineWidth = 20.0f;
    
    DotPosition* _dst;
    
    MenuType _toType;
    
    MenuType _fromType;
    
    ActionBar* _actionBar;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Color Transform Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ActionBar::ColorTransform : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(ColorTransform);
    
    virtual void onStart() override {
        auto a = (ActionBar*)_target;
        _from = a->_activeColorSet;
        
        a->_colorSet = _to;
        
    }
    
    virtual void onUpdate(float t) override {
        auto a = (ActionBar*)_target;
        
        a->_activeColorSet.bg = ViewUtil::interpolateColor4F(_from.bg, _to.bg, t);
        a->_activeColorSet.text = ViewUtil::interpolateColor4F(_from.text, _to.text, t);
        a->_activeColorSet.normal = ViewUtil::interpolateColor4F(_from.normal, _to.normal, t);
        a->_activeColorSet.press = ViewUtil::interpolateColor4F(_from.press, _to.press, t);
        
        a->applyColorSet(a->_activeColorSet);
    };
    
    void setColorSet(const ColorSet& toColorSet) {
        setTimeValue(0.25, 0.1);
        
        _to = toColorSet;
    }
    
    ColorSet _from;
    
    ColorSet _to;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Text Transform Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
#define TEXT_DURATION (0.17f)
#define TEXT_DELAY  (0.05f)
#define MOVE_DURATION (.6f)

class ActionBar::TextTransform : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(TextTransform);
    
    virtual void onStart() override {
        _toPosition = _actionBar->_textLabel[_toIndex]->getPositionX();
        if (_fadeType) {
            auto label = _actionBar->_textLabel[1-_toIndex];
            if (label) {
                label->setVisible(true);
            }
            // 들어오는 텍스트
            label = _actionBar->_textLabel[_toIndex];
            if (label) {
                label->setVisible(true);
            }
        } else {
            auto label = _actionBar->_textLabel[_toIndex];
            if (label) {
                label->setVisible(true);
                label->setOpacity(0xFF);
            }
        }
    }
    
    
    virtual void onUpdate(float t) override {
        if (_fadeType) {
            // 나가는 텍스트
            auto label = _actionBar->_textLabel[1-_toIndex];
            if (label) {
                label->setOpacity((GLubyte)(0xFF*(1-t)));
            }
            // 들어오는 텍스트
            label = _actionBar->_textLabel[_toIndex];
            if (label) {
                label->setOpacity((GLubyte)(0xFF*t));
            }
        } else {
            t = cocos2d::tweenfunc::cubicEaseOut(t);
            t *= _duration;
            
            // 나가는 텍스트
            auto label = _actionBar->_textLabel[1-_toIndex];
            if (label) {
                if (label->getSystemFontName().length()>0) {
                    // system font 아닐때만 이거 쓰자..
                int len = label->getStringLength();
                for (int i = 0; i < len; i++) {
                    float tt = t - i*(TEXT_DELAY/2);
                    if (tt > 0) {
                        float f = tt / (TEXT_DURATION/2);
                        if (f > 1) f = 1;
                        auto s = label->getLetter(i);
                            if (s) {
                        f = 1 - f;
                        s->setScale(f);
                        s->setOpacity((GLubyte)(0xFF*f));
                    }
                }
            }
                } else {
                    label->setVisible(false);
                }
            }
            
            if (t > _gap * 0.6) {
                t -= _gap * 0.6;
                
                // 들어오는 텍스트
                label = _actionBar->_textLabel[_toIndex];
                
                if (label) {
                    std::string systemFontName = label->getSystemFontName();
                    if (systemFontName.length()>0) {
                        // 시스템 폰트가 아닐 때만 이거 쓰자
                    int len = label->getStringLength();
                    for (int i = 0; i < len; i++) {
                        float tt = t - i*TEXT_DELAY;
                        if (tt > 0) {
                            if (!label->isVisible()) {
                                label->setVisible(true);
                            }
                            float f = tt / TEXT_DURATION;
                            if (f > 1) f = 1;
                            auto s = label->getLetter(i);
                                if (s) {
                            s->setScale(0.5 + 0.5 * f);
                            s->setOpacity((GLubyte)(0xFF*f));
                        }
                    }
                }
                    } else {
                        label->setVisible(false);
                    }
                }
            }
        }
    }
    
    virtual void onEnd() override {
        auto label = _actionBar->_textLabel[1-_toIndex];
        if (label) {
            if (label->getSystemFontName().length()>0) {
                // 시스템 폰트 아닐때만
                label->setVisible(false);
                int len = label->getStringLength();
                for (int i = 0; i < len; i++) {
                    auto s = label->getLetter(i);
                    if (s) {
                        s->setScale(1);
                        s->setOpacity(0xFF);
                    }
                }
            } else {
                label->setVisible(true);
            }
        }
    }
        
    void onCancel() {
        auto label = _actionBar->_textLabel[_toIndex];
        if (label) {
            if (label->getSystemFontName().length()>0) {
                // 시스템 폰트 아닐때만
            label->setVisible(false);
            int len = label->getStringLength();
            for (int i = 0; i < len; i++) {
                auto s = label->getLetter(i);
                    if (s) {
                s->setScale(1);
                s->setOpacity(0xFF);
            }
        }
            } else {
                label->setVisible(true);
            }
        }
    }
    
    void setFadeType() {
        _fadeType = true;
    }
    
    void setElasticType() {
        _fadeType = false;
    }
    
    void setActionBar(ActionBar* actionBar) {
        _actionBar = actionBar;
    }

    void setTextIndex(const int textIndex) {
        float duration = 0;
        _gap = 0;
        
        if (_fadeType) {
            duration = 0.25;
        } else {
            // 나가는 텍스트
            auto label = _actionBar->_textLabel[1-textIndex];
            if (label) {
                // scale down
                int len = label->getStringLength();
                _gap = TEXT_DURATION + TEXT_DELAY*len;
            }

            // 들어오는 텍스트
            label = _actionBar->_textLabel[textIndex];
            if (label) {
                // scale up
                if (label->getSystemFontName().length()>0) {
                    // 시스템 폰트 아님
                int len = label->getStringLength();
                for (int i = 0; i < len; i++) {
                    auto s = label->getLetter(i);
                        if (s) {
                    s->setScale(0);
                }
                    }
                duration = TEXT_DURATION + TEXT_DELAY*len + 0.1;
                } else {
                    label->setVisible(false);
                }
            }
            duration = std::max(duration, MOVE_DURATION);
            
            duration += _gap + 0.1;
        }
        
        setTimeValue(duration, 0.1);
        
        _toIndex = textIndex;
    }
    
    bool _fadeType;
    
    float _toPosition;
    
    int _toIndex;
    
    float _gap;
    
    ActionBar* _actionBar;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Button Show/Hide Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ActionBar::ButtonAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(ButtonAction);
    
    virtual void onStart() override {
        _from = _target->getScale();
        if (_show) {
            _to = 1.0;
        } else {
            _to = 0.0;
        }
        _target->setVisible(true);
        auto button = (SMButton*)_target;
        button->setIconColor(SMButton::State::NORMAL, _actionBar->_activeColorSet.normal);
        button->setIconColor(SMButton::State::PRESSED, _actionBar->_activeColorSet.press);
    }
    
    virtual void onUpdate(float t) override {
        float tt = cocos2d::tweenfunc::cubicEaseOut(t);
        float scale = ViewUtil::interpolation(_from, _to, tt);
        _target->setScale(scale);
    }
    
    virtual void onEnd() override {
        if (!_show) {
            _target->setVisible(false);
        }
    }
    
    void setShow(ActionBar* actionBar, const float delay) {
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = true;
    }
    
    void setHide(ActionBar* actionBar, const float delay) {
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = false;
    }
    
protected:
    bool _show;
    
    float _from, _to;
    
    ActionBar* _actionBar;
};

class ActionBar::ButtonFadeAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(ButtonFadeAction);
    
    virtual void onStart() override {
        _from = _target->getOpacity()/255.0;
        if (_show) {
            _to = 1.0;
        } else {
            _to = 0.0;
        }
        _target->setVisible(true);
        auto button = (SMButton*)_target;
        button->setIconColor(SMView::State::NORMAL, _actionBar->_activeColorSet.normal);
        button->setIconColor(SMView::State::PRESSED, _actionBar->_activeColorSet.press);
    }
    
    virtual void onUpdate(float t) override {
        GLubyte o = (GLubyte)(0xFF*ViewUtil::interpolation(_from, _to, t));
        _target->setOpacity(o);
    }
    
    virtual void onEnd() override {
        if (!_show) {
            _target->setVisible(false);
        }
    }
    
    void setShow(ActionBar* actionBar, const float delay) {
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = true;
    }
    
    void setHide(ActionBar* actionBar, const float delay) {
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = false;
    }
    
protected:
    bool _show;
    
    float _from, _to;
    
    ActionBar* _actionBar;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// DropDown Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ActionBar::DropDownAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(DropDownAction);
    
    virtual void onStart() override {
        if (_showAction) {
            _from = _target->getScale();
            if (_show) {
                _to = 1.0;
            } else {
                _to = 0.0;
            }
            _target->setVisible(true);
        } else {
            _from = _target->getRotation();
            if (_up) {
                _to = 180;
            } else {
                _to = 360;
            }
            _target->setVisible(true);
        }
    }
    
    virtual void onUpdate(float t) override {
        if (_showAction) {
            float tt = cocos2d::tweenfunc::cubicEaseOut(t);
            float scale = ViewUtil::interpolation(_from, _to, tt);
            _target->setScale(scale);
        } else {
            float tt = cocos2d::tweenfunc::backEaseOut(t);
            float rotate = ViewUtil::interpolation(_from, _to, tt);
            _target->setRotation(rotate);
        }
    }
    
    virtual void onEnd() override {
        if (_showAction) {
            if (!_show) {
                _target->removeFromParent();
                _actionBar->_dropdownButton = nullptr;
                _actionBar->updateTextPosition(false);
            }
        } else {
            if (!_up) {
                _target->setRotation(0);
            }
        }
    }
    
    void setShow(ActionBar* actionBar, const float delay) {
        _showAction = true;
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = true;
    }
    
    void setHide(ActionBar* actionBar, const float delay) {
        _showAction = true;
        _actionBar = actionBar;
        setTimeValue(0.25, delay);
        _show = false;
    }
    
    void setUp(ActionBar* actionBar, const float delay) {
        _showAction = false;
        _actionBar = actionBar;
        setTimeValue(0.50, delay);
        _up = true;
    }
    
    void setDown(ActionBar* actionBar, const float delay) {
        _showAction = false;
        _actionBar = actionBar;
        setTimeValue(0.50, delay);
        _up = false;
    }
    
protected:
    bool _showAction;
    
    bool _show;
    
    bool _up;
    
    float _from, _to;
    
    ActionBar* _actionBar;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// ActionBar Main Class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ActionBar::ActionBar() :
_textIndex(0),
_buttonIndex(0),
_textString(""),
_menuButtonType(MenuType::NONE),
_menuTransform(nullptr),
_textTransform(nullptr),
_colorTransform(nullptr),
_colorSet(ColorSet::NONE),
_activeColorSet(ColorSet::NONE),
_dropdown(DropDown::NOTHING),
_dropdownButton(nullptr),
_dropdownAction(nullptr),
_textTransType(TextTransition::ELASTIC),
_listener(nullptr),
_overlapChild(nullptr),
_useSystemFont(false)
{
}

ActionBar::~ActionBar()
{
    CC_SAFE_RELEASE(_menuTransform);
    CC_SAFE_RELEASE(_colorTransform);
    CC_SAFE_RELEASE(_textTransform);
    CC_SAFE_RELEASE(_dropdownAction);
}

ActionBar* ActionBar::create()
{
    auto view = new (std::nothrow) ActionBar();
    
    if (view && view->init()) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
        view = nullptr;
    }
    
    return view;
}

bool ActionBar::init()
{
    auto s = _director->getWinSize();
    
    setContentSize(cocos2d::Size(s.width, ACTION_BAR_HEIGHT));
    setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    setPosition(0, s.height);
    
//    auto bg= SMView::create(0, 0, 0, s.width, MENU_BAR_HEIGHT);
//    bg->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
//    addChild(bg);
    
//    auto bottomLine = SMView::create(0, 0, 0, s.width, 2.0f);
//    bottomLine->setBackgroundColor4F(MAKE_COLOR4D(239, 62, 67, 1.0f));
//    bg->addChild(bottomLine);
    
//    auto logo =  SMImageView::createWithSpriteFileName("images/mp_login_logo.png");
//    addChild(logo);
//    logo->setScaleType(SMImageView::ScaleType::CENTER_INSIDE);
//    logo->setContentSize(cocos2d::Size(logo->getContentSize().width, 68));
//    logo->setPosition(cocos2d::Vec2(s.width/2-logo->getContentSize().width/2, 10));
    
    _textContainer = TextContainer::create();
    _textContainer->setTag(MenuType::DROPDOWN);
    _textContainer->setContentSize(cocos2d::Size(0, MENU_BAR_HEIGHT));
    _textContainer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _textContainer->setPosition(s.width/2, MENU_BAR_HEIGHT/2);
    addChild(_textContainer);
    
    for (int i = 0; i < 2; i++) {
        _textLabel[i] = nullptr;
        for (int j = 0; j < 2; j++) {
            _actionButton[i][j] = nullptr;
        }
    }
    
    _menuButton = SMButton::create(MenuType::MENU_MENU, SMButton::Style::DEFAULT, 5+BUTTON_SIZE/2, 5+BUTTON_SIZE/2, BUTTON_SIZE, BUTTON_SIZE);
    _menuButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
//    _menuButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x000000, 1));
//    _menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1));
    
    _buttonContainer = cocos2d::Node::create();
    _buttonContainer->setContentSize(cocos2d::Size(BUTTON_SIZE, BUTTON_SIZE));
    _buttonContainer->setCascadeColorEnabled(true);
    _buttonContainer->setCascadeOpacityEnabled(true);
    for (int i = 0; i < 4; i++) {
        _menuLine[i] = ShapeRoundLine::create();
        _menuCircle[i] = ShapeSolidCircle::create();
        _buttonContainer->addChild(_menuLine[i]);
        _buttonContainer->addChild(_menuCircle[i]);
        
        _menuCircle[i]->setPosition(sDotMenu[i].from+CENTER);
        _menuCircle[i]->setContentSize(cocos2d::Size(DOT_DIAMETER, DOT_DIAMETER));
        _menuCircle[i]->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    }
    _menuButton->setButton(SMButton::State::NORMAL, _buttonContainer);
    _menuButton->setOnClickListener(this);
    _menuButton->setPushDownScale(0.95);
    addChild(_menuButton);
    
    setMenuButtonType(MenuType::MENU_MENU, true);
//    setColorSet(ColorSet::TRANSLUCENT, true);
    
    return true;
}

int ActionBar::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    
    if (action == MotionEvent::ACTION_DOWN) {
        if (_listener) {
            _listener->onActionBarTouch();
        }
    }
    
    return TOUCH_TRUE;
}

void ActionBar::setActionBarListener(ActionBarListener* l)
{
    _listener = l;
}


void ActionBar::setMenuButtonType(MenuType menuButtonType, bool immediate, bool swipe)
{
    if (_menuButtonType == menuButtonType)
        return;
    
    DotPosition* to;
    switch (menuButtonType) {
        case MenuType::MENU_MENU:  to = sDotMenu; _menuButton->setTag(MenuType::MENU_MENU); break;
        case MenuType::MENU_CLOSE: to = sDotClose; _menuButton->setTag(MenuType::MENU_CLOSE); break;
        case MenuType::MENU_BACK:  to = sDotBack; _menuButton->setTag(MenuType::MENU_BACK); break;
        case MenuType::MENU_DOT:   to = sDotDot; _menuButton->setTag(MenuType::MENU_DOT); break;
        default:
            return;
    }
    
    // transform중이면 멈춤
    auto action = getActionByTag(ACTION_TAG_MENU);
    if (action) {
        stopAction(_menuTransform);
    }
    
    if (_menuButtonType == MenuType::NONE || immediate) {
        // 이전 메뉴 없음 -> 즉시 세팅
        for (int i = 0; i < 4; i++) {
            auto l = _menuLine[i];
            l->setLineWidth(to[i].diameter);
            l->line(to[i].from+CENTER, to[i].to+CENTER);
            
            if (menuButtonType == MenuType::MENU_MENU) {
                _menuLine[i]->setVisible(false);
                _menuCircle[i]->setVisible(true);
            } else {
                _menuLine[i]->setVisible(true);
                _menuCircle[i]->setVisible(false);
            }
        }

        auto angle = 0;
        switch (menuButtonType) {
            default:
            case MenuType::MENU_MENU:
                angle = 0;
                break;
            case MenuType::MENU_CLOSE:
                angle = 180;
                break;
            case MenuType::MENU_BACK:
                angle = 90+180+45;
                break;
            case MenuType::MENU_DOT:
                angle = 0;
                break;
        }
        _buttonContainer->setRotation(angle);
        _menuButtonType = menuButtonType;
        
        return;
    }
    
    if (_menuTransform == nullptr) {
        _menuTransform = MenuTransform::create(false);
        _menuTransform->setActionBar(this);
        _menuTransform->setTag(ACTION_TAG_MENU);
    }
    
    _menuTransform->setMenuType(_menuButtonType, menuButtonType, 0.45);
    if (!swipe) {
    runAction(_menuTransform);
    }

    _menuButtonType = menuButtonType;
}

void ActionBar::applyColorSet(const ColorSet& colorSet)
{
    // bg color
    setBackgroundColor4F(colorSet.bg);
    
    // text color
    if (_textLabel[0]) {
        _textLabel[0]->setColor(ViewUtil::color4FToColor3B(colorSet.text));
    }
    if (_textLabel[1]) {
        _textLabel[1]->setColor(ViewUtil::color4FToColor3B(colorSet.text));
    }
    if (_dropdownButton) {
        _dropdownButton->setColor(ViewUtil::color4FToColor3B(colorSet.text));
    }
    
    // menu button color
    _menuButton->setButtonColor(SMButton::State::NORMAL, colorSet.normal);
    _menuButton->setButtonColor(SMButton::State::PRESSED, colorSet.press);
    
    
    // right button color
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            auto button = _actionButton[i][j];
            if (button) {
                button->setIconColor(SMButton::State::NORMAL, colorSet.normal);
                button->setIconColor(SMButton::State::PRESSED, colorSet.press);
            }
        }
    }
}

void ActionBar::setColorSet(const ColorSet& colorSet, bool immediate)
{
    if (_colorSet == colorSet)
        return;
    
    // color transform중이면 멈춤
    auto action = getActionByTag(ACTION_TAG_COLOR);
    if (action) {
        stopAction(action);
    }
    
    if (immediate) {
        _colorSet = colorSet;
        _activeColorSet = colorSet;
        
        applyColorSet(colorSet);
        return;
    }
    
    if (_colorTransform == nullptr) {
        _colorTransform = ColorTransform::create(false);
        _colorTransform->setTag(ACTION_TAG_COLOR);
    }
    
    _colorTransform->setColorSet(colorSet);
    runAction(_colorTransform);
}

void ActionBar::setTextTransitionType(TextTransition type)
{
    _textTransType = type;
}

void ActionBar::setButtonTransitionType(ButtonTransition type)
{
    _buttonTransType = type;
}

void ActionBar::setTextWithDropDown(const std::string& textString, bool immediate)
{
    setText(textString, immediate, true);
}

std::string ActionBar::getText() {
    return _textString;
}

void ActionBar::useSystemFont() {
    _useSystemFont = true;
}

void ActionBar::setText(const std::string& textString, bool immediate, bool dropdown)
{
    if (_textString == textString)
        return;
    
    _textString = textString;
    _textIndex = 1 - _textIndex;
    
    if (_textLabel[_textIndex] == nullptr) {
        if (_useSystemFont) {
            _textLabel[_textIndex] = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontBold, 40);
        } else {
        _textLabel[_textIndex] = cocos2d::Label::createWithTTF("", SMFontConst::NotoSansMedium, 40);
        }
        _textLabel[_textIndex]->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _textLabel[_textIndex]->setCascadeOpacityEnabled(true);
        _textContainer->stub[_textIndex]->addChild(_textLabel[_textIndex]);
    }
    
    _textLabel[_textIndex]->setString(textString);
    _textLabel[_textIndex]->setColor(ViewUtil::color4FToColor3B(_activeColorSet.text));
    _textLabel[_textIndex]->setVisible(false);
    
    // 위치 조정
    updateTextPosition(dropdown);
    
    // text transform중이면 멈춤
    auto action = getActionByTag(ACTION_TAG_TEXT);
    if (action) {
        stopAction(action);
    }
    
    if (immediate) {
        _textLabel[_textIndex]->setVisible(true);
        if (_textLabel[1-_textIndex]) {
            _textLabel[1-_textIndex]->setVisible(false);
        }
        
        return;
    }

    if (_textTransform == nullptr) {
        _textTransform = TextTransform::create(false);
        _textTransform->setTag(ACTION_TAG_TEXT);
        _textTransform->setActionBar(this);
    }
    if (_textTransType == TextTransition::ELASTIC) {
        _textTransform->setElasticType();
    } else {
        _textTransform->setFadeType();
    }
    
    _textTransform->setTextIndex(_textIndex);
    
    if (_textTransType != TextTransition::SWIPE) {
    runAction(_textTransform);
    }
}

void ActionBar::setOneButton(MenuType buttonType, bool immediate, bool swipe)
{
    setTwoButton(buttonType, MenuType::NONE, immediate, swipe);
}

#define BUTTON_DELAY    (0.1)
void ActionBar::setTwoButton(MenuType buttonType1, MenuType buttonType2, bool immediate, bool swipe)
{
    float delay = 0;
    
    const MenuType type[] = { buttonType1, buttonType2 };
    
    if (!swipe) {
    // 표시중인 button hide
    for (int i = 0; i < 2; i++) {
        auto button = _actionButton[_buttonIndex][i];
        if (button && button->isVisible()) {
            if (immediate) {
                button->setVisible(false);
            } else {
                auto action = getActionByTag(ACTION_TAG_BUTTON);
                if (action) {
                    stopAction(action);
                }
                
                    if (_buttonTransType==ButtonTransition::BTN_ELASTIC) {
                auto buttonAction = ButtonAction::create(true);
                buttonAction->setTag(ACTION_TAG_BUTTON);
                buttonAction->setHide(this, delay);
                button->runAction(buttonAction);
                    } else {
                        auto buttonAction = ButtonFadeAction::create(true);
                        buttonAction->setTag(ACTION_TAG_BUTTON);
                        buttonAction->setHide(this, 0);
                        button->runAction(buttonAction);
                    }
                
                delay += BUTTON_DELAY;
            }
        }
    }
    
    if (delay > 0) {
        delay += 0.2;
    }
    } else {
        immediate = true;
    }

    // 새로운 button show
    _buttonIndex = 1 - _buttonIndex;
    float x = _contentSize.width - 20;
    int index = 0;
    
    for (int i = 0; i < 2; i++) {
        if (type[i] == MenuType::NONE)
            continue;
        
        auto button = _actionButton[_buttonIndex][index];
        if (button == nullptr) {
            // 버튼 생성
            button = SMButton::create(MenuType::NONE, SMButton::Style::DEFAULT, 0, 0, BUTTON_SIZE, BUTTON_SIZE, 0.5, 0.5);
            button->setIconColor(SMButton::State::NORMAL, _activeColorSet.normal);
            button->setIconColor(SMButton::State::PRESSED, _activeColorSet.press);
            button->setOnClickListener(this);
            button->setPushDownScale(0.95);
            addChild(button);
            _actionButton[_buttonIndex][index] = button;
        }
        
        cocos2d::Node* icon = nullptr;
        bool textIcon = false;
        if (button->getTag() != type[i]) {
            button->setTag(type[i]);
            switch (type[i]) {
                case MenuType::CART:      icon = cocos2d::Sprite::create("images/ic_titlebar_cart.png"); break;
                case MenuType::SEARCH:    icon = cocos2d::Sprite::create("images/ic_titlebar_search.png"); break;
                case MenuType::CONFIRM:   icon = cocos2d::Sprite::create("images/ic_titlebar_check.png"); break;
                case MenuType::DELETE:    icon = cocos2d::Sprite::create("images/ic_titlebar_delete.png"); break;
                case MenuType::CAMERA:    icon = cocos2d::Sprite::create("images/ic_titlebar_camera.png"); break;
                case MenuType::ALBUM:     icon = cocos2d::Sprite::create("images/ic_titlebar_cart.png"); break;
                case MenuType::NEXT:
                {
                    icon = cocos2d::Label::createWithTTF("NEXT", SMFontConst::NotoSansLight, 38);
                    icon->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                    textIcon = true;
                }
                    break;
                case MenuType::DONE:
                {
                    icon = cocos2d::Label::createWithTTF("DONE", SMFontConst::NotoSansLight, 38);
                    icon->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                    textIcon = true;
                }
                    break;
                case MenuType::CANCEL:
                {
                    icon = cocos2d::Label::createWithTTF("CANCEL", SMFontConst::NotoSansLight, 34);
                    icon->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                    textIcon = true;
                }
                    break;
                    
                default:
                    CCASSERT(false, "ID NOT FOUND");
                    break;
            }
            button->setIcon(SMView::State::NORMAL, icon);
        }
        if (icon) {
            int width = 90;
            if (textIcon) {
                width = icon->getContentSize().width;
                x -= 20;
            }
            
            x -= width/2;
            button->setPosition(x, MENU_BAR_HEIGHT/2);
            x -= width/2 + 20;
        }
        
        if (immediate) {
            button->setScale(1.0);
            button->setVisible(true);
        } else {
            auto action = button->getActionByTag(ACTION_TAG_BUTTON);
            if (action) {
                button->stopAction(action);
            }
            
            if (_buttonTransType == BTN_ELASTIC) {
                button->setScale(0);
                button->setOpacity(0xFF);
            auto buttonAction = ButtonAction::create(true);
            buttonAction->setTag(ACTION_TAG_BUTTON);
            buttonAction->setShow(this, delay);
            button->runAction(buttonAction);
            } else {
                button->setOpacity(0);
                button->setScale(1);
                auto buttonAction = ButtonFadeAction::create(true);
                buttonAction->setTag(ACTION_TAG_BUTTON);
                buttonAction->setShow(this, 0.1);
                button->runAction(buttonAction);
            }
            
            delay += BUTTON_DELAY;
        }
        
        index++;
    }
}

void ActionBar::showButton(bool show, bool immediate)
{
    for (int i = 0;  i < 2; i++) {
        auto button = _actionButton[_buttonIndex][i];
        if (button && button->getTag() != MenuType::NONE) {
            if (immediate) {
                if (show) {
                    button->setScale(1);
                    button->setVisible(true);
                } else {
                    button->setScale(0);
                    button->setVisible(false);
                }
            } else {
                auto action = button->getActionByTag(ACTION_TAG_BUTTON);
                if (action) {
                    button->stopAction(action);
                }
                auto buttonAction = ButtonAction::create(true);
                buttonAction->setTag(ACTION_TAG_BUTTON);
                if (show) {
                    buttonAction->setShow(this, 0);
                } else {
                    buttonAction->setHide(this, 0);
                }
                button->runAction(buttonAction);
            }
        }
    }
    // menu button
    if (!_menuButton)
        return;
    
    auto button = _menuButton;
    if (immediate) {
        if (show) {
            button->setScale(1);
            button->setVisible(true);
        } else {
            button->setScale(0);
            button->setVisible(false);
        }
    } else {
        auto action = button->getActionByTag(ACTION_TAG_BUTTON);
        if (action) {
            button->stopAction(action);
        }
        auto buttonAction = ButtonAction::create(true);
        buttonAction->setTag(ACTION_TAG_BUTTON);
        if (show) {
            buttonAction->setShow(this, 0);
        } else {
            buttonAction->setHide(this, 0);
        }
        button->runAction(buttonAction);
    }
}

void ActionBar::showActionButtonWithDelay(bool show, float delay)
{
    for (int i = 0;  i < 2; i++) {
        auto button = _actionButton[_buttonIndex][i];
        if (button && button->getTag() != MenuType::NONE) {
            auto action = button->getActionByTag(ACTION_TAG_BUTTON);
            if (action) {
                button->stopAction(action);
            }
            auto buttonAction = ButtonAction::create(true);
            buttonAction->setTag(ACTION_TAG_BUTTON);
            if (show) {
                buttonAction->setShow(this, delay);
            } else {
                buttonAction->setHide(this, delay);
            }
            button->runAction(buttonAction);
        }
    }
}

void ActionBar::showMenuButton(bool show, bool immediate)
{
    // menu button
    if (!_menuButton)
        return;
    
    auto button = _menuButton;
    if (immediate) {
        if (show) {
            button->setScale(1);
            button->setVisible(true);
        } else {
            button->setScale(0);
            button->setVisible(false);
        }
    } else {
        auto action = button->getActionByTag(ACTION_TAG_BUTTON);
        if (action) {
            button->stopAction(action);
        }
        auto buttonAction = ButtonAction::create(true);
        buttonAction->setTag(ACTION_TAG_BUTTON);
        if (show) {
            buttonAction->setShow(this, 0);
        } else {
            buttonAction->setHide(this, 0);
        }
        button->runAction(buttonAction);
    }
}

void ActionBar::updateTextPosition(bool dropdown)
{
    float textWidth = _textLabel[_textIndex]->getContentSize().width;
    float containerWidth = textWidth;
    
    if (dropdown) {
        containerWidth += 16 + 36;
    }
    
    _textContainer->setContentSize(cocos2d::Size(containerWidth, MENU_BAR_HEIGHT));
    _textContainer->stub[0]->setPosition(containerWidth/2, MENU_BAR_HEIGHT/2);
    _textContainer->stub[1]->setPosition(containerWidth/2, MENU_BAR_HEIGHT/2);
    if (dropdown) {
        _textLabel[_textIndex]->setPositionX(-16-18);
    } else {
        _textLabel[_textIndex]->setPositionX(0);
    }
    
    if (_dropdownButton && dropdown) {
        _dropdownButton->setPosition((_contentSize.width+containerWidth)/2 - 18, MENU_BAR_HEIGHT/2);
    }
}

void ActionBar::setDropDown(DropDown dropdown, bool immediate, float delay)
{
    if (dropdown == _dropdown)
        return;
    
    if (dropdown == DropDown::NOTHING) {
        _textContainer->setOnClickListener(nullptr);
    } else {
        _textContainer->setOnClickListener(this);
    }
    
    if (_dropdownButton) {
        auto action = _dropdownButton->getActionByTag(ACTION_TAG_DROPDOWN);
        if (action) {
            _dropdownButton->stopAction(action);
        }
    }
    
    if (immediate) {
        _dropdown = dropdown;
        if (dropdown == DropDown::NOTHING) {
            if (_dropdownButton) {
                removeChild(_dropdownButton);
                _dropdownButton = nullptr;
            }
        } else {
            if (_dropdownButton == nullptr) {
                _dropdownButton = cocos2d::Sprite::create("images/ic_top_arrow.png");
                _dropdownButton->setColor(ViewUtil::color4FToColor3B(_activeColorSet.text));
                addChild(_dropdownButton);
            }
            if (dropdown == DropDown::UP) {
                _dropdownButton->setRotation(180);
            } else {
                _dropdownButton->setRotation(0);
            }
            _dropdownButton->setScale(1);
            _dropdownButton->setVisible(true);
        }
        
        updateTextPosition(dropdown != DropDown::NOTHING);
        
        return;
    }
    // animation
    if (_dropdownAction == nullptr) {
        _dropdownAction = DropDownAction::create(false);
        _dropdownAction->setTag(ACTION_TAG_DROPDOWN);
    }
    
    bool created = false;
    if (_dropdownButton == nullptr) {
        _dropdownButton = cocos2d::Sprite::create("images/ic_top_arrow.png");
        _dropdownButton->setColor(ViewUtil::color4FToColor3B(_activeColorSet.text));
        _dropdownButton->setVisible(false);
        _dropdownButton->setScale(0);
        addChild(_dropdownButton);
        updateTextPosition(true);
        created = true;
    }
    
    if (dropdown == DropDown::UP) {
        if (created) {
            _dropdownButton->setRotation(180);
            _dropdownAction->setShow(this, delay);
            _dropdownButton->runAction(_dropdownAction);
        } else {
            _dropdownAction->setUp(this, delay);
            _dropdownButton->runAction(_dropdownAction);
        }
    } else if (dropdown == DropDown::DOWN) {
        if (created) {
            _dropdownButton->setRotation(0);
            _dropdownAction->setShow(this, delay);
            _dropdownButton->runAction(_dropdownAction);
        } else {
            _dropdownAction->setDown(this, delay);
            _dropdownButton->runAction(_dropdownAction);
        }
    } else { // NOTHING
        _dropdownAction->setHide(this, delay);
        _dropdownButton->runAction(_dropdownAction);
    }
    
    _dropdown = dropdown;
}


void ActionBar::onClick(SMView* view)
{
    if (_menuButton->getActionByTag(SMViewConstValue::Tag::USER+1)) {
        // 메뉴 버튼 transform중이면 리턴
        return;
    }
    
    if (_listener) {
        if (_listener->onActionBarClick(view)) {
            // 해당 리스너에서 처리되었으면 액션바에서 처리 안함.
            return;
        }
    }
    
    switch (view->getTag()) {
        case MenuType::MENU_MENU:
            break;
        case MenuType::CART:
        {
                // cart를 띄울지 보낼지...
        }
            break;
        case MenuType::CANCEL:
        {
            
        }
            break;
            
    }
}

void ActionBar::setTextOffsetY(float textOffsetY)
{
    if (_textContainer) {
        _textContainer->setPositionY(MENU_BAR_HEIGHT/2+textOffsetY);
    }
}

void ActionBar::setOverlapChild(SMView* child)
{
    _overlapChild = child;
}

bool ActionBar::containsPoint(const cocos2d::Vec2& point)
{
    if (_overlapChild) {
        auto worldPt = convertToWorldSpace(point);
        auto pt = _overlapChild->convertToNodeSpace(worldPt);
        if (_overlapChild->containsPoint(pt)) {
            return false;
        }
    }
    
    return SMView::containsPoint(point);
}

std::vector<ActionBar::MenuType> ActionBar::getButtonTypes()
{
    std::vector<ActionBar::MenuType> buttonTypes;
    for (int i=0; i<2; i++) {
        auto button = _actionButton[_buttonIndex][i];
        if (button && button->isVisible()) {
            buttonTypes.push_back((ActionBar::MenuType)button->getTag());
        }
    }
    
    return buttonTypes;
}

void ActionBar::onSwipeStart()
{
    if (_textTransType == TextTransition::SWIPE && _textTransform && _menuTransform) {
        _textTransform->onStart();
        _menuTransform->onStart();
    }

}

void ActionBar::onSwipeUpdate(float t)
{
    if (_textTransType == TextTransition::SWIPE && _textTransform && _menuTransform) {
        _textTransform->onUpdate(t);
        _menuTransform->onUpdate(t);
    
        for (int i = 0; i < 2; i++) {
            auto button = _actionButton[1 - _buttonIndex][i];
            if (button && button->isVisible()) {
                button->setOpacity((GLubyte)(0xFF*(1-t)));
            }
            button = _actionButton[_buttonIndex][i];
            if (button && button->isVisible()) {
                button->setOpacity((GLubyte)(0xFF*(t)));
            }
        }
    }

}

void ActionBar::onSwipeComplete()
{
    if (_textTransType == TextTransition::SWIPE && _textTransform && _menuTransform) {
        _textTransform->onEnd();
        _menuTransform->onEnd();
        
        for (int i = 0; i < 2; i++) {
            auto button = _actionButton[1 - _buttonIndex][i];
            if (button && button->isVisible()) {
                button->setVisible(false);
            }
            button = _actionButton[_buttonIndex][i];
            if (button && button->isVisible()) {
                button->setOpacity(0xFF);
            }
        }
    }
}

void ActionBar::onSwipeCancel()
{
    if (_textTransType == TextTransition::SWIPE && _textTransform && _menuTransform) {
        _textTransform->onCancel();
        _textIndex = 1 - _textIndex;
        _textString = _textLabel[_textIndex]->getString();
        
        _menuButtonType = (MenuType)_menuTransform->_fromType;
        _menuTransform->onCancel();
        
        _buttonIndex = 1 - _buttonIndex;
        for (int i = 0; i < 2; i++) {
            auto button = _actionButton[1 - _buttonIndex][i];
            if (button && button->isVisible()) {
                button->setVisible(false);
            }
            button = _actionButton[_buttonIndex][i];
            if (button && button->isVisible()) {
                button->setOpacity(0xFF);
            }
        }

    }
}
