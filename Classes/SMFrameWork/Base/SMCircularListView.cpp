//
//  SMCircularListView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 11..
//
//

#include "SMCircularListView.h"
#include "../Interface/SMScroller.h"
#include "../Interface/VelocityTracker.h"
#include "../Const/SMViewConstValue.h"
#include "ViewAction.h"
#include "../Util/ViewUtil.h"
#include <cocos2d.h>
#include <cmath>

#define CLEANUP_FLAG (true)
#define FLAG_SCROLL_UPDATE (1<<0)

#define ACTION_TAG_CONSUME (1000)
#define ACTION_TAG_POSITION (1001)


class SMCircularListView::ReuseScrapper {
private:
    int _numberOfTypes;
    
    std::map<std::string, int> _key;
    std::vector<cocos2d::Vector<cocos2d::Node*>> _data;
    
public:
    cocos2d::Node* _internalReuseNode;
    
    std::string _internalReuseIdentifier;
    
public:
    int getReuseType(const std::string& reuseIdentifier);
    void scrap(const std::string& reuseIdentifier, cocos2d::Node* const parent, cocos2d::Node* const child, const bool cleanup=true);
    
    void popBack(const std::string& reuseIdentifier);
    cocos2d::Node* back(const std::string& reuseIdentifier);
    
    ReuseScrapper();
    ~ReuseScrapper();
};

class SMCircularListView::CellsAction : public ViewAction::TransformAction
{
public:
    CREATE_DELAY_ACTION(CellsAction);
    
    virtual void onUpdate(float t) override {
        TransformAction::onUpdate(t);
        ((SMCircularListView*)_target)->onCellAction(getTag(), _cells, t, false);
    }
    
    virtual void onEnd() override {
        ((SMCircularListView*)_target)->onCellAction(getTag(), _cells, 1, true);
    }
    
    void setTargetCells(const std::vector<cocos2d::Node*>& cells) {
        _cells = cells;
    }
    
private:

    std::vector<cocos2d::Node*> _cells;
    
};

class CellPositionAction : ViewAction::TransformAction
{
public:
    CREATE_DELAY_ACTION(CellPositionAction);
    virtual void onUpdate(float dt) override {
        TransformAction::onUpdate(dt);
        
        auto listView = (SMCircularListView*)_target;
        for (auto cell : _cells) {
            if (listView->cellDeleteUpdate) {
                listView->cellDeleteUpdate(cell, dt);
            }
        }
    }
    
    virtual void onEnd() override {
        TransformAction::onEnd();
        
        auto listView = (SMCircularListView*)_target;
        for (auto cell : _cells) {
            listView->removeChild(cell);
        }
    }
    
    void setTargetCells(const std::vector<cocos2d::Node*>& cells) {
        _cells = cells;
    }
    
    std::vector<cocos2d::Node*> _cells;
};

SMCircularListView::ReuseScrapper::ReuseScrapper() :
_numberOfTypes(0)
{
    
}

SMCircularListView::ReuseScrapper::~ReuseScrapper()
{
    for (size_t i=0; i<_data.size(); i++) {
        _data[i].clear();
    }
    
    _data.clear();
}

int SMCircularListView::ReuseScrapper::getReuseType(const std::string &reuseIdentifier)
{
    int reuseType;
    
    std::map<std::string, int>::iterator iter = _key.find(reuseIdentifier);
    if (iter==_key.end()) {
        // 못찾으면 타입을 추가
        reuseType = _numberOfTypes;
        _key.insert(std::pair<std::string, int>(reuseIdentifier, _numberOfTypes++));
        _data.push_back(cocos2d::Vector<cocos2d::Node*>());
    } else {
        // 찾았으면 찾은 타입을 반환
        reuseType = iter->second;
    }
    
    return reuseType;
}

void SMCircularListView::ReuseScrapper::scrap(const std::string &reuseIdentifier, cocos2d::Node *const parent, cocos2d::Node *const child, const bool cleanup)
{
    int reuseType = getReuseType(reuseIdentifier);
    
    cocos2d::Vector<cocos2d::Node*>& queue = _data[reuseType];
    
    if (parent) {
        child->retain();
        parent->removeChild(child, cleanup);
    }
    
    queue.pushBack(child);
    
    if (parent) {
        child->release();
    }
}

cocos2d::Node* SMCircularListView::ReuseScrapper::back(const std::string &reuseIdentifier)
{
    int reuseType = getReuseType(reuseIdentifier);
    
    if (_data[reuseType].size()>0) {
        return _data[reuseType].back();
    }
    
    return nullptr;
}

void SMCircularListView::ReuseScrapper::popBack(const std::string &reuseIdentifier)
{
    int reuseType = getReuseType(reuseIdentifier);
    
    _data[reuseType].popBack();
}



// SMCircularListView

cocos2d::Node* SMCircularListView::dequeueReusableCellWithIdentifier(const std::string &identifier)
{
    _reuseScrapper->_internalReuseIdentifier = identifier;
    _reuseScrapper->_internalReuseNode = _reuseScrapper->back(identifier);
    
    return _reuseScrapper->_internalReuseNode;
}

SMCircularListView::SMCircularListView() :
_lastScrollPosition(0),
_reuseScrapper(nullptr),
_actionLock(false),
cellForRowAtIndex(nullptr),
numberOfRows(nullptr),
cellDeleteUpdate(nullptr),
positionCell(nullptr),
scrollAlignedCallback(nullptr),
pageScrollCallback(nullptr),
_fillWithCellsFirstTime(false),
initFillWithCells(nullptr),
_velocityTracker(nullptr),
_scroller(nullptr)
{
    
}

SMCircularListView::~SMCircularListView()
{
    CC_SAFE_DELETE(_velocityTracker);
    CC_SAFE_DELETE(_reuseScrapper);
    CC_SAFE_DELETE(_scroller);
    unscheduleScrollUpdate();
}

SMCircularListView* SMCircularListView::create(const Config &config)
{
    SMCircularListView * listView = new (std::nothrow) SMCircularListView();
    if (listView && listView->initWithConfig(config)) {
        listView->autorelease();
    } else {
        CC_SAFE_DELETE(listView);
    }
    
    return listView;
}

bool SMCircularListView::initWithConfig(const SMCircularListView::Config &config)
{
    _config = config;
    
    _reuseScrapper = new ReuseScrapper();
    
    if (_config.circular) {
        _scroller = new InfinityScroller();
    } else {
        _scroller = new FinityScroller();
    }
    
    _scroller->setCellSize(_config.cellSize);
    _scroller->setWindowSize(_config.windowSize);
    _scroller->setScrollMode(_config.scrollMode);
    _scroller->onAlignCallback = CC_CALLBACK_1(SMCircularListView::onScrollAligned, this);
    
    _lastScrollPosition = _scroller->getScrollPosition();
    _velocityTracker = new VelocityTracker();
    _velocityTracker->clear();
    
    scheduleScrollUpdate();
    
    return true;
}

void SMCircularListView::onScrollAligned(bool aligned)
{
    if (scrollAlignedCallback) {
        if (aligned) {
            _currentPage = getAlignedIndex();
            
            if (!_config.circular) {
                if (_currentPage<0) {
                    _currentPage = 0;
                } else if (_currentPage>=numberOfRows()) {
                    _currentPage = numberOfRows() - 1;
                }
            }
            
            scrollAlignedCallback(aligned, _currentPage, false);
        } else {
            scrollAlignedCallback(aligned, 0, false);
        }
    }
}

void SMCircularListView::setScrollPosition(float position)
{
    _scroller->setScrollPosition(position);
    positionChildren(position, numberOfRows());
}

int SMCircularListView::getAlignedIndex()
{
    float scrollPosition = ((int)(_scroller->getNewScrollPosition()*10))/10.0f;
    
    int index;
    if (scrollPosition>=0) {
        index = (int)(scrollPosition/_config.cellSize);
    } else {
        index =  std::floor(scrollPosition/_config.cellSize);
    }
    
    return index;
}

int SMCircularListView::getIndexForCell(cocos2d::Node *cell)
{
    auto children = getChildren();
    for (auto child : children) {
        if (cell==child) {
            auto protocol = dynamic_cast<CellProtocol*>(child);
            if (protocol) {
                return convertToIndex(protocol->getCellIndex(), numberOfRows());
            }
        }
    }
    
    // not found
    return -1;
}

int SMCircularListView::convertToIndex(int realIndex, int numRows)
{
    int index;
    if (realIndex>=0) {
        index = realIndex % numRows;
    } else {
        index = (numRows-std::abs(realIndex % numRows)) % numRows;
    }
    return index;
}

void SMCircularListView::positionChildren(float scrollPosition, int numRows)
{
    // 소수점 두자리부터 절삭
    scrollPosition = ((int)(scrollPosition*10))/10.0f;
    
    float maxScrollSize = _config.cellSize * numRows + _config.preloadPadding*2;
    float adjPosition = scrollPosition - _config.anchorPosition - _config.preloadPadding;
    float norPosition;
    
    if (adjPosition>=0) {
        norPosition = ::fmodf(adjPosition, maxScrollSize);
    } else {
        norPosition = ::fmodf(maxScrollSize-std::abs(::fmodf(adjPosition, maxScrollSize)), maxScrollSize);
    }
    
    float xx = -::fmod(norPosition, _config.cellSize);
    
    int start, end;
    if (adjPosition>=0) {
        start = (int)adjPosition / _config.cellSize;
    } else {
        start = std::floor(adjPosition/_config.cellSize);
    }
    
    end = start + std::ceil(_config.windowSize / _config.cellSize);
    if (xx + _config.cellSize * (end-start) < _config.windowSize) {
        end += 1;
    }
    
    int first = end;
    int last = start;
    
    auto children = getChildren();
    for (auto child : children) {
        auto cell = dynamic_cast<CellProtocol*>(child);
        
        if (cell && !cell->_deleted) {
            int realIndex = cell->getCellIndex();
            if (realIndex>=start && realIndex<end) {
                float x = xx + (realIndex - start) * _config.cellSize;
                cell->setCellPosition(x);
                if (positionCell) {
                    positionCell(child, x, false);
                }
                
                first = std::min(first, realIndex);
                last = std::max(last, realIndex);
            } else {
                auto reuseIdentifier = cell->getCellIdentifier();
                if (reuseIdentifier.empty()) {
                    removeChild(child, CLEANUP_FLAG);
                } else {
                    _reuseScrapper->scrap(reuseIdentifier, this, child, CLEANUP_FLAG);
                }
            }
        }
    }
    
    // 앞부터
    while (first>start) {
        int realIndex = first-1;
        if (!_config.circular && (realIndex<0 || realIndex>=numRows)) {
            first--;
            last = std::max(last, realIndex+1);
            continue;
        }
        
        int index = convertToIndex(realIndex, numRows);
        
        _reuseScrapper->_internalReuseIdentifier = "";
        _reuseScrapper->_internalReuseNode = nullptr;
        
        auto cell = cellForRowAtIndex(index);
        auto protocol = dynamic_cast<CellProtocol*>(cell);
        CCASSERT(protocol!=nullptr, "cell must override CellProtocol");
        
        if (protocol) {
            protocol->setCellIndex(realIndex);
            protocol->setReuseIdentifier(_reuseScrapper->_internalReuseIdentifier);
            addChild(cell);
            
            if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == cell) {
                _reuseScrapper->popBack(_reuseScrapper->_internalReuseIdentifier);
            }
            
            float x = xx + (realIndex-start) * _config.cellSize;
            protocol->setCellPosition(x);
            if (positionCell) {
                positionCell(cell, x, true);
            }
        }
        
        _reuseScrapper->_internalReuseIdentifier = "";
        _reuseScrapper->_internalReuseNode = nullptr;
        
        if (!cell) {
            break;
        }
        
        first--;
        last = std::max(last, realIndex+1);
    }
    
    // 뒤부터
    while (last+1<end) {
        float realIndex = last + 1;
        if (!_config.circular && (realIndex<0 || realIndex>=numRows)) {
            last++;
            continue;
        }
        
        int index = convertToIndex(realIndex, numRows);
        
        _reuseScrapper->_internalReuseIdentifier = "";
        _reuseScrapper->_internalReuseNode = nullptr;

        auto cell = cellForRowAtIndex(index);
        auto protocol = dynamic_cast<CellProtocol*>(cell);
        
        if (protocol) {
            protocol->setCellIndex(realIndex);
            protocol->setReuseIdentifier(_reuseScrapper->_internalReuseIdentifier);
            addChild(cell);
            
            if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode==cell) {
                _reuseScrapper->popBack(_reuseScrapper->_internalReuseIdentifier);
            }
            
            float x = xx + (realIndex-start) * _config.cellSize;
            protocol->setCellPosition(x);
            if (positionCell) {
                positionCell(cell, x, true);
            }
        }
        
        _reuseScrapper->_internalReuseIdentifier = "";
        _reuseScrapper->_internalReuseNode = nullptr;
        
        if (!cell) {
            break;
        }
        
        last++;
    }
    
    if (pageScrollCallback) {
        float position = ::fmodf((_config.anchorPosition+norPosition)/_config.cellSize, (float)numRows);
        pageScrollCallback(position);
    }
}

void SMCircularListView::scheduleScrollUpdate()
{
    registerUpdate(FLAG_SCROLL_UPDATE);
}

void SMCircularListView::unscheduleScrollUpdate()
{
    unregisterUpdate(FLAG_SCROLL_UPDATE);
}

void SMCircularListView::onUpdateOnVisit()
{
    if (_contentSize.width<=0 || _contentSize.height<=0 || _actionLock) {
        return;
    }
    
    int numRows = numberOfRows();
    
    _scroller->setScrollSize(_config.cellSize * numRows);
    
    bool updated = _scroller->update();
    float scrollPosition = _scroller->getScrollPosition();
    
    positionChildren(scrollPosition, numRows);
    
    if (!updated) {
        unscheduleScrollUpdate();
    }
    
    _deltaScroll = _lastScrollPosition - scrollPosition;
    _lastScrollPosition = scrollPosition;
    
    if (!_fillWithCellsFirstTime) {
        _fillWithCellsFirstTime = true;
        if (initFillWithCells) {
            initFillWithCells();
        }
    }
}

int SMCircularListView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    if (_actionLock) {
        return TOUCH_TRUE;
    }
    
    if (!_inScrollEvent && _scroller->isTouchable()) {
        if (action==MotionEvent::ACTION_DOWN && _scroller->getState() != SMScroller::State::STOP) {
            scheduleScrollUpdate();
        }
        int ret = SMView::dispatchTouchEvent(action, touch, point, event);
        if (ret==TOUCH_INTERCEPT) {
            return ret;
        }
    }
    
    float x = point->x;
    float y = point->y;
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _inScrollEvent = false;
            _touchFocused = true;
            
            _lastMotionX = x;
            _lastMotionY = y;
            _scroller->onTouchDown();
            
            _velocityTracker->addMovement(event);
        }
            break;
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
        {
            _touchFocused = false;
            if (_inScrollEvent) {
                _inScrollEvent = false;
                
                float vx, vy;
                _velocityTracker->getVelocity(0, &vx, &vy);
                
                if (isVertical()) {
                    if (std::abs(vy) > 200) {
                        if (std::abs(vy)>_config.maxVelocity) {
                            vy = ViewUtil::signum(vy) * _config.maxVelocity;
                        } else if (_config.minVelocity>0 && std::abs(vy)<_config.minVelocity) {
                            vy = ViewUtil::signum(vy) * _config.minVelocity;
                        }
                        _scroller->onTouchFling(-vy, _currentPage);
                    } else {
                        _scroller->onTouchUp();
                    }
                } else {
                    if (std::abs(vx) > 200) {
                        if (std::abs(vx)>_config.maxVelocity) {
                            vx = ViewUtil::signum(vx) * _config.maxVelocity;
                        } else if (_config.minVelocity>0 && std::abs(vx)<_config.minVelocity) {
                            vx = ViewUtil::signum(vx) * _config.minVelocity;
                        }
                        _scroller->onTouchFling(vx, _currentPage);
                    } else {
                        _scroller->onTouchUp();
                    }
                }
                
                scheduleScrollUpdate();
            } else {
                _scroller->onTouchUp();
                scheduleScrollUpdate();
            }
            
            _velocityTracker->clear();
        }
            break;
        case MotionEvent::ACTION_MOVE:
        {
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
            
            if (_touchFocused && !_inScrollEvent) {
                float ax = std::abs(x-_lastMotionX);
                float ay = std::abs(y-_lastMotionY);
                
                int dir = ViewUtil::getDirection(ax, ay);
                
                if (isVertical()) {
                    if ((dir==ViewUtil::UP || dir==ViewUtil::DOWN) && std::abs(ay)>SMViewConstValue::Config::SCROLL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
                } else {
                    if ((dir==ViewUtil::LEFT || dir==ViewUtil::RIGHT) && std::abs(ax)>SMViewConstValue::Config::SCROLL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
                }
                
                if (_inScrollEvent) {
                    if (_touchMotionTarget!=nullptr) {
                        cancelTouchEvent(_touchMotionTarget, touch, point, event);
                        _touchMotionTarget = nullptr;
                    }
                }
            }
            
            if (_inScrollEvent) {
                if (isVertical()) {
                    _scroller->onTouchScroll(-deltaY);
                } else {
                    _scroller->onTouchScroll(deltaX);
                }
                _lastMotionX = x;
                _lastMotionY = y;
                scheduleScrollUpdate();
            }
        }
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    return TOUCH_TRUE;
}

bool SMCircularListView::sortFunc(CellProtocol *l, CellProtocol *r)
{
    return l->getCellPosition() < r->getCellPosition();
}

bool SMCircularListView::deleteCell(int targetIndex, float deleteDt, float deleteDelay, float positionDt, float positionDelay)
{
    int numRows = numberOfRows();
    if (numRows<=1 || targetIndex<0 || targetIndex>=numRows) {
        return false;
    }
    
    _deleteIndex = convertToIndex(targetIndex, numRows);
    
    // make cell array from children
    auto children = getChildren();
    CCASSERT(children.size()>0, "children size must bigger than zero");
    if (children.size()==0) {
        return false;
    }
    std::vector<CellProtocol*> cells;
    for (auto child : children) {
        auto cell = dynamic_cast<CellProtocol*>(child);
        if (cell) {
            cells.emplace_back(cell);
        }
    }
    
    // sort cells
    std::sort(cells.begin(), cells.end(), sortFunc);
    
    int realIndex = cells.at(0)->getCellIndex();
    int index = convertToIndex(realIndex, numRows);
    int diff = realIndex - index;
    
    
    int deleteCount = 0;
    for (auto cell : cells) {
        int idx = convertToIndex(cell->getCellIndex(), numRows);
        if (idx==targetIndex) {
            cell->markDelete();
            deleteCount++;
        }
        cell->setCellIndex(cell->getCellIndex()-diff-deleteCount);
    }
    
    float scrollPosition = _scroller->getScrollPosition();
    float maxScrollSize = _config.cellSize*numRows + _config.preloadPadding*2;
    float adjPosition = scrollPosition - _config.anchorPosition - _config.preloadPadding;
    float norPosition;
    
    if (adjPosition>0) {
        norPosition = ::fmodf(adjPosition, maxScrollSize);
    } else {
        norPosition = ::fmodf(maxScrollSize-std::abs(::fmodf(adjPosition, maxScrollSize)), maxScrollSize);
    }
    
    _scroller->setScrollPosition(norPosition);
    
    // 지운거 만큼 추가
    int count = deleteCount;
    auto lastCell = cells.at(cells.size()-1);
    realIndex = lastCell->getCellIndex()+2;
    float position = lastCell->getCellPosition() + _config.cellSize;
    
    for (; count>0; ) {
//    while (count>0) {
        index = convertToIndex(realIndex, numRows);
        
        if (index!=targetIndex) {
            _reuseScrapper->_internalReuseIdentifier = "";
            _reuseScrapper->_internalReuseNode = nullptr;
            
            auto cell = cellForRowAtIndex(index);
            if (cell) {
                auto protocol = dynamic_cast<CellProtocol*>(cell);
                
                protocol->setCellIndex(realIndex);
                protocol->setReuseIdentifier(_reuseScrapper->_internalReuseIdentifier);
                addChild(cell);
                
                if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode==cell) {
                    _reuseScrapper->popBack(_reuseScrapper->_internalReuseIdentifier);
                }
                
                protocol->setCellPosition(position);
                if (positionCell) {
                    positionCell(cell, position, true);
                }
                
                position += _config.cellSize;
            }
            _reuseScrapper->_internalReuseIdentifier = "";
            _reuseScrapper->_internalReuseNode = nullptr;
            
            if (!cell) {
                break;
            }
            
            count--;
        }
        
        realIndex++;
    }
    
    // resort children
    children = getChildren();
    cells.clear();
    for (auto child : children) {
        auto cell = dynamic_cast<CellProtocol*>(child);
        if (cell) {
            cells.emplace_back(cell);
        }
    }
    std::sort(cells.begin(), cells.end(), sortFunc);
    
    position = cells.at(0)->getCellPosition();
    
    std::vector<cocos2d::Node*> deleteCells;
    std::vector<cocos2d::Node*> remainCells;
    
    for (auto cell : cells) {
        index = convertToIndex(cell->getCellIndex(), numRows);
        auto child = dynamic_cast<cocos2d::Node*>(cell);
        
        if (!cell->_deleted) {
            cell->_aniSrc = cell->getCellPosition();
            cell->_aniDst = position;
            cell->_aniIndex = cell->getCellIndex();
            position += _config.cellSize;
            
            remainCells.emplace_back(child);
        } else {
            deleteCells.emplace_back(child);
        }
    }
    
    auto deleteAction = CellsAction::create();
    deleteAction->setTag(ACTION_TAG_CONSUME);
    deleteAction->setTargetCells(deleteCells);
    deleteAction->setTimeValue(deleteDt, deleteDelay);
    runAction(deleteAction);
    
    auto positionAction = CellsAction::create();
    positionAction->setTag(ACTION_TAG_POSITION);
    positionAction->setTargetCells(remainCells);
    positionAction->setTimeValue(positionDt, positionDelay);
    runAction(positionAction);
    
    _actionLock = true;
    return true;
}

bool SMCircularListView::deleteCell(cocos2d::Node *target, float deleteDt, float deleteDelay, float positionDt, float positionDelay)
{
    return deleteCell(getIndexForCell(target), deleteDt, deleteDelay, positionDt, positionDelay);
}

void SMCircularListView::onCellAction(int tag, std::vector<cocos2d::Node *> cells, float dt, bool complete)
{
    if (tag==ACTION_TAG_CONSUME) {
        if (complete) {
            for (auto cell : cells) {
                removeChild(cell);
            }
        } else {
            if (cellDeleteUpdate) {
                for (auto cell : cells) {
                    cellDeleteUpdate(cell, dt);
                }
            }
        }
//    } else if (tag==ACTION_TAG_POSITION) {
    } else if (ACTION_TAG_POSITION) {
        if (complete) {
            for (auto cell : cells) {
                auto protocol = dynamic_cast<CellProtocol*>(cell);
                protocol->setCellIndex(protocol->_aniIndex);
            }
            
            float scrollPosition = _config.cellSize*convertToIndex(_deleteIndex, numberOfRows());
            _scroller->setScrollPosition(scrollPosition);
            
            if (scrollAlignedCallback) {
                scrollAlignedCallback(true, _deleteIndex, true);
            }
            
            _actionLock = false;
        } else {
            dt = cocos2d::tweenfunc::cubicEaseOut(dt);
            for (auto cell : cells) {
                auto protocol = dynamic_cast<CellProtocol*>(cell);
                float x = ViewUtil::interpolation(protocol->_aniSrc, protocol->_aniDst, dt);
                protocol->setCellPosition(x);
                if (positionCell) {
                    positionCell(cell, x, false);
                }
            }
        }
    }
}

cocos2d::Vector<cocos2d::Node*>& SMCircularListView::getVisibleCells()
{
    return getChildren();
}

void SMCircularListView::stop(bool align)
{
    unscheduleScrollUpdate();
    
    _scroller->onTouchDown();
    if (align) {
        _scroller->onTouchUp();
    }
}

void SMCircularListView::scrollByWithDuration(float distance, float dt)
{
    if (std::abs(distance)>0) {
        ((InfinityScroller*)_scroller)->scrollByWithDuration(distance, dt);
        scheduleScrollUpdate();
    }
}

void SMCircularListView::runFling(float velocity)
{
    _scroller->onTouchFling(velocity, 0);
    scheduleScrollUpdate();
}

void SMCircularListView::updateData()
{
    scheduleScrollUpdate();
}
