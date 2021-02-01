//
//  ViewAction.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "ViewAction.h"

#include <2d/CCActionEase.h>
#include <2d/CCTweenFunction.h>
#include <cmath>
#include <algorithm>
#include <cocos2d.h>
#include <2d/CCGrid.h>
#include <2d/CCNodeGrid.h>

using namespace ViewAction;

/*
 RingWave* RingWave::show(cocos2d::Node* parent, float x, float y, float size, float duration, float delay) {
 
 auto view = new RingWave();
 
 if (view && view->initWithParam(size, duration, delay)) {
 view->autorelease();
 
 if (parent) {
 parent->addChild(view);
 view->setPosition(x, y);
 }
 } else {
 CC_SAFE_DELETE(view);
 view = nullptr;
 }
 
 return view;
 }
 */


class WaveCircleAction : public cocos2d::ActionInterval {
public:
    WaveCircleAction() {}
    virtual ~WaveCircleAction() {}
    
    static WaveCircleAction* create(float duration, ShapeNode* shape, float size) {
        
        auto action = new WaveCircleAction();
        
        if (action && action->initWithDuration(duration)) {
            action->autorelease();
            action->_shape = shape;
            action->_size = size;
        }
        
        return action;
    }
    
    virtual void update(float t) override {
        
        float r1 = (float)(_size * std::sin(t * M_PI_2));
        float r2 = (float)(_size * (1.0 - std::cos(t * M_PI_2)));
        
        float d = r1 - r2;
        float a = (float)(std::sin(t * M_PI));
        
        _shape->setOpacity(0.7*a*0xFF);
        _shape->setContentSize(cocos2d::Size(r1, r1));
        _shape->setLineWidth(d / 4);
        
        if (t >= 1) {
            _target->removeFromParentAndCleanup(true);
        }
    }
    
private:
    ShapeNode* _shape;
    float _size;
};


RingWave* RingWave::show(cocos2d::Node* parent, float x, float y, float size, float duration, float delay, cocos2d::Color4F* color, OnRingWaveComplete ringWaveComplete) {
    
    auto view = new RingWave();
    
    if (view && view->initWithParam(size, duration, delay, color)) {
        view->autorelease();
        view->_ringWaveComplete = ringWaveComplete;
        if (parent) {
            parent->addChild(view);
            view->setPosition(x, y);
        }
    } else {
        CC_SAFE_DELETE(view);
        view = nullptr;
    }
    
    return view;
}

RingWave::RingWave() : _circle(nullptr), _ringWaveComplete(nullptr) {
}

RingWave::~RingWave() {
}

bool RingWave::initWithParam(float size, float duration, float delay, cocos2d::Color4F* color) {
    
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    _circle = ShapeCircle::create();
    
    addChild(_circle);
    
    _circle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _circle->setPosition(0, 0);
    
    if (color) {
        _circle->setColor4F(*color);
    }
    _circle->setOpacity(0);
    
    cocos2d::Action* action = nullptr;
    
    auto wave = cocos2d::EaseOut::create(WaveCircleAction::create(duration, _circle, size), 2.0);
    if (delay > 0) {
        action = cocos2d::Sequence::create(cocos2d::DelayTime::create(delay),
                                           wave,
                                           cocos2d::CallFunc::create([&]{
                                                if (_ringWaveComplete) {
                                                    _ringWaveComplete();
                                                }
                                            }),
                                           nullptr);
    } else {
        action = cocos2d::Sequence::create(wave, cocos2d::CallFunc::create([&]{
            if (_ringWaveComplete) {
                _ringWaveComplete();
            }
        }), NULL);
//        action = wave;
    }
    
    runAction(action);
    
    return true;
}



//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Transform Action
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
TransformAction::TransformAction() :
_scaleAction(false),
_positionXAction(false),
_positionYAction(false),
_alphaAction(false),
_rotateAction(false),
_removeOnFinish(false),
_enableOnFinish(false),
_disableOnFinish(false),
_invisibleOnFinish(false),
_callbackOnFinish(nullptr),
_updateCallback(nullptr),
_action(nullptr),
_easingParam(0),
_smooth(false),
_view(nullptr),
_tweenType(cocos2d::tweenfunc::TweenType::Circ_EaseOut)
{
}

TransformAction::~TransformAction() {
    CC_SAFE_RELEASE(_action);
}

void TransformAction::onStart() {
    if (_scaleAction) {
        _fromScale = _target->getScale();
    }
    if (_rotateAction) {
        _fromAngle = _target->getRotation();
    }
    if (_positionXAction || _positionYAction) {
        _fromPosition = _target->getPosition();
    }
    if (_alphaAction) {
        _fromAlpha = (float)_target->getOpacity() / 0xFF;
    }
    _target->setVisible(true);
    if (_smooth) {
        _view = dynamic_cast<SMView*>(_target);
    }
}

void TransformAction::onUpdate(float t) {
    if (_tweenType != cocos2d::tweenfunc::TweenType::Linear) {
        t = cocos2d::tweenfunc::tweenTo(t, _tweenType, &_easingParam);
    }
    
    if (_scaleAction) {
        float scale = ViewUtil::interpolation(_fromScale, _toScale, t);
        _target->setScale(scale);
    }
    if (_rotateAction) {
        float angle = ViewUtil::interpolation(_fromAngle, _toAngle, t);
        _target->setRotation(angle);
    }
    if (_alphaAction) {
        float alpha = ViewUtil::interpolation(_fromAlpha, _toAlpha, t);
        alpha = std::max(.0f, std::min(1.0f, alpha));
        _target->setOpacity((GLubyte)(0xFF*alpha));
    }
    if (_positionXAction && _positionYAction) {
        float x = ViewUtil::interpolation(_fromPosition.x, _toPosition.x, t);
        float y = ViewUtil::interpolation(_fromPosition.y, _toPosition.y, t);
        if (_view) {
            _view->setPosition(x, y, false);
        } else {
        _target->setPosition(x, y);
    }
    } else if (_positionXAction) {
        float x = ViewUtil::interpolation(_fromPosition.x, _toPosition.x, t);
        if (_view) {
            _view->setPositionX(x, false);
        } else {
            _target->setPositionX(x);
        }
    } else if (_positionYAction) {
        float y = ViewUtil::interpolation(_fromPosition.y, _toPosition.y, t);
        if (_view) {
            _view->setPositionY(y, false);
        } else {
            _target->setPositionY(y);
        }
    }
    if (_updateCallback) {
        _updateCallback(_target, getTag(), t);
    }
};

void TransformAction::onEnd() {
    if (_enableOnFinish) {
        auto view = dynamic_cast<SMView*>(_target);
        if (view) {
            view->setEnabled(true);
        }
    }
    if (_disableOnFinish) {
        auto view = dynamic_cast<SMView*>(_target);
        if (view) {
            view->setEnabled(false);
        }
    }
    if (_invisibleOnFinish) {
        _target->setVisible(false);
    }
    if (_removeOnFinish) {
        if (_target->getParent()) {
            _target->removeFromParent();
        }
    }
    if (_callbackOnFinish) {
        _callbackOnFinish(getTarget(), getTag());
    }
    if (_action) {
        _target->runAction(_action);
        CC_SAFE_RELEASE_NULL(_action);
    }
}

TransformAction& TransformAction::toScale(float scale) {
    _scaleAction = true;
    _toScale = scale;
    return *this;
}

TransformAction& TransformAction::toPositionX(float positionX) {
    _positionXAction = true;
    _toPosition = cocos2d::Vec2(positionX, 0);
    return *this;
}

TransformAction& TransformAction::toPositionY(float positionY) {
    _positionYAction = true;
    _toPosition = cocos2d::Vec2(0, positionY);
    return *this;
}

TransformAction& TransformAction::toPosition(const cocos2d::Vec2& position) {
    _positionXAction = true;
    _positionYAction = true;
    _toPosition = position;
    return *this;
}

TransformAction& TransformAction::toAlpha(const float alpha) {
    _alphaAction = true;
    _toAlpha = alpha;
    return *this;
}

TransformAction& TransformAction::removeOnFinish() {
    _removeOnFinish = true;
    return *this;
}

TransformAction& TransformAction::enableOnFinish() {
    _enableOnFinish = true;
    return *this;
}

TransformAction& TransformAction::enableSmooth() {
    _smooth = true;
    return *this;
}

TransformAction& TransformAction::disableOnFinish() {
    _disableOnFinish = true;
    return *this;
}

TransformAction& TransformAction::invisibleOnFinish() {
    _invisibleOnFinish = true;
    return *this;
}

TransformAction& TransformAction::runFuncOnFinish(std::function<void(cocos2d::Node* target, int tag)> callback) {
    _callbackOnFinish = callback;
    return *this;
}

TransformAction& TransformAction::runActionOnFinish(cocos2d::Action* action) {
    _action = action;
    if (_action) {
        _action->retain();
    }
    return *this;
}

TransformAction& TransformAction::setUpdateCallback(std::function<void(cocos2d::Node* target, int tag, float t)> callback) {
    _updateCallback = callback;
    return *this;
}


ViewAction::TransformAction& ViewAction::TransformAction::setTweenFunc(cocos2d::tweenfunc::TweenType type, float easingParam) {
    _tweenType = type;
    _easingParam = easingParam;
    return *this;
}





//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Rung Wave 2
//-------------------------------------------------------------------------------------
class RingWave2::WaveAction : public cocos2d::ActionInterval {
public:
    
    static WaveAction* create(float duration, float minRadius, float maxRadius) {
        auto action = new WaveAction();
        if (action && action->initWithDuration(duration)) {
            action->autorelease();
            action->_minRadius = minRadius;
            action->_maxRadius = maxRadius;
        }
        
        return action;
    }
    
    virtual void update(float t) override {
        auto ring = (ShapeCircle*)_target;
        
        float d = _maxRadius - _minRadius;
        float outR = _minRadius + d * cocos2d::tweenfunc::cubicEaseOut(t);
        float inR = _minRadius + d * cocos2d::tweenfunc::cubicEaseIn(t);
        ring->setContentSize(cocos2d::Size(outR*2, outR*2));
        ring->setLineWidth(outR-inR);
        
        float a = 1.0 - cocos2d::tweenfunc::sineEaseIn(t);
        ring->setOpacity((GLubyte)(0xFF*a));
        
    }
    
private:
    float _minRadius;
    float _maxRadius;
};

RingWave2* RingWave2::create(float minRadius, float maxRadius) {
    auto view = new RingWave2();
    if (view && view->initWithParam(minRadius, maxRadius)) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
        view = nullptr;
    }
    
    return view;
}

bool RingWave2::initWithParam(float minRadius, float maxRadius) {
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setCascadeColorEnabled(true);
    
    _ring = ShapeCircle::create();
    _ring->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_ring);
    
    auto action = cocos2d::RepeatForever::create(cocos2d::Sequence::create(WaveAction::create(.6, minRadius, maxRadius), cocos2d::DelayTime::create(0.1), nullptr));
    _ring->runAction(action);
    
    runAction(cocos2d::FadeIn::create(0.2));
    
    return true;
}

void RingWave2::hide() {
    auto action = TransformAction::create();
    action->toAlpha(0).removeOnFinish();
    action->setTimeValue(0.5, 0);
    runAction(action);
}

PageTurnAction::PageTurnAction() : _directionLeft(true)
, _forward(true)
{
    
}

PageTurnAction::~PageTurnAction()
{
    
}

PageTurnAction * PageTurnAction::create(float duration, const cocos2d::Size &gridSize, bool directionLeft, bool forward)
{
    auto action = new (std::nothrow)PageTurnAction();
    if (action && action->initWithDuration(duration, gridSize)) {
        action->_directionLeft = directionLeft;
        action->_forward = forward;
        action->autorelease();
        return action;
    }
    
    CC_SAFE_DELETE(action);
    return nullptr;
}

PageTurnAction * PageTurnAction::clone() const
{
    return PageTurnAction::create(_duration, _gridSize, _directionLeft, _forward);
}

cocos2d::GridBase * PageTurnAction::getGrid()
{
    auto result = cocos2d::Grid3D::create(_gridSize, _gridNodeTarget->getGridRect());
    if (result!=nullptr) {
        result->setNeedDepthTestForBlit(true);
    }
    
    return result;
}

void PageTurnAction::update(float time)
{
    float tt = MAX(0, time - 0.25f);
    float deltaAy = (tt * tt * 500);
    float ay = -100 - deltaAy;
    
    float deltaTheta = sqrtf(time);
    float theta = deltaTheta > 0.5f ? (float)M_PI_2*deltaTheta : (float)M_PI_2*(1-deltaTheta);
    
    float rotateByYAxis = (2-time)* M_PI;
    
    float sinTheta = sinf(theta);
    float cosTheta = cosf(theta);
    
    for (int i = 0; i <= _gridSize.width; ++i)
    {
        for (int j = 0; j <= _gridSize.height; ++j)
        {
            // Get original vertex
            cocos2d::Vec3 p = getOriginalVertex(cocos2d::Vec2(i ,j));
            
            p.x -= getGridRect().origin.x;
            float R = sqrtf((p.x * p.x) + ((p.y - ay) * (p.y - ay)));
            float r = R * sinTheta;
            float alpha = asinf( p.x / R );
            float beta = alpha / sinTheta;
            float cosBeta = cosf( beta );
            
            // If beta > PI then we've wrapped around the cone
            // Reduce the radius to stop these points interfering with others
            if (beta <= M_PI)
            {
                p.x = ( r * sinf(beta));
            }
            else
            {
                // Force X = 0 to stop wrapped
                // points
                p.x = 0;
            }
            
            p.y = ( R + ay - ( r * (1 - cosBeta) * sinTheta));
            
            // We scale z here to avoid the animation being
            // too much bigger than the screen due to perspective transform
            
            p.z = (r * ( 1 - cosBeta ) * cosTheta);// "100" didn't work for
            p.x = p.z * sinf(rotateByYAxis) + p.x * cosf(rotateByYAxis);
            p.z = p.z * cosf(rotateByYAxis) - p.x * sinf(rotateByYAxis);
            p.z/=7;
            //    Stop z coord from dropping beneath underlying page in a transition
            // issue #751
            if( p.z < 0.5f )
            {
                p.z = 0.5f;
            }
            
            // Set new coords
            p.x += getGridRect().origin.x;
            setVertex(cocos2d::Vec2(i, j), p);
            
        }
    }
}

CurtainAction::CurtainAction() : _pos(cocos2d::Vec2::ZERO)
{
    
}

CurtainAction::~CurtainAction()
{
    
}

CurtainAction * CurtainAction::create(float duration, const cocos2d::Size& gridSize, const cocos2d::Vec2 pos)
{
    auto action = new (std::nothrow)CurtainAction();
    if (action && action->initWithDuration(duration, gridSize)) {
        action->autorelease();
        action->_pos = pos;
        return action;
    }
    
    CC_SAFE_DELETE(action);
    return nullptr;
}

cocos2d::GridBase* CurtainAction::getGrid()
{
    auto result = cocos2d::Grid3D::create(_gridSize, _gridNodeTarget->getGridRect());
    if (result!=nullptr) {
        result->setNeedDepthTestForBlit(true);
    }
    
    return result;
}

CurtainAction * CurtainAction::clone() const
{
    return CurtainAction::create(_duration, _gridSize, _pos);
}

void CurtainAction::update(float time)
{

    
    
    cocos2d::Vec2 np = cocos2d::Vec2(_pos.x - _pos.x*time, _pos.y);
    
    CCLOG("[[[[[ np : %f", np.x);
    
    const float Frills = 3;
    
    for (int i = 0; i <= _gridSize.width; ++i)
    {
        for (int j = 0; j <= _gridSize.height; ++j)
        {
            cocos2d::Vec3 p = getOriginalVertex(cocos2d::Vec2(i ,j));
            
            float dy = p.y - np.y;
            float bend = 0.25f * (1.0f - expf(-dy * dy * 10.0f));
            float x = p.x;
            
            p.z = 0.1 + 0.1f * sin(-1.4f * cos(x * x * Frills * 2.0 * M_PI)) * (1.0 - np.x);
            p.x = (p.x) * np.x + p.x * bend * (1.0 - np.x);

            setVertex(cocos2d::Vec2(i, j), p);
        }
    }
    
//    CGPoint np = CGPointMake(point.x/boundsSize.width, point.y/boundsSize.height);
    
//    float tt = MAX(0, time - 0.25f);
//    float deltaAy = (tt * tt * 500);
//    float ay = -100 - deltaAy;
//
//    float deltaTheta = sqrtf(time);
//    float theta = deltaTheta > 0.5f ? (float)M_PI_2*deltaTheta : (float)M_PI_2*(1-deltaTheta);
//
//    float rotateByYAxis = (2-time)* M_PI;
//
//    float sinTheta = sinf(theta);
//    float cosTheta = cosf(theta);
//
//    for (int i = 0; i <= _gridSize.width; ++i)
//    {
//        for (int j = 0; j <= _gridSize.height; ++j)
//        {
//            // Get original vertex
//            cocos2d::Vec3 p = getOriginalVertex(cocos2d::Vec2(i ,j));
//
//            p.x -= getGridRect().origin.x;
//            float R = sqrtf((p.x * p.x) + ((p.y - ay) * (p.y - ay)));
//            float r = R * sinTheta;
//            float alpha = asinf( p.x / R );
//            float beta = alpha / sinTheta;
//            float cosBeta = cosf( beta );
//
//            // If beta > PI then we've wrapped around the cone
//            // Reduce the radius to stop these points interfering with others
//            if (beta <= M_PI)
//            {
//                p.x = ( r * sinf(beta));
//            }
//            else
//            {
//                // Force X = 0 to stop wrapped
//                // points
//                p.x = 0;
//            }
//
//            p.y = ( R + ay - ( r * (1 - cosBeta) * sinTheta));
//
//            // We scale z here to avoid the animation being
//            // too much bigger than the screen due to perspective transform
//
//            p.z = (r * ( 1 - cosBeta ) * cosTheta);// "100" didn't work for
//            p.x = p.z * sinf(rotateByYAxis) + p.x * cosf(rotateByYAxis);
//            p.z = p.z * cosf(rotateByYAxis) - p.x * sinf(rotateByYAxis);
//            p.z/=7;
//            //    Stop z coord from dropping beneath underlying page in a transition
//            // issue #751
//            if( p.z < 0.5f )
//            {
//                p.z = 0.5f;
//            }
//
//            // Set new coords
//            p.x += getGridRect().origin.x;
//            setVertex(cocos2d::Vec2(i, j), p);
//
//        }
//    }
}


//class CurtainAction : public cocos2d::Grid3DAction
//{
//public:
//    static CurtainAction * create(float duration, const cocos2d::Size& gridSize);
//    
//public:
//    virtual cocos2d::GridBase* getGrid() override;
//    virtual CurtainAction* clone() const override;
//    virtual void update(float t) override;
//    
//protected:
//    CurtainAction();
//    virtual ~CurtainAction();
//    };
