//
//  CircularProgress.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 1..
//
//  Material CircularProgressView (Android opensource)

#include "CircularProgress.h"
#include "../Base/ShaderNode.h"
#include "../Const/SMViewConstValue.h"
#include "../Util/ViewUtil.h"
#include <cmath>
#include <algorithm>

#define INDETERMINATE_ANIM_STEPS (3)
#define INDETERMINATE_ANIM_DURATION  (4.0)

const float CircularProgress::MIN_SWEEP = 15;
const float CircularProgress::MAX_SWEEP = 360.0 * (INDETERMINATE_ANIM_STEPS-1.0) / INDETERMINATE_ANIM_STEPS + MIN_SWEEP;

class CircularProgress::FrontEndExtendAction : public cocos2d::ActionInterval {
public:
    static FrontEndExtendAction* create(float duration, float fromAngle, float toAngle) {
        auto action = new FrontEndExtendAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        action->_fromAngle = fromAngle;
        action->_toAngle = toAngle;
        
        return action;
    }
    
    virtual void update(float t) override {
        auto c = static_cast<CircularProgress*>(_target);
        auto d = ViewUtil::getDecelateInterpolation(t);
        
        c->_indeterminateRotateOffset = ViewUtil::interpolation(_fromAngle, _toAngle, t);
        c->_indeterminateSweep = ViewUtil::interpolation(MIN_SWEEP, MAX_SWEEP, d);
        
        c->drawArc(c->_initialStartAngle + c->_startAngle + c->_indeterminateRotateOffset, c->_indeterminateSweep);
    }
    
private:
    float _fromAngle, _toAngle;
};

class CircularProgress::BackEndRetractAction : public cocos2d::ActionInterval {
public:
    static BackEndRetractAction* create(float duration, float start, float end, float fromAngle, float toAngle) {
        auto action = new BackEndRetractAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        action->_start = start;
        action->_end = end;
        action->_fromAngle = fromAngle;
        action->_toAngle = toAngle;
        
        return action;
    }
    
    virtual void update(float t) override {
        auto c = static_cast<CircularProgress*>(_target);
        auto d = ViewUtil::getDecelateInterpolation(t);
        
        c->_indeterminateRotateOffset = ViewUtil::interpolation(_fromAngle, _toAngle, t);
        c->_startAngle = ViewUtil::interpolation(_start, _end, d);
        c->_indeterminateSweep = MAX_SWEEP - c->_startAngle + _start;
        
        c->drawArc(c->_initialStartAngle + c->_startAngle + c->_indeterminateRotateOffset, c->_indeterminateSweep);
    }
    
private:
    float _fromAngle, _toAngle;
    float _start, _end;
};

//---------------------------------------------------------------------------------------------------
//
// Action for Determinate Progress
//
//---------------------------------------------------------------------------------------------------
#define DETERMINATE_ANIM_SYNC_DURATION (0.5)
#define DETERMINATE_ANIM_SWOOP_DURATION (3.0)


class CircularProgress::ProgressAction : public cocos2d::ActionInterval {
public:
    static ProgressAction* create(float duration) {
        auto action = new ProgressAction();
        action->initWithDuration(duration);
        action->autorelease();
        action->_startTime = cocos2d::Director::getInstance()->getGlobalTime();
        
        return action;
    }
    
    void setValue(float actualProgress, float currentProgress) {
        _actualProgress = actualProgress;
        _currentProgress = currentProgress;
    }
    
    void forceUpdate() {
        float elapsed = cocos2d::Director::getInstance()->getGlobalTime() - _startTime;
        float t = std::min(elapsed / getDuration(), 1.0f);
        
        update(t);
    }
    
    virtual void update(float t) override {
        auto c = static_cast<CircularProgress*>(_target);
        
        float p = ViewUtil::interpolation(_actualProgress, _currentProgress, t);
        if (p != c->_actualProgress) {
            c->_actualProgress = p;
            
            float sweepAngle = 360.0 * c->_actualProgress / c->_maxProgress;
            c->drawArc(c->_initialStartAngle + c->_startAngle, sweepAngle);
            
            if (c->_onProgressUpdateCallback) {
                c->_onProgressUpdateCallback(c, c->_actualProgress, c->_maxProgress);
            }
        }
    }
    
private:
    float _actualProgress, _currentProgress;
    float _startTime;
};

class CircularProgress::StartRotateAction : public cocos2d::ActionInterval {
public:
    static StartRotateAction* create(float duration) {
        auto action = new StartRotateAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        return action;
    }
    
    virtual void update(float t) override {
        auto c = static_cast<CircularProgress*>(_target);
        auto d = ViewUtil::getDecelateInterpolation(t, 2);
        
        c->_startAngle = ViewUtil::interpolation(0, 360.0, d);
        
        float sweepAngle = 360.0 * c->_actualProgress / c->_maxProgress;
        c->drawArc(c->_initialStartAngle + c->_startAngle, sweepAngle);
    }
    
};

//---------------------------------------------------------------------------------------------------
//
// Main Class
//
//---------------------------------------------------------------------------------------------------
CircularProgress::CircularProgress() :
_started(false),
_autoLineWidth(true),
_progressAction(nullptr),
_startRoteteAction(nullptr),
_indeterminateAction(nullptr),
_indeterminateRotateOffset(0.0),
_indeterminateSweep(0),
_initialStartAngle(0.0),
_startAngle(0),
_currentProgress(0.0),
_actualProgress(0.0),
_maxProgress(0.0),
_onProgressUpdateCallback(nullptr)
{
}

CircularProgress::~CircularProgress()
{
    CC_SAFE_RELEASE(_indeterminateAction);
    CC_SAFE_RELEASE(_startRoteteAction);
    CC_SAFE_RELEASE(_progressAction);
}

CircularProgress* CircularProgress::createDeterminate() {
    auto node = new CircularProgress();
    
    if (node && node->init(false)) {
        node->autorelease();
    }
    
    return node;
}

CircularProgress* CircularProgress::createIndeterminate() {
    auto node = new CircularProgress();
    
    if (node && node->init(true)) {
        node->autorelease();
    }
    
    return node;
}

bool CircularProgress::init(const bool isIndeterminate) {
    if (cocos2d::Node::init()) {
        
        _isIndeterminate = isIndeterminate;
        
        setCascadeColorEnabled(true);
        setCascadeOpacityEnabled(true);
        
        _circle = ShapeArcRing::create();
        _circle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _circle->setScale(-1.0, 1.0); // clockwise
        
        addChild(_circle);
        
        
        if (_isIndeterminate) {
            // auto start
            start();
        } else {
            _startAngle = 0.0;
            _actualProgress = 0.0;
            _initialStartAngle = 0.0;
        }
        
        return true;
    }
    
    return false;
}

void CircularProgress::setLineWidth(const float lineWidth) {
    _circle->setLineWidth(lineWidth);
    _autoLineWidth = false;
}


void CircularProgress::setContentSize(const cocos2d::Size& size) {
    cocos2d::Node::setContentSize(size);
    
    _circle->setContentSize(size);
    _circle->setPosition(size.width/2, size.height/2);
    
    if (_autoLineWidth) {
        _circle->setLineWidth((size.width + size.height) / 20);
    }
}

void CircularProgress::drawArc(float startAngle, float sweepAngle) {
    _circle->setRotation(startAngle);
    
    if (sweepAngle < 0) {
        sweepAngle = std::abs(std::max(sweepAngle, -360.0f));
        _circle->setRotation(startAngle - sweepAngle - 90.0);
    } else {
        sweepAngle = std::min(sweepAngle, 360.0f);
        _circle->setRotation(startAngle - 90.0);
    }
    
    _circle->setArcRatio(sweepAngle / 360.0);
}


cocos2d::FiniteTimeAction* CircularProgress::createIndeterminateAction(const int step) {
    
    float duration = INDETERMINATE_ANIM_DURATION / INDETERMINATE_ANIM_STEPS / 2.0;
    
    // Extending the front of the arc
    auto frontEndExtend = FrontEndExtendAction::create(duration,
                                                       step * 720.0 / INDETERMINATE_ANIM_STEPS, (step + 0.5) * 720.0 / INDETERMINATE_ANIM_STEPS);
    
    // Retracting the back end of the arc
    float start = -90.0 + step * (MAX_SWEEP - MIN_SWEEP);
    auto backEndRetract = BackEndRetractAction::create(duration, start, start + MAX_SWEEP - MIN_SWEEP,
                                                       (step + 0.5) * 720.0 / INDETERMINATE_ANIM_STEPS, (step + 1.0) * 720.0 / INDETERMINATE_ANIM_STEPS);
    
    return cocos2d::Sequence::createWithTwoActions(frontEndExtend, backEndRetract);
}


//---------------------------------------------------------------------------------------------------
void CircularProgress::setMaxProgress(const float maxProgress) {
    if (_isIndeterminate)
        return;
    
    CCASSERT(maxProgress > 0, "_maxProgress > 0");
    
    _maxProgress = maxProgress;
}

void CircularProgress::setProgress(const float currentProgress) {
    if (_isIndeterminate)
        return;
    
    CCASSERT(_maxProgress > 0, "setMaxProgress() first");
    
    if (!_started) {
        start();
    }
    float p = std::max(0.0f, std::min(currentProgress, _maxProgress));
    if (p == _currentProgress)
        return;
    _currentProgress = p;
    
    if (_progressAction) {
        if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS3)) {
            // 애니메이션 진행중이면 강제 업데이트
            _progressAction->forceUpdate();
            stopAction(_progressAction);
        }
    } else {
        _progressAction = ProgressAction::create(DETERMINATE_ANIM_SYNC_DURATION);
        _progressAction->retain();
        _progressAction->setTag(SMViewConstValue::Tag::ACTION_PROGRESS3);
    }
    
    _progressAction->setValue(_actualProgress, _currentProgress);
    runAction(_progressAction);
    
    float sweepAngle = 360.0 * _actualProgress / _maxProgress;
    drawArc(_initialStartAngle + _startAngle, sweepAngle);
}

void CircularProgress::setStartAngle(const float startAngle) {
    _initialStartAngle = startAngle;
}


void CircularProgress::start() {
    if (_isIndeterminate) {
        
        _indeterminateRotateOffset = 0.0;
        _indeterminateSweep = MIN_SWEEP;
        _startAngle = -90.0;
        
        if (_indeterminateAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS1)) {
                stopAction(_indeterminateAction);
            }
        } else {
            _indeterminateAction = cocos2d::RepeatForever::create(
                                                                  cocos2d::Sequence::create( /* INDETERMINATE_ANIM_STEPS */
                                                                                            createIndeterminateAction(0),
                                                                                            createIndeterminateAction(1),
                                                                                            createIndeterminateAction(2), nullptr));
            _indeterminateAction->setTag(SMViewConstValue::Tag::ACTION_PROGRESS1);
            _indeterminateAction->retain();
        }
        
        runAction(_indeterminateAction);
    } else {
        if (_startRoteteAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS2)) {
                stopAction(_startRoteteAction);
            }
        }
        if (_progressAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS3)) {
                stopAction(_progressAction);
            }
        }
        _started = true;
        _actualProgress = 0.0;
        
        if (!_startRoteteAction) {
            _startRoteteAction = StartRotateAction::create(DETERMINATE_ANIM_SWOOP_DURATION);
            _startRoteteAction->setTag(SMViewConstValue::Tag::ACTION_PROGRESS2);
            _startRoteteAction->retain();
        }
        
        runAction(_startRoteteAction);
        
        setProgress(0);
    }
}

void CircularProgress::stop() {
    if (_isIndeterminate) {
        if (_indeterminateAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS1)) {
                stopAction(_indeterminateAction);
            }
        }
    } else {
        if (_startRoteteAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS2)) {
                stopAction(_startRoteteAction);
            }
        }
        if (_progressAction) {
            if (getActionByTag(SMViewConstValue::Tag::ACTION_PROGRESS3)) {
                stopAction(_progressAction);
            }
        }
    }
}
