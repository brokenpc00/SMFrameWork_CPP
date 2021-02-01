//
//  SMPageView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 9..
//
//

#include "SMPageView.h"
#include "../Interface/SMScroller.h"
#include <cocos2d.h>

#define CLEANUP_FALG (true)

SMPageView::SMPageView() :
onPageScrollCallback(nullptr),
onPageChangedCallback(nullptr)
{
    
}

SMPageView::~SMPageView()
{
    
}

SMPageView* SMPageView::create(SMTableView::Orientation orient, float x, float y, float width, float height, float anchorX, float anchorY)
{
    SMPageView* pageView = new (std::nothrow)SMPageView();
    if (pageView && pageView->initWithOrientAndSize(orient, orient==Orientation::HORIZONTAL?width:height)) {
        pageView->setContentSize(cocos2d::Size(width, height));
        pageView->setPosition(x, y);
        pageView->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
        
        pageView->autorelease();
    } else {
        CC_SAFE_DELETE(pageView);
    }
    
    return pageView;
}

bool SMPageView::initWithOrientAndSize(SMTableView::Orientation orient, float pageSize)
{
    if (SMTableView::initWithOrientAndColumns(orient, 1)) {
        SMTableView::hintFixedCellSize(pageSize);
        
        _pageScroller->setCellSize(pageSize);
        _pageScroller->pageChangedCallback = CC_CALLBACK_1(SMPageView::scrollPageChanged, this);
        
        return true;
    }
    
    return false;
}

void SMPageView::initFixedPages(const int numOfPages, const float pageSize, const int initPage)
{
    initFixedColumnInfo(numOfPages, pageSize, initPage);
}

SMScroller* SMPageView::initScroller()
{
    _scroller = _pageScroller = new PageScroller();
    return _scroller;
}

void SMPageView::scrollFling(const float velocity)
{
    int movePage = (int)(_scroller->getScrollPosition()/_scroller->getWindowSize());
    if (velocity>0) {
        // 왼쪽 또는 위쪽 이면 다음 페이지를 보여준다.
        if (movePage==_currentPage) {
            _scroller->onTouchFling(velocity, _currentPage+1);
            return;
        }
    } else {
        // 오른쪽 또는 아래쪽 이면 이전 페이지를 보여준다.
        if (movePage==_currentPage-1) {
            _scroller->onTouchFling(velocity, _currentPage-1);
            return;
        }
    }
    // 그외 나머지면 현재 페이지를 보여준다.
    _scroller->onTouchFling(velocity, _currentPage);
}

void SMPageView::scrollPageChanged(const int currentPage)
{
    _currentPage = currentPage;
    if (onPageChangedCallback) {
        onPageChangedCallback(_currentPage);
    }
}

void SMPageView::onScrollChanged(float position, float distance)
{
    if (onPageScrollCallback) {
        onPageScrollCallback(position/((PageScroller*)_scroller)->getCellSize(), distance);
    }
}

void SMPageView::goPage(int page, bool immediate)
{
    CCASSERT(page>=0 && page <= _pageScroller->getMaxPageNo(), "Invalid Page");
    auto scroller = (PageScroller*)getScroller();
    if (immediate) {
        fakeSetCurrentPage(page);
    } else {
        jumpPage(page, scroller->getCellSize());
    }
//    setScrollPosition(page*getContentSize().width);
//    scheduleScrollUpdate();
}


int SMPageView::getCurrentPage() {
    return _currentPage;
}

void SMPageView::fakeSetCurrentPage(int page) {
    setScrollPosition(page*_scroller->getCellSize());
    _currentPage = page;
}

