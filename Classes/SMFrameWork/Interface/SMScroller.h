//
//  SMScroller.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef SMScroller_h
#define SMScroller_h

#include <functional>
#include <cocos2d.h>

class Bundle;
class VelocityTracker;
class ScrollController;


class SMScroller
{
public:
    SMScroller();
    virtual ~SMScroller();
    
    enum class State {
        STOP = 0,
        SCROLL = 1,
        FLING = 2
    };
    
    enum ScrollMode {
        BASIC = 0,
        PAGER,
        ALIGNED
    };


public:
    virtual void reset();
    
    virtual bool update();
    
    State getState() { return _state; }
    
    virtual void setWindowSize(const float windowSize);
    
    virtual float getWindowSize() const { return _windowSize; }
    
    
    virtual void setScrollSize(const float scrollSize);
    
    virtual float getScrollSize() const { return _maxPosition - _minPosition; }
    
    
    virtual void setScrollPosition(const float position, bool immediate=true);
    
    virtual float getScrollPosition() const { return _position - _minPosition; }
    
    float getNewScrollPosition() const { return _newPosition - _minPosition; }
    
    void setStartPosition(const float startPos) { _startPos += startPos; }
    
    float getStartPosition() const { return _startPos; }
    
    
    bool isTouchable();
    
    float getScrollSpeed() { return _scrollSpeed; }
    
    virtual void justAtLast() {}

    virtual void onTouchDown(const int param = 0) = 0;
    virtual void onTouchUp(const int param = 0) = 0;
    virtual void onTouchScroll(const float delta, const int param = 0) = 0;
    virtual void onTouchFling(const float velocity, const int param = 0) = 0;
    
    virtual void setHangSize(const float size) { _hangSize = size; }
    
    virtual void scrollBy(float distance) {};
    
    void setScrollMode(int scrollMode);
    
    void setCellSize(float cellSize) { _cellSize = cellSize; }
    
    float getCellSize() { return _cellSize; }
    
    std::function<void(bool aligned)> onAlignCallback;
    
    int getMaxPageNo();
    
protected:
    virtual bool runFling() { return false; }
    
    virtual bool runScroll() { return false; }
    
protected:
    State _state;
    
    float _position;
    float _newPosition;
    float _lastPosition;
    
    float _windowSize;
    float _minPosition;
    float _maxPosition;
    
    float _timeStart;
    float _timeDuration;
    
    float _startPos;
    float _stopPos;
    
    float _velocity;
    float _accelate;
    float _touchDistance;
    
    float _hangSize;
    
    float _scrollSpeed;
    
    float _cellSize;
    
    int _scrollMode;
    
public:
    virtual void clone(SMScroller* scroller);
};


class FlexibleScroller : public SMScroller
{
public:
    FlexibleScroller();
    virtual ~FlexibleScroller();
    
    virtual bool update() override;
    
    virtual void setWindowSize(const float windowSize) override;
    
    virtual void setScrollSize(const float scrollSize) override;
    
    virtual void setScrollPosition(const float position, bool immediate=true) override;
    
    virtual float getScrollPosition() const override;
    
    virtual void reset() override;
    
    virtual void justAtLast() override;
    
    virtual void onTouchDown(const int unused = 0) override;
    
    virtual void onTouchUp(const int unused = 0) override;
    
    virtual void onTouchScroll(const float delta, const int unused = 0) override;
    
    virtual void onTouchFling(const float velocity, const int unused = 0) override;
    
    void scrollTo(float position);
    
    void scrollToWithDuration(float position, float duration);
    
    bool onAutoScroll();
    
    virtual void setHangSize(const float size) override;
    
protected:
    ScrollController* _controller;
    
    bool _autoScroll;
    
};


// for page view

class PageScroller : public FlexibleScroller
{
public:
    PageScroller();
    
    virtual ~PageScroller();

    virtual void reset() override;
    
    virtual bool update() override;
    
    virtual void onTouchUp(const int unused = 0) override;
    
    virtual void onTouchFling(const float velocity, const int currentPage) override;
    
    virtual bool runScroll() override;
    
    std::function<void(const int)> pageChangedCallback;
    
    void setCurrentPage(const int page, bool immediate = true);
    
    void setBounceBackEnable(bool enable) { _bounceBackEnabled = enable; }
    
private:
    
    bool _bounceBackEnabled;
};


// for circular page scroll

class InfinityScroller : public SMScroller
{
public:
    
    InfinityScroller();
    virtual ~InfinityScroller();
    
    virtual void reset() override;
    
    virtual void onTouchDown(const int unused = 0) override;
    
    virtual void onTouchUp(const int unused = 0) override;
    
    virtual void onTouchScroll(const float delta, const int unused = 0) override;
    
    virtual void onTouchFling(const float velocity, const int unused = 0) override;
    
    void scrollByWithDuration(float distance, float duration);
    
protected:
    virtual bool runFling() override;
    
    virtual bool runScroll() override;
};



// finity scroller

class FinityScroller : public FlexibleScroller
{
public:
    FinityScroller();
    
    virtual ~FinityScroller();
    
    virtual void reset() override;
    
    virtual bool update() override;
    
    virtual void onTouchUp(const int unused = 0) override;
    
    virtual void onTouchFling(const float velocity, const int currentPage) override;
    
    virtual void setScrollSize(const float scrollSize) override;
    
    virtual void setWindowSize(const float windowSize) override;
    
protected:
    virtual bool runFling() override;
    
    virtual bool runScroll() override;
};

// scroll protocol
class _ScrollProtocol
{
protected:
    SMScroller* getScroller();
    
    virtual bool updateScrollInParentVisit(float& deltaScroll) { return false; }
    
    void setScrollParent(_ScrollProtocol* parent);
    
    virtual void notifyScrollUpdate() {}
    
    void setInnerScrollMargin(const float margin) { _innerScrollMargin = margin; }
    
    virtual void setMinScrollSize(const float minScrollSize) { _minScrollSize = minScrollSize; }
    
    void setTableRect(cocos2d::Rect* tableRect);
    
    void setScrollRect(cocos2d::Rect* scrollRect);
    
    void setBaseScrollPosition(float position) { _baseScrollPosition = position; }
    
    float getBaseScrollPosition() { return _baseScrollPosition; }
    
public:
    _ScrollProtocol();
    
    virtual ~_ScrollProtocol();
    
protected:
    
    SMScroller* _scroller;
    
    VelocityTracker* _velocityTracker;
    
    _ScrollProtocol* _scrollParent;
    
    float _innerScrollMargin;
    
    float _minScrollSize;
    
    float _baseScrollPosition;
    
    bool _inScrollEvent;
    
    cocos2d::Rect* _tableRect;
    cocos2d::Rect* _scrollRect;
    
    friend class SMTableView;
    friend class OverlapContainer;
    
};


#endif /* SMScroller_h */
