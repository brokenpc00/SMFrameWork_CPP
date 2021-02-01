//
//  OverlapContainer.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 11..
//
//

#ifndef OverlapContainer_h
#define OverlapContainer_h

#include "../Base/SMView.h"
#include "../Base/SMTableView.h"

class SMPageView;
class Intent;
class VelocityTracker;
class OverlapScroller;

class OverlapContainer : public SMView, public _ScrollProtocol
{
public:
    enum class HeaderMode {
        FIXED,
        SCROLL
    };
    
    static OverlapContainer* create(float x, float y, float width, float height, float anchorX=0, float anchorY=0);
    
    void setPreloadPaddingSize(const float paddingSize);
    void setHeaderView(cocos2d::Node* node);
    void addOverlayChild(cocos2d::Node* node);
    void removeOverlayChild(cocos2d::Node* node, bool cleanup=true);
    void addTableView(SMTableView* tableView);
    void build();
    void setHeaderSpace(const float headerSpace);;
    void setHeaderScrollMode(const HeaderMode headerMode);
    bool jumpPage(const int pageNo);
    std::function<void(float divPosition, float delta)> onScrollCallback;
    float getDividePosition() {return _divPosition;}
    void onScrollUpdate();
    void setOnPageScrollCallback(std::function<void(float position, float distance)> callback);
    void setOnPageChangedCallback(std::function<void(int page)> callback);
    void setTableBackgrounColor4F(const cocos2d::Color4F& color);
    void setTopAdjustmentHeight(float height);
    float getTableOffsetY(){return _tableOffsetY;}
    void setHeaderTopMost(bool bTopMost);
    
    SMView * getHeaderContainer() {return _headerContainer;}
public:
    virtual void setContentSize(const cocos2d::Size& size) override;
    virtual bool isTouchEnable() const override {return true;}

protected:
    OverlapContainer();
    virtual ~OverlapContainer();
    
    virtual bool initWithSize(float width, float height);
    virtual void onUpdateOnVisit() override;
    virtual void notifyScrollUpdate() override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
private:
    bool updateScrollPosition();
    
private:
    float _headerSize;
    float _divPosition;
    float _tableOffsetY;
    cocos2d::Rect _tableRect;
    SMView* _headerContainer;
    SMView* _overlayContainer;
    cocos2d::Node* _headerView;
    
    std::vector<SMTableView*> _items;
    SMTableView* _targetTableView;
    HeaderMode _headerMode;
    SMPageView* _tableContainer;
    float _deltaScroll;
    
    float _adjustmentHeight;
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(OverlapContainer);
    friend class _ScrollProtocol;
};


#endif /* OverlapContainer_h */
