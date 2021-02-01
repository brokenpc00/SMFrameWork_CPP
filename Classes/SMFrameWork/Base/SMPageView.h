//
//  SMPageView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 9..
//
//

#ifndef SMPageView_h
#define SMPageView_h

#include "SMTableView.h"

class PageScroller;

class SMPageView : public SMTableView
{
public:
    static SMPageView* create(Orientation orient, float x, float y, float width, float height, float anchorX=0, float anchorY=0);
    void initFixedPages(const int numOfPages, const float pageSize, const int initPage=0);
    std::function<void(float position, float distance)> onPageScrollCallback;
    std::function<void(int page)> onPageChangedCallback;
    virtual void scrollFling(const float velocity) override;
    
    void goPage(int page, bool immediate=false);

    int getCurrentPage();
    
    void fakeSetCurrentPage(int page);

protected:
    SMPageView();
    virtual ~SMPageView();
    
    virtual bool initWithOrientAndSize(Orientation orient, float pageSize);
    virtual SMScroller* initScroller() override;
    virtual void onScrollChanged(float position, float distance) override;
    
private:
    void scrollPageChanged(const int currentPage);
    PageScroller* _pageScroller;
    
    CC_DISALLOW_COPY_AND_ASSIGN(SMPageView);
    
private:
    ssize_t getColumnCount() const {return getContainerCount();}
    virtual void scrollTo(float position) override {}
    virtual void scrollBy(float offset) override {}
    virtual bool resizeRowForCell(cocos2d::Node* cell, float newSize, float duration=0, float delay=0) override {return false;}
    virtual bool resizeRowForIndexPath(IndexPath& indexPath, float newSize, float duration=0, float delay=0) override {return false;}
    virtual void setScrollMarginSize(const float topMargin, const float bottomMargin) override {}
    virtual void hintFixedCellSize(const float cellSize) override {}
};

#endif /* SMPageView_h */
