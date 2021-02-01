//
//  OverlapContainer.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 11..
//
//

#include "OverlapContainer.h"
#include "../Interface/SMScroller.h"
#include "../Interface/VelocityTracker.h"
#include "../Const/SMViewConstValue.h"
#include "../Base/SMTableView.h"
#include "../Base/SMPageView.h"

#define CLEANUP_FLAG (true)
#define FLAG_SCROLL_UPDATE (1<<0)
#define FLAG_CONTENT_UPDATE (1<<1)
#define FLAG_HEADER_POSITION (1<<2)
#define FLAG_SCROLL_NOTIFY (1<<3)

OverlapContainer::OverlapContainer() :
_headerView(nullptr),
_headerContainer(nullptr),
_tableContainer(nullptr),
_overlayContainer(nullptr),
_headerSize(0),
_headerMode(HeaderMode::SCROLL),
_targetTableView(nullptr),
_adjustmentHeight(0.0),
onScrollCallback(nullptr)
{
    
}

OverlapContainer::~OverlapContainer()
{
    std::for_each(_items.begin(), _items.end(), [&](SMTableView * tableView){
        CC_SAFE_RELEASE(tableView);
    });
}

OverlapContainer * OverlapContainer::create(float x, float y, float width, float height, float anchorX, float anchorY)
{
    OverlapContainer * oc = new (std::nothrow) OverlapContainer();
    if (oc && oc->initWithSize(width, height)) {
        oc->setPosition(cocos2d::Vec2(x, y));
        oc->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
        
        oc->autorelease();
    } else {
        CC_SAFE_DELETE(oc);
    }
    return oc;
}

bool OverlapContainer::initWithSize(float width, float height)
{
    if (!SMView::init()) {
        return false;
    }
    
    _headerContainer = SMView::create();
    this->addChild(_headerContainer);
    _headerContainer->setLocalZOrder(10);
    
    _tableContainer = SMPageView::create(SMTableView::Orientation::HORIZONTAL, 0, 0, width, height);
    _tableContainer->setScrollParent(this);
    _tableContainer->cellForRowAtIndexPath = [this](const IndexPath& indexPath)->cocos2d::Node* {
        return _items.at(indexPath.getIndex());
    };
    _tableContainer->numberOfRowsInSection = [this](int section) {
        return _items.size();
    };
    this->addChild(_tableContainer);
    _tableContainer->setLocalZOrder(20);
    
    _overlayContainer = SMView::create();
    this->addChild(_overlayContainer);
    setContentSize(cocos2d::Size(width, height));
    _overlayContainer->setLocalZOrder(30);
    
    return true;
}

void OverlapContainer::setHeaderTopMost(bool bTopMost)
{
    if (bTopMost) {
        _headerContainer->setLocalZOrder(90);
        _tableContainer->setLocalZOrder(10);
    } else {
        _headerContainer->setLocalZOrder(10);
        _tableContainer->setLocalZOrder(20);
    }
}

void OverlapContainer::setTableBackgrounColor4F(const cocos2d::Color4F &color)
{
    _tableContainer->setBackgroundColor4F(color);
}

void OverlapContainer::setOnPageScrollCallback(std::function<void (float, float)> callback)
{
    _tableContainer->onPageScrollCallback = callback;
}

void OverlapContainer::setOnPageChangedCallback(std::function<void (int)> callback)
{
    _tableContainer->onPageChangedCallback = callback;
}

void OverlapContainer::setHeaderScrollMode(const HeaderMode headerMode)
{
    _headerMode = headerMode;
}

void OverlapContainer::setTopAdjustmentHeight(float height)
{
    _adjustmentHeight = height;
}

void OverlapContainer::setContentSize(const cocos2d::Size &size)
{
    SMView::setContentSize(size);
    
    _headerContainer->setContentSize(size);
    _tableContainer->setContentSize(size);
    
    registerUpdate(FLAG_CONTENT_UPDATE);
}

void OverlapContainer::setHeaderSpace(const float headerSpace)
{
    _divPosition = _headerSize = headerSpace;
    
    registerUpdate(FLAG_CONTENT_UPDATE);
}

void OverlapContainer::setHeaderView(cocos2d::Node *node)
{
    if (_headerView!=nullptr && _headerView != node) {
        // already exist node
        cocos2d::Node::removeChild(_headerView, true);
    }
    
    _headerView = node;
    
    if (_headerView!=nullptr) {
        _headerContainer->addChild(_headerView);
        
        if (_headerSize<=0) {
            _divPosition = _headerSize = _headerView->getContentSize().height;
        }
    }
    
    registerUpdate(FLAG_CONTENT_UPDATE);
}

void OverlapContainer::addOverlayChild(cocos2d::Node *node)
{
    _overlayContainer->addChild(node);
}

void OverlapContainer::removeOverlayChild(cocos2d::Node *node, bool cleanup)
{
    _overlayContainer->removeChild(node, cleanup);
}

void OverlapContainer::addTableView(SMTableView *tableView)
{
    _items.push_back(tableView);
    tableView->retain();
    tableView->setScrollParent(this);
}

void OverlapContainer::build()
{
    // horizontal
    _tableContainer->initFixedPages((int)_items.size(), _contentSize.width);
    
    std::for_each(_items.begin(), _items.end(), [&](SMTableView* tableView){
        tableView->setMinScrollSize(_contentSize.height + _headerSize);
    });
    
    onScrollUpdate();
}

void OverlapContainer::setPreloadPaddingSize(const float paddingSize)
{
    _tableContainer->setPreloadPaddingSize(paddingSize);
}

bool OverlapContainer::jumpPage(const int pageNo)
{
    CCASSERT(pageNo>=0 && pageNo<_items.size(), "page number must be 0 between array size-1");
    
    auto scroller = (PageScroller*)_tableContainer->getScroller();
    return _tableContainer->jumpPage(pageNo, scroller->getCellSize());
}

void OverlapContainer::notifyScrollUpdate()
{
    registerUpdate(FLAG_SCROLL_UPDATE);
}

bool OverlapContainer::updateScrollPosition()
{
    if (_targetTableView==nullptr) {
        return false;
    }
    
    float lastDivPosition = _divPosition;
    
    auto protocol = dynamic_cast<_ScrollProtocol*>(_targetTableView);
    if (protocol==nullptr) {
        return false;
    }
    
    auto scroller = protocol->getScroller();
    if (scroller==nullptr) {
        return false;
    }
    float deltaScroll = 0;
    float prevScrollPosition = scroller->getScrollPosition();
    bool updated = protocol->updateScrollInParentVisit(deltaScroll);
    float scrollPosition = scroller->getScrollPosition();
    bool overBoundary = false;
    
    if (deltaScroll<0 && (_headerSize-scrollPosition>_divPosition || scrollPosition<_headerSize || _divPosition>_adjustmentHeight)) {
        _divPosition = std::min(_headerSize, std::max(_adjustmentHeight, _divPosition+deltaScroll));
    }
    
//    if (prevScrollPosition>_headerSize && scrollPosition<=_headerSize) {
//        overBoundary = true;
//    }
    
    if (_headerSize - _divPosition > scrollPosition) {
        _tableOffsetY = _divPosition = _headerSize - scrollPosition;
        
        if (_divPosition>_headerSize) {
            _divPosition = _headerSize;
        }
    }
    
//    if (prevScrollPosition>=0 && scrollPosition<0) {
//        overBoundary = true;
//    }
    
//    if (deltaScroll!=0 || overBoundary) {
    if (deltaScroll!=0) {
        std::for_each(_items.begin(), _items.end(), [&](SMTableView* tableView){
            if (tableView!=protocol) {
                float position = tableView->getScrollPosition();
                float newPosition = position;
                
                if (deltaScroll<0) {
                    // 위로 스크롤 하냐
                    if (scrollPosition>=0 && position<=_headerSize) {
                        newPosition = position - deltaScroll;
                        if (newPosition>_headerSize) {
                            newPosition = _headerSize;
                        }
                    }
                } else if (deltaScroll>0) {
                    // 아래로 스크롤 하냐
//                    if ((scrollPosition<=_headerSize && scrollPosition>=0) || overBoundary) {
                    if (scrollPosition<=_headerSize && scrollPosition>=0) {
                        float d = deltaScroll;
                        if (scrollPosition<0) {
                            d += scrollPosition;
                        }
                        newPosition = position - d;
                    }
                }
                
                if (newPosition<0) {
                    newPosition = 0;
                } else if (newPosition<_headerSize-_divPosition) {
                    newPosition = _headerSize - _divPosition;
                }
                
                if (position!=newPosition) {
                    tableView->getScroller()->setScrollPosition(newPosition);
                    
                    float dummy = 0;
                    auto p = dynamic_cast<_ScrollProtocol*>(tableView);
                    if (p) {
                        p->notifyScrollUpdate();
                        p->updateScrollInParentVisit(dummy);
                    }
                }
            }
            tableView->setBaseScrollPosition(_headerSize-_divPosition);
        });
    }
    
    if (lastDivPosition!=_divPosition) {
        if (_headerView && _headerMode==HeaderMode::SCROLL) {
            registerUpdate(FLAG_HEADER_POSITION);
        }
        
        onScrollUpdate();
    }
    
    if (scrollPosition>=scroller->getScrollSize()) {
        if (prevScrollPosition>scrollPosition) {
            deltaScroll = 0;
        } else if (prevScrollPosition<scroller->getScrollSize()) {
            deltaScroll = scroller->getScrollSize() - scrollPosition;
        }
    }
    
    if (lastDivPosition!=_divPosition || deltaScroll!=0) {
        _deltaScroll = deltaScroll;
        registerUpdate(FLAG_SCROLL_NOTIFY);
    }
    
    return updated;
}

void OverlapContainer::onScrollUpdate()
{
    _tableRect.setRect(0, 0, _contentSize.width, _contentSize.height - _divPosition + 1);
    _tableContainer->setScissorRect(_tableRect);
    _tableContainer->setScissorEnable(true);
    
    std::for_each(_items.begin(), _items.end(), [&](SMTableView* tableView){
        tableView->setTableRect(&_tableRect);
    });
    
    _tableContainer->setScrollRect(&_tableRect);
}

void OverlapContainer::onUpdateOnVisit()
{
    if (isUpdate(FLAG_SCROLL_UPDATE)) {
        if (!updateScrollPosition()) {
            unregisterUpdate(FLAG_SCROLL_UPDATE);
        }
    }
    
    if (isUpdate(FLAG_CONTENT_UPDATE)) {
        unregisterUpdate(FLAG_CONTENT_UPDATE);
        
        registerUpdate(FLAG_HEADER_POSITION);
        
        std::for_each(_items.begin(), _items.end(), [&](SMTableView* tableView){
            tableView->setInnerScrollMargin(_headerSize);
        });
    }
    
    if (isUpdate(FLAG_HEADER_POSITION)) {
        unregisterUpdate(FLAG_HEADER_POSITION);
        
        if (_headerView!=nullptr) {
            float headerPosition = 0;
            if (_headerMode==HeaderMode::SCROLL) {
                headerPosition = _headerSize - _divPosition;
            }
            _headerView->setPositionY(_contentSize.height - _headerView->getContentSize().height + headerPosition);
        }
    }
    
    if (isUpdate(FLAG_SCROLL_NOTIFY)) {
        unregisterUpdate(FLAG_SCROLL_NOTIFY);
        
        if (onScrollCallback) {
            onScrollCallback(_divPosition, _deltaScroll);
        }
    }
}

int OverlapContainer::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    if (action==MotionEvent::ACTION_CANCEL) {
        cancelTouchEvent(_tableContainer, touch, point, event);
        cancelTouchEvent(_headerContainer, touch, point, event);
        cancelTouchEvent(_overlayContainer, touch, point, event);
        
        _touchMotionTarget = nullptr;
        _touchTargeted = false;
        
        return TOUCH_FALSE;
    }
    
    int ret = TOUCH_TRUE;
    
    SMView* tableTarget = nullptr;
    SMView* targetContainer = (SMView*)_tableContainer->getMotionTarget();
    if (targetContainer!=nullptr) {
        tableTarget = (SMView*)targetContainer->getMotionTarget();
    }
    
    if (_touchMotionTarget == _headerContainer) {
        bool cancelled = false;
        
        if (tableTarget!=nullptr) {
            auto protocol = dynamic_cast<_ScrollProtocol*>(tableTarget);
            if (protocol!=nullptr && protocol->_inScrollEvent) {
                cancelTouchEvent(_headerContainer, touch, point, event);
                _touchMotionTarget = nullptr;
                cancelled = true;
            }
        }
        
        if (!cancelled) {
            ret = _headerContainer->dispatchTouchEvent(action, touch, point, event);
            if (ret==TOUCH_FALSE || action==MotionEvent::ACTION_UP || action==MotionEvent::ACTION_CANCEL) {
                _touchMotionTarget = nullptr;
            }
        }
    }
    
    if (ret!=TOUCH_INTERCEPT) {
        ret = _tableContainer->dispatchTouchEvent(action, touch, point, event);
        if (ret==TOUCH_INTERCEPT)
        {
            if (_touchMotionTarget==_headerContainer) {
                cancelTouchEvent(_headerContainer, touch, point, event);
                _touchMotionTarget = nullptr;
            } else if (_touchMotionTarget==_overlayContainer) {
                cancelTouchEvent(_overlayContainer, touch, point, event);
                _touchMotionTarget = nullptr;
            }
        }
    } else if (tableTarget) {
        cancelTouchEvent(_tableContainer, touch, point, event);
    }
    
    targetContainer = (SMView*)_tableContainer->getMotionTarget();
    if (targetContainer!=nullptr) {
        tableTarget = (SMView*)targetContainer->getMotionTarget();
    }
    
    if (tableTarget && _targetTableView!=tableTarget) {
        _targetTableView = dynamic_cast<SMTableView*>(tableTarget);
        
        std::for_each(_items.begin(), _items.end(), [&](SMTableView* tableView){
            if (tableView!=_targetTableView) {
                tableView->stop();
            }
        });
    }
    
    if (ret!=TOUCH_INTERCEPT) {
        if (action==MotionEvent::ACTION_DOWN) {
            bool touched = false;
            if (TOUCH_FALSE != _overlayContainer->dispatchTouchEvent(action, touch, point, event)) {
                _touchMotionTarget = _overlayContainer;
                _touchTargeted = true;
                touched = true;
            }
            
            if (!touched && _headerView && point->y > _contentSize.height - _divPosition) {
                if (TOUCH_FALSE != _headerContainer->dispatchTouchEvent(action, touch, point, event)) {
                    _touchMotionTarget = _headerContainer;
                    _touchTargeted = true;
                }
            }
        }
    }
    
    if (ret==TOUCH_FALSE) {
        return TOUCH_TRUE;
    }
    
    return ret;
}
