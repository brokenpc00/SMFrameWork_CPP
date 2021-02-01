//
//  MotionEvent.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef MotionEvent_h
#define MotionEvent_h

#include <base/CCTouch.h>
#include <base/CCEvent.h>
#include <base/ccUtils.h>

class MotionEvent {
    
public:
    static const int ACTION_DOWN            = 1;
    static const int ACTION_UP              = 2;
    static const int ACTION_MOVE            = 3;
    static const int ACTION_CANCEL          = 4;
    static const int ACTION_POINTER_DOWN    = 5;
    static const int ACTION_POINTER_UP      = 6;
    
    MotionEvent() {}
    
    cocos2d::Touch* getTouch(int pointerIndex) {
        return (pointerIndex<_pointerCount)?(*_touches)[pointerIndex]:nullptr; }
    
    void set(const int action, const std::vector<cocos2d::Touch*>* touches, const float eventTime, const int actionIndex) {
        _action = action;
        _touches = touches;
        _pointerCount = touches->size();
        _eventTime = eventTime;
        _actionIndex = actionIndex;
    }
    
    int getAction() const { return _action; }
    
    ssize_t getTouchCount() const { return _pointerCount; }
    
    float getEventTime() const { return _eventTime; }
    
    int getActionIndex() { return _actionIndex; }
    
private:
    int _action;
    
    int _actionIndex;
    
    ssize_t _pointerCount;
    
    const std::vector<cocos2d::Touch*>* _touches;
    
    float _eventTime;
};

#endif /* MotionEvent_h */
