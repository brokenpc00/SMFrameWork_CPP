//
//  SMScroller.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#include "SMScroller.h"
#include "../Util/ViewUtil.h"
#include "../Base/Intent.h"
#include "../Base/SMView.h"
#include "../Const/SMViewConstValue.h"
#include <base/CCDirector.h>
#include "VelocityTracker.h"
#include "ScrollController.h"
#include <2d/CCTweenFunction.h>
#include <cmath>
#include <cocos2d.h>

#define SCROLL_TIME (0.2f)
#define GRAVITY     (9.8f*1000)
#define _SIGNUM(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#define SPEED_LOW   (100.0)


static bool interpolate(float& from, float to, float tolerance) {
    if (from != to) {
        from = from + (to-from) / 3;//1.5;
        if (std::abs(from-to) < tolerance) {
            from = to;
            return false; // done
        }
        return true; // still need update
    }
    return false;
}


static inline float decPrecesion(float value) {
#ifdef DECREASE_PRECESION
    return ((int)(ViewUtil::round(value * 100)))/100.0f;
#else
    return value;
#endif
}

static inline float decPrecesion2(float value) {
    return ((int)(ViewUtil::round(value * 100)))/100.0f;
}


_ScrollProtocol::_ScrollProtocol() :
_scrollParent(nullptr),
_scroller(nullptr),
_velocityTracker(nullptr),
_innerScrollMargin(0),
_minScrollSize(0),
_baseScrollPosition(0),
_inScrollEvent(false),
_tableRect(nullptr),
_scrollRect(nullptr)
{
}

_ScrollProtocol::~_ScrollProtocol()
{
    CC_SAFE_DELETE(_scroller);
    CC_SAFE_DELETE(_velocityTracker);
    CC_SAFE_DELETE(_tableRect);
    CC_SAFE_DELETE(_scrollRect);
}

SMScroller* _ScrollProtocol::getScroller()
{
    return _scroller;
}

void _ScrollProtocol::setScrollParent(_ScrollProtocol* parent)
{
    _scrollParent = parent;
}

void _ScrollProtocol::setTableRect(cocos2d::Rect* tableRect)
{
    if (tableRect) {
        if (_tableRect) {
            *_tableRect = *tableRect;
        } else {
            _tableRect = new cocos2d::Rect(*tableRect);
        }
    } else {
        CC_SAFE_DELETE(_tableRect);
    }
}

void _ScrollProtocol::setScrollRect(cocos2d::Rect* scrollRect)
{
    if (scrollRect) {
        if (_scrollRect) {
            *_scrollRect = *scrollRect;
        } else {
            _scrollRect = new cocos2d::Rect(*scrollRect);
        }
    } else {
        CC_SAFE_DELETE(_scrollRect);
    }
}



// SMScroller abstract class

SMScroller::SMScroller() :
_scrollSpeed(0)
{
    _cellSize = 0;
    _scrollMode = ScrollMode::BASIC;
    onAlignCallback = nullptr;
    reset();
}


SMScroller::~SMScroller()
{
    
}

void SMScroller::reset()
{
    _state = State::STOP;
    _position = _newPosition = 0;
    _minPosition = 0;
    _maxPosition = 0;
    _hangSize = 0;
}

void SMScroller::setWindowSize(const float windowSize)
{
    _windowSize = windowSize;
    _maxPosition = windowSize;
    
    if (_windowSize <= 0) {
        _windowSize = 1;
    }
}

void SMScroller::setScrollMode(int scrollMode)
{
    _scrollMode = scrollMode;
}

void SMScroller::setScrollSize(const float scrollSize)
{
    if (scrollSize > _windowSize) {
        _maxPosition = _minPosition + scrollSize - _windowSize + _hangSize;
    } else {
        _maxPosition = _minPosition;
    }
}

void SMScroller::setScrollPosition(const float position, bool immediate)
{
    if (immediate) {
        _newPosition = _position = position;
    } else {
        _newPosition = position;
    }
    _state = State::STOP;
}

bool SMScroller::update()
{
    bool updated = false;
    
    updated |= runScroll();
    updated |= runFling();
    updated |= SMView::smoothInterpolate(_position, _newPosition-_hangSize, SMViewConstValue::Config::TOLERANCE_POSITION);
    
    return updated;
}

void SMScroller::clone(SMScroller* scroller)
{
    this->_state = scroller->_state;
    this->_position =  scroller->_position;
    this->_newPosition = scroller->_newPosition;
    
    this->_windowSize = scroller->_windowSize;
    this->_minPosition = scroller->_minPosition;
    this->_maxPosition = scroller->_maxPosition;
    
    this->_timeStart = scroller->_timeStart;
    this->_timeDuration = scroller->_timeDuration;
    
    this->_startPos = scroller->_startPos;
    this->_velocity = scroller->_velocity;
    this->_accelate = scroller->_accelate;
    this->_touchDistance = scroller->_touchDistance;
    
    this->_hangSize = scroller->_hangSize;
}

bool SMScroller::isTouchable()
{
    return (_state == State::STOP) || (_scrollSpeed < SPEED_LOW);
}

int SMScroller::getMaxPageNo()
{
    int maxPageNo;
    
    //    maxPageNo = (int)(getScrollSize() / _cellSize);
    //    maxPageNo = (int)(0.1 + getScrollSize() / _cellSize);
    maxPageNo = ceilf(getScrollSize() / _cellSize);
    
    return maxPageNo;
}


// FlexibleScroller

FlexibleScroller::FlexibleScroller()
{
    _controller = new ScrollController();
    reset();
}

FlexibleScroller::~FlexibleScroller()
{
    if (_controller) {
        delete _controller;
    }
}

void FlexibleScroller::reset()
{
    SMScroller::reset();
    _controller->reset();
    _position = 0;
    _newPosition = 0;
    _scrollSpeed = 0;
    _autoScroll = false;
}

bool FlexibleScroller::update()
{
    bool updated = _controller->update();
    updated |= onAutoScroll();
    
    _newPosition = decPrecesion(_controller->getPanY());
    
    updated |= interpolate(_position, _newPosition, 0.1);
    
    _scrollSpeed = std::abs(_lastPosition - _position) * 60;
    
    _lastPosition = _position;
    
    if (!updated) {
        _state = State::STOP;
    } else {
        _state = State::SCROLL;
    }
    
    return updated;
}

void FlexibleScroller::setHangSize(const float size)
{
    _controller->setHangSize(size);
}

void FlexibleScroller::setWindowSize(const float windowSize)
{
    SMScroller::setWindowSize(windowSize);
    _controller->setViewSize(windowSize);
}

void FlexibleScroller::setScrollSize(const float scrollSize)
{
    SMScroller::setScrollSize(scrollSize);
    _controller->setScrollSize(scrollSize);
}

void FlexibleScroller::setScrollPosition(const float position, bool immediate)
{
    SMScroller::setScrollPosition(position, immediate);
    _controller->setPanY(position);
}

float FlexibleScroller::getScrollPosition() const
{
//    return _controller->getPanY();
    return _position;
}

void FlexibleScroller::justAtLast()
{
    // 마지막에 limit를 초과하는지 검사
    _controller->stopIfExceedLimit();
}

void FlexibleScroller::onTouchDown(const int unused)
{
    _autoScroll = false;
    _controller->stopFling();
}

void FlexibleScroller::onTouchUp(const int unused)
{
    _controller->startFling(0);
}

void FlexibleScroller::onTouchScroll(const float delta, const int unused)
{
    _controller->pan(-delta);
}

void FlexibleScroller::onTouchFling(const float velocity, const int unused)
{
    float v = -velocity;
    
    _controller->startFling(v);
}

bool FlexibleScroller::onAutoScroll()
{
    if (!_autoScroll) {
        return false;
    }
    
    float globalTime = cocos2d::Director::getInstance()->getGlobalTime();
    float nowTime = globalTime - _timeStart;
    if(nowTime > _timeDuration) {
        nowTime = _timeDuration;
        _autoScroll = false;
    }
    
    float oldPosition = _newPosition;
    float distance =  _velocity * nowTime + 0.5 * _accelate * nowTime * nowTime;
    float newPosition = decPrecesion(_startPos + distance);
    float dir = _SIGNUM(_velocity);
    
    float maxScrollWindowSize = _windowSize * 0.9;
    if (std::abs(newPosition - oldPosition) > maxScrollWindowSize) {

        // 한꺼번에 확확 넘어가는걸 방지
        // 한 프레임에 스크롤이 화면의 90%가 넘지 않도록 계산
        // 2차 방정식 근의 공식 Ax^2+Bx+C=0.
        //  x=-b+or-sqrt(b^2-4*a*c)/2*a

        // x = now
        // a = 0.5*_accelate
        // b = -_velocity
        // c = newDistance
        float newPosition2 = oldPosition + dir * maxScrollWindowSize;
        float newDistance = newPosition2 - _startPos;
        
        float a = 0.5f*_accelate;
        float b = -_velocity;
        float c = newDistance;
        
        double discriminant = b*b-4*a*c;
        float newNowTime;
        bool bUserMath = true;
        if (bUserMath) {
            if (discriminant > 0)
            {
                newNowTime=((-b)+sqrt(discriminant))/(2*a);
            } else  if (discriminant==0) {
                newNowTime=(-b)/(2*a);
            } else {
                newNowTime=((-b)-sqrt(discriminant))/(2*a);
            }
        } else {
            // ???
            newNowTime = (float)(-b - dir*std::sqrt(b*b - 4*a*c)) / (2*a);
        }

        _timeStart = globalTime - newNowTime;
        newPosition = newPosition2;
        newPosition = decPrecesion(newPosition);
    }
    
    if (newPosition > _maxPosition) {
        _controller->setPanY(_maxPosition);
    } else if (newPosition < _minPosition) {
        _controller->setPanY(_minPosition);
    } else {
        _controller->setPanY(newPosition, true);
    }
    
    return true;
}

void FlexibleScroller::scrollTo(float position)
{
    scrollToWithDuration(position, -1);
}

void FlexibleScroller::scrollToWithDuration(float position, float duration)
{
    if (position < _minPosition) {
        position = _minPosition;
    }
    
    if (std::abs(position - _newPosition) < 1)
        return;
    
    onTouchDown();
    
    const float PIXELS_PER_SEC = 20000.0;
    
    float dist = position - _newPosition;
    
    float dir = _SIGNUM(dist);
    
    if (duration <= 0) {
        _timeDuration = std::min(1.5f, std::max(0.15f, std::abs(dist) / PIXELS_PER_SEC));
    } else {
        _timeDuration = duration;
    }
    
    _accelate = -dir*GRAVITY;

    // decelerate
    // duration 동안 감속
    _velocity = -_accelate * _timeDuration;
    
    
    float dist0 =  _velocity * _timeDuration + 0.5 * _accelate * _timeDuration * _timeDuration;
    float scale = dist / dist0;
    
    _velocity *= scale;
    _accelate *= scale;
    
    _startPos = _newPosition;
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    _autoScroll = true;
}




// page scroller

PageScroller::PageScroller() :
_bounceBackEnabled(true)
{
    
}

PageScroller::~PageScroller()
{
    
}

bool PageScroller::update()
{
    bool updated = _controller->update();
    updated |= runScroll();
    
    _newPosition = decPrecesion2(_controller->getPanY());
    
    if (!_bounceBackEnabled) {
        if (_newPosition < 0) {
            _newPosition = 0;
            _controller->setPanY(0);
        } else if (_newPosition > getScrollSize()) {
            _newPosition = getScrollSize();
            _controller->setPanY(getScrollSize());
        }
    }
    
    updated |= interpolate(_position, _newPosition, 0.1);
    
    _scrollSpeed = std::abs(_lastPosition - _position) * 60;
    
    _lastPosition = _position;
    
    return updated;
}

void PageScroller::reset()
{
    SMScroller::reset();
    _controller->reset();
    _position = 0;
    _newPosition = 0;
}

void PageScroller::onTouchUp(const int unused)
{
    float page = _newPosition / _cellSize;

    int maxPageNo = getMaxPageNo();
    if (page < 0) {
        page = 0;
    } else if (page > maxPageNo) {
        page = maxPageNo;
    } else {
        int iPage = std::floor(page);
        float offset = page - iPage;
        
        if (offset <= 0.5) {
            page = iPage;
        } else {
            page = iPage + 1;
        }
    }
 
    _startPos = _newPosition;
    _stopPos = page * _cellSize;
    
    if ((_startPos <= 0 && page == 0) || (_startPos >= _cellSize * maxPageNo && page == maxPageNo) || _startPos == _stopPos) {
        // 첫페이지 또는 마지막 페이지 초기화
        
        _controller->startFling(0);
        
        if (pageChangedCallback) {
            pageChangedCallback(page);
        }
        
        return;
    }
    
    // 페이지 스크롤
    _state = State::SCROLL;
    
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    float distance = std::abs(_startPos - _stopPos);
    _timeDuration = 0.05 +  0.15 * (1.0 - distance / _cellSize);
}

void PageScroller::onTouchFling(const float velocity, const int currentPage)
{
    float v = velocity;
    const float maxVelocity = 15000;
    
    if (std::abs(velocity) > maxVelocity) {
        v = _SIGNUM(v) * maxVelocity;
    }
    
    float position = _newPosition;
    if ((int)position < (int)_minPosition || (int)position > (int)_maxPosition) {
        onTouchUp();
        return;
    }
    
    int maxPageNo = getMaxPageNo();
    
    int page;
    if (v < 0) {
        page = currentPage + 1;
    } else {
        page = currentPage - 1;
    }
    
    if (page < 0) {
        page = 0;
    } else if (page > maxPageNo) {
        page = maxPageNo;
    }
    
    _startPos = _newPosition;
    _stopPos = page * _cellSize;
    
    
    _state = State::SCROLL;
    
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    
    _timeDuration = 0.05 +  0.15 * (1.0 + std::abs(v) / maxVelocity);
}

bool PageScroller::runScroll()
{
    if (_state != State::SCROLL) {
        return false;
    }
    
    float dt = cocos2d::Director::getInstance()->getGlobalTime()-_timeStart;
    float rt = (float)dt/_timeDuration;
    
    if (rt < 1) {
        float f = 1 - (float)std::sin(rt * M_PI_2);
        float newPosition = decPrecesion2(_stopPos + f * (_startPos - _stopPos));
        _controller->setPanY(newPosition);
    } else {
        _state = State::STOP;
        _controller->setPanY(_stopPos);
        
        if (pageChangedCallback) {
            pageChangedCallback(std::floor(_stopPos/_cellSize));
        }
    }
    
    return true;
}

void PageScroller::setCurrentPage(const int page, bool immediate)
{
    _newPosition = _cellSize * page;
    _controller->setPanY(_newPosition);
    
    if (immediate) {
        _position = _newPosition;
    }
}


// infinity scroller
InfinityScroller::InfinityScroller()
{
    
}

InfinityScroller::~InfinityScroller()
{
    
}

void InfinityScroller::reset()
{
    SMScroller::reset();
}

bool InfinityScroller::runScroll()
{
    if (_state != State::SCROLL) {
        return false;
    }
    
    float dt = cocos2d::Director::getInstance()->getGlobalTime()-_timeStart;
    float t = (float)dt/_timeDuration;
    
    if (t < 1) {
        t = cocos2d::tweenfunc::cubicEaseOut(t);
        _newPosition = decPrecesion(ViewUtil::interpolation(_startPos, _stopPos, t));
    } else {
        _state = State::STOP;
        _newPosition = _stopPos;
        
        if (_scrollMode != BASIC) {
            if (onAlignCallback) {
                onAlignCallback(true);
            }
        }
        return false;
    }
    
    return true;
}

bool InfinityScroller::runFling()
{
    if (_state != State::FLING) {
        return false;
    }
    
    float globalTime = cocos2d::Director::getInstance()->getGlobalTime();
    float nowTime = globalTime - _timeStart;
    if(nowTime > _timeDuration) {
        _newPosition = _stopPos;
        _state = State::STOP;
        nowTime = _timeDuration;
        // STOP
        if (_scrollMode != BASIC) {
            if (onAlignCallback) {
                onAlignCallback(true);
            }
        }
        return false;
    }
    
    float distance = _velocity * nowTime + 0.5 * _accelate * nowTime * nowTime;
    _newPosition = decPrecesion(_startPos + distance);
    
    return true;
}

void InfinityScroller::onTouchDown(const int unused)
{
    _state = State::STOP;
    _startPos = _newPosition;
    _touchDistance = 0;
}

void InfinityScroller::onTouchUp(const int unused)
{
    _state = State::STOP;
    
    if (_scrollMode != BASIC) {
        // 멈출때 가장 근접한 position으로
        float r = ::fmodf(_newPosition, _cellSize);
        float distance;
        
        if (std::abs(r) <= _cellSize/2) {
            distance = -r;
        } else {
            if (r >= 0) distance = _cellSize - r;
            else distance = -(_cellSize + r);
        }
        if (distance == 0) {
            if (onAlignCallback) {
                onAlignCallback(true);
            }
            return;
        }
        
        _state = State::FLING;
        float dir = _SIGNUM(distance);
        _accelate = +dir*GRAVITY;
        _startPos = _newPosition;
        _timeDuration = std::abs(distance) * 0.25 / 1000;
        
        _startPos = _newPosition;
        _velocity = distance / _timeDuration - 0.5 * _accelate * _timeDuration;
        _stopPos = _startPos + distance;
    }
}

void InfinityScroller::onTouchScroll(const float delta, const int unused)
{
    _touchDistance -= delta;
    _newPosition = decPrecesion(_startPos + _touchDistance);
    
    if (_scrollMode != BASIC) {
        if (onAlignCallback) {
            onAlignCallback(false);
        }
    }
}

void InfinityScroller::onTouchFling(const float velocity, const int currentPage)
{
    float dir = _SIGNUM(velocity);
    float v0 = std::abs(velocity);
    
    const float maxVelocity = 25000;
    v0 = std::min(maxVelocity, v0);
    
    // 멈추는 예정 시간
    _timeDuration = v0 / GRAVITY;
    
    _state = State::FLING;
    _startPos = _newPosition;
    
    _velocity = -dir*v0;
    _accelate = +dir*GRAVITY;
    
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    float distance = _velocity * _timeDuration + 0.5 * _accelate * _timeDuration * _timeDuration;
    
    if (_scrollMode == PAGER) {
        if (std::abs(distance) > _cellSize) {
            distance = ViewUtil::signum(distance) * _cellSize;
        }
    }
    
    _stopPos = _startPos + distance;
    
    if (_scrollMode != BASIC) {
        // 멈출때 가까운 position
        float r = ::fmodf(_stopPos, _cellSize);
        if (std::abs(r) <= _cellSize/2) {
            if (r >= 0) distance -= r;
            else distance += - r;
        } else {
            if (r >= 0) distance += _cellSize - r;
            else distance -= _cellSize + r;
        }
        
        _velocity = distance / _timeDuration - 0.5 * _accelate * _timeDuration;
        _stopPos = _startPos + distance;
    }
}

void InfinityScroller::scrollByWithDuration(float distance, float duration)
{
    _state = State::SCROLL;
    _stopPos = _startPos + distance;
    
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    _timeDuration = duration;
    
    if (_scrollMode != BASIC) {
        if (onAlignCallback) {
            onAlignCallback(false);
        }
    }
}



// finity scroller

FinityScroller::FinityScroller()
{
    
}

FinityScroller::~FinityScroller()
{
    
}

bool FinityScroller::update()
{
    bool updated = _controller->update();
    updated |= runScroll();
    updated |= runFling();
    
    _newPosition = decPrecesion2(_controller->getPanY());
    
    updated |= interpolate(_position, _newPosition, 0.1);
    
    _scrollSpeed = std::abs(_lastPosition - _position) * 60;
    
    _lastPosition = _position;
    
    return updated;
}

void FinityScroller::reset()
{
    SMScroller::reset();
    _controller->reset();
    _position = 0;
    _newPosition = 0;
}

bool FinityScroller::runScroll()
{
    if (_state != State::SCROLL) {
        return false;
    }
    
    float dt = cocos2d::Director::getInstance()->getGlobalTime()-_timeStart;
    float t = (float)dt/_timeDuration;
    
    if (t < 1) {
        t = cocos2d::tweenfunc::cubicEaseOut(t);
        _newPosition = decPrecesion(ViewUtil::interpolation(_startPos, _stopPos, t));
        _controller->setPanY(_newPosition);
    } else {
        _state = State::STOP;
        _newPosition = _stopPos;
        _controller->setPanY(_stopPos);
        
        if (onAlignCallback) {
            onAlignCallback(true);
        }
        return false;
    }
    
    return true;
}

bool FinityScroller::runFling()
{
    if (_state != State::FLING) {
        return false;
    }
    
    float globalTime = cocos2d::Director::getInstance()->getGlobalTime();
    float nowTime = globalTime - _timeStart;
    if(nowTime > _timeDuration) {
        _state = State::STOP;
        
        _newPosition = _stopPos;
        _controller->setPanY(_stopPos);
        // STOP
        if (onAlignCallback) {
            onAlignCallback(true);
        }
        return false;
    }
    
    float distance = _velocity * nowTime + 0.5 * _accelate * nowTime * nowTime;
    _newPosition = decPrecesion(_startPos + distance);
    _controller->setPanY(_newPosition);
    return true;
}

void FinityScroller::onTouchUp(const int unused)
{
    float page = _newPosition / _cellSize;
    int maxPageNo = getMaxPageNo();
    
    if (page < 0) {
        page = 0;
    } else if (page > maxPageNo) {
        page = maxPageNo;
    } else {
        int iPage = std::floor(page);
        float offset = page - iPage;
        
        if (offset <= 0.5) {
            page = iPage;
        } else {
            page = iPage + 1;
        }
    }
    
    _startPos = _newPosition;
    _stopPos = page * _cellSize;
    
    if ((_startPos <= 0 && page == 0) || (_startPos >= _cellSize * maxPageNo && page == maxPageNo) || _startPos == _stopPos) {
        _controller->startFling(0);
        
        if (onAlignCallback) {
            onAlignCallback(true);
        }
        
        return;
    }
    
    _state = State::SCROLL;
    
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    float distance = std::abs(_startPos - _stopPos);
    _timeDuration = 0.05 +  0.35 * (1.0 - distance / _cellSize);
}

void FinityScroller::onTouchFling(const float velocity, const int currentPage)
{
    float dir = _SIGNUM(velocity);
    float v0 = std::abs(velocity);
    
    const float maxVelocity = 25000;
    v0 = std::min(maxVelocity, v0);
    
    _startPos = _newPosition;
    
    // 멈추는 예정 시간
    _timeDuration = v0 / GRAVITY;
    _velocity = -dir*v0;
    _accelate = +dir*GRAVITY;
    _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
    
    float distance = _velocity * _timeDuration + 0.5 * _accelate * _timeDuration * _timeDuration;
    
    if (_startPos + distance < _minPosition) {
        if (_startPos <= 0) {
            _state = State::STOP;
            _controller->startFling(0);
            if (onAlignCallback) {
                onAlignCallback(true);
            }
            return;
        }
        distance = _minPosition - _startPos;
        
        _state = State::SCROLL;
        _stopPos = _minPosition;
        _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
        _timeDuration = 0.25;
        return;
    } else if (_startPos + distance > _maxPosition) {
        if (_startPos >= _maxPosition) {
            _state = State::STOP;
            _controller->startFling(0);
            if (onAlignCallback) {
                onAlignCallback(true);
            }
            return;
        }
        distance = _maxPosition - _startPos;
        
        _state = State::SCROLL;
        _stopPos = _maxPosition;
        _timeStart = cocos2d::Director::getInstance()->getGlobalTime();
        _timeDuration = 0.25;
        return;
    }
    
    _state = State::FLING;
    _stopPos = _startPos + distance;
    
    if (_scrollMode != BASIC) {
        // 멈출때 가까운 Position
        float r = ::fmodf(_stopPos, _cellSize);
        if (std::abs(r) <= _cellSize/2) {
            if (r >= 0) distance -= r;
            else distance += - r;
        } else {
            if (r >= 0) distance += _cellSize - r;
            else distance -= _cellSize + r;
        }
        
        _velocity = distance / _timeDuration - 0.5 * _accelate * _timeDuration;
        _stopPos = _startPos + distance;
    }
}

void FinityScroller::setWindowSize(const float windowSize)
{
    SMScroller::setWindowSize(windowSize);
    _controller->setViewSize(_cellSize);
}

void FinityScroller::setScrollSize(const float scrollSize)
{
    _maxPosition = _minPosition + scrollSize - _cellSize;
    _controller->setScrollSize(scrollSize);
    _controller->setViewSize(_cellSize);
}
