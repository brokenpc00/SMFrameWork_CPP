//
//  SMScene.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "SMScene.h"
#include "SMView.h"
#include "ViewAction.h"
#include "../Interface/MotionEvent.h"
#include <base/CCEventListenerTouch.h>
#include "Intent.h"
#include <algorithm>
#include <cocos2d.h>

SMScene::SMScene() :
_rootView(nullptr),
_sceneParam(nullptr),
_sceneResult(nullptr)
{
}

SMScene::~SMScene()
{
    CC_SAFE_RELEASE_NULL(_sceneParam);
    CC_SAFE_RELEASE_NULL(_sceneResult);
}

bool SMScene::initWithSceneParam(Intent* sceneParam, SwipeType type)
{
    
    auto size = _director->getWinSize();
    
    setContentSize(size);
    _rootView = SMView::create(0, 0, size.width, size.height);
    Scene::addChild(_rootView, 0, "");
    
    _swipeType = type;
    
    if (sceneParam != nullptr) {
        _sceneParam = sceneParam;
        _sceneParam->retain();
    }
    
    if (init()) {
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        auto keyboardListener = cocos2d::EventListenerKeyboard::create();
        keyboardListener->onKeyReleased = CC_CALLBACK_2(SMScene::onKeyReleased, this);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
#endif
        
        setCascadeOpacityEnabled(true);
        
        return true;
    }
    
    Scene::removeChild(_rootView);
    
    return false;
}

void SMScene::setRootView(SMView* newRootView) {
    if (newRootView == nullptr || newRootView == _rootView)
        return;
    
    Scene::removeChild(_rootView);
    Scene::addChild(newRootView, 0, "");
    _rootView = newRootView;
    
}


void SMScene::onKeyReleased(const cocos2d::EventKeyboard::KeyCode keyCode, const cocos2d::Event * event)
{
    //    if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE) {
    //        if (!onBackPressed()) {
    //            if (_director->getSceneStackCount() <= 1) {
    //                _director->end();
    //#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    //                exit(0);
    //#endif
    //            } else {
    //                // TODO : 기본 트랜지션 구현해야함.
    //                auto prevScene = _director->getPreviousScene();
    //                auto tansScene = cocos2d::TransitionFlipX::create(.5, prevScene);
    //                _director->popSceneWithTransition(tansScene);
    //            }
    //        }
    //    }
}

void SMScene::setSceneResult(Intent* intent)
{
    if (_sceneResult == intent)
        return;
    
    if (_sceneResult != nullptr) {
        CC_SAFE_RELEASE(_sceneResult);
    }
    
    _sceneResult = intent;
    _sceneResult->retain();
}

void SMScene::startScene(SMScene* scene)
{
    cocos2d::TransitionScene* transition = nullptr;
    
    switch (scene->getSwipeType()) {
        case MENU:
            break;
        case NONE:
        case BACK:
            transition = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
            break;
        case DISMISS:
            transition = SceneTransition::SlideInToTop::create(SceneTransition::Time::NORMAL, scene);
            break;
    }
    
    if (transition) {
        cocos2d::Director::getInstance()->pushScene(transition);
    }
}

void SMScene::finishScene(Intent* result)
{
    auto scene = cocos2d::Director::getInstance()->getPreviousScene();
    if (scene == nullptr)
        return;
    
    setSceneResult(result);
    
    cocos2d::TransitionScene* transition = nullptr;
    switch (_swipeType) {
        case MENU:
            // APP 종료해야함.
            CC_ASSERT(0);
            break;
        case NONE:
        case BACK:
            transition = SceneTransition::SlideOutToRight::create(SceneTransition::Time::NORMAL, scene);
            break;
        case DISMISS:
            transition = SceneTransition::SlideOutToBottom::create(SceneTransition::Time::NORMAL, scene);
            break;
    }
    transition->setTag(getTag());
    cocos2d::Director::getInstance()->popSceneWithTransition(transition);
    
}


void SMScene::onSceneResult(SMScene* scene, Intent* result)
{
}


bool SMScene::canSwipe(const cocos2d::Vec2& worldPoint, const SwipeType type)
{
    return true;
}

void SMScene::addChild(Node* child)
{
    _rootView->addChild(child);
}

void SMScene::addChild(Node* child, int localZOrder)
{
    _rootView->addChild(child, localZOrder);
}

void SMScene::addChild(Node* child, int localZOrder, int tag)
{
    _rootView->addChild(child, localZOrder, tag);
}

void SMScene::addChild(Node* child, int localZOrder, const std::string &name)
{
    _rootView->addChild(child, localZOrder, name);
}

cocos2d::Node* SMScene::getChildByTag(int tag) const
{
    return _rootView->getChildByTag(tag);
}

cocos2d::Node* SMScene::getChildByName(const std::string& name) const
{
    return _rootView->getChildByName(name);
}

void SMScene::removeChild(Node* child, bool cleanup)
{
    _rootView->removeChild(child, cleanup);
}

void SMScene::removeChildByTag(int tag, bool cleanup)
{
    _rootView->removeChildByTag(tag, cleanup);
}

void SMScene::removeChildByName(const std::string &name, bool cleanup)
{
    _rootView->removeChildByName(name, cleanup);
}

#define ACTION_TAG_TRANS (10)

void SMScene::setTransitionAnim(SMView* view, float offsetX, float offsetY, float duration, float delay, bool isIn)
{
    auto aa = view->getActionByTag(ACTION_TAG_TRANS);
    if (aa) {
        view->stopAction(aa);
    }
    
    float x = view->getPositionX();
    float y = view->getPositionY();
    
    float sx, sy, tx, ty;
    
    if (isIn) {
        sx = x + offsetX;
        sy = y + offsetY;
        tx = x;
        ty = y;
    } else {
        sx = x;
        sy = y;
        tx = x + offsetX;
        ty = y + offsetY;
    }
    view->setPosition(sx, sy);
    view->setEnabled(false);
    view->setVisible(true);
    
    if (isIn) {
        view->setOpacity(0);
    } else {
        view->setOpacity(0xFF);
    }
    
    auto action = ViewAction::TransformAction::create();
    action->setTag(ACTION_TAG_TRANS);
    
    action->toPosition(cocos2d::Vec2(tx, ty));
    action->setTweenFunc(cocos2d::tweenfunc::Cubic_EaseOut);
    if (isIn) {
        action->enableOnFinish();
        action->toAlpha(1);
    } else {
        action->toAlpha(0);
        action->invisibleOnFinish();
    }
    action->setTimeValue(duration, delay);
    
    view->runAction(action);
}
