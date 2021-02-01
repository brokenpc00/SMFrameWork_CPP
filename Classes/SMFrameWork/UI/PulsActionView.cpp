//
//  PulsActionView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
//

#include "PulsActionView.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"

PulsActionView* PulsActionView::puls(SMView *baseView, cocos2d::Node *parent, float x, float y)
{
    auto view = new (std::nothrow) PulsActionView();
    
    if (view && view->initWithParam(baseView, parent, x, y)) {
        view->autorelease();
        parent->addChild(view);
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

PulsActionView::PulsActionView() :
_baseView(nullptr),
_actionSequence(nullptr),
_removeSelfOnExit(false)
{
    
}

PulsActionView::~PulsActionView()
{
    
}

void PulsActionView::onExit()
{
    // baseview exit
    
    SMView::onExit();
}

void PulsActionView::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t flags)
{
    if (_baseView) {
        auto s = _baseView->getContentSize();
        auto p = _baseView->convertToWorldSpace(cocos2d::Vec2(s.width/2, s.height/2));
        setPosition(p);
    }
    
    cocos2d::Node::visit(renderer, parentTransform, flags);
}

#define SHRINK_DELAY (0.1)
#define STEP_DURATION (0.25)
#define STEP_DELAY (0.1)
#define BOUNCE_TIME (0.8)
#define WAVE_RADIUS (300)

bool PulsActionView::initWithParam(SMView *baseView, cocos2d::Node *parent, float x, float y)
{
    _baseView = baseView;
//    _parent = parent;
    
    cocos2d::Color4F color = cocos2d::Color4F(0xff/255.0f, 0x9a/255.0f, 0x96/255.0f, 1.0f);
    
//    ViewAction::RingWave2::create(50, 300);
    ViewAction::RingWave::show(this, 0, 0, WAVE_RADIUS, STEP_DURATION, SHRINK_DELAY, (cocos2d::Color4F*)&color);
    
    _actionSequence = cocos2d::Sequence::create(
                                                cocos2d::EaseOut::create(cocos2d::ScaleTo::create(SHRINK_DELAY, 0.0), 2.0),
                                                
                                                cocos2d::EaseOut::create(cocos2d::ScaleTo::create(STEP_DURATION*0.5, 1.0), 3.0),
                                                
                                                ViewAction::Bounce::create(BOUNCE_TIME, 0.4, 2),
                                                
                                                cocos2d::CallFuncN::create([this](Node* target) {
                                                _removeSelfOnExit = true;
                                                removeFromParent();}),
                                                
                                            nullptr);
    
    if (_baseView) {
//        baseView->runAction(_actionSequence);
    } else {
        parent->runAction(_actionSequence);
    }
    
    setPosition(x, y);
//    baseView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    return true;
}

void PulsActionView::disconnect()
{
    _baseView = nullptr;
}

