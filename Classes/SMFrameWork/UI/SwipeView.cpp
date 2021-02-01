//
//  SwipeView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 27..
//
//

#include "SwipeView.h"

#include "../Interface/SMScroller.h"
#include "../Interface/VelocityTracker.h"
#include "../Util/ViewUtil.h"
#include "../Const/SMViewConstValue.h"

#define FLAG_SCROLL_UPDATE   (1<<0)

class SwipeView::SwipeContainer : public SMView {
    
public:
    CREATE_VIEW(SwipeContainer);
    
    virtual void setPositionX(float x, bool immediate = true) override {
        SMView::setPositionX(x, immediate);
        udateOpenState();
    }
    
protected:
    SwipeContainer() : _openState(0.0) {}
    
    virtual void onSmoothUpdate(const uint32_t flags, float dt) override {
        if (flags | VIEWFLAG_POSITION) {
            udateOpenState();
        }
    }
    
private:
    void udateOpenState() {
        auto parent = (SwipeView*)getParent();
        
        if (parent && parent->_swipeSize != 0.0) {
            float t = -_position.x / parent->_swipeSize;
            if (t < 0) t = 0;
            else if (t > 1) t = 1;
            t = std::abs(t);
            if (_openState != t) {
                _openState = t;
                ((SwipeView*)getParent())->onUpdateOpenState(t);
            }
        }
    }
    
    float _openState;
};


SwipeView::SwipeView() :
_swipeContainer(nullptr),
_backContainer(nullptr),
_scroller(nullptr),
_velocityTracker(nullptr),
_swipeSize(0),
_openState(0),
_fakeFlingDirection(0),
_isTouchFocused(false),
_listener(nullptr),
_swipeLock(false)
{
}

SwipeView::~SwipeView() {
    CC_SAFE_DELETE(_scroller);
    CC_SAFE_DELETE(_velocityTracker);
}

void SwipeView::onExit() {
    SMView::onExit();
    close(true);
}

bool SwipeView::init() {
    SMView::init();
    
    _backContainer = SMView::create();
    _backContainer->setContentSize(_contentSize);
    addChild(_backContainer);
    
    _swipeContainer = SwipeContainer::create();
    _swipeContainer->setContentSize(_contentSize);
    addChild(_swipeContainer);
    
    _scroller = new PageScroller();
    _scroller->pageChangedCallback = CC_CALLBACK_1(SwipeView::openStateChanged, this);
    
    return true;
}

void SwipeView::open(bool immediate) {
    _scroller->onTouchDown();
    
    if (immediate) {
        _scroller->setScrollPosition(_swipeSize);
        _swipeContainer->setPositionX(-_swipeSize);
        onUpdateOpenState(1);
    } else {
        _scroller->setScrollPosition(_swipeSize);
        _swipeContainer->setPositionX(-_swipeSize, false);
    }
    if (_velocityTracker) {
        _velocityTracker->clear();
    }
    _openState = 0;
}

void SwipeView::close(bool immediate) {
    _scroller->onTouchDown();
    
    if (immediate) {
        _scroller->setScrollPosition(0);
        _swipeContainer->setPositionX(0);
        onUpdateOpenState(0);
    } else {
        _scroller->setScrollPosition(0);
        _swipeContainer->setPositionX(0, false);
    }
    if (_velocityTracker) {
        _velocityTracker->clear();
    }
    _openState = 0;
    
    _isTouchFocused = false;
    _inScrollEvent = false;
}

float SwipeView::getSwipePosition() {
    return _swipeContainer->getPositionX();
}

void SwipeView::setContentSize(const cocos2d::Size& size) {
    SMView::setContentSize(size);
    if (_backContainer) {
        _backContainer->setContentSize(size);
    }
    if (_swipeContainer) {
        _swipeContainer->setContentSize(size);
    }
}

void SwipeView::setSwipeWidth(const float width) {
    _swipeSize = width;
    _scroller->setCellSize(_swipeSize);
    _scroller->setWindowSize(_swipeSize);
    _scroller->setScrollSize(_swipeSize*2);
}

void SwipeView::openStateChanged(const int openState) {
    _openState = openState;
}

bool SwipeView::updateScrollPosition() {
    
    if (_fakeFlingDirection != 0) {
        if (_fakeFlingDirection > 0) {
            _scroller->onTouchFling(-5000, _openState);
        } else {
            _scroller->onTouchFling(+5000, _openState);
        }
        _fakeFlingDirection = 0;
    }
    
    bool updated = _scroller->update();
    float position = _scroller->getScrollPosition();
    
    _swipeContainer->setPositionX(-position, false);
    
    return updated;
}

void SwipeView::onUpdateOnVisit() {
    
    if (isUpdate(FLAG_SCROLL_UPDATE)) {
        if (!updateScrollPosition()) {
            unregisterUpdate(FLAG_SCROLL_UPDATE);
        }
    }
}

int SwipeView::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    
    if (!_inScrollEvent && _scroller->isTouchable()) {
        if (_scroller->getState() != SMScroller::State::STOP) {
            _scroller->onTouchDown();
            _scroller->onTouchUp();
        }
        int ret = SMView::dispatchTouchEvent(action, touch, point, event);
        if (ret == TOUCH_INTERCEPT) {
            return TOUCH_INTERCEPT;
        }
    }
    
    float x = point->x;
    float y = point->y;
    
    if (!_velocityTracker) {
        _velocityTracker = new VelocityTracker();
    }
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
            _inScrollEvent = false;
            
            if (!_swipeLock) {
            _isTouchFocused = true;
            _lastMotionX = x;
            _lastMotionY = y;
                _firstMotionTime = _director->getGlobalTime();
                
            _scroller->onTouchDown();
            _velocityTracker->addMovement(event);
            }
            break;
            
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            if (_isTouchFocused) {
                if (_inScrollEvent) {
                    _inScrollEvent = false;
                    
                    float vx, vy;
                    _velocityTracker->getVelocity(0, &vx, &vy);
                    
                    // Velocity tracker에서 계산되지 않을 경우 보정
                    if (vx == 0 && vy == 0) {
                        auto dt = _director->getGlobalTime() - _firstMotionTime;
                        if (dt > 0) {
                            auto p1 = touch->getStartLocationInView();
                            auto p2 = touch->getLocationInView();
                            vx = (p2.x - p1.x) / dt;
                            vy = -(p2.y - p1.y) / dt;
                        }
                    }

                    float abs_vx = std::abs(vx);
                    if (/*Math.abs(vx) > Math.abs(vy)/2 &&*/ abs_vx > SMViewConstValue::Config::MIN_VELOCITY) {
                        if (abs_vx > 1000) {
                            vx = ViewUtil::signum(vx) * abs_vx;
                        }
                        _scroller->onTouchFling(vx, _openState);
                    } else {
                        _scroller->onTouchUp();
                    }
                } else {
                    _scroller->onTouchUp();
                }
                
                _velocityTracker->clear();
                registerUpdate(FLAG_SCROLL_UPDATE);
                
                _isTouchFocused = false;
            }
            
            break;
        case MotionEvent::ACTION_MOVE:
            if (_isTouchFocused) {
                _velocityTracker->addMovement(event);
                float deltaX;
//                float deltaY;
                
                if (!_inScrollEvent) {
                    deltaX = x - _lastMotionX;
//                    deltaY = y - _lastMotionY;
                } else {
                    cocos2d::Vec2 delta = touch->getLocation() - touch->getPreviousLocation();
                    deltaX = delta.x;
//                    deltaY = delta.y;
                }
                
                if (!_inScrollEvent) {// && _touchMotionTarget == _swipeContainer) {
                    float ax = std::abs(x - _lastMotionX);
                    float ay = std::abs(y - _lastMotionY);
                    // 첫번째 스크롤 이벤트에서만 체크한다
                    if (ax > ay && ax > SMViewConstValue::Config::SCROLL_HORIZONTAL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
                    if (_inScrollEvent) {
                        if (_touchMotionTarget != nullptr) {
                            cancelTouchEvent(_touchMotionTarget, touch, point, event);
                            _touchMotionTarget = nullptr;
                        }
                    }
                }
                //----
                if (_inScrollEvent) {
                    _scroller->onTouchScroll(+deltaX);
                    _lastMotionX = x;
                    _lastMotionY = y;
                }
                
                registerUpdate(FLAG_SCROLL_UPDATE);
            }
            
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    return TOUCH_TRUE;
}


void SwipeView::addSwipeChild(cocos2d::Node* child) {
    _swipeContainer->addChild(child);
}

void SwipeView::removeSwipeChild(cocos2d::Node* child, bool cleanup) {
    _swipeContainer->removeChild(child, cleanup);
}

void SwipeView::addBackChild(cocos2d::Node* child) {
    _backContainer->addChild(child);
    //    _backContainer->setPositionX(_contentSize.width - child->getContentSize().width);
}



