//
//  WasteBasketActionNode.c
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "WasteBasketActionNode.h"
#include "../Base/ViewAction.h"
#include "../Util/ViewUtil.h"

#define MOVE_DURATION (0.2f)        // 살짝 이동하고
#define OPEN_DURATION (0.1f)        // 뚜껑 열리고
#define EXIT_DURATION (0.15f)       // 사라진다.

#define TRASH_SIZE  80.0f
#define TRASH_SHADOW_SIZE   90.0f
#define TRASH_TOP_ANCHOR    cocos2d::Vec2(8/50.0, (50-13)/50.0)
#define TRASH_TOP_POS           cocos2d::Vec2(TRASH_SIZE/2.0f-(25-8), TRASH_SIZE/2.0f+(25-13))
#define TRASH_BODY_POS          cocos2d::Vec2(TRASH_SIZE/2.0f, TRASH_SIZE/2.0f)

WasteBasketActionNode * WasteBasketActionNode::show(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    auto node = new (std::nothrow)WasteBasketActionNode();
    
    if (node && node->initWithParam(parent, from, to)) {
        node->autorelease();
        parent->addChild(node);
    } else {
        CC_SAFE_DELETE(node);
    }
    
    return node;
}

WasteBasketActionNode * WasteBasketActionNode::showForList(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    auto node = new (std::nothrow)WasteBasketActionNode();
    
    if (node && node->initWithParam2(parent, from, to)) {
        node->autorelease();
        parent->addChild(node);
    } else {
        CC_SAFE_DELETE(node);
    }
    
    return node;
}

WasteBasketActionNode * WasteBasketActionNode::showForUtil(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    auto node = new (std::nothrow)WasteBasketActionNode();
    
    if (node && node->initWithParam3(parent, from, to)) {
        node->autorelease();
        parent->addChild(node);
    } else {
        CC_SAFE_DELETE(node);
    }
    
    return node;
}

WasteBasketActionNode::WasteBasketActionNode()
{
    
}

WasteBasketActionNode::~WasteBasketActionNode()
{
    
}

void WasteBasketActionNode::onExit()
{
    cocos2d::Node::onExit();
    
    if (getParent() && !_removeSelfOnExit) {
        // release self
        _removeSelfOnExit = true;
        removeFromParent();
    }
}

bool WasteBasketActionNode::initWithParam(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    setContentSize(cocos2d::Size(TRASH_SIZE, TRASH_SIZE));
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    // shadow
    auto shadow = ShapeSolidCircle::create();
    shadow->setContentSize(cocos2d::Size(TRASH_SHADOW_SIZE, TRASH_SHADOW_SIZE));
    shadow->setAntiAliasWidth(20);
    shadow->setPosition(-5, -10);
    shadow->setColor4F(cocos2d::Color4F(0, 0, 0, .15));
    addChild(shadow);
    
    // bg
    auto bg = ShapeSolidCircle::create();
    bg->setContentSize(cocos2d::Size(TRASH_SIZE, TRASH_SIZE));
    bg->setColor4F(MAKE_COLOR4F(0xff5825, 0.8f));
    addChild(bg);
    
    auto icon1 = cocos2d::Sprite::create("images/delete_top.png");  // 뚜껑
    auto icon2 = cocos2d::Sprite::create("images/delete_body.png"); // 쓰레기통
    icon1->setAnchorPoint(TRASH_TOP_ANCHOR);
    icon1->setPosition(TRASH_TOP_POS);
    icon2->setPosition(TRASH_BODY_POS);
    addChild(icon1);
    addChild(icon2);
    
    setPosition(from);

    auto move = cocos2d::EaseSineInOut::create(cocos2d::MoveTo::create(MOVE_DURATION, to));
    auto scale = cocos2d::ScaleTo::create(MOVE_DURATION, 1.5);
    auto step1 = cocos2d::Spawn::create(move, scale, nullptr);

    auto open = cocos2d::EaseIn::create(cocos2d::RotateTo::create(OPEN_DURATION, -30), 1.0);
    auto close = cocos2d::RotateTo::create(0.05, 0);
    auto step2 = cocos2d::Sequence::create(
                                           cocos2d::DelayTime::create(MOVE_DURATION+0.1),
                                           open,
                                           cocos2d::DelayTime::create(0.7),
                                           close,
                                           nullptr);
    icon1->runAction(step2);

    auto bounce = cocos2d::EaseInOut::create(ViewAction::ScaleSine::create(0.4, 1.5), 2.0);

    auto exit = cocos2d::Spawn::create(cocos2d::EaseIn::create(cocos2d::ScaleTo::create(EXIT_DURATION, 0.7), 3.0),
                                       cocos2d::FadeTo::create(EXIT_DURATION, 0),
                                       nullptr);

    auto seq = cocos2d::Sequence::create(step1,
                                         cocos2d::DelayTime::create(1.0), // delay for genie
                                         bounce,
                                         //                                         cocos2d::DelayTime::create(.5),
                                         exit,
                                         cocos2d::CallFuncN::create([this](Node* target) { _removeSelfOnExit = true; target->removeFromParent();}),
                                         nullptr);
    
    runAction(seq);
    
    return true;
}

// 이렇게도 해보고
bool WasteBasketActionNode::initWithParam2(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    setContentSize(cocos2d::Size(80, 80));
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    // shadow
    auto shadow = ShapeSolidCircle::create();
    shadow->setContentSize(cocos2d::Size(90, 90));
    shadow->setAntiAliasWidth(20);
    shadow->setPosition(-5, -10);
    shadow->setColor4F(cocos2d::Color4F(0, 0, 0, .15));
    addChild(shadow);
    
    // bg
    auto bg = ShapeSolidCircle::create();
    bg->setContentSize(cocos2d::Size(80, 80));
    bg->setColor4F(MAKE_COLOR4F(0xFF5825, .8));
    addChild(bg);
    
    auto icon1 = cocos2d::Sprite::create("images/delete_top.png");  // 뚜껑
    auto icon2 = cocos2d::Sprite::create("images/delete_body.png"); // 쓰레기통
    icon1->setAnchorPoint(cocos2d::Vec2(8/50.0, (50-13)/50.0));
    icon1->setPosition(40-(25-8), 40+(25-13));
    icon2->setPosition(40, 40);
    addChild(icon1);
    addChild(icon2);
    
    setPosition(from);
    setScaleX(-1);
    
    // 1) 이동
    auto move = cocos2d::EaseSineInOut::create(cocos2d::MoveTo::create(0.1, to));
    auto scale = cocos2d::ScaleTo::create(0.1, -1.2, 1.2);
    auto step1 = cocos2d::Spawn::create(move, scale, nullptr);
    
    // 2) 뚜껑 (스프라이트)
    auto open = cocos2d::EaseIn::create(cocos2d::RotateTo::create(0.05, -30), 1.0);
    auto close = cocos2d::RotateTo::create(0.05, 0);
    auto step2 = cocos2d::Sequence::create(
                                           cocos2d::DelayTime::create(0.1),
                                           open,
                                           cocos2d::DelayTime::create(0.5),
                                           close,
                                           nullptr);
    icon1->runAction(step2);
    
    // 종료
    auto exit = cocos2d::Spawn::create(cocos2d::EaseIn::create(cocos2d::ScaleTo::create(EXIT_DURATION, -0.7, 0.7), 3.0),
                                       cocos2d::FadeTo::create(EXIT_DURATION, 0),
                                       nullptr);
    
    
    auto seq = cocos2d::Sequence::create(step1,
                                         cocos2d::DelayTime::create(0.35), // delay for genie
                                         exit,
                                         cocos2d::CallFuncN::create([this](Node* target) { _removeSelfOnExit = true; target->removeFromParent();}),
                                         nullptr);
    
    runAction(seq);

    return true;
}

// 저렇게도 해보고
bool WasteBasketActionNode::initWithParam3(cocos2d::Node *parent, const cocos2d::Vec2 &from, const cocos2d::Vec2 &to)
{
    setContentSize(cocos2d::Size(80, 80));
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    // shadow
    auto shadow = ShapeSolidCircle::create();
    shadow->setContentSize(cocos2d::Size(90, 90));
    shadow->setAntiAliasWidth(20);
    shadow->setPosition(-5, -10);
    shadow->setColor4F(cocos2d::Color4F(0, 0, 0, .15));
    addChild(shadow);
    
    // bg
    auto bg = ShapeSolidCircle::create();
    bg->setContentSize(cocos2d::Size(80, 80));
    bg->setColor4F(MAKE_COLOR4F(0xFF5825, .8));
    addChild(bg);
    
    auto icon1 = cocos2d::Sprite::create("images/delete_top.png");  // 뚜껑
    auto icon2 = cocos2d::Sprite::create("images/delete_body.png"); // 쓰레기통
    icon1->setAnchorPoint(cocos2d::Vec2(8/50.0, (50-13)/50.0));
    icon1->setPosition(40-(25-12), 40+(25));
    icon2->setPosition(40, 40);
    addChild(icon1);
    addChild(icon2);
    
    setPosition(from);
    
    auto scale = cocos2d::ScaleTo::create(0.1, 1.2);
    auto open = cocos2d::EaseIn::create(cocos2d::RotateTo::create(OPEN_DURATION, -30), 1.0);
    auto close = cocos2d::RotateTo::create(0.05, 0);
    auto step2 = cocos2d::Sequence::create(
                                           open,
                                           cocos2d::DelayTime::create(0.4),
                                           close,
                                           nullptr);
    icon1->runAction(step2);

    auto exit = cocos2d::Spawn::create(cocos2d::EaseIn::create(cocos2d::ScaleTo::create(EXIT_DURATION, 0.7), 3.0),
                                       cocos2d::FadeTo::create(EXIT_DURATION, 0),
                                       nullptr);
    
    
    auto seq = cocos2d::Sequence::create(scale, //step1,
                                         cocos2d::DelayTime::create(0.8), // delay for genie
                                         exit,
                                         cocos2d::CallFuncN::create([this](Node* target) { _removeSelfOnExit = true; target->removeFromParent();}),
                                         nullptr);
    
    runAction(seq);
    
    return true;
}
































