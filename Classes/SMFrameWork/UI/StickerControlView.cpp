//
//  StickerControlView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "StickerControlView.h"
#include "../Base/SMButton.h"
#include "../Base/ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "../Base/ViewAction.h"
#include "DashedRoundRect.h"
#include "WasteBasketActionNode.h"
#include "../Base/Sticker.h"
#include "../Base/MeshSprite.h"
#include <math.h>
#include <cmath>

#define SIZE_BTN_TAG 100
#define BORDER_MARGIN 30

#define UTILBUTTON_ID_DELETE 2000

static const cocos2d::Color4F MENU_BUTTON_A = MAKE_COLOR4F(0xFFFFFF, 0.7);
static const cocos2d::Color4F MENU_BUTTON_B = MAKE_COLOR4F(0x222222, 0.7);
static const cocos2d::Color4F MENU_OUTLINE_A = MAKE_COLOR4F(0x222222, 0.7);
static const cocos2d::Color4F MENU_OUTLINE_B = MAKE_COLOR4F(0xFFFFFF, 0.7);

static cocos2d::Color4F WAVE_COLOR(0xFF/255.0, 0xFF/255.0, 0xFF/255.0, 0.5);

#define UTIL_BUTTON_MODE_NONE   (-1)
#define UTIL_BUTTON_MODE_REMOVE (1)


StickerControlView * StickerControlView::create()
{
    auto view = new (std::nothrow)StickerControlView();
    
    if (view && view->init()) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

StickerControlView::StickerControlView() :
_reset(false)
, _listener(nullptr)
, _sizeButtonIndicator(nullptr)
, _highlightSizeButton(false)
, _uiView(nullptr)
, _borderSprite(nullptr)
, _sizeButton(nullptr)
, _utilButton(nullptr)
, _targetNode(nullptr)
{
    
}

StickerControlView::~StickerControlView()
{
    
}

bool StickerControlView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    _uiView = SMView::create(0, 0, 0, 10, 10);
    _uiView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _uiView->setIgnoreTouchBounds(true);
    addChild(_uiView);
    
    // dashed-line border
    _borderSprite = DashedRoundRect::create("images/dash_line_s.png");
    _borderSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _borderSprite->setCornerRadius(20.0f);
    _borderSprite->setColor(cocos2d::Color3B(0xe6, 0xe6, 0xe6));
    _uiView->addChild(_borderSprite);
    
    
    // size button
    _sizeButton = SMButton::create(SIZE_BTN_TAG, SMButton::Style::SOLID_CIRCLE, 0, 0, 140, 140, 0.5f, 0.5f);
    _sizeButton->setPadding(30.0f);
    _sizeButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
    _sizeButton->setButtonColor(SMButton::State::PRESSED, cocos2d::Color4F(0.9f, 0.9f, 0.9f, 1.0f));
    _sizeButton->setOutlineWidth(5.0f);
    _sizeButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xe6e6e9, 1.0f));
    _sizeButton->setIcon(SMButton::State::NORMAL, "images/size_arrow.png");
    _sizeButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    auto shadow = ShapeSolidCircle::create();
    _sizeButton->setBackgroundNode(shadow);
    shadow->setContentSize(cocos2d::Size(90.0f, 90.0f));
    shadow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    shadow->setAntiAliasWidth(20.0f);
    shadow->setPosition(cocos2d::Vec2(70.0f, 65.0f));
    _sizeButton->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.15f));
    _sizeButton->setOnTouchListener(this);
    _uiView->addChild(_sizeButton);
    
    
    // trash button
    _utilButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, 140, 140, 0.5f, 0.5f);
    _utilButton->setPadding(30.0f);;
    auto shadow2 = ShapeSolidCircle::create();
    _utilButton->setBackgroundNode(shadow2);
    shadow2->setContentSize(cocos2d::Size(90.0f, 90.0f));
    shadow2->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    shadow2->setAntiAliasWidth(20.0f);
    shadow2->setPosition(cocos2d::Vec2(70, 65));
    _utilButton->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.15f));
    _utilButton->setOnClickListener(this);
    _uiView->addChild(_utilButton);
    // 일단 안보임
    _uiView->setVisible(false);
    
    _utilButtonMode = UTIL_BUTTON_MODE_NONE;
    
    return true;
}

void StickerControlView::setStickerListener(StickerControlListener *l)
{
    _listener = l;
}

void StickerControlView::onClick(SMView *view)
{
    if (_listener) {
        _listener->onStickerMenuClick(_targetNode, view->getTag());
    }
}

void StickerControlView::startGeineRemove(cocos2d::Node *node)
{
    if (node && node==_targetNode) {
        auto dst = convertToNodeSpace(_targetNode->convertToWorldSpace(cocos2d::Vec2::ZERO));
        auto size = _utilButton->getContentSize();
        auto stc = convertToNodeSpace(_utilButton->convertToWorldSpace(cocos2d::Vec2(size.width/2, size.height/2)));
        WasteBasketActionNode::showForUtil(this, stc, dst);
        
        auto sticker = dynamic_cast<Sticker*>(node);
        if (sticker) {
            auto sprite = dynamic_cast<MeshSprite*>(sticker->getSprite());
            if (sprite) {
                // mesh sprite로 genie 효과... 일반 sprite는 당연히 안됨...
                if (!sprite->isMesh()) {    // mesh 가 아니면 의미 없다.
                    auto rect = sprite->getTextureRect();
                    sprite->setTextureRect(rect);
                }
            }
        }
    }
}

int StickerControlView::dispatchTouchEvent(MotionEvent *event, SMView *view, bool checkBounds)
{
    int ret = SMView::dispatchTouchEvent(event, view, checkBounds);
    if (checkBounds && event->getAction()==MotionEvent::ACTION_DOWN && view==_uiView) {
        if (_sizeButtonIndicator) {
            auto action = ViewAction::TransformAction::create();
            action->toAlpha(0).removeOnFinish();
            action->setTimeValue(0.5f, 0.0f);
            _sizeButtonIndicator->runAction(action);
            _sizeButtonIndicator = nullptr;
        }
    }
    return ret;
}

int StickerControlView::onTouch(SMView *view, const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    if (action==MotionEvent::ACTION_DOWN) {
        // 눌린 반응 - 링 웨이브
        auto size = view->getContentSize();
        ViewAction::RingWave::show(view, size.width/2, size.height/2, 200, 0.25f, 0.0f, &WAVE_COLOR);
    }

    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _grabPt = *point;
            return TOUCH_FALSE;
        }
            break;
        case MotionEvent::ACTION_MOVE:
        {
            auto pt = 0.5f * (view->getPosition() - cocos2d::Size(BORDER_MARGIN, BORDER_MARGIN) + (*point) - _grabPt);
            float dist = pt.getLength();
            
            // size button 의 대칭점(스티커 중심으로)을 구한다.
            auto ppt = _uiView->convertToWorldSpace(pt) - _uiView->convertToWorldSpace(cocos2d::Vec2::ZERO);
            float rot = std::atan2(ppt.y, ppt.x);
            
            auto tsize = _targetNode->getContentSize();
            float ww = tsize.width/2;
            float hh = tsize.height/2;
            float baseDist = std::sqrt(ww*ww + hh*hh);
            float baseRot = std::atan2(hh, ww);

            // StickerCanvas에서의 실제 사이즈를 구하기 위해 scale을 계산
            float canvasScale = 1.0;
            for (auto p = _targetNode->getParent();  p != nullptr; p = p->getParent()) {
                canvasScale *= p->getScale();
            }
            
            // StickerControl(view가 붙어 있는 넘 - parent)의 world scale을 계산
            float controlScale = 1.0;
            for (auto p = getParent();  p != nullptr; p = p->getParent()) {
                controlScale *= p->getScale();
            }

            baseDist *= canvasScale / controlScale;

            // 실제 거리 및 scale 계산
            float scale = dist / baseDist;
            if (scale * tsize.width <= BORDER_MARGIN || scale * tsize.height <= BORDER_MARGIN) {
                scale = std::max((1+BORDER_MARGIN) / tsize.width, (1+BORDER_MARGIN) / tsize.height);
            }
            _targetNode->setScale(scale);
            _targetNode->setRotation(-ViewUtil::toDegrees(rot-baseRot));
            return TOUCH_TRUE;
        }
            break;
            
    }
    
    return TOUCH_FALSE;
}


// 이거 안쓰일 듯..
void StickerControlView::linkStickerNode(cocos2d::Node *node)
{
    if (_targetNode!=node) {    // 이넘이 그넘이 아닐때
        _targetNode = node;
        if (node) {
            _uiView->setVisible(true);
            
            Sticker::ControlType type = Sticker::ControlType::NONE;
            
            auto sticker = dynamic_cast<Sticker*>(node);
            if (sticker) {
                type = sticker->getControlType();
            }
            
            // delete 밖에 없다. fan, pack, unpack은 나중에 필요하면 구현
            if (type==Sticker::ControlType::DELETE) {
                if (sticker->isRemovable()) {
                    _utilButton->setVisible(true);
                    if (_utilButtonMode != UTIL_BUTTON_MODE_REMOVE) {
                        _utilButtonMode = UTIL_BUTTON_MODE_REMOVE;
                        _utilButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xff683a, 1.0));
                        _utilButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xff683a, 0.9));
                        _utilButton->setIcon(SMButton::State::NORMAL, "images/delete_full.png");
                        _utilButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
                        _utilButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F::WHITE);
                        _utilButton->setTag(UTILBUTTON_ID_DELETE);
                    }
                } else {
                    _utilButton->setVisible(false);
                }
            }
            
            _reset = true;
            
            
            // 계속 UI 업뎃해야 함
            registerUpdate(USER_VIEW_FLAG(1));
            
            if (_highlightSizeButton) {
                _highlightSizeButton = false;
                
                auto ringWave = ViewAction::RingWave2::create(60, 102);
                ringWave->setPosition(_sizeButton->getContentSize()/2);
                ringWave->setColor4F(MAKE_COLOR4F(0xff9a96, 0.6f));
                _sizeButton->addChild(ringWave);
                _sizeButtonIndicator = ringWave;
            }
        } else {
            _uiView->setVisible(false);
            _utilButton->setVisible(false);
            
            if (_sizeButtonIndicator) {
                _sizeButton->removeChild(_sizeButtonIndicator);
                _sizeButtonIndicator = nullptr;
            }
        }
    }
}

void StickerControlView::onUpdateOnVisit()
{
    // 화면 갱신
    if (_targetNode==nullptr) {
        return;
    }
    
    float localScale = getScale();
    float localRotation = getRotation();
    // parent의 scale과 rotation 모두 적용
    for (auto p = getParent(); p!=nullptr; p=p->getParent()) {
        localScale *= p->getScale();
        localRotation += p->getRotation();
    }
    
    auto targetPosition = _targetNode->getParent()->convertToWorldSpace(_targetNode->getPosition());
    float targetScale = _targetNode->getScale();
    float targetRotation = _targetNode->getRotation();
    // _targetNode parent의 scale과 rotation 모두 적용
    for (auto p=_targetNode->getParent(); p!=nullptr; p=p->getParent()) {
        targetScale *= p->getScale();
        targetRotation += p->getRotation();
    }
    
    float scale = targetScale / localScale;
    float rotation = targetRotation - localRotation;
    auto position = convertToNodeSpace(targetPosition);
    cocos2d::Size size = _targetNode->getContentSize() * scale;
    
    if (_reset || size.width!=_targetSize.width || size.height!=_targetSize.height) {
        _reset = false;
        _targetSize = size;
        
        auto viewSize = size + cocos2d::Size(BORDER_MARGIN, BORDER_MARGIN);
        _uiView->setContentSize(viewSize);
        _borderSprite->setContentSize(viewSize);
        _borderSprite->setPosition(viewSize/2); // dash-line border는 가운데
        _sizeButton->setPosition(viewSize); // size button은 우측 상단에
        _utilButton->setPosition(cocos2d::Vec2::ZERO);  // trash button은 좌측 하단에
    }
    
    _uiView->setPosition(position);
    _uiView->setRotation(rotation);
}
