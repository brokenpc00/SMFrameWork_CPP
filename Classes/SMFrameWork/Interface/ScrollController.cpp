//
//  ScrollController.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 7..
//
//

#include <base/CCDirector.h>
#include "ScrollController.h"
#include "Dynamics.h"
#include "../Util/ViewUtil.h"
#include <math.h>
#include <cocos2d.h>

//#define SCROLLING_STIFFNESS 100.0f
#define SCROLLING_STIFFNESS 150.0f
//#define SCROLLING_STIFFNESS 200.0f

#define SCROLLING_DAMPING 1.0f
//#define SCROLLING_DAMPING 1.3f

const float ScrollController::REST_VELOCITY_TOLERANCE = 0.5f;
const float ScrollController::REST_POSITION_TOLERANCE = 0.5f;
const float ScrollController::PAN_OUTSIDE_SNAP_FACTOR = 0.2f;

ScrollController::ScrollController() :
_panMinY(0),
_panMaxY(0),
_panY(0),
_needUpdate(false),
_viewSize(0),
_scrollSize(0),
_hangSize(0)
{
    _panDynamicsY = new Dynamics();
    
    reset();
}

ScrollController::~ScrollController()
{
    delete _panDynamicsY;
    
}


void ScrollController::reset()
{
    _panDynamicsY->reset();
    _panDynamicsY->setFriction(3.0f);

    // bouncing... 조절...
    _panDynamicsY->setSpring(SCROLLING_STIFFNESS, SCROLLING_DAMPING);
    
    _panY = 0.0f;
    updateLimits();
    
    _needUpdate = false;
}

float ScrollController::getPanY() const
{
    return _panY;
}

void ScrollController::setPanY(const float panY, const bool force)
{
    _panY = panY;
    
    if (force) {
        _panDynamicsY->setState(_panY, 0, cocos2d::Director::getInstance()->getGlobalTime());
    }
}

void ScrollController::setViewSize(const float viewSize)
{
    _viewSize = viewSize;
    
    updateLimits();
}

void ScrollController::setScrollSize(const float scrollSize)
{
    _scrollSize = scrollSize;
    
    updateLimits();
}

void ScrollController::pan(float dy)
{
    if ((getPanY() > _panMaxY && dy > 0) || (getPanY() < _panMinY && dy < 0)) {
        dy *= PAN_OUTSIDE_SNAP_FACTOR;
    }
    
    float newPanY = getPanY() + dy;
    
    setPanY(newPanY);
}

bool ScrollController::update()
{
    if (_needUpdate) {
        float nowTime = cocos2d::Director::getInstance()->getGlobalTime();
        _panDynamicsY->update(nowTime);
        
        bool isAtRest =
        _panDynamicsY->isAtRest(REST_VELOCITY_TOLERANCE, REST_POSITION_TOLERANCE);
        
        setPanY(_panDynamicsY->getPosition());
        
        if (isAtRest) {
            stopFling();
        }
    }
    
    return _needUpdate;
}

void ScrollController::startFling(float vy)
{
    float now = cocos2d::Director::getInstance()->getGlobalTime();
    
    if ((vy < 0 && _panY < _panMinY) || (vy > 0 && _panY > _panMaxY)) {
        vy /= 5;
    }
    
    _panDynamicsY->setState(getPanY(), vy, now);
    
    _panDynamicsY->setMinPosition(_panMinY-_hangSize);
    _panDynamicsY->setMaxPosition(_panMaxY);
    
    _needUpdate = true;
}

void ScrollController::stopFling()
{
    _needUpdate = false;
}

void ScrollController::updatePanLimits()
{
    _panMinY = 0;
    _panMaxY = std::max(0.0f, _scrollSize - _viewSize);
}

void ScrollController::updateLimits()
{
    updatePanLimits();
}

bool ScrollController::isPanning() const
{
    return _needUpdate;
}

void ScrollController::stopIfExceedLimit()
{
    _panDynamicsY->setMinPosition(_panMinY-_hangSize);
    _panDynamicsY->setMaxPosition(_panMaxY);
    
    if ((getPanY() > _panMaxY) || (getPanY() < _panMinY)) {
        startFling(0);
    }
}

void ScrollController::setHangSize(float size)
{
    _hangSize = size;
    _panDynamicsY->setMinPosition(_panMinY-_hangSize);
}
