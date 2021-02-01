//
//  SearchEditBox.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 21..
//
//

#include "SearchEditBox.h"
#include "../Base/SMButton.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"
#include "../Util/ViewUtil.h"
#include "../Const/SMFontColor.h"
#include "../Const/SMViewConstValue.h"
#include "../Util/StringUtil.h"

#include <cocos2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cstring>


#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define FONT_NAME       (SMFontConst::SystemFontLight)
#define FONT_SIZE       (17)

#define BUTTON_ID_CLEAR (100)
#define BUTTON_ID_MASK  (101)

static const std::string sPlaceHolderText("검색");

#define ANIM_TAG_TOACTIVE       (1)
#define ANIM_TAG_TOINACTIVE     (2)

#define INACTIVE_WIDTH      (460)
#define INACTIVE_POS_X      (20+90+20)
#define INACTIVE_EDIT_WIDTH (INACTIVE_WIDTH-(100))

#define ACTIVE_WIDTH        (415)
#define ACTIVE_POS_X        (40)
#define ACTIVE_EDIT_WIDTH (ACTIVE_WIDTH-(200))

#define EDIT_HEIGHT         (90)
#define EDIT_ICON_WIDTH     (30)
#define EDIT_ICON_X         (34+15)
#define EDIT_ICON_GAP         (14)


class ActiveIconAction : public cocos2d::ActionInterval {
public:
    
    static ActiveIconAction* create() {
        auto action = new (std::nothrow)ActiveIconAction();
        
        if (action && action->initWithDuration(0.5)) {
            action->autorelease();
        }
        
        return action;
    }
    
    virtual void update(float t)  override {
        auto tt = cocos2d::tweenfunc::backEaseOut(t);
        float scale = 1.0 + 0.3 * std::sin(M_PI * tt);
        _target->setScale(scale);
    }
};


class SearchEditBox::Container : public SMView {
public:
    CREATE_VIEW(Container);
    
    Container() : _text(""), _border(nullptr) {}
    
    virtual bool init() override {
        
        auto r = ShapeSolidRoundRect::create();
        r->setCornerRadius(EDIT_HEIGHT/2);
        setBackgroundNode(r);
        setBackgroundColor4F(cocos2d::Color4F::WHITE);
        
        // 테두리
        _border = ShapeRoundedRect::create();
        _border->setLineWidth(3);
        _border->setCornerRadius(EDIT_HEIGHT/2);
        _border->setColor4F(SMColorConst::COLOR_F_ADAFB3);
        addChild(_border);
        
        // 아이콘
        _icon = cocos2d::Sprite::create("images/ic_titlebar_search.png");
        _icon->setColor(SMColorConst::COLOR_B_ADAFB3);
        addChild(_icon);
        
        // PlaceHolder Label
        _label = cocos2d::Label::createWithTTF(sPlaceHolderText, SMFontConst::NotoSansLight, FONT_SIZE);
        _label->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
        _label->setColor(SMColorConst::COLOR_B_ADAFB3);
        addChild(_label);
        
        float ww = EDIT_ICON_WIDTH + _label->getContentSize().width/2 + EDIT_ICON_GAP;
        float xx = (INACTIVE_WIDTH  - ww)/2 + EDIT_ICON_WIDTH/2 - INACTIVE_POS_X/2;// + 10;
        _icon->setPosition(xx, EDIT_HEIGHT/2);
        _label->setPosition(xx + EDIT_ICON_WIDTH/2 + EDIT_ICON_GAP, EDIT_HEIGHT/2);
        
        // Clear 버튼
        _clearButton = SMButton::create(BUTTON_ID_CLEAR, SMButton::Style::DEFAULT);
        _clearButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _clearButton->setContentSize(cocos2d::Size(120, 100));
        _clearButton->setPosition(INACTIVE_WIDTH - 45, EDIT_HEIGHT/2);
        _clearButton->setIcon(SMView::State::NORMAL, "images/ic_titlebar_close.png");
        _clearButton->setIconColor(SMView::State::NORMAL, SMColorConst::COLOR_F_ADAFB3);
        _clearButton->setIconColor(SMView::State::PRESSED, SMColorConst::COLOR_F_ADAFB3);
        _clearButton->setPushDownScale(0.92);
        _clearButton->setPushDownOffset(cocos2d::Vec2(0, -5));
        _clearButton->setEnabled(false);
        _clearButton->setOpacity(0);
        addChild(_clearButton);
        
        // 창 Size
        setContentSize(cocos2d::Size(INACTIVE_WIDTH, EDIT_HEIGHT));
        
        return true;
    }
    
    void setEditBox(SearchEditBox* editbox) {
        _editBox = editbox;
    }
    
    void setText(const std::string& text) {
        if (_text.compare(text) == 0)
            return;
        
        _text = text;
        if (_text.empty()) {
            _label->setString(sPlaceHolderText);
            _label->setVisible(true);
            
            _clearButton->setOpacity(0);
            _clearButton->setEnabled(false);
        } else {
            _label->setString(_text);
            _label->setVisible(false);
            
            _clearButton->setOpacity(0xFF);
            _clearButton->setEnabled(true);
        }
    }
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override {
        SMView::setContentSize(contentSize);
        if (_border) {
            _border->setContentSize(contentSize);
            _clearButton->setPositionX(contentSize.width - 45);
        }
    }
    
    void transform(bool toActive) {
        
        _fromWidth = getContentSize().width;
        _fromX = getPositionX();
        _fromIconX = _icon->getPositionX();
        _fromColor = _icon->getColor();
        _fromAlpha = _clearButton->getOpacity() / 255.0;
        
        float ww = EDIT_ICON_WIDTH + EDIT_ICON_GAP + _label->getContentSize().width;
        
        if (toActive) {
            _toX = ACTIVE_POS_X;
            _toWidth = ACTIVE_WIDTH;
            _toColor = SMColorConst::COLOR_B_222222;
            _toAlpha = _text.empty() ? 0 : 1;
            _toIconX = EDIT_ICON_X;
        } else {
            _toX = INACTIVE_POS_X;
            _toWidth = INACTIVE_WIDTH;
            _toColor = SMColorConst::COLOR_B_ADAFB3;
            _toAlpha = 0;
            
            if (_text.empty()) {
                // 텍스트 없으면 중앙에
                _toIconX = (INACTIVE_WIDTH-ww)/2 + EDIT_ICON_WIDTH/2 - INACTIVE_POS_X/2;// + 10;
                
                _label->setString(sPlaceHolderText);
                
            } else {
                // 텍스트 있으면 왼쪽정렬
                _toIconX = EDIT_ICON_X;
                
                
                _label->setString(_text);
//                auto lines = HashTagUtil::seperateByWordBreak(_text, FONT_NAME, FONT_SIZE, ACTIVE_EDIT_WIDTH);
//                if (lines.size() > 1) {
//                    _label->setString(lines.at(0) + "...");
//                } else {
//                    _label->setString(lines.at(0));
//                }
            }
            
            _label->setVisible(true);
        }
        
        auto a1 = getActionByTag(ANIM_TAG_TOACTIVE);
        auto a2 = getActionByTag(ANIM_TAG_TOINACTIVE);
        
        if (a1) {
            stopAction(a1);
        }
        if (a2) {
            stopAction(a2);
        }
        
        auto action = ViewAction::TransformAction::create();
        if (toActive) {
            action->setTag(ANIM_TAG_TOACTIVE);
            action->runFuncOnFinish(CC_CALLBACK_2(Container::funcActiveComplete, this));
            action->setTimeValue(.5, .05);
        } else {
            _editBox->setActive(false);
            action->setTag(ANIM_TAG_TOINACTIVE);
            action->setTimeValue(.5, 0);
        }
        action->setTweenFunc(cocos2d::tweenfunc::TweenType::Cubic_EaseOut);
        action->setUpdateCallback(CC_CALLBACK_3(Container::procTransform, this));
        runAction(action);
    }
    
    void procTransform(cocos2d::Node* target, int tag, float t) {
        // 위치
        float x = ViewUtil::interpolation(_fromX, _toX, t);
        setPositionX(x, false);
        
        // 크기
        float width = ViewUtil::interpolation(_fromWidth, _toWidth, t);
        setContentSize(cocos2d::Size(width, EDIT_HEIGHT));
        
        // 아이콘/텍스트 컬러
        auto color = ViewUtil::interpolateColor3B(_fromColor, _toColor, t);
        _icon->setColor(color);
        //        _label->setColor(color);
        
        // ClearButton alpha
        float alpha = ViewUtil::interpolation(_fromAlpha, _toAlpha, t);
        _clearButton->setOpacity((GLubyte)(0xFF*alpha));
        
        // PlaceHolder
        float px = ViewUtil::interpolation(_fromIconX, _toIconX, t);
        _icon->setPositionX(px);
        _label->setPositionX(px + _icon->getContentSize().width/2 + 20);
    }
    
    void funcActiveComplete(cocos2d::Node* target, int tag) {
        _editBox->enableEditBox(true);
        
        if (_text.empty()) {
            _clearButton->setEnabled(false);
            _clearButton->setOpacity(0);
        } else {
            if (!_text.empty()) {
                _clearButton->setEnabled(true);
                _clearButton->setOpacity(0xFF);
                _label->setVisible(false);
            }
        }
    }
    
    void setOnClickListener(OnClickListener* l) {
        SMView::setOnClickListener(l);
        _clearButton->setOnClickListener(l);
    }
    
private:
    
    SearchEditBox* _editBox;
    
    ShapeRoundedRect* _border;
    
    cocos2d::Sprite* _icon;
    
    std::string _text;
    
    cocos2d::Label* _label;
    
    SMButton* _clearButton;
    
    float _fromWidth, _toWidth;
    
    float _fromX, _toX;
    
    float _fromIconX, _toIconX;
    
    float _fromAlpha, _toAlpha;
    
    cocos2d::Color3B _fromColor, _toColor;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Search EditBox Class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
SearchEditBox* SearchEditBox::create(SearchEditBoxListener* l) {
    SearchEditBox* view = new (std::nothrow)SearchEditBox();
    
    if (view != nullptr) {
        if (view->initWithListener(l)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

SearchEditBox::SearchEditBox() :
_isActive(false),
_keybordOpened(false)
{
}

SearchEditBox::~SearchEditBox()
{
}

#define ICON_POSITION_X     (50+62/2)
#define HINT_POSITION_X     (50+62+30+5)
#define CLEAR_POSITION_X    (VIEW_WIDTH-120/2)

bool SearchEditBox::initWithListener(SearchEditBoxListener* l) {
    
    _listener = l;
    
    auto s = _director->getWinSize();
    setContentSize(cocos2d::Size(s.width, TOP_MENU_HEIGHT));
    
    _container = Container::create();
    _container->setTag(BUTTON_ID_MASK);
    _container->setPosition(INACTIVE_POS_X, (TOP_MENU_HEIGHT-EDIT_HEIGHT)/2);
    _container->setOnClickListener(this);
    _container->setEditBox(this);
    addChild(_container);
    
    // 에디트 박스
    auto editBg = cocos2d::ui::Scale9Sprite::create();
    _editBox = cocos2d::ui::EditBox::create(cocos2d::Size(ACTIVE_WIDTH-(34+EDIT_ICON_WIDTH/2+20 + 80), TOP_MENU_HEIGHT), editBg);
    _editBox->setCascadeOpacityEnabled(true);
    _editBox->setFontColor(SMColorConst::COLOR_B_222222);
    _editBox->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
//    _editBox->setVerticalAlignment(0);
    _editBox->setPosition(cocos2d::Vec2(ACTIVE_POS_X+34+EDIT_ICON_WIDTH+20-4, 0));
    _editBox->setFontName(FONT_NAME);
    _editBox->setFontSize(FONT_SIZE);
    _editBox->setReturnType(cocos2d::ui::EditBox::KeyboardReturnType::SEARCH);
    _editBox->setInputMode(cocos2d::ui::EditBox::InputMode::SINGLE_LINE);
    _editBox->setInputFlag(cocos2d::ui::EditBox::InputFlag::SENSITIVE);
    _editBox->setMaxLength(100);
    _editBox->setDelegate(this);
//    _editBox->disableKeyboardShowAuto();
    addChild(_editBox);
    
    enableEditBox(false);
    
    return true;
}

void SearchEditBox::editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) {
}

void SearchEditBox::editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) {
}

void SearchEditBox::editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) {
    _container->setText(text);
    
    if (_listener) {
        _listener->onEditInputChanged(this);
    }
}

void SearchEditBox::editBoxReturn(cocos2d::ui::EditBox* editBox) {
    if (_isActive) {
        setActive(false);
        if (_listener) {
            _listener->onEditReturn(this);
        }
    }
}

void SearchEditBox::keyboardWillShow(cocos2d::IMEKeyboardNotificationInfo& info) {
    _keybordOpened = true;
}

void SearchEditBox::keyboardWillHide(cocos2d::IMEKeyboardNotificationInfo& info) {
    _keybordOpened = false;
}

void SearchEditBox::enableEditBox(bool enabled) {
    
    if (enabled) {
        _editBox->setOpacity(0xFF);
        _editBox->setEnabled(true);
        _editBox->setFocused(true);
//        _editBox->openKeyboard();
    } else {
        _editBox->setOpacity(0);
        _editBox->setEnabled(false);
    }
    
}

void SearchEditBox::setActive(bool active) {
    if (_isActive == active)
        return;
    
    _isActive = active;
    _container->transform(active);
    
    if (!active) {
        _editBox->setOpacity(0);
        _editBox->setEnabled(false);
    }
    
    if (_listener) {
        _listener->onEditActive(this, active);
    }
}

std::string SearchEditBox::getText() {
    std::string str(_editBox->getText());
    
    return StringUtil::trim(str);
}

void SearchEditBox::setText(const std::string& text) {
    _editBox->setText(text.c_str());
    if (!text.empty()) {
        editBoxTextChanged(_editBox, text);
        setActive(false);
    }
}

void SearchEditBox::onClick(SMView* view) {
    switch (view->getTag()) {
        case BUTTON_ID_CLEAR:
        {
            auto text = _editBox->getText();
            if (text && std::strlen(text) > 0) {
                _editBox->setText("");
                editBoxTextChanged(_editBox, "");
            }
        }
            break;
        case BUTTON_ID_MASK:
            if (!_isActive) {
                setActive(true);
            }
            break;
    }
}

void SearchEditBox::closeKeyboard() {
    if (_keybordOpened) {
        _editBox->setEnabled(false);
//        _editBox->closeKeyboard();
    }
}

void SearchEditBox::onCloseKeyboard() {
    closeKeyboard();
}

void SearchEditBox::cancelInput() {
    setActive(false);
    closeKeyboard();
}


bool SearchEditBox::containsPoint(const cocos2d::Vec2& point) {
    return true;
}

int SearchEditBox::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    int ret = SMView::dispatchTouchEvent(action, touch, point, event);
    if (action == MotionEvent::ACTION_DOWN) {
        if (_touchMotionTarget == nullptr || dynamic_cast<Container*>(_touchMotionTarget) == nullptr) {
            auto children = getChildren();
            for (auto child : children) {
                auto editBox = dynamic_cast<Container*>(child);
                if (editBox) {
                    setActive(false);
                    closeKeyboard();
                    break;
                }
            }
        }
    }
    return ret;
    
}

