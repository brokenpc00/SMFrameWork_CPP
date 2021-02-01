//
//  SceneTransition.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "SceneTransition.h"
#include <base/CCDirector.h>
#include <2d/CCAction.h>
#include <2d//CCActionInterval.h>
#include <2d/CCActionInstant.h>
#include <2d/CCActionEase.h>
#include "../Base/SMScene.h"
#include "../Base/ViewAction.h"
#include "../Base/ShaderNode.h"
#include <base/CCEventDispatcher.h>

using namespace SceneTransition;

const float Time::NORMAL = 0.30f;
const float Time::FAST   = 0.20f;
const float Time::SLOW   = 0.50f;

#define DEFAULT_DELAY_TIME (0.1f)

class BaseSceneTransition::ProgressUpdater : public ViewAction::DelayBaseAction {
    
public:
    static ProgressUpdater* create(float t) {
        auto action = new (std::nothrow)ProgressUpdater();
        if (action) {
            action->setTimeValue(t, 0);
            action->autorelease();
        }
        
        return action;
    }
    
    virtual void onUpdate(float t) override {
        ((BaseSceneTransition*)_target)->updateProgress(t);
    }
    
    virtual void onEnd() override {
        ((BaseSceneTransition*)_target)->updateComplete();
    }
};

//---------------------------------------------------------------------------
// Base Scene Transition
//---------------------------------------------------------------------------
BaseSceneTransition::BaseSceneTransition() : _dimLayer(nullptr), _lastProgress(-1), _menuDrawStub(nullptr) {
}

BaseSceneTransition::~BaseSceneTransition() {
    CC_SAFE_RELEASE(_dimLayer);
}

cocos2d::Scene* BaseSceneTransition::getOutScene() const {
    return _outScene;
}

void BaseSceneTransition::updateProgress(const float progress) {
    if (_lastProgress != progress) {
        if (_isInSceneOnTop) {
            if (_inScene) {
                ((SMScene*)_inScene)->onTransitionProgress(SMScene::Transition::Transition_IN, getTag(), progress);
            }
            if (_outScene) {
                ((SMScene*)_outScene)->onTransitionProgress(SMScene::Transition::Transition_PAUSE, getTag(), progress);
            }
        } else {
            if (_inScene) {
                ((SMScene*)_inScene)->onTransitionProgress(SMScene::Transition::Transition_RESUME, getTag(), progress);
            }
            if (_outScene) {
                ((SMScene*)_outScene)->onTransitionProgress(SMScene::Transition::Transition_OUT, getTag(), progress);
            }
        }
        _lastProgress = progress;
    }
}

void BaseSceneTransition::updateComplete() {
    if (_isInSceneOnTop) {
    ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_PAUSE, getTag());
    ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_IN, getTag());
            _inScene->onEnterTransitionDidFinish();
    } else {
    ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_OUT, getTag());
    ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_RESUME, getTag());
            _inScene->onEnterTransitionDidFinish();
    }
}


void BaseSceneTransition::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) {
    Scene::draw(renderer, transform, flags);
    
    if (isDimLayerEnable() && _lastProgress > 0 && _dimLayer == nullptr) {
        _dimLayer = ShapeSolidRect::create();
        _dimLayer->retain();
        _dimLayer->setContentSize(_director->getWinSize());
        _dimLayer->setColor(cocos2d::Color3B::BLACK);
        _dimLayer->setOpacity(0);
    }
    
    if(_isInSceneOnTop) {
        // new scene enter
        _outScene->visit(renderer, transform, flags);
        
        if (_menuDrawStub && _menuDrawType == ON_OFF) {
            _menuDrawStub->setVisible(true);
            _menuDrawStub->visit(renderer, transform, flags);
        }
        
        if (_lastProgress > 0.0 && _lastProgress < 1.0 &&_dimLayer) {
            _dimLayer->setOpacity((GLubyte)(0.4 * 255.0 * _lastProgress));
            _dimLayer->visit(renderer, transform, flags);
        }
        
        _inScene->visit(renderer, transform, flags);
        
        if (_menuDrawStub && _menuDrawType == OFF_ON) {
            _menuDrawStub->setVisible(true);
            _menuDrawStub->visit(renderer, transform, flags);
        }

    } else {
        //top scene exit
        _inScene->visit(renderer, transform, flags);
        
        if (_menuDrawStub && _menuDrawType == OFF_ON) {
            _menuDrawStub->setVisible(true);
            _menuDrawStub->visit(renderer, transform, flags);
        }
        
        if (_lastProgress > 0.0 && _lastProgress < 1.0 && _dimLayer) {
            _dimLayer->setOpacity((GLubyte)(0.4 * 255.0 * (1.0 - _lastProgress)));
            _dimLayer->visit(renderer, transform, flags);
        }
        _outScene->visit(renderer, transform, flags);
        
        if (_menuDrawStub && _menuDrawType == ON_OFF) {
            _menuDrawStub->setVisible(true);
            _menuDrawStub->visit(renderer, transform, flags);
        }
    }
}

void BaseSceneTransition::onEnter() {
    TransitionScene::onEnter();
    
    bool iMenu = ((SMScene*)_inScene)->isMainMenuEnabled();
    bool oMenu = ((SMScene*)_outScene)->isMainMenuEnabled();
    
    if (oMenu) {
        if (iMenu) {
            _menuDrawType = MenuDrawType::ON_ON;
        } else {
            _menuDrawType = MenuDrawType::ON_OFF;
        }
    } else {
        if (iMenu) {
            _menuDrawType = MenuDrawType::OFF_ON;
        } else {
            _menuDrawType = MenuDrawType::OFF_OFF;
        }
    }
    
//    if (_menuDrawType == MenuDrawType::ON_OFF || _menuDrawType == OFF_ON) {
//        _menuDrawStub = cocos2d::Node::create();
//        auto menuBar = MainMenuBar::getInstance();
//        ViewUtil::adoptionTo(menuBar, _menuDrawStub);
//        _menuDrawStub->retain();
//
//        if (_menuDrawType == OFF_ON) {
//            menuBar->setVisible(true);
//        }
//    }
    
    auto in = getInAction();
    auto out = getOutAction();
    
    if (!in) {
        in = cocos2d::DelayTime::create(_duration);
    }
    
    if (_isInSceneOnTop) {
        // 새 scene 들어옴
        auto seq = cocos2d::Sequence::create(
                                             cocos2d::DelayTime::create(DEFAULT_DELAY_TIME),
                                             in,
                                             nullptr);
        _inScene->runAction(seq);
        if (out) {
            auto seq = cocos2d::Sequence::create(
                                                 cocos2d::DelayTime::create(DEFAULT_DELAY_TIME),
                                                 out,
                                                 nullptr);
            _outScene->runAction(seq);
        }
        
        runAction(
                  cocos2d::Sequence::create(cocos2d::DelayTime::create(DEFAULT_DELAY_TIME),
                                            ProgressUpdater::create(_duration),
                                            cocos2d::CallFunc::create(CC_CALLBACK_0(TransitionScene::finish, this)),
                                            nullptr));
        
    } else {
        // 기존 scene 나감
        _inScene->runAction(in);
        
        if (out) {
            _outScene->runAction(out);
        }
        
        runAction(
                  cocos2d::Sequence::create(
                                            ProgressUpdater::create(_duration),
                                            cocos2d::CallFunc::create(CC_CALLBACK_0(TransitionScene::finish, this)),
                                            nullptr));
    }
    
    if (!isNewSceneEnter()) {
        auto outScene = (SMScene*)_outScene;
        auto inScene = (SMScene*)_inScene;
        inScene->onSceneResult(outScene, outScene->getSceneResult());
    }
    
    if (_isInSceneOnTop) {    
        ((SMScene*)_outScene)->onTransitionStart(SMScene::Transition::Transition_PAUSE, getTag());
    ((SMScene*)_inScene)->onTransitionStart(SMScene::Transition::Transition_IN, getTag());
    } else {
        ((SMScene*)_outScene)->onTransitionStart(SMScene::Transition::Transition_OUT, getTag());
        ((SMScene*)_inScene)->onTransitionStart(SMScene::Transition::Transition_RESUME, getTag());
    }
}


void BaseSceneTransition::onExit() {
    TransitionScene::onExit();
    
//    if (_menuDrawStub) {
//        auto sharedLayer = _director->getSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_UI_AND_POPUP);
//        auto menuBar = MainMenuBar::getInstance();
//        ViewUtil::adoptionTo(menuBar, sharedLayer);
//
//        _menuDrawStub->release();
//        _menuDrawStub = nullptr;
//
//        if (_menuDrawType == ON_OFF) {
//            menuBar->setVisible(false);
//        }
//    }
}



//---------------------------------------------------------------------------
// Swipe Back Transition
//---------------------------------------------------------------------------
SwipeBack::SwipeBack() : _isCanceled(false) {
}

SwipeBack::~SwipeBack() {
}

SwipeBack* SwipeBack::create(Scene* scene) {
    
    auto t = new (std::nothrow)SwipeBack();
    
    if (t && t->initWithDuration(0, scene)) {
        t->autorelease();
        return t;
    }
    
    CC_SAFE_DELETE(t);
    return nullptr;
}

void SwipeBack::updateProgress(const float progress) {
    if (_lastProgress != progress) {
        if (_isInSceneOnTop) {
            if (_inScene) {
                ((SMScene*)_inScene)->onTransitionProgress(SMScene::Transition::Transition_SWIPE_IN, getTag(), progress);
            }
            if (_outScene) {
                ((SMScene*)_outScene)->onTransitionProgress(SMScene::Transition::Transition_PAUSE, getTag(), progress);
            }
        } else {
            if (_inScene) {
                ((SMScene*)_inScene)->onTransitionProgress(SMScene::Transition::Transition_RESUME, getTag(), progress);
            }
            if (_outScene) {
                ((SMScene*)_outScene)->onTransitionProgress(SMScene::Transition::Transition_SWIPE_OUT, getTag(), progress);
            }
        }
        _lastProgress = progress;
    }
}

void SwipeBack::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) {
    float progress = _outScene->getPositionX() / _director->getWinSize().width;
    updateProgress(progress);
    
    if (_menuDrawStub) {
        if (_menuDrawType == MenuDrawType::ON_OFF) {
            _menuDrawStub->setPosition(_outScene->getPosition());
        } else if (_menuDrawType == MenuDrawType::OFF_ON) {
            _menuDrawStub->setPosition(_inScene->getPosition());
        }
    }
    
    BaseSceneTransition::draw(renderer, transform, flags);
}

void SwipeBack::cancel() {
    // 나갈 Scene이 다시 들어옴.
    _isCanceled = true;
    
    _outScene->setVisible(true);
    _outScene->setPosition(0,0);
    _outScene->setScale(1.0f);
    _outScene->setRotation(0.0f);
    _outScene->setAdditionalTransform(nullptr);
    _outScene->onEnterTransitionDidFinish();
    
    _inScene->setVisible(false);
    _inScene->setPosition(0,0);
    _inScene->setScale(1.0f);
    _inScene->setRotation(0.0f);
    _inScene->setAdditionalTransform(nullptr);
    
    this->schedule(CC_SCHEDULE_SELECTOR(SwipeBack::cancelNewScene), 0);
}

void SwipeBack::cancelNewScene(float dt) {
    this->unschedule(CC_SCHEDULE_SELECTOR(SwipeBack::cancelNewScene));
    
    cocos2d::Director::getInstance()->replaceScene(_inScene);
    cocos2d::Director::getInstance()->pushScene(_outScene);
    
    _inScene->setVisible(true);
}
    
void SwipeBack::onEnter() {
    TransitionScene::onEnter();
    _eventDispatcher->setEnabled(true);
    
    if (_isInSceneOnTop) {
        ((SMScene*)_outScene)->onTransitionStart(SMScene::Transition::Transition_PAUSE, getTag());
        ((SMScene*)_inScene)->onTransitionStart(SMScene::Transition::Transition_SWIPE_IN, getTag());
    } else {
        ((SMScene*)_outScene)->onTransitionStart(SMScene::Transition::Transition_SWIPE_OUT, getTag());
        ((SMScene*)_inScene)->onTransitionStart(SMScene::Transition::Transition_RESUME, getTag());
    }
    
    bool iMenu = ((SMScene*)_inScene)->isMainMenuEnabled();
    bool oMenu = ((SMScene*)_outScene)->isMainMenuEnabled();
    
    if (oMenu) {
        if (iMenu) {
            _menuDrawType = MenuDrawType::ON_ON;
        } else {
            _menuDrawType = MenuDrawType::ON_OFF;
        }
    } else {
        if (iMenu) {
            _menuDrawType = MenuDrawType::OFF_ON;
        } else {
            _menuDrawType = MenuDrawType::OFF_OFF;
        }
    }
    
//    if (_menuDrawType == MenuDrawType::ON_OFF || _menuDrawType == OFF_ON) {
//        _menuDrawStub = cocos2d::Node::create();
//        auto menuBar = MainMenuBar::getInstance();
//        ViewUtil::adoptionTo(menuBar, _menuDrawStub);
//        _menuDrawStub->retain();
//
//        if (_menuDrawType == OFF_ON) {
//            menuBar->setVisible(true);
//        }
//    }
}

void SwipeBack::onExit() {
    Scene::onExit();
    
    _eventDispatcher->setEnabled(true);
    if (_isCanceled) {
        if (_isInSceneOnTop) {
            ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_SWIPE_OUT, getTag());
            ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_RESUME, getTag());
            _outScene->onEnterTransitionDidFinish();
        } else {
            ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_RESUME, getTag());
            ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_SWIPE_OUT, getTag());
            _outScene->onEnterTransitionDidFinish();
        }
        _inScene->onExit();
    } else {
        if (_isInSceneOnTop) {
            ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_PAUSE, getTag());
            ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_SWIPE_IN, getTag());
            _inScene->onEnterTransitionDidFinish();
        } else {
            ((SMScene*)_outScene)->onTransitionComplete(SMScene::Transition::Transition_SWIPE_OUT, getTag());
            ((SMScene*)_inScene)->onTransitionComplete(SMScene::Transition::Transition_RESUME, getTag());
            _inScene->onEnterTransitionDidFinish();
        }
        _outScene->onExit();
    }
    
//    if (_menuDrawStub) {
//        auto sharedLayer = _director->getSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_UI_AND_POPUP);
//        auto menuBar = MainMenuBar::getInstance();
//        ViewUtil::adoptionTo(menuBar, sharedLayer);
//
//        _menuDrawStub->release();
//        _menuDrawStub = nullptr;
//
//        if (_isCanceled) {
//            if (_menuDrawType == ON_OFF) {
//                menuBar->setVisible(true);
//            } else if (_menuDrawType == OFF_ON) {
//                menuBar->setVisible(false);
//            }
//        } else {
//            if (_menuDrawType == ON_OFF) {
//                menuBar->setVisible(false);
//            } else if (_menuDrawType == OFF_ON) {
//                menuBar->setVisible(true);
//            }
//        }
//    }
}

//---------------------------------------------------------------------------
// Swipe Dismiss Transition
//---------------------------------------------------------------------------
SwipeDismiss* SwipeDismiss::create(Scene* scene) {
    
    auto t = new (std::nothrow)SwipeDismiss();
    
    if (t && t->initWithDuration(0, scene)) {
        t->autorelease();
        return t;
    }
    
    CC_SAFE_DELETE(t);
    return nullptr;
}

void SwipeDismiss::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) {
    _lastProgress = -_outScene->getPositionY() / _director->getWinSize().height;
    
    if (_menuDrawStub) {
        if (_menuDrawType == MenuDrawType::ON_OFF) {
            _menuDrawStub->setPosition(_outScene->getPosition());
        } else if (_menuDrawType == MenuDrawType::OFF_ON) {
            _menuDrawStub->setPosition(_inScene->getPosition());
        }
    }
    
    BaseSceneTransition::draw(renderer, transform, flags);
}

//---------------------------------------------------------------------------
// SlideInToTop
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* CrossFade::getInAction() {
    _inScene->setOpacity(0);
    return cocos2d::FadeIn::create(_duration);
}

cocos2d::FiniteTimeAction* CrossFade::getOutAction() {
    return cocos2d::FadeOut::create(_duration);
}

//---------------------------------------------------------------------------
// SlideInToTop
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* SlideInToTop::getInAction() {
    _inScene->setPosition(0, -_director->getWinSize().height);
    return cocos2d::EaseCubicActionOut::create(cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0)));
}

//---------------------------------------------------------------------------
// SlideOutToBottom
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* SlideOutToBottom::getOutAction() {
    return cocos2d::EaseSineOut::create(cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, -_director->getWinSize().height)));
}

//---------------------------------------------------------------------------
// SlideInToLeft
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* SlideInToLeft::getInAction() {
    _inScene->setPosition(_director->getWinSize().width, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::OFF_ON) {
        _menuDrawStub->setPosition(_inScene->getPosition());
    }

    auto a = ViewAction::TransformAction::create();
    a->toPositionX(0).setTweenFunc(cocos2d::tweenfunc::Cubic_EaseOut);
    a->setTimeValue(_duration, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::OFF_ON) {
        a->setUpdateCallback([this](cocos2d::Node* target, int tag, float t) {
            _menuDrawStub->setPosition(target->getPosition());
        });
    }
    return a;
    
//    return cocos2d::EaseCubicActionOut::create(cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0)));
}

cocos2d::FiniteTimeAction* SlideInToLeft::getOutAction() {
    auto a = ViewAction::TransformAction::create();
    a->toPositionX(-_director->getWinSize().width * 0.3);
    a->setTimeValue(_duration, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::ON_OFF) {
        a->setUpdateCallback([this](cocos2d::Node* target, int tag, float t) {
            _menuDrawStub->setPosition(target->getPosition());
        });
    }
    return a;

//    return cocos2d::MoveTo::create(_duration, cocos2d::Vec2(-_director->getWinSize().width * 0.3, 0));
}

//---------------------------------------------------------------------------
// SlideOutToRight
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* SlideOutToRight::getOutAction() {
    auto a = ViewAction::TransformAction::create();
    a->toPositionX(_director->getWinSize().width).setTweenFunc(cocos2d::tweenfunc::Sine_EaseOut);
    a->setTimeValue(_duration, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::ON_OFF) {
        a->setUpdateCallback([this](cocos2d::Node* target, int tag, float t) {
            _menuDrawStub->setPosition(target->getPosition());
        });
    }
    return a;

//    return cocos2d::EaseSineOut::create(cocos2d::MoveTo::create(_duration, cocos2d::Vec2(_director->getWinSize().width, 0)));
}

cocos2d::FiniteTimeAction* SlideOutToRight::getInAction() {
    _inScene->setPosition(-_director->getWinSize().width * 0.3, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::OFF_ON) {
        _menuDrawStub->setPosition(_inScene->getPosition());
    }
    
    auto a = ViewAction::TransformAction::create();
    a->toPositionX(0);
    a->setTimeValue(_duration, 0);
    if (_menuDrawStub && _menuDrawType == MenuDrawType::OFF_ON) {
        a->setUpdateCallback([this](cocos2d::Node* target, int tag, float t) {
            _menuDrawStub->setPosition(target->getPosition());
        });
    }
    return a;
    
//    return cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
}

//---------------------------------------------------------------------------
// EditMainIn
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* EditMainIn::getInAction() {
    //    _inScene->setOpacity(0);
    
    auto hold = cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
    return hold;
    //    auto fadeIn = cocos2d::FadeIn::create(_duration/2);
    //    auto spawn = cocos2d::Spawn::create(hold, fadeIn, nullptr);
    //
    //    return spawn;
}

cocos2d::FiniteTimeAction* EditMainIn::getOutAction() {
    auto s = _director->getWinSize();
    _outScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    auto scaleTo = cocos2d::ScaleTo::create(_duration, 1.3);
    auto fadeOut = cocos2d::FadeOut::create(_duration);
    auto spawn = cocos2d::Spawn::create(scaleTo, fadeOut, nullptr);
    
    return cocos2d::EaseCircleActionOut::create(spawn);
}

//---------------------------------------------------------------------------
// EditMainOut
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* EditMainOut::getOutAction() {
    return cocos2d::FadeOut::create(_duration);
}

cocos2d::FiniteTimeAction* EditMainOut::getInAction() {
    
    auto s = _director->getWinSize();
    _inScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _inScene->setScale(1.3);
    _inScene->setOpacity(0);
    
    auto scaleTo = cocos2d::ScaleTo::create(_duration, 1.0);
    auto fadeIn = cocos2d::FadeIn::create(_duration);
    auto spawn = cocos2d::Spawn::create(scaleTo, fadeIn, nullptr);
    
    return cocos2d::EaseCircleActionOut::create(spawn);
}

void EditMainOut::onExit() {
    BaseSceneTransition::onExit();
    
    _inScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
    _inScene->setPosition(cocos2d::Vec2::ZERO);
}

//---------------------------------------------------------------------------
// EditSubInOut
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* EditSubInOut::getInAction() {
    auto hold = cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
    return hold;
}

cocos2d::FiniteTimeAction* EditSubInOut::getOutAction() {
    auto hold = cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
    return hold;
}


//---------------------------------------------------------------------------
// SignComplete
//---------------------------------------------------------------------------
cocos2d::FiniteTimeAction* SignComplete::getInAction() {
    auto s = _director->getWinSize();
    _inScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _inScene->setScale(1.5);
    _inScene->setOpacity(0);
    
    auto scaleTo = cocos2d::ScaleTo::create(_duration, 1.0);
    auto fadeIn = cocos2d::FadeIn::create(_duration);
    auto spawn = cocos2d::Spawn::create(scaleTo, fadeIn, nullptr);
    
    return cocos2d::EaseCubicActionOut::create(spawn);
}

cocos2d::FiniteTimeAction* SignComplete::getOutAction() {
    auto s = _director->getWinSize();
    _outScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    auto scaleTo = cocos2d::ScaleTo::create(_duration, 0.8);
    auto fadeOut = cocos2d::FadeOut::create(_duration);
    auto spawn = cocos2d::Spawn::create(scaleTo, fadeOut, nullptr);
    
    return cocos2d::EaseCubicActionOut::create(spawn);
}

cocos2d::FiniteTimeAction* FadeIn::getInAction() {
    auto hold = cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
    return hold;
}

cocos2d::FiniteTimeAction* FadeIn::getOutAction() {
    auto scaleTo = cocos2d::ScaleTo::create(_duration, 1.0f);
    auto fadeOut = cocos2d::FadeOut::create(_duration);
    auto spawn = cocos2d::Spawn::create(scaleTo, fadeOut, nullptr);
    
    return cocos2d::EaseCircleActionOut::create(spawn);
}


cocos2d::FiniteTimeAction* FadeOut::getInAction() {
    _inScene->setScale(1);
    _inScene->setOpacity(0xFF);
    
    auto hold = cocos2d::MoveTo::create(_duration, cocos2d::Vec2(0, 0));
    return hold;
}

cocos2d::FiniteTimeAction* FadeOut::getOutAction() {
    auto s = _director->getWinSize();
    _outScene->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    auto fadeTo = cocos2d::FadeTo::create(_duration, 0);
    return cocos2d::EaseCubicActionOut::create(fadeTo);
}
