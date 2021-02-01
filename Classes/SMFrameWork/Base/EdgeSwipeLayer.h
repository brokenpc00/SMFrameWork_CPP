//
//  EdgeSwipeLayer.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
//

#ifndef EdgeSwipeLayer_h
#define EdgeSwipeLayer_h

#include "SMView.h"


class PageScroller;
class VelocityTracker;

class EdgeSwipeForMenu : public SMView
{
public:
    CREATE_VIEW(EdgeSwipeForMenu);
    
    void setSwipeWidth(const float width);
    void setEdgeWidth(const float width);
    
    void open(bool immediate=true);
    void close(bool immediate=true);
    
    bool isOpen() {return _openState==1;}
    bool inScrollEvent() {return _inScrollEvent;}
    bool isScrollTargeted() {return _scrollEventTargeted;}
    bool isScrollArea(const cocos2d::Vec2& worldPoint);
    void closeComplete();
    
    virtual void update(float dt) override;
    
protected:
    EdgeSwipeForMenu();
    virtual ~EdgeSwipeForMenu();
    
    virtual bool init() override;
    void updateScrollPosition(float dt);
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    void openStateChanged(const int openState);
    
private:
    int _openState;
    int _fakeFlingDirection;
    float _swipeSize;
    float _edgeSize;
    float _position;
    bool _inScrollEvent;
    float _lastMotionX;
    float _lastMotionY;
    float _lastScrollPosition;
    bool _scrollEventTargeted;
    
    PageScroller* _scroller;
    VelocityTracker* _velocityTracker;
    
    
    DISALLOW_COPY_AND_ASSIGN(EdgeSwipeForMenu);
};

class EdgeSwipeForBack : public SMView
{
public:
    CREATE_VIEW(EdgeSwipeForBack);
    
    void setSwipeWidth(const float width);
    void setEdgeWidth(const float width);
    void back(bool immediate=true);
    
    bool isOpened() {return _openState==1;}
    bool inScrollEvent() {return _inScrollEvent;}
    bool isScrollTargeted() {return _scrollEventTargeted;}
    bool isScrollArea(const cocos2d::Vec2& worldPoint);
    
    std::function<void(int, float)> _swipeUpdateCallback;
    
    void reset();
    virtual void update(float dt) override;
    
protected:
    EdgeSwipeForBack();
    virtual ~EdgeSwipeForBack();
    
    virtual bool init() override;
    void updateScrollPosition(float dt);
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    void openStateChanged(const int openState);
    

    int _fakeFlingDirection;
    int _openState;
    float _swipeSize;
    float _edgeSize;
    float _position;
    bool _inScrollEvent;
    float _lastMotionX;
    float _lastMotionY;
    float _lastScrollPosition;
    bool _scrollEventTargeted;
    
    PageScroller* _scroller;
    VelocityTracker* _velocityTracker;

private:    
    DISALLOW_COPY_AND_ASSIGN(EdgeSwipeForBack);
};

class EdgeSwipeForDismiss : public EdgeSwipeForBack
{
public:
    CREATE_VIEW(EdgeSwipeForDismiss);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Point* point, MotionEvent* event) override;
};

#endif /* EdgeSwipeLayer_h */
