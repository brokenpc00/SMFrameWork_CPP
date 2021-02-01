//
//  FocusingView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 3..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "FocusingView.h"
#include "CaptureView.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"
#include "../Util/ViewUtil.h"
#include <2d/CCTweenFunction.h>

#define ACTION_TAG_FINISH (SMViewConstValue::Tag::USER+1)
#define FOCUS_LAYER_TIMEOUT (3.0f)
#define FOCUS_SUBRING_WIDTH (30.0f)
#define FOCUS_MAINRING_WIDTH (4.0f)
#define FOCUS_RADIUS (95.0f)


class FocusLayer::FinishAction : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(FinishAction);
    
    virtual void onUpdate(float t) override {
        auto layer = (FocusLayer*)_target;
        layer->onFinishUpdate(t);
    }
    
    virtual void onEnd() override {
        _target->setVisible(false);
    }
};

class FocusLayer::FocusingAction : public cocos2d::ActionInterval {
public:
    static  FocusingAction * create() {
        auto action = new (std::nothrow)FocusingAction();
        
        if (action && action->initWithDuration(0.0)) {
            action->autorelease();
            action->setDuration(0.5);
        }
        
        return action;
    }
    
    virtual void update(float t) override {
        auto layer = (FocusLayer*)_target;
        auto r1 = layer->_mainRing;
        float s = std::cos(2 * M_PI * t);
        
        auto opacity = (GLubyte)(0xFF * (0.8 + 0.2*s));
        r1->setOpacity(opacity);
    }
};

FocusLayer * FocusLayer::create()
{
    FocusLayer * layer = new(std::nothrow)FocusLayer();
    if (layer != nullptr) {
        if (layer->init()) {
            layer->autorelease();
        } else {
            CC_SAFE_DELETE(layer);
        }
    }
    return layer;
}

FocusLayer::FocusLayer() : _finishAction(nullptr)
{
    
}

FocusLayer::~FocusLayer()
{
    CC_SAFE_RELEASE(_finishAction);
}

bool FocusLayer::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    // Touch하는 좌표 기준이므로 계산하기 쉽게 가운데로 Anchor를 잡는다.
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    // 작은 Ring
    _subRing = ShapeCircle::create();
    _subRing->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _subRing->setLineWidth(FOCUS_SUBRING_WIDTH);
    addChild(_subRing);
    
    // 큰 Ring
    _mainRing = ShapeCircle::create();
    _mainRing->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _mainRing->setLineWidth(FOCUS_MAINRING_WIDTH);
    addChild(_mainRing);
    
    _arcRing = ShapeArcRing::create();
    _arcRing->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _arcRing->setLineWidth(4);
    _arcRing->setColor4F(MAKE_COLOR4F(0xFF000B, 1));
    addChild(_arcRing);

    _crosshairH = ShapeSolidRect::create();
    _crosshairH->setContentSize(cocos2d::Size(32, 3));
    _crosshairH->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_crosshairH);
    
    _crosshairV = ShapeSolidRect::create();
    _crosshairV->setContentSize(cocos2d::Size(3, 32));
    _crosshairV->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_crosshairV);
    
    setRadius(FOCUS_RADIUS);
    
    return true;
}

void FocusLayer::setRadius(float radius)
{
    _radius = radius;
    
    _subRing->setContentSize(cocos2d::Size(radius*2+2, radius*2+2));
    _mainRing->setContentSize(cocos2d::Size(radius*2, radius*2));
    _arcRing->setContentSize(cocos2d::Size(radius*2, radius*2));
}

void FocusLayer::startAutoFocus() {
    stopAllActions();
    
    setVisible(true);
    setOpacity(0);
    setScale(0.95);
    
    _arcRing->setVisible(false);
    _crosshairH->setColor4F(cocos2d::Color4F::WHITE);
    _crosshairV->setColor4F(cocos2d::Color4F::WHITE);
    
    _subRing->setColor4F(cocos2d::Color4F(0, 0, 0, 0.1));
    _subRing->setLineWidth(30);
    
    runAction(cocos2d::FadeIn::create(0.1));
    runAction(cocos2d::ScaleTo::create(0.1, 1.0));
    runAction(cocos2d::RepeatForever::create(FocusingAction::create()));
    
    if (isScheduled(schedule_selector(FocusLayer::onTimeOut))) {
        unschedule(schedule_selector(FocusLayer::onTimeOut));
    }
    
    schedule(schedule_selector(FocusLayer::onTimeOut), FOCUS_LAYER_TIMEOUT);
}

void FocusLayer::finishAutoFocus() {
    if (isScheduled(schedule_selector(FocusLayer::onTimeOut))) {
        unschedule(schedule_selector(FocusLayer::onTimeOut));
    }
    
    if (!isVisible() || getActionByTag(ACTION_TAG_FINISH)) {
        // 이미 종료
        return;
    }
    
    if (_finishAction == nullptr) {
        _finishAction = FinishAction::create(false);
        _finishAction->setTag(ACTION_TAG_FINISH);
    }
    
    _finishAction->setTimeValue(.8, 0);
    runAction(_finishAction);
}

void FocusLayer::onTimeOut(float dt) {
    finishAutoFocus();
}

void FocusLayer::onFinishUpdate(float t) {
    if (t < 0.9) {
        t = t / 0.7;
        if (t > 1) t = 1;
        t = cocos2d::tweenfunc::cubicEaseOut(t);
        
        _arcRing->setVisible(true);
        _arcRing->draw(180, -360*t);
        
        auto color = ViewUtil::interpolateColor4F(cocos2d::Color4F::WHITE, MAKE_COLOR4F(0xFF000B, 1), t);
        _crosshairH->setColor4F(color);
        _crosshairV->setColor4F(color);
        
        color = ViewUtil::interpolateColor4F(cocos2d::Color4F(0, 0, 0, 0.1), MAKE_COLOR4F(0xFF000B, 0.3), t);
        _subRing->setColor4F(color);
        _subRing->setLineWidth(30 - 15*t);
        
    } else {
        t = (t - 0.9)/0.1;
        setScale(1.0 - 0.2*t);
        setOpacity((GLubyte)(0xFF * (1.0-t)));
    }
}





// focusing view
FocusingView * FocusingView::create(OnFocusingListener *l)
{
    FocusingView * view = new (std::nothrow)FocusingView();
    
    if (view!=nullptr) {
        if (view->initWithListener(l)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

FocusingView::FocusingView()
{
    
}

FocusingView::~FocusingView()
{
    
}

bool FocusingView::initWithListener(OnFocusingListener *l)
{
    if (!SMView::init()) {
        return false;
    }
    
    _listener = l;
    
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setTouchMask(TOUCH_MASK_CLICK);
    
    // focus indicator
    _focusLayer = FocusLayer::create();
    addChild(_focusLayer);
    
    return true;
}

void FocusingView::setFocusLayer(float x, float y, float scale, bool isMirror, const cocos2d::Size &size)
{
    _baseScale = scale;
    _isMirror = isMirror;
    
    setPosition(x, y);
    // 가로 세로를 바꿔서 표현
    setContentSize(cocos2d::Size(size.height*_baseScale , size.width*_baseScale));
}

void FocusingView::performClick(const cocos2d::Vec2 &worldPoint)
{
    SMView::performClick(worldPoint);
    
    // touch가 들어오면 focusing
    auto touchPoint = convertToNodeSpace(worldPoint);
    
    // 정규화 : 0.0~1.0
    auto pt = touchPoint;
    pt.x /= _contentSize.width;
    pt.y /= _contentSize.height;
    
    if (_isMirror) {
        pt.x = 1.0 - pt.x;
    }
    
    // cocos2dx 좌표계를 Camera 좌표계롤 변경
    cocos2d::Vec2 focusPoint(1.0f - pt.y, 1.0f - pt.x);
    
    // interface로 event 전달
    if (_listener) {
        if (_listener->onFocusingInLayer(touchPoint, focusPoint)) {
            // layer 보이고
            _focusLayer->setVisible(true);
            // 자리잡고
            _focusLayer->setPosition(touchPoint);
            // focusing animation 시작
            _focusLayer->startAutoFocus();
        }
    }
}

void FocusingView::setEnableFocus(bool enabled)
{
    setEnabled(enabled);
    _focusLayer->setVisible(false);
}

