//
//  SwipeView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 27..
//
//

#ifndef SwipeView_h
#define SwipeView_h

#include "../Base/SMView.h"

class PageScroller;
class VelocityTracker;

class OnSwipeStateChangeListener
{
public:
    virtual void onSwipeStateChanged(SMView* view, bool opened) = 0;
};

class SwipeView : public SMView
{
    
public:
    CREATE_VIEW(SwipeView);
    
    void setOnSwipeStateChangeListener(OnSwipeStateChangeListener* l) {_listener = l;}
    
    /**
     * Swipe width 설정
     *
     * @param width
     */
    void setSwipeWidth(const float width);
    
    /**
     * fake open
     *
     * @param immediate true=애니메이션 없이 즉시 열림, false=애니메이션
     */
    void open(bool immediate = true);
    
    /**
     * fake close
     *
     * @param immediate true=애니메이션 없이 즉시 열림, false=애니메이션
     */
    void close(bool immediate = true);
    
    /**
     * Open 상태
     *
     * @return true=열려있음, false=닫혀있음
     */
    bool isOpened() { return _openState == 1; }
    
    /**
     * Swipe Child 추가 (위쪽 child)
     *
     * @param child 추가할 child node
     */
    void addSwipeChild(cocos2d::Node* child);
    
    /**
     * Swipe Child 삭제 (위쪽 child)
     *
     * @param child
     * @param cleanup
     */
    void removeSwipeChild(cocos2d::Node* child, bool cleanup = true);
    
    /**
     * Back Child 추가 (아래쪽 child)
     *
     * @param child 추가할 child node
     */
    void addBackChild(cocos2d::Node* child);
    
    /**
     * Back Child 삭제 (위쪽 child)
     *
     * @param child
     * @param cleanup
     */
    void removeBackChild(cocos2d::Node* child, bool cleanup = true);
    
    virtual void onUpdateOpenState(const float t) {}
    
    float getSwipePosition();
    
    void setSwipeLock(bool lock) {_swipeLock = lock;}
    
protected:
    SwipeView();
    virtual ~SwipeView();
    
    bool init() override;
    
    bool updateScrollPosition();
    
    virtual void onUpdateOnVisit() override;
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
public:
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    
    virtual void onExit() override;
    
private:
    void openStateChanged(const int openState);
    
private:
    class SwipeContainer;
    
    int _fakeFlingDirection;
    
    int _openState;
    
    float _swipeSize;
    
    bool _inScrollEvent;
    
    bool _isTouchFocused;
    
    float _lastMotionX;
    
    float _lastMotionY;
    
    float _lastScrollPosition;
    
    float _firstMotionTime;
    
    PageScroller* _scroller;
    
    VelocityTracker* _velocityTracker;
    
    SwipeContainer* _swipeContainer;
    
    SMView* _backContainer;
    
    OnSwipeStateChangeListener* _listener;
    
    bool _swipeLock;
    
public:
    SMView * getSwipeContainer() { return (SMView*)_swipeContainer; }
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(SwipeView);
};


#endif /* SwipeView_h */
