//
//  EdgeSwipeLayer.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
//

#include "EdgeSwipeLayer.h"
#include "../Interface/SMScroller.h"
#include "../Interface/VelocityTracker.h"
#include "../UI/SideMenu.h"


EdgeSwipeForMenu::EdgeSwipeForMenu() :
_scroller(nullptr),
_velocityTracker(nullptr),
_swipeSize(0),
_edgeSize(0),
_openState(0),
_fakeFlingDirection(0),
_scrollEventTargeted(false),
_position(0)
{
    
}

EdgeSwipeForMenu::~EdgeSwipeForMenu()
{
    CC_SAFE_DELETE(_scroller);
    CC_SAFE_DELETE(_velocityTracker);
}

bool EdgeSwipeForMenu::init()
{
    SMView::init();
    
    _scroller = new (std::nothrow) PageScroller();
    if (_scroller) {
        _scroller->setBounceBackEnable(false);
        _scroller->pageChangedCallback = CC_CALLBACK_1(EdgeSwipeForMenu::openStateChanged, this);
        return true;
    }
    
    return false;
}

void EdgeSwipeForMenu::open(bool immediate)
{
    if (immediate) {
        // 메뉴가 좌측일경우... 우측일 경우는 전체 가로 size만큼
        _scroller->setScrollPosition(0);
        _openState = 0;
    } else {
        scheduleUpdate();
        // 메뉴가 좌측일 경우... 우측일 경우는 1
        _fakeFlingDirection = -1;
        scheduleUpdate();
    }
    if (_velocityTracker) {
        _velocityTracker->clear();
    }
}

void EdgeSwipeForMenu::close(bool immediate)
{
    if (immediate) {
        // 메뉴가 좌측일 경우... 우측일 경우는 전체 가로 size에서 뺀만큼
        _scroller->setScrollPosition(_swipeSize);
        _openState = 1;
    } else {
        scheduleUpdate();
        // 메뉴가 좌측일 경우... 우측일 경우는 -1
        _fakeFlingDirection = +1;
    }
    if (_velocityTracker) {
        _velocityTracker->clear();
    }
}

void EdgeSwipeForMenu::closeComplete()
{
    unscheduleUpdate();
}

void EdgeSwipeForMenu::setSwipeWidth(const float width)
{
    _swipeSize = width;
    _scroller->setCellSize(_swipeSize);
    _scroller->setWindowSize(_swipeSize);
    _scroller->setScrollSize(_swipeSize*2);
    
    _scroller->setScrollPosition(_swipeSize);
    _openState = 1;
}

void EdgeSwipeForMenu::setEdgeWidth(const float width)
{
    _edgeSize = width;
}

void EdgeSwipeForMenu::openStateChanged(const int openState)
{
    _openState = openState;
}

void EdgeSwipeForMenu::update(float dt)
{
    updateScrollPosition(dt);
}

void EdgeSwipeForMenu::updateScrollPosition(float dt)
{
    if (_fakeFlingDirection != 0) {
        // 좌측 메뉴일 경우... 우측일경우는 반대...
        float velocity = _fakeFlingDirection>0 ? -5000 : +5000;
        _scroller->onTouchFling(velocity, _openState);
        _fakeFlingDirection = 0;
    }
    
    _scroller->update();
    float position = _scroller->getScrollPosition();
    if (position!=_position) {
        _position = position;
        // 메뉴 Open이 필요한 경우 여기서 Open시킨다. 우측 메뉴인경우 수치 조정해야 함.
        SideMenu::getInstance()->setOpenPosition(_swipeSize-_position);
    }
}

bool EdgeSwipeForMenu::isScrollArea(const cocos2d::Vec2 &worldPoint)
{
    if (_openState==0) {
        return true;
    }
    
    // 좌측 메뉴일 경우... 우측인 경우는 반대
    if (worldPoint.x<_edgeSize) {
        return true;
    }
    
    return false;
}

int EdgeSwipeForMenu::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    float x = point->x;
    float y = point->y;
    
    if (!_velocityTracker) {
        _velocityTracker = new VelocityTracker();
    }
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _lastMotionX = x;
            _lastMotionY = y;
            
            // 현재 메뉴상태를 얻어옴. 열려 있는지, 닫혀 있는지.. 아니면 움직이는 중인지
            kSideMenuState state = SideMenu::getInstance()->getState();
            if (state==kSideMenuStateClose) {
                // 메뉴가 닫힌 상태라면
                if (x < _edgeSize) {
                    _scrollEventTargeted = true;
                }
            } else {
                if (x > _swipeSize - _position) {
                    _scrollEventTargeted = true;
                }
            }
            
            if (_scrollEventTargeted) {
                _scroller->onTouchDown();
                _velocityTracker->addMovement(event);
            }
        }
            break;
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
        {
            if (_scrollEventTargeted) {
                if (_inScrollEvent) {
                    _inScrollEvent = false;
                    
                    float vx, vy;
                    _velocityTracker->getVelocity(0, &vx, &vy);
                    if (std::abs(vx)>SMViewConstValue::Config::MIN_VELOCITY) {
                        _scroller->onTouchFling(vx, _openState);
                    } else {
                        _scroller->onTouchUp();
                    }
                    scheduleUpdate();
                } else {
                    int state = SideMenu::getInstance()->getState();
                    if (state!=kSideMenuStateClose) {
                        _scroller->onTouchUp();
                    }
                }
            }
            
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _velocityTracker->clear();
        }
            break;
        case MotionEvent::ACTION_MOVE:
        {
            if (_scrollEventTargeted) {
                _velocityTracker->addMovement(event);
                
                float deltaX, deltaY;
                
                if (!_inScrollEvent) {
                    deltaX = x - _lastMotionX;
                    deltaY = y - _lastMotionY;
                } else {
                    cocos2d::Vec2 delta = touch->getLocation() - touch->getPreviousLocation();
                    deltaX = delta.x;
                    deltaY = delta.y;
                }
                
                if (!_inScrollEvent) {
                    float ax = std::abs(x - _lastMotionX);
                    // 첫번째 스크롤 이벤트에서만 체크
                    if (ax > SMViewConstValue::Config::SCROLL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
                    if (_inScrollEvent) {
                        if (_touchMotionTarget!=nullptr) {
                            cancelTouchEvent(_touchMotionTarget, touch, point, event);
                            _touchMotionTarget = nullptr;
                        }
                    }
                }
                if (_inScrollEvent) {
                    _scroller->onTouchScroll(+deltaX);
                    _lastMotionX = x;
                    _lastMotionY = y;
                    
                    scheduleUpdate();
                }
            }
        }
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    if (action==MotionEvent::ACTION_UP) {
        return TOUCH_FALSE;
    }
    
    return _scrollEventTargeted?TOUCH_TRUE:TOUCH_FALSE;
}



EdgeSwipeForBack::EdgeSwipeForBack() :
_scroller(nullptr),
_velocityTracker(nullptr),
_swipeSize(0),
_edgeSize(0),
_openState(0),
_fakeFlingDirection(0),
_scrollEventTargeted(false),
_position(0)
{
    
}

EdgeSwipeForBack::~EdgeSwipeForBack()
{
    CC_SAFE_DELETE(_scroller);
    CC_SAFE_DELETE(_velocityTracker);
}

bool EdgeSwipeForBack::init()
{
    SMView::init();
    
    _scroller = new (std::nothrow) PageScroller();
    if (_scroller) {
        _scroller->pageChangedCallback = CC_CALLBACK_1(EdgeSwipeForBack::openStateChanged, this);
        return true;
    }
    return false;
}

void EdgeSwipeForBack::back(bool immediate)
{
    if (immediate) {
        _scroller->setScrollPosition(_swipeSize);
    } else {
        scheduleUpdate();
        _fakeFlingDirection = 1;
    }
    if (_velocityTracker) {
        _velocityTracker->clear();
    }
}

void EdgeSwipeForBack::reset()
{
    _scroller->setCurrentPage(1, true);
    _openState = 0;
    _position = 0;
    
    _inScrollEvent = false;
    _scrollEventTargeted = false;
    
    unscheduleUpdate();
}

void EdgeSwipeForBack::setSwipeWidth(const float width)
{
    _swipeSize = width;
    _scroller->setCellSize(_swipeSize);
    _scroller->setWindowSize(_swipeSize);
    _scroller->setScrollSize(_swipeSize*2);
}

void EdgeSwipeForBack::setEdgeWidth(const float width)
{
    _edgeSize = width;
}

void EdgeSwipeForBack::openStateChanged(const int openState)
{
    _openState = openState;
}

void EdgeSwipeForBack::update(float dt)
{
    updateScrollPosition(dt);
}

void EdgeSwipeForBack::updateScrollPosition(float dt)
{
    if (_fakeFlingDirection!=0) {
        float velocity = _fakeFlingDirection>0?-5000:+5000;
        _scroller->onTouchFling(velocity, _openState);
        _fakeFlingDirection = 0;
    }
    
    _scroller->update();
    _position = _scroller->getScrollPosition() - _swipeSize;
 
    if (_swipeUpdateCallback) {
        _swipeUpdateCallback(_openState, -_position);
    }
}

bool EdgeSwipeForBack::isScrollArea(const cocos2d::Vec2 &worldPoint)
{
    if (worldPoint.x < _edgeSize) {
        return true;
    }
    
    return false;
}

int EdgeSwipeForBack::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    float x = point->x;
    float y = point->y;
    
    if (!_velocityTracker) {
        _velocityTracker = new VelocityTracker();
    }
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _lastMotionX = x;
            _lastMotionY = y;
            
            if (((int)std::abs(_position)) <= 1 && x < _edgeSize) {
                _scrollEventTargeted = true;
            } else {
                _scrollEventTargeted = false;
            }
            
            if (_scrollEventTargeted) {
                _scroller->onTouchDown();
                _velocityTracker->addMovement(event);
            }
        }
            break;
            
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            if (_scrollEventTargeted ) {
                if (_inScrollEvent) {
                    _inScrollEvent = false;
                    
                    float vx, vy;
                    _velocityTracker->getVelocity(0, &vx, &vy);
                    if (std::abs(vx) > 1000) {
                        _scroller->onTouchFling(vx, 1 - _openState);
                    } else {
                        _scroller->onTouchUp();
                    }
                    scheduleUpdate();
                }
            }
            
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _velocityTracker->clear();
            break;
        case MotionEvent::ACTION_MOVE:
            if (_scrollEventTargeted) {
                _velocityTracker->addMovement(event);
                float deltaX;
                float deltaY;
                
                if (!_inScrollEvent) {
                    deltaX = x - _lastMotionX;
                    deltaY = y - _lastMotionY;
                } else {
                    cocos2d::Vec2 delta = touch->getLocation() - touch->getPreviousLocation();
                    deltaX = delta.x;
                    deltaY = delta.y;
                }
                
                if (!_inScrollEvent) {
                    float ax = std::abs(x - _lastMotionX);
                    // 첫번째 스크롤 이벤트에서만 체크한다
                    if (ax > SMViewConstValue::Config::SCROLL_TOLERANCE) {
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
                    scheduleUpdate();
                }
            }
            
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    if (action == MotionEvent::ACTION_UP) {
        return TOUCH_FALSE;
    }
    
    return _scrollEventTargeted?TOUCH_TRUE:TOUCH_FALSE;
}



int EdgeSwipeForDismiss::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    float x = point->x;
    float y = point->y;
    
    if (!_velocityTracker) {
        _velocityTracker = new VelocityTracker();
    }
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _lastMotionX = x;
            _lastMotionY = y;
            
            if (_position == 0 && y > _swipeSize - _edgeSize ) {
                _scrollEventTargeted = true;
            } else {
                _scrollEventTargeted = false;
            }
            
            if (_scrollEventTargeted) {
                _scroller->onTouchDown();
                _velocityTracker->addMovement(event);
            }
        }
            break;
            
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            if (_scrollEventTargeted ) {
                if (_inScrollEvent) {
                    _inScrollEvent = false;
                    
                    float vx, vy;
                    _velocityTracker->getVelocity(0, &vx, &vy);
                    if (std::abs(vy) > 1000) {
                        _scroller->onTouchFling(-vy, 1 - _openState);
                    } else {
                        _scroller->onTouchUp();
                    }
                    scheduleUpdate();
                }
            }
            
            _scrollEventTargeted = false;
            _inScrollEvent = false;
            _velocityTracker->clear();
            break;
        case MotionEvent::ACTION_MOVE:
            if (_scrollEventTargeted) {
                _velocityTracker->addMovement(event);
                float deltaX;
                float deltaY;
                
                if (!_inScrollEvent) {
                    deltaX = x - _lastMotionX;
                    deltaY = y - _lastMotionY;
                } else {
                    cocos2d::Vec2 delta = touch->getLocation() - touch->getPreviousLocation();
                    deltaX = delta.x;
                    deltaY = delta.y;
                }
                
                if (!_inScrollEvent) {
                    float ay = std::abs(y - _lastMotionY);
                    // 첫번째 스크롤 이벤트에서만 체크한다
                    if (ay > SMViewConstValue::Config::SCROLL_TOLERANCE) {
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
                    _scroller->onTouchScroll(-deltaY);
                    _lastMotionX = x;
                    _lastMotionY = y;
                    scheduleUpdate();
                }
            }
            
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    if (action == MotionEvent::ACTION_UP) {
        return TOUCH_FALSE;
    }
    
    return _scrollEventTargeted?TOUCH_TRUE:TOUCH_FALSE;
}
