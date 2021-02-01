//
//  SMTableView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#include "SMTableView.h"
#include "Intent.h"
#include "../Const/SMViewConstValue.h"
#include "../Util/ViewUtil.h"
#include <2d/CCActionInterval.h>
#include <cocos2d.h>
#include "../Interface/VelocityTracker.h"

#define CLEANUP_FALG (true)
#define FLAG_SCROLL_UPDATE   (1<<0)

const uint8_t SMTableView::ITEM_FLAG_DELETE = (1<<0);
const uint8_t SMTableView::ITEM_FLAG_RESIZE = (1<<1);
const uint8_t SMTableView::ITEM_FLAG_INSERT = (1<<2);



// index path
IndexPath::IndexPath() : _section(0), _column(0), _index(0)
{
}

IndexPath::IndexPath(const int index) : _section(0), _column(0), _index(index)
{
}

IndexPath::IndexPath(const int section, const int index) : _section(section), _column(0), _index(index)
{
}

IndexPath::IndexPath(const int section, const int column, const int index) : _section(section), _column(column), _index(index)
{
}

IndexPath& IndexPath::operator=(const IndexPath& rhs) {
    if (this == &rhs) {
        return *this;
    }
    
    _section = rhs._section;
    _column = rhs._column;
    _index = rhs._index;
    
    return *this;
}

bool IndexPath::operator==(const IndexPath& rhs) const {
    return _index == rhs._index && _section == rhs._section;
}

bool IndexPath::operator!=(const IndexPath& rhs) const {
    return _index != rhs._index || _section != rhs._section;
}

bool IndexPath::operator<=(const IndexPath& rhs) const {
    return _index <= rhs._index && _section == rhs._section;
}

bool IndexPath::operator>=(const IndexPath& rhs) const {
    return _index >= rhs._index && _section == rhs._section;
}

bool IndexPath::operator<(const IndexPath& rhs) const {
    return _index < rhs._index && _section == rhs._section;
}

bool IndexPath::operator>(const IndexPath& rhs) const {
    return _index > rhs._index && _section == rhs._section;
}

IndexPath& IndexPath::operator++() {
    ++_index;
    return *this;
}

IndexPath& IndexPath::operator--() {
    --_index;
    return *this;
}

IndexPath& IndexPath::operator+=(const int value) {
    _index += value;
    return *this;
}

IndexPath& IndexPath::operator-=(const int value) {
    _index -= value;
    return *this;
}

IndexPath IndexPath::operator++(int) {
    IndexPath tmp(*this);
    ++*this;
    return tmp;
}

IndexPath IndexPath::operator--(int) {
    IndexPath tmp(*this);
    --*this;
    return tmp;
}

IndexPath IndexPath::operator+(const int value) {
    IndexPath tmp(*this);
    tmp._index += value;
    return tmp;
}

IndexPath IndexPath::operator-(const int value) {
    IndexPath tmp(*this);
    tmp._index -= value;
    return tmp;
}




// item
SMTableView::Item::Item() :
_reuseType(0),
_size(0),
_newSize(0),
_flags(0),
_state(nullptr),
_tag(0),
_reload(false),
_dontReuse(false)
{
}

SMTableView::Item::Item(const IndexPath& indexPath, const int type, const float size) :
_indexPath(indexPath),
_reuseType(type),
_size(size),
_newSize(size),
_flags(0),
_state(nullptr),
_tag(0),
_reload(false),
_dontReuse(false)
{
}

SMTableView::Item::~Item() {
}

SMTableView::Item& SMTableView::Item::operator=(const Item &rhs) {
    if (this == &rhs) {
        return *this;
    }
    
    this->_indexPath = rhs._indexPath;
    this->_tag = rhs._tag;
    this->_reuseType = rhs._reuseType;
    this->_size = rhs._size;
    this->_newSize = rhs._newSize;
    this->_flags = rhs._flags;
    this->_state = rhs._state;
    
    if (this->_state) {
        this->_state->retain();
    }
    
    return *this;
}



// cursor
SMTableView::Cursor::Cursor() : _location(0), _position(0)
{
}

SMTableView::Cursor& SMTableView::Cursor::operator=(const Cursor &rhs) {
    if (this == &rhs) {
        return *this;
    }
    
    this->_iter = rhs._iter;
    this->_location = rhs._location;
    this->_position = rhs._position;
    this->_data = rhs._data;
    
    return *this;
}

bool SMTableView::Cursor::operator==(const Cursor& rhs) const {
    return _position == rhs._position;
}

bool SMTableView::Cursor::operator!=(const Cursor& rhs) const {
    return _position != rhs._position;
}

bool SMTableView::Cursor::operator>(const Cursor& rhs) const {
    return _position > rhs._position;
}

bool SMTableView::Cursor::operator<(const Cursor& rhs) const {
    return _position < rhs._position;
}

bool SMTableView::Cursor::operator>=(const Cursor& rhs) const {
    return _position >= rhs._position;
}

bool SMTableView::Cursor::operator<=(const Cursor& rhs) const {
    return _position <= rhs._position;
}

SMTableView::Cursor& SMTableView::Cursor::operator++() {
    if (!isEnd()) {
        ++_position;
        _location += getItem()._size;
        ++_iter;
    }
    
    return *this;
}

SMTableView::Cursor& SMTableView::Cursor::operator--() {
    if (!isBegin()) {
        --_position;
        --_iter;
        _location -= getItem()._size;
    }
    
    return *this;
}

SMTableView::Cursor SMTableView::Cursor::operator++(int) {
    Cursor tmp(*this);
    ++*this;
    return tmp;
}

SMTableView::Cursor SMTableView::Cursor::operator--(int) {
    Cursor tmp(*this);
    --*this;
    return tmp;
}

int SMTableView::Cursor::operator+(const Cursor& rhs) const {
    return this->_position + rhs._position;
}

int SMTableView::Cursor::operator-(const Cursor& rhs) const {
    return this->_position - rhs._position;
}

SMTableView::Cursor& SMTableView::Cursor::advance(int offset) {
    if (offset > 0) {
        for (;offset > 0; offset--) {
            ++*this;
        }
    } else if (offset < 0) {
        for (;offset < 0; offset++) {
            --*this;
        }
    }
    
    return *this;
}

void SMTableView::Cursor::init(std::list<Item>& data) {
    _iter = data.end();
    _position = 0;
    _location = 0;
    _data = &data;
}





// column info
SMTableView::Cursor* SMTableView::ColumnInfo::obtainCursorBuffer(Cursor& cursor) {
    Cursor* clonCursor = new Cursor(cursor);
    _buffer.push_back(clonCursor);
    
    return clonCursor;
}

void SMTableView::ColumnInfo::recycleCursorBuffer(Cursor* cursor) {
    if (cursor) {
        for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter) {
            if (*iter == cursor) {
                _buffer.erase(iter);
                delete cursor;
                break;
            }
        }
    }
}

void SMTableView::ColumnInfo::resizeCursorBuffer(Cursor& targetCursor, const float deltaSize) {
    for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter) {
        Cursor& cursor = **iter;
        
        if (cursor > targetCursor) {
            cursor.offsetLocation(deltaSize);
        }
    }
}

void SMTableView::ColumnInfo::deleteCursorBuffer(Cursor& targetCursor) {
    float size = targetCursor.getItem()._size;
    
    for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter) {
        Cursor& cursor = **iter;
        
        if (cursor > targetCursor) {
            cursor.offset(-1, -size);
        } else if (cursor == targetCursor) {
            cursor.incIterator();
        }
    }
}

void SMTableView::ColumnInfo::insertCursorBuffer(Cursor& targetCursor) {
    float size = targetCursor.getItem()._size;
    
    for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter) {
        Cursor& cursor = **iter;
        
        if (cursor >= targetCursor) {
            cursor.offset(+1, +size);
        }
    }
}


SMTableView::ColumnInfo::ColumnInfo() {
}

SMTableView::ColumnInfo::~ColumnInfo() {
    for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); iter++) {
        delete (*iter);
    }
}

void SMTableView::ColumnInfo::init(SMTableView* parent, ssize_t column) {
    _parent = parent;
    _column = column;
    
    _numAliveItem = 0;
    _lastIndexPath = IndexPath(0, (int)_column, 0);
    _resizeReserveSize = 0;
    
    _data.clear();
    
    _firstCursor.init(_data);
    _lastCursor.init(_data);
    _viewFirstCursor.init(_data);
    _viewLastCursor.init(_data);
    
    for (std::vector<Cursor*>::iterator iter = _buffer.begin(); iter != _buffer.end(); iter++) {
        delete (*iter);
    }
    _buffer.clear();
}

/**
 * 뷰 전방 inflate
 */
SMTableView::Cursor SMTableView::ColumnInfo::advanceViewFirst() {
    // 앞 방향으로 진행의 경우 데이터는 이미 추가가 되어 있다.
    CC_ASSERT(_data.size() > 0 && _viewFirstCursor.getPosition() > 0);
    
    return --_viewFirstCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::advanceViewLast(const IndexPath& indexPath, const int type, const float size, const int8_t flags) {
    Cursor cursor;
    
    if (_data.size() == 0) {
        // 첫번째 데이터
        _data.push_back(Item(indexPath, type, size));
        
        _firstCursor.setIterator(_data.begin());
        
        _lastCursor = _firstCursor;
        ++_lastCursor;
        
        _viewFirstCursor = _firstCursor;
        _viewLastCursor = _lastCursor;
        
        _lastIndexPath = indexPath;
        _numAliveItem++;
        
        cursor = _viewFirstCursor;
    } else if (_viewLastCursor.isEnd()) {
        // 마지막 데이터
        cursor = _viewLastCursor;
        cursor.setIterator(_data.insert(_viewLastCursor.getIterator(), Item(indexPath, type, size)));
        
        _viewLastCursor = cursor;
        _lastCursor = ++_viewLastCursor;
        
        _lastIndexPath = indexPath;
        _numAliveItem++;
        
    } else {
        // 이미 생성되어 있는거...
        cursor = _viewLastCursor++;
    }
    
    return cursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::retreatViewFirst() {
    return ++_viewFirstCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::retreatViewLast() {
    return --_viewLastCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::getFirstCursor() {
    return _firstCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::getLastCursor() {
    return _lastCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::getViewFirstCursor() {
    return _viewFirstCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::getViewLastCursor() {
    return _viewLastCursor;
}

SMTableView::Cursor SMTableView::ColumnInfo::getFirstCursor(const int offset) {
    Cursor tmp(_firstCursor);
    return tmp.advance(offset);
}

SMTableView::Cursor SMTableView::ColumnInfo::getLastCursor(const int offset) {
    Cursor cursor(_lastCursor);
    return cursor.advance(offset);
}

SMTableView::Cursor SMTableView::ColumnInfo::getViewFirstCursor(const int offset) {
    Cursor tmp(_viewFirstCursor);
    return tmp.advance(offset);
}

SMTableView::Cursor SMTableView::ColumnInfo::getViewLastCursor(const int offset) {
    Cursor tmp(_viewLastCursor);
    return tmp.advance(offset);
}

void SMTableView::ColumnInfo::rewindViewLastCursor() {
    _viewLastCursor = _firstCursor;
}

void SMTableView::ColumnInfo::setViewCursor(Cursor& cursor) {
    _viewFirstCursor = cursor;
    _viewLastCursor = cursor;
    _viewLastCursor++;
}


void SMTableView::ColumnInfo::resizeCursor(Cursor& cursor) {
    if (cursor.getItem()._size != cursor.getItem()._newSize) {
        Item& item = cursor.getItem();
        
        float deltaSize = item._newSize - item._size;
        item._size = item._newSize;
        
        Cursor* c[3] = { &_viewFirstCursor, &_viewLastCursor, &_lastCursor };
        for (int i = 0; i < 3; i++) {
            if (*c[i] > cursor) {
                c[i]->offsetLocation(deltaSize);
            }
        }
        
        resizeCursorBuffer(cursor, deltaSize);
        
        
        _resizeReserveSize -= deltaSize;
    }
}

void SMTableView::ColumnInfo::markDeleteCursor(Cursor& cursor) {
    if (_data.size() == 0)
        return;
    
    if (_column != cursor.getIndexPath().getColumn()) {
        // 다른 컬럼
        IndexPath indexPath = cursor.getItem()._indexPath;
        
        // index보다 큰 item index - 1
        Cursor c;
        // 뒤어서부터 찾아봄.
        if (!_viewLastCursor.isEnd() && _viewLastCursor.getIndexPath() <= indexPath) {
            c = _viewLastCursor;
        } else if (!_viewFirstCursor.isEnd() && _viewFirstCursor.getIndexPath() <= indexPath) {
            c = _viewFirstCursor;
        } else {
            c = _firstCursor;
        }
        
        // 경계점까지 cursor 증가
        for (; c < _lastCursor && c.getIndexPath() <= indexPath; ++c);
        
        // 나머지 모든 index 감소시킴
        std::for_each(c.getIterator(), _data.end(), [&](Item &i) {
            i._indexPath--;
        });
    } else {
        // 현재 컬럼에서 삭제
        _numAliveItem--;
        CCASSERT(_numAliveItem >= 0, "_numAliveItem < 0");
        
        // Cursor 에 삭제된 아이템 표시
        cursor.getItem()._flags |= ITEM_FLAG_DELETE;
        
        Cursor c = cursor;
        ++c;
        
        // 나머지 모든 index 감소시킴
        std::for_each(c.getIterator(), _data.end(), [&](Item &i) {
            i._indexPath--;
        });
        
        if (_numAliveItem == 0) {
            // 모든 아이템 삭제됨.
            _lastIndexPath = IndexPath();
        }
    }
    
    // lastIndex 세팅
    if (_numAliveItem > 0) {
        for (std::list<Item>::reverse_iterator iter = _data.rbegin(); iter != _data.rend(); iter++) {
            if (!(*iter).isDeleted()) {
                _lastIndexPath = (*iter)._indexPath;
                break;
            }
        }
    }
}

void SMTableView::ColumnInfo::deleteCursor(Cursor& cursor) {
    
    deleteCursorBuffer(cursor);
    
    Cursor* c[4] = { &_firstCursor, &_viewFirstCursor, &_viewLastCursor, &_lastCursor };
    for (int i = 0; i < 4; i++) {
        if (cursor < *c[i]) {
            c[i]->offset(-1, -cursor.getItem()._size);
        } else if (cursor == *c[i]) {
            c[i]->incIterator();
        }
    }
    
    // 데이터 삭제
    _data.erase(cursor.getIterator());
}

SMTableView::Cursor SMTableView::ColumnInfo::markInsertItem(const IndexPath& indexPath) {
    Cursor cursor;
    
    if (_data.size() > 0) {
        // 다른 컬럼
        // index보다 큰거나 같은 item index + 1
        Cursor c;
        // 뒤에서 부터 찾아봄
        if (!_viewLastCursor.isEnd() && _viewLastCursor.getIndexPath() <= indexPath) {
            c = _viewLastCursor;
        } else if (!_viewFirstCursor.isEnd() && _viewFirstCursor.getIndexPath() <= indexPath) {
            c = _viewFirstCursor;
        } else {
            c = _firstCursor;
        }
        
        // 경계점까지 cursor 증가
        for (; c < _lastCursor && c.getIndexPath() < indexPath; ++c);
        
        // 이 위치에 추가
        cursor = c;
        
        // 나머지 모든 index 증가시킴
        std::for_each(c.getIterator(), _data.end(), [&](Item &i) {
            i._indexPath++;
        });
        
        for (std::list<Item>::reverse_iterator iter = _data.rbegin(); iter != _data.rend(); iter++) {
            if (!(*iter).isDeleted()) {
                _lastIndexPath = (*iter)._indexPath;
                break;
            }
        }
    } else {// _data.size() == 0
        // 첫번째 새로운 데이터 (최초추가)
        cursor = _firstCursor;
    }
    
    return cursor;
}


SMTableView::Cursor SMTableView::ColumnInfo::insertItem(const IndexPath& indexPath, const int type, const float estimateSize) {
    
    Cursor cursor = markInsertItem(indexPath);
    
    if (indexPath > _lastIndexPath) {
        _lastIndexPath = indexPath;
    }
    
    cursor.setIterator(_data.insert(cursor.getIterator(), Item(indexPath, type, 0)));
    
    Cursor* c[3] = { &_viewFirstCursor, &_viewLastCursor, &_lastCursor };
    for (int i = 0; i < 3; i++) {
        if (*c[i] >= cursor) {
            c[i]->offset(+1, +cursor.getItem()._size);
        }
    }
    
    if (cursor.isBegin()) {
        _firstCursor = cursor;
    }
    
    _numAliveItem++;
    
    insertCursorBuffer(cursor);
    
    _resizeReserveSize += estimateSize;
    
    return cursor;
}


SMTableView::ReuseScrapper::ReuseScrapper() : _numberOfTypes(0)
{
}

SMTableView::ReuseScrapper::~ReuseScrapper()
{
    for (int i = 0; i < _data.size(); i++) {
        _data[i].clear();
    }
    
    _data.clear();
}

int SMTableView::ReuseScrapper::getReuseType(const std::string& reuseIdentifire) {
    int reuseType;
    
    std::map<std::string, int>::iterator iter = _key.find(reuseIdentifire);
    if (iter == _key.end()) {
        // 키 존재하지 않음 => 추가
        reuseType = _numberOfTypes;
        _key.insert(std::pair<std::string, int>(reuseIdentifire, _numberOfTypes++));
        _data.push_back(cocos2d::Vector<cocos2d::Node*>());
    } else {
        reuseType = iter->second;
    }
    
    return reuseType;
}

void SMTableView::ReuseScrapper::scrap(const int reuseType, cocos2d::Node* const parent, cocos2d::Node* const child, const bool cleanup) {
    cocos2d::Vector<cocos2d::Node*>& queue = _data[reuseType];
    
    if (parent) {
        child->retain();
        parent->removeChild(child, cleanup);
    }
    
    if (!dynamic_cast<_DeletedNode*>(child)) {
        queue.pushBack(child);
    }
    
    if (parent) {
        child->release();
    }
}

cocos2d::Node* SMTableView::ReuseScrapper::back(const int reuseType) {
    if (_data[reuseType].size() > 0) {
        return _data[reuseType].back();
    }
    
    return nullptr;
}

void SMTableView::ReuseScrapper::popBack(const int reuseType) {
    _data[reuseType].popBack();
}

void SMTableView::ReuseScrapper::clear() {
    for (int i = 0; i < _data.size(); i++) {
        _data[i].clear();
    }
    //    _key.clear();
    //    _data.clear();
}

cocos2d::Node* SMTableView::dequeueReusableCellWithIdentifier(const std::string& identifier) {
    _reuseScrapper->_internalReuseType = _reuseScrapper->getReuseType(identifier);
    _reuseScrapper->_internalReuseNode = _reuseScrapper->back(_reuseScrapper->_internalReuseType);
    
    return _reuseScrapper->_internalReuseNode;
}


void SMTableView::removeChildAndReuseScrap(const ssize_t container, const int reuseType, cocos2d::Node* child, const bool cleanup) {
    if (reuseType >= 0) {
        _reuseScrapper->scrap(reuseType, _container[container], child, cleanup);
    } else {
        removeChild(container, child, cleanup);
    }
}

SMTableView::_DeletedNode* SMTableView::_DeletedNode::create() {
    _DeletedNode* deleted = new (std::nothrow)_DeletedNode();
    if (deleted != nullptr) {
        if (deleted->init()) {
            deleted->autorelease();
        } else {
            CC_SAFE_DELETE(deleted);
        }
    }
    return deleted;
}


cocos2d::Node* SMTableView::_BaseAction::getChild() {
    auto info = _parent->_column + _col;
    int offset = _cursor->getPosition() - info->getViewFirstCursor().getPosition();
    if (*_cursor < info->getViewFirstCursor() || *_cursor >= info->getViewLastCursor() || _parent->getChildrenCount(_col) <= offset)
        return nullptr;
    
    return _parent->_container[_col]->getChildren().at(offset);
}

void SMTableView::_BaseAction::startWithTarget(cocos2d::Node *target) {
    cocos2d::ActionInterval::startWithTarget(target);
    _startSize = _cursor->getItem()._size;
}

SMTableView::_DeleteAction* SMTableView::_DeleteAction::create(SMTableView* parent, int column, Cursor& cursor) {
    auto action = new (std::nothrow)_DeleteAction();
    
    if (action) {
        action->_parent = parent;
        action->_col = column;
        action->_cursor = parent->_column[column].obtainCursorBuffer(cursor);
        action->autorelease();
    }
    
    return action;
}

void SMTableView::_DeleteAction::update(float t) {
    Item& item = _cursor->getItem();
    item._newSize = _startSize * (1-t);
    cocos2d::Node* child = getChild();
    
    if (child) {
        if (_parent->onCellResizeCallback) {
            _parent->onCellResizeCallback(child, item._newSize);
        }
        
        if (t < 1) {
            if (_parent->onCellDeleteCallback) {
                _parent->onCellDeleteCallback(child, t);
            }
        }
    } else {
        child = _parent->findFromHolder(item._tag);
        if (child) {
            if (_parent->onCellResizeCallback) {
                _parent->onCellResizeCallback(child, item._newSize);
            }
            
            if (_parent->onCellDeleteCallback) {
                _parent->onCellDeleteCallback(child, t);
            }
        }
    }
    
    if (t < 1) {
        _parent->_column[_col].resizeCursor(*_cursor);
        _parent->scheduleScrollUpdate();
        _parent->_animationDirty = true;
    } else {
        complete();
    }
}

void SMTableView::_DeleteAction::complete() {
    
    Item& item = _cursor->getItem();
    
    cocos2d::Node* childInHolder = _parent->findFromHolder(item._tag);
    if (childInHolder) {
        _parent->_reuseScrapper->scrap(item._reuseType, nullptr, childInHolder);
        _parent->eraseFromHolder(item._tag);
    }
    
    _parent->_column[_col].resizeCursor(*_cursor);
    _parent->deleteCursor(_col, *_cursor);
    _parent->_column[_col].recycleCursorBuffer(_cursor);
    
    item._tag = 0;
    item._flags = 0;
    
    _parent->scheduleScrollUpdate();
    _parent->_animationDirty = true;
    
    if (_parent->onCellDeleteCompletionCallback) {
        _parent->onCellDeleteCompletionCallback();
    }
}


SMTableView::_ResizeAction* SMTableView::_ResizeAction::create(SMTableView* parent, int column, Cursor& cursor, float newSize) {
    _ResizeAction* action = new (std::nothrow)_ResizeAction();
    
    if (action) {
        action->_parent = parent;
        action->_col = column;
        action->_cursor = parent->_column[column].obtainCursorBuffer(cursor);
        action->_newSize = newSize;
        action->_insert = false;
        action->autorelease();
    }
    
    return action;
}

SMTableView::_InsertAction* SMTableView::_InsertAction::create(SMTableView* parent, int column, Cursor& cursor, float newSize) {
    _InsertAction* action = new (std::nothrow)_InsertAction();
    
    if (action) {
        action->_parent = parent;
        action->_col = column;
        action->_cursor = parent->_column[column].obtainCursorBuffer(cursor);
        action->_newSize = newSize;
        action->_insert = true;
        action->autorelease();
    }
    
    return action;
}

cocos2d::Node* SMTableView::_ResizeAction::updateResize(float t) {
    Item& item = _cursor->getItem();
    item._newSize = _startSize + (_newSize - _startSize) * t;
    _parent->_column[_col].resizeCursor(*_cursor);
    
    cocos2d::Node* child = getChild();
    
    if (child) {
        if (_parent->onCellResizeCallback) {
            _parent->onCellResizeCallback(child, item._newSize);
        }
        
        if (_insert && _parent->onCellInsertCallback) {
            _parent->onCellInsertCallback(child, t);
        }
    }
    
    _parent->scheduleScrollUpdate();
    _parent->_animationDirty = true;
    
    return child;
}

void SMTableView::_ResizeAction::update(float t) {
    if (t < 1) {
        updateResize(t);
    } else {
        complete();
    }
}

void SMTableView::_ResizeAction::complete() {
    Item& item = _cursor->getItem();
    
    cocos2d::Node* child = updateResize(1);
    
    if (!child) {
        // 완료시 화면에 child가 없으면 다음번 add될때 최종 resize한다.
        item._flags = ITEM_FLAG_RESIZE;
        if (_insert) {
            item._flags |= ITEM_FLAG_INSERT;
        }
    } else {
        item._flags = 0;
    }
    item._tag = 0;
    
    _parent->_column[_col].recycleCursorBuffer(_cursor);
    _parent->scheduleScrollUpdate();
    _parent->_animationDirty = true;
    
    if (_parent->onCellResizeCompletionCallback) {
        _parent->onCellResizeCompletionCallback(child);
    }
}

SMTableView::_DelaySequence* SMTableView::_DelaySequence::create(float delay, _BaseAction* action) {
    _DelaySequence* sequence = new (std::nothrow)_DelaySequence();
    
    if (sequence) {
        sequence->initWithTwoActions(cocos2d::DelayTime::create(std::max(.0f, delay)), action);
        sequence->_action = action;
        sequence->autorelease();
    }
    
    return sequence;
}

SMTableView::_BaseAction* SMTableView::_DelaySequence::getAction() {
    return _action;
}

void SMTableView::initFixedColumnInfo(int numPages, float pageSize, int initPage) {
    for (int i = 0; i < numPages; i++) {
        _column[0].advanceViewLast(IndexPath(0, 0, i), -1, pageSize);
    }
    if (initPage == 0) {
        _column[0].rewindViewLastCursor();
    } else {
        auto cursor = _column[0].getFirstCursor(initPage);
        _column[0].setViewCursor(cursor);
        _column[0].retreatViewLast();
    }
}


SMTableView::_PageJumpAction* SMTableView::_PageJumpAction::create(SMTableView* parent, Cursor& cursor, float pageSize, int fromPage, int toPage, int direction) {
    auto action = new (std::nothrow)_PageJumpAction();
    
    if (action) {
        action->_parent = parent;
        action->_cursor = parent->_column[0].obtainCursorBuffer(cursor);
        action->_pageSize = pageSize;
        action->_fromPage = fromPage;
        action->_toPage = toPage;
        action->_direction = direction;
        action->autorelease();
    }
    
    return action;
}

void SMTableView::_PageJumpAction::update(float t) {
    float position;
    
    if (_direction < 0) {
        position = -_pageSize + _pageSize * t;
        for (int i = 0; i < 2; i++) {
            if (i < _parent->_container[0]->getChildrenCount()) {
                _parent->getChildAt(0, i)->setPositionX(position);
            }
            position += _pageSize;
        }
    } else {
        position = -_pageSize * t;
        for (int i = 0; i < 2; i++) {
            if (i < _parent->_container[0]->getChildrenCount()) {
                _parent->getChildAt(0, i)->setPositionX(position);
            }
            position += _pageSize;
        }
    }
    
    float p1 = _fromPage*_pageSize;
    float p2 = _toPage*_pageSize;
    float scrollPosition = p1 + t * (p2 - p1);
    _parent->onScrollChanged(scrollPosition, 0);
    
    if (t >= 1.0) complete();
}

void SMTableView::_PageJumpAction::complete() {
    
    cocos2d::Node* child = nullptr;
    int index = _direction>0?0:1;
    if (index < _parent->getChildrenCount(0)) {
        child = _parent->getChildAt(0, index);
    }
    
    if (child != nullptr) {
        Item& item = _cursor->getItem();
        _parent->removeChildAndReuseScrap(0, item._reuseType, child, true);
    }
    
    _parent->_column[0].setViewCursor(*_cursor);
    _parent->_column[0].recycleCursorBuffer(_cursor);
    
    _parent->_forceJumpPage = false;
    _parent->_currentPage = _toPage;
    _parent->_scroller->setScrollPosition(_pageSize*_toPage);
    _parent->scheduleScrollUpdate();
}


bool SMTableView::jumpPage(const int pageNo, const float pageSize) {
    if (_forceJumpPage)
        return false;
    
    int currentPage = (int)(_scroller->getNewScrollPosition() / pageSize);
    ColumnInfo* info = _column;
    
    if (pageNo == currentPage) {
        return false;
    } else if (pageNo == currentPage + 1) {
        // 다음 페이지
        _scroller->onTouchFling(-10000, currentPage);
        scheduleScrollUpdate();
    } else if (pageNo == currentPage - 1) {
        // 이전 페이지
        _scroller->onTouchFling(+10000, currentPage);
        scheduleScrollUpdate();
    } else {
        ssize_t numChild = getChildrenCount(0);
        // 현재 페이지와 다른 페이지 삭제
        Cursor cursor = info->getViewLastCursor();
        for (ssize_t i = numChild-1; i >= 0; i--) {
            --cursor;
            if (cursor.getIndexPath().getIndex() != currentPage) {
                cocos2d::Node* child = getChildAt(0, i);
                Item& item = cursor.getItem();
                
                removeChildAndReuseScrap(0, item._reuseType, child, true);
            }
        }
        
        // 적절한 위치에 추가.
        float position;
        int direction;
        
        if (pageNo > currentPage) {
            // 뒤쪽 페이지
            position = pageSize;
            direction = +1;
        } else {
            position = -pageSize;
            direction = -1;
        }
        
        cursor = info->getFirstCursor();
        for (int i = 0; i < pageNo; i++) {
            cursor++; // target cursor
        }
        
        _reuseScrapper->_internalReuseType = -1;
        _reuseScrapper->_internalReuseNode = nullptr;
        
        cocos2d::Node* child = cellForRowAtIndexPath(cursor.getIndexPath());
        
        if (!child) {
            CC_ASSERT(0);
        }
        if (child->getParent()) {
            _scroller->onTouchUp();
            _forceJumpPage = false;
            return false; // TODO : 땜빵
        }
        
        // order
        child->setLocalZOrder(cursor.getPosition());
        
        Item& item = cursor.getItem();
        if (_reuseScrapper->_internalReuseType >= 0) {
            item._reuseType = _reuseScrapper->_internalReuseType;
        }
        
        child->setPositionX(position);
        addChild(0, child);
        sortAllChildren(0);
        
        if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == child) {
            _reuseScrapper->popBack(_reuseScrapper->_internalReuseType);
            _reuseScrapper->_internalReuseType = -1;
            _reuseScrapper->_internalReuseNode = nullptr;
        }
        
        auto remainAction = getActionByTag(SMViewConstValue::Tag::ACTION_LIST_JUMP);
        if (remainAction) {
            ((_PageJumpAction*)remainAction)->complete();
            stopAction(remainAction);
        }
        
        _forceJumpPage = true;
        
        auto action = _PageJumpAction::create(this, cursor, pageSize, currentPage, pageNo, direction);
        action->setTag(SMViewConstValue::Tag::ACTION_LIST_JUMP);
        action->setDuration(.25f);
        runAction(action);
    }
    
    return true;
}


SMTableView::SMTableView() :
_orient(Orientation::VERTICAL),
_lastScrollPosition(0),
_lastItemCount(0),
_hintFixedChildSize(0),
_hintIsFixedSize(false),
_justAtLast(false),
_canExactScrollSize(false),
_column(nullptr),
_firstMargin(0),
_lastMargin(0),
_preloadPadding(0),
_internalActionTag(SMViewConstValue::Tag::ACTION_LIST_ITEM_DEFAULT),
_reuseScrapper(nullptr),
_refreshView(nullptr),
_refreshState(RefreshState::NONE),
_lastRefreshState(RefreshState::NONE),
_refreshSize(0),
_lastRefreshSize(0),
cellForRowAtIndexPath(nullptr),
numberOfRowsInSection(nullptr),
onCellResizeCallback(nullptr),
onCellResizeCompletionCallback(nullptr),
onCellDeleteCompletionCallback(nullptr),
onCellInsertCallback(nullptr),
onScrollCallback(nullptr),
onRefreshDataCallback(nullptr),
canRefreshData(nullptr),
onLoadDataCallback(nullptr),
onInitFillWithCells(nullptr),
_animationDirty(false),
_forceJumpPage(false),
_currentPage(0),

_lastMotionX(0),
_lastMotionY(0),
_skipUpdateOnVisit(false),
onLoadDataCallbackTemp(nullptr),
_touchFocused(false),
_fillWithCellsFirstTime(false),

_accelScrollEnable(false),
_accelCount(0),
_lastVelocityX(0),
_lastVelocityY(0),
_lastFlingTime(0),

_initRefreshEnable(false),

_maxVelocicy(SMViewConstValue::Config::MAX_VELOCITY),
_reloadExceptHeader(false)
{
}

SMTableView::~SMTableView()
{
    CC_SAFE_RELEASE(_refreshView);
    
    CC_SAFE_DELETE_ARRAY(_column);
    CC_SAFE_DELETE(_reuseScrapper);
    
    unscheduleScrollUpdate();
}

SMTableView* SMTableView::create(SMTableView::Orientation orient) {
    return SMTableView::createMultiColumn(orient, 1);
}

SMTableView* SMTableView::create(Orientation orient, float x, float y, float width, float height, float anchorX, float anchorY) {
    return SMTableView::createMultiColumn(orient, 1, x, y, width, height, anchorX, anchorY);
}

SMTableView* SMTableView::createMultiColumn(SMTableView::Orientation orient, int numColumns) {
    SMTableView* view = new(std::nothrow) SMTableView();
    
    if (view && view->initWithOrientAndColumns(orient, numColumns)) {
        view->autorelease();
        return view;
    } else {
        delete view;
        view = nullptr;
        return nullptr;
    }
}

SMTableView* SMTableView::createMultiColumn(Orientation orient, int numColumns, float x, float y, float width, float height, float anchorX, float anchorY) {
    SMTableView* view = createMultiColumn(orient, numColumns);
    if (view != nullptr) {
        view->setContentSize(cocos2d::Size(width, height));
        view->setPosition(x, y);
        view->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
    }
    return view;
}

bool SMTableView::initWithOrientAndColumns(Orientation orient, int numColumns) {
    if (initWithContainer(numColumns)) {
        _orient = orient;
        
        _column = new ColumnInfo[numColumns];
        for (ssize_t col = 0; col < numColumns; col++) {
            _column[col].init(this, col);
        }
        
        _reuseScrapper = new ReuseScrapper();
        
        initScroller();
        _lastScrollPosition = _scroller->getScrollPosition();
        
        _velocityTracker = new VelocityTracker();
        _velocityTracker->clear();
        
        scheduleScrollUpdate();
        
        return true;
    }
    
    return false;
}

SMScroller* SMTableView::initScroller() {
    _scroller = new FlexibleScroller();
    return _scroller;
}

void SMTableView::setContentSize(const cocos2d::Size& size) {
    SMView::setContentSize(size);
    
    if (isVertical()) {
        _scroller->setWindowSize(size.height);
        for (ssize_t col = 0; col < _numContainer; col++) {
            _container[col]->setContentSize(cocos2d::Size(size.width/_numContainer, size.height));
            _container[col]->setPositionX(col*size.width/_numContainer);
        }
    } else {
        _scroller->setWindowSize(size.width);
        for (ssize_t col = 0; col < _numContainer; col++) {
            _container[col]->setContentSize(cocos2d::Size(size.width, size.height/_numContainer));
            _container[col]->setPositionY(size.height - (col+1)*size.height/_numContainer);
        }
    }
    
    scheduleScrollUpdate();
}

void SMTableView::hintFixedCellSize(const float cellSize) {
    _hintIsFixedSize = true;
    _hintFixedChildSize = cellSize;
}

/**
 * View가 미리 생성되는 경계선 바깥쪽 padding
 *
 * @param paddingPixels Scroll padding pixels
 **/
void SMTableView::setPreloadPaddingSize(const float paddingSize) {
    if (paddingSize >= 0) {
        _preloadPadding = paddingSize;
    }
    
    scheduleScrollUpdate();
}

void SMTableView::setScrollMarginSize(const float firstMargin, const float lastMargin) {
    _firstMargin = firstMargin;
    _lastMargin = lastMargin;
    
    scheduleScrollUpdate();
}


/**
 * 스크롤 위치에 따라 child 좌표 세팅
 *
 **/
void SMTableView::positionChildren(const float scrollPosition, const float containerSize, const float headerSize, const float footerSize) {
    
    float startLocation = headerSize + _firstMargin + _innerScrollMargin - scrollPosition;
    float lastLocation = 0;
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        ssize_t numChild = getChildrenCount(col);
        
        if (numChild > 0) {
            ColumnInfo* info = _column + col;
            
            Cursor cursor = info->getViewFirstCursor();
            
            for (int i = 0; i < numChild; i++, cursor++) {
                cocos2d::Node* child = getChildAt(col, i);
                
                Item& item = cursor.getItem();
                
                if (item._reload) {
                    item._reload = false;
                    // cell 리로드
                    removeChild(col, child);
                    
                    _reuseScrapper->_internalReuseType = -1;
                    _reuseScrapper->_internalReuseNode = nullptr;
                    child = cellForRowAtIndexPath(item._indexPath);
                    if (!child) {
                        CC_ASSERT(0);
                    }
                    
                    addChild(col, child);
                    child->setLocalZOrder(cursor.getPosition());
                    
                    if (_reuseScrapper->_internalReuseType >= 0) {
                        item._reuseType = _reuseScrapper->_internalReuseType;
                    }
                    
                    if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == child) {
                        _reuseScrapper->popBack(_reuseScrapper->_internalReuseType);
                        _reuseScrapper->_internalReuseType = -1;
                        _reuseScrapper->_internalReuseNode = nullptr;
                    }
                    
                    if (_hintIsFixedSize) {
                        item._newSize = _hintFixedChildSize;
                    } else {
                        if (isVertical()) {
                            item._newSize = child->getContentSize().height;
                        } else {
                            item._newSize = child->getContentSize().width;
                        }
                    }
                    
                    info->resizeCursor(cursor);
                    sortAllChildren(col);
                }
                
                // Resize 처리
                if (item._size != item._newSize) {
                    info->resizeCursor(cursor);
                    
                    if (onCellResizeCallback) {
                        onCellResizeCallback(child, item._newSize);
                    }
                }
                
                float location = startLocation + cursor.getLocation();
                
                if (isVertical()) {
                    float childSize;
                    if (_hintIsFixedSize) {
                        childSize = _hintFixedChildSize;
                    } else {
                        childSize = child->getContentSize().height;
                    }
                    onPositionCell(child, containerSize - (location + childSize), false);
                } else {
                    onPositionCell(child, location, false);
                }
                
                child->setLocalZOrder(cursor.getPosition());
            }
            
            lastLocation = std::max(lastLocation, info->getLastCursor().getLocation());
        }
    }
    
    lastLocation += startLocation;
    
    if (_headerView && _isHeaderInList) {
        if (isVertical()) {
            onPositionHeader(_headerView, containerSize - startLocation, false);
        } else {
            onPositionHeader(_headerView, startLocation - headerSize, false);
        }
    }
    
    if (_footerView && _isFooterInList) {
        if (isVertical()) {
            onPositionFooter(_footerView, containerSize - (lastLocation + footerSize), false);
        } else {
            onPositionFooter(_footerView, lastLocation, false);
        }
    }
    
    if (_refreshView && _refreshState != RefreshState::NONE) {
        if (isVertical()) {
            _refreshView->setPositionY(containerSize - startLocation);
        } else {
            _refreshView->setPositionX(startLocation - _refreshView->getContentSize().width);
        }
    }
}

void SMTableView::onPositionCell(cocos2d::Node* cell, const float position, const bool isAdded) {
    if (isVertical()) {
        cell->setPositionY(position);
    } else {
        cell->setPositionX(position);
    }
}

void SMTableView::onPositionHeader(cocos2d::Node* headerView, const float position, const bool isAdded) {
    if (isVertical()) {
        headerView->setPositionY(position);
    } else {
        headerView->setPositionX(position);
    }
}

void SMTableView::onPositionFooter(cocos2d::Node* footerView, const float position, const bool isAdded) {
    if (isVertical()) {
        footerView->setPositionY(position);
    } else {
        footerView->setPositionX(position);
    }
}


/**
 * 스크롤 위치에 따라 화면에 보이지 않는 child 제거
 *
 **/
void SMTableView::clippingChildren(const float scrollPosition, const float containerSize, const float headerSize, const float footerSize) {
    
    float startLocation = headerSize + _firstMargin + _innerScrollMargin - scrollPosition;
    float lastLocation = 0;
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        ssize_t numChild = getChildrenCount(col);
        ColumnInfo* info = _column + col;
        
        // 상단 제거
        if (numChild > 0) {
            
            for (ssize_t i = 0; i < numChild; i++) {
                Cursor cursor = info->getViewFirstCursor();
                
                cocos2d::Node* child = getChildAt(col, 0);
                
                if (child != nullptr && startLocation + cursor.getLastLocation() <= -_preloadPadding) {
                    Item& item = cursor.getItem();
                    
                    if (item.isDeleted()) {
                        //Delete중인 상단 child는 hold (Animation이 진행되어야 하기 때문에)
                        removeChildAndHold(col, item._tag, child, false);
                    } else {
                        removeChildAndReuseScrap(col, item._reuseType, child, CLEANUP_FALG);
                    }
                    
                    info->retreatViewFirst();
                } else {
                    break;
                }
            }
        }
        // 하단 제거
        numChild = getChildrenCount(col);
        if (numChild > 0) {
            
            for (ssize_t i = numChild-1; i >= 0; i--) {
                Cursor cursor = info->getViewLastCursor(-1);
                
                cocos2d::Node* child = getChildAt(col, i);
                
                if (child != nullptr && startLocation + cursor.getLocation() >= containerSize + _preloadPadding) {
                    Item& item = cursor.getItem();
                    
                    if (item.isDeleted()) {
                        // 삭제중인 하단 child는 즉시 삭제
                        stopAndCompleteChildAction(item._tag);
                    } else {
                        if (item._tag) {
                            // Animation중인 하단 child는 즉시 적용
                            stopAndCompleteChildAction(item._tag);
                            item._tag = 0;
                        }
                        removeChildAndReuseScrap(col, item._reuseType, child, CLEANUP_FALG);
                        
                        info->retreatViewLast();
                    }
                    
                } else {
                    break;
                }
            }
        }
        
        lastLocation = std::max(lastLocation, info->getLastCursor().getLocation());
    }
    
    lastLocation += startLocation;
    
    // 헤더 제거
    if (_headerView && _isHeaderInList) {
        if (startLocation < -_preloadPadding) {
            cocos2d::Node::removeChild(_headerView, CLEANUP_FALG);
            _isHeaderInList = false;
        }
    }
    
    // 푸터 제거
    if (_footerView && _isFooterInList) {
        if (lastLocation > containerSize + _preloadPadding) {
            cocos2d::Node::removeChild(_footerView, CLEANUP_FALG);
            _isFooterInList = false;
        }
    }
}

bool SMTableView::fillListBack(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, const float footerSize) {
    
    float scrollLocation = _firstMargin + _innerScrollMargin - scrollPosition;
    float limitLocation = containerSize + _preloadPadding - scrollLocation;
    
    if (_headerView != nullptr) {
        scrollLocation += headerSize;
        limitLocation -= headerSize;
    }
    
    cocos2d::Node* child;
    ColumnInfo* info;
    bool added = false;
    
    int lastIndex = 0;
    for (ssize_t col = 0; col < _numContainer; col++) {
        lastIndex += _column[col].getAliveItemCount();
    }
    
    while (adapterItemCount > 0) {
        float lastLocation = FLT_MAX;
        bool isAtLast = (lastIndex == adapterItemCount);
        
        // 다음 child 추가할 컬럼 / 인덱스 찾기
        ssize_t column = -1;
        for (ssize_t col = 0; col < _numContainer; col++) {
            info = _column + col;
            
            if (isAtLast) {
                if (info->getViewLastCursor().getLocation() < lastLocation && !info->isAtLast()) {
                    column = col;
                    lastLocation = info->getViewLastCursor().getLocation();
                }
            } else {
                if (info->getViewLastCursor().getLocation() < lastLocation) {
                    column = col;
                    lastLocation = info->getViewLastCursor().getLocation();
                }
            }
        }
        if (lastLocation >= limitLocation || column < 0) {
            break;
        }
        
        // 다음 추가할 아이템 찾기
        info = _column + column;
        IndexPath indexPath;
        
        if (info->isAtLast()) {
            // 이전에 생성된 아이템 없음 => 추가
            indexPath = IndexPath(0, (int)column, lastIndex);
            lastIndex++;
        } else {
            // 이전에 생성된 아이템 있음.
            indexPath = info->getViewLastCursor().getIndexPath();
        }
        
        _reuseScrapper->_internalReuseType = -1;
        _reuseScrapper->_internalReuseNode = nullptr;
        
        child = cellForRowAtIndexPath(indexPath);
        
        if (!child) {
            CC_ASSERT(0);
        }
        if (child->getParent()) {
            break; // TODO : 땜빵. 죽지만 않도록..
        }
        
        float childSize;
        bool reload = false;
        if (!info->isAtLast()) {
            Item& item = info->getViewLastCursor().getItem();
            
            if (item._reload) {
                item._reload = false;
                if (_hintIsFixedSize) {
                    childSize = _hintFixedChildSize;
                } else {
                    childSize = isVertical()?child->getContentSize().height:child->getContentSize().width;
                }
                item._newSize = childSize;
                reload = true;
            } else {
                childSize = item._size;
            }
            
        } else {
            if (_hintIsFixedSize) {
                childSize = _hintFixedChildSize;
            } else {
                childSize = isVertical()?child->getContentSize().height:child->getContentSize().width;
            }
        }
        
        // cursor 진행
        Cursor cursor = info->advanceViewLast(IndexPath(0, (int)column, indexPath.getIndex()), _reuseScrapper->_internalReuseType, childSize);
        
        if (reload) {
            info->resizeCursor(cursor);
        }
        
        // order
        child->setLocalZOrder(cursor.getPosition());
        
        Item& item = cursor.getItem();
        if (_reuseScrapper->_internalReuseType >= 0) {
            item._reuseType = _reuseScrapper->_internalReuseType;
        }
        
        if (item._flags & ITEM_FLAG_RESIZE) {
            if (onCellResizeCallback) {
                onCellResizeCallback(child, item._size);
            }
        }
        if (item._tag == 0) {
            if (item._flags & ITEM_FLAG_INSERT) {
                if (onCellInsertCallback) {
                    onCellInsertCallback(child, 1);
                }
            }
            item._flags = 0;
        }
        
        childSize = item._size;
        
        // View 내 위치
        float locationInView = headerSize + _firstMargin + _innerScrollMargin - scrollPosition;
        if (isVertical()) {
            onPositionCell(child, _contentSize.height - locationInView - cursor.getLastLocation(), true);
        } else {
            onPositionCell(child, locationInView + cursor.getLocation(), true);
        }
        
        addChild(column, child);
        
        if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == child) {
            _reuseScrapper->popBack(_reuseScrapper->_internalReuseType);
            _reuseScrapper->_internalReuseType = -1;
            _reuseScrapper->_internalReuseNode = nullptr;
        }
        
        added = true;
    }
    
    return added;
}

bool SMTableView::fillListFront(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, const float footerSize) {
    
    float limitLocation = -_preloadPadding + scrollPosition - (headerSize + _firstMargin + _innerScrollMargin);
    
    cocos2d::Node* child;
    ColumnInfo* info;
    bool added = false;
    
    while (adapterItemCount > 0) {
        float firstLocation = FLT_MIN;
        
        // 다음 child 추가할 컬럼 / 인덱스 찾기
        ssize_t column = -1;
        for (ssize_t col = 0; col < _numContainer; col++) {
            info = _column + col;
            
            if (info->getViewFirstCursor().getLocation() > firstLocation && !info->isAtFirst()) {
                column = col;
                firstLocation = info->getViewFirstCursor().getLocation();
            }
        }
        if (firstLocation <= limitLocation || column < 0 ) {
            break;
        }
        
        info = _column + column;
        
        Cursor cursor = info->advanceViewFirst();
        Item& item = cursor.getItem();
        
        if (item.isDeleted()) {
            // 삭재중 아이템
            child = findFromHolder(item._tag);
            if (child == nullptr) {
                child = _DeletedNode::create();
                
                if (isVertical())
                    child->setContentSize(cocos2d::Size(child->getContentSize().width, item._size));
                else
                    child->setContentSize(cocos2d::Size(item._size, child->getContentSize().width));
            }
        } else {
            _reuseScrapper->_internalReuseType = -1;
            _reuseScrapper->_internalReuseNode = nullptr;
            child = cellForRowAtIndexPath(item._indexPath);
            if (_reuseScrapper->_internalReuseType >= 0) {
                item._reuseType = _reuseScrapper->_internalReuseType;
            }
            
            if (item._reload) {
                item._reload = false;
                if (_hintIsFixedSize) {
                    item._newSize = _hintFixedChildSize;
                } else {
                    item._newSize  = isVertical()?child->getContentSize().height:child->getContentSize().width;
                }
                info->resizeCursor(cursor);
            }
            
        }
        if (!child) {
            CC_ASSERT(0);
        }
        if (child->getParent()) {
            break; // TODO : 땜빵
        }
        
        // order
        child->setLocalZOrder(cursor.getPosition());
        
        if (item._flags & ITEM_FLAG_RESIZE) {
            if (onCellResizeCallback && !dynamic_cast<_DeletedNode*>(child)) {
                onCellResizeCallback(child, item._size);
            }
        }
        if (item._tag == 0) {
            if (item._flags & ITEM_FLAG_INSERT) {
                if (onCellInsertCallback) {
                    onCellInsertCallback(child, 1);
                }
            }
            item._flags = 0;
        }
        
        // View 내 위치
        float locationInView = headerSize + _firstMargin + _innerScrollMargin - scrollPosition;
        if (isVertical()) {
            onPositionCell(child, _contentSize.height - locationInView - cursor.getLastLocation(), true);
        } else {
            onPositionCell(child, locationInView + cursor.getLocation(), true);
        }
        
        addChild(column, child);
        if (item._flags & ITEM_FLAG_DELETE) {
            eraseFromHolder(item._tag);
        } else if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == child) {
            _reuseScrapper->popBack(_reuseScrapper->_internalReuseType);
            _reuseScrapper->_internalReuseType = -1;
            _reuseScrapper->_internalReuseNode = nullptr;
        }
        
        added = true;
    }
    
    if (added) {
        for (ssize_t col = 0; col < _numContainer; col++) {
            sortAllChildren(col);
        }
    }
    
    return added;
}

/**
 * 리스트 채움
 *
 */
bool SMTableView::fillList(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, float footerSize) {
    
    bool backAdded = fillListBack(adapterItemCount, scrollPosition, containerSize, headerSize, footerSize);
    bool frontAdded = fillListFront(adapterItemCount, scrollPosition, containerSize, headerSize, footerSize);
    
    if (_headerView) {
        if (!_isHeaderInList) {
            if (scrollPosition < headerSize + _firstMargin + _innerScrollMargin) {
                cocos2d::Node::addChild(_headerView);
                if (isVertical()) {
                    float position = scrollPosition + containerSize - (headerSize + _firstMargin + _innerScrollMargin);
                    onPositionHeader(_headerView, position, true);
                } else {
                    float position = -scrollPosition + _firstMargin + _innerScrollMargin;
                    onPositionHeader(_headerView, position, true);
                }
                _isHeaderInList = true;
                frontAdded |= true;
            }
        }
    }
    
    if (_footerView) {
        while (!_isFooterInList) {
            float lastLocation = 0;
            int aliveItemCount = 0;
            
            for (ssize_t col = 0; col < _numContainer; col++) {
                if (!_column[col].isAtLast()) {
                    aliveItemCount = -1;
                    break;
                }
                aliveItemCount += _column[col].getAliveItemCount();
                lastLocation = std::max(_column[col].getLastCursor().getLocation(), lastLocation);
            }
            
            if (aliveItemCount >= adapterItemCount > 0 && scrollPosition + containerSize  > headerSize + _firstMargin + _innerScrollMargin + lastLocation) {
                
                if (onLoadDataCallback) {
                    if (!_progressLoading) {
                        _progressLoading = true;
                        
                        onLoadDataCallback(_footerView);
                    }
                } else if (onLoadDataCallbackTemp) {
                    break;
                }
                
                cocos2d::Node::addChild(_footerView);
                
                if (isVertical()) {
                    float position = scrollPosition + containerSize - (headerSize + _firstMargin + _innerScrollMargin + lastLocation) - _footerView->getContentSize().height;
                    onPositionFooter(_footerView, position, true);
                } else {
                    float position = (headerSize + _firstMargin + _innerScrollMargin + lastLocation) - scrollPosition;
                    onPositionFooter(_footerView, position, true);
                }
                
                _isFooterInList = true;
                backAdded |= true;
                
            }
            break;
        }
    }
    
    return frontAdded | backAdded;
}


/**
 * 스크롤 사이즈 조정
 *
 */
float SMTableView::measureScrollSize() {
    
    float headerSize = 0;
    if (_headerView && _headerView->isVisible()) {
        headerSize = isVertical()?_headerView->getContentSize().height:_headerView->getContentSize().width;
    }
    
    float footerSize = 0;
    if (_footerView && _footerView->isVisible()) {
        footerSize = isVertical()?_footerView->getContentSize().height:_footerView->getContentSize().width;
    }
    
    // 스크롤 사이즈 최종 계산
    float scrollSize = 0;
    _canExactScrollSize = false;
    if (_hintIsFixedSize) {
        scrollSize = std::ceil((float)_lastItemCount / _numContainer) * _hintFixedChildSize;
        _canExactScrollSize = true;
    } else {
        int aliveItemCount = 0;
        for (ssize_t col = 0; col < _numContainer; col++) {
            aliveItemCount += _column[col].getAliveItemCount();
        }
        
        if (aliveItemCount >= _lastItemCount) {
            // 마지막일때 정확한 계산
            for (ssize_t col = 0; col < _numContainer; col++) {
                scrollSize = std::max(scrollSize, _column[col].getLastCursor().getLocation());
            }
            _canExactScrollSize = true;
        } else {
            // 마지막 아닐때 평균으로 계산(정확한 사이즈를 알수 없으므로...)
            if (aliveItemCount > 0) {
                float containerSize = isVertical()?_contentSize.height:_contentSize.width;
                float columnSizeTotal = 0;
                
                for (ssize_t col = 0; col < _numContainer; col++) {
                    columnSizeTotal += _column[col].getLastCursor().getLocation();
                }
                
                scrollSize = (_lastItemCount * columnSizeTotal / aliveItemCount) / _numContainer;
                scrollSize += containerSize * 0.3f; // 30%여분 추가
            }
            _justAtLast = false;
        }
    }
    scrollSize += headerSize + footerSize + _firstMargin + _innerScrollMargin + _lastMargin;
    
    return scrollSize;
}

void SMTableView::scheduleScrollUpdate() {
    registerUpdate(FLAG_SCROLL_UPDATE);
    if (_scrollParent) {
        _scrollParent->notifyScrollUpdate();
    }
}

void SMTableView::unscheduleScrollUpdate() {
    unregisterUpdate(FLAG_SCROLL_UPDATE);
}

bool SMTableView::updateScrollInParentVisit(float& deltaScroll) {
    
    _needUpdate = false;
    _deltaScroll = 0;
    
    if (isUpdate(FLAG_SCROLL_UPDATE)) {
        _skipUpdateOnVisit = false;
        onUpdateOnVisit();
        _skipUpdateOnVisit = true;
    }
    
    deltaScroll = _deltaScroll;
    
    return _needUpdate;
}

void SMTableView::onUpdateOnVisit() {
    if (_skipUpdateOnVisit) {
        _skipUpdateOnVisit = false;
        return;
    }
    
    if (_contentSize.width <= 0 || _contentSize.height <= 0)
        return;
    
    CCASSERT(cellForRowAtIndexPath != nullptr, "must register cellForRowAtIndexPath() callback");
    CCASSERT(numberOfRowsInSection != nullptr, "must register numberOfRowsInSection() callback");
    
    if (_reloadFlag) {
        _reloadFlag = false;
        
        _velocityTracker->clear();
        
        _scroller->reset();
        _scroller->setScrollPosition(getBaseScrollPosition());
        _lastScrollPosition = _scroller->getScrollPosition();
        
        _lastItemCount = 0;
        _inScrollEvent = false;
        _touchFocused = false;
        _justAtLast = false;
        _forceJumpPage = false;
        _fillWithCellsFirstTime = false;
        
        for (ssize_t col = 0; col < _numContainer; col++) {
            ColumnInfo* info = _column + col;
            
            // 수행중인 Animation 종료
            for (Cursor cursor = info->getFirstCursor(); cursor < info->getViewLastCursor(); ++cursor) {
                if (cursor.getItem()._tag > 0) {
                    stopActionByTag(cursor.getItem()._tag);
                }
            }
            
            // child 제거
            ssize_t numChild = getChildrenCount(col);
            if (numChild) {
                Cursor cursor = info->getViewFirstCursor();
                for (ssize_t i = 0; i < numChild; i++, ++cursor) {
                    cocos2d::Node* child = getChildAt(col, 0);
                    // reload면 reuse하지 않는다.
                    removeChild(col, child, true);
                    /*
                     if (dynamic_cast<_DeletedNode*>(child) || cursor.getItem()._dontReuse) {
                     removeChild(col, child);
                     } else {
                     removeChildAndReuseScrap(col, cursor.getItem()._reuseType, child, CLEANUP_FALG);
                     }
                     */
                }
                // scrapper clear
                _reuseScrapper->clear();
                
                // holder clear
                clearInstantHolder();
            }
            
            info->init(this, col);
            
            if (onLoadDataCallbackTemp) {
                onLoadDataCallback = onLoadDataCallbackTemp;
                onLoadDataCallbackTemp = nullptr;
            }
        }
        
        if (!_reloadExceptHeader) {
            if (_headerView && _isHeaderInList) {
                cocos2d::Node::removeChild(_headerView, CLEANUP_FALG);
                _isHeaderInList = false;
            }
        }
        _reloadExceptHeader = false;
        
        if (_footerView && _isFooterInList) {
            cocos2d::Node::removeChild(_footerView, CLEANUP_FALG);
            _isFooterInList = false;
        }
    }
    
    if (_forceJumpPage) {
        return;
    }
    
    bool updated = false;
    
    const ssize_t adapterItemCount = numberOfRowsInSection(0);
    if (_lastItemCount != adapterItemCount) {
        _lastItemCount = adapterItemCount;
        updated |= true;
    }
    
    updated |= _scroller->update();
    float scrollPosition = _scroller->getScrollPosition();
    float containerSize = isVertical()?_contentSize.height:_contentSize.width;
    float headerSize = 0;
    float footerSize = 0;
    
    if (_refreshView) {
        bool updateRefresh = false;
        
        if (_refreshState != _lastRefreshState) {
            if (_lastRefreshState == RefreshState::NONE) {
                cocos2d::Node::addChild(_refreshView, 1);
            }
            
            if (_refreshState == RefreshState::NONE) {
                cocos2d::Node::removeChild(_refreshView);
            } else if (_refreshState == RefreshState::REFRESHING) {
                // 터치 release됨, Refresh 시작
                _scroller->setHangSize(_refreshTriggerSize);
                updated |= _scroller->update();
                scrollPosition = _scroller->getScrollPosition();
            } else if (_lastRefreshState == RefreshState::REFRESHING && _refreshState == RefreshState::NONE) {
                //                if (_refreshSize <= _refreshMinSize) {
                //                    auto action = _RefreshConsumeAction::create(this, _refreshMinSize);
                //                    action->setDuration(SMViewConstValue::Config::LIST_HIDE_REFRESH_TIME);
                //                    action->setTag(SMViewConstValue::Tag::ACTION_LIST_HIDE_REFRESH);
                //                    runAction(action);
                //                }
                _scroller->setHangSize(0);
                
                
            }
            updateRefresh = true;
        }
        
        if (_refreshState != RefreshState::NONE) {
            
            _refreshSize = -_scroller->getScrollPosition();
            
            if (_refreshState == RefreshState::REFRESHING) {
                if (_refreshSize < _refreshMinSize) {
                    _refreshSize = std::max(-_scroller->getScrollPosition(), _refreshMinSize);
                }
            } else {
                _refreshSize = std::max(.0f, -_scroller->getScrollPosition());
            }
            if (_refreshSize != _lastRefreshSize) {
                updateRefresh = true;
            }
        } else {
            _refreshSize = 0;
        }
        
        if (updateRefresh) {
            _lastRefreshState = _refreshState;
            _lastRefreshSize = _refreshSize;
            
            if (!getActionByTag(SMViewConstValue::Tag::ACTION_LIST_HIDE_REFRESH)) {
                if (onRefreshDataCallback) {
                    onRefreshDataCallback(_refreshView, _refreshState, _refreshSize);
                }
                
                // TODO : 수정해야함..
                if (_refreshSize <= 10 && _refreshState == RefreshState::EXIT) {
                    _refreshState = RefreshState::NONE;
                }
            }
            updated = true;
        }
    }
    
    if (_headerView != nullptr) {
        headerSize = isVertical()?_headerView->getContentSize().height:_headerView->getContentSize().width;
    }
    if (_footerView != nullptr) {
        footerSize = isVertical()?_footerView->getContentSize().height:_footerView->getContentSize().width;
    }
    
    if (_animationDirty) {// && _scroller->getState() == SMScroller::State::STOP) {
        _animationDirty = false;
        
        positionChildren(scrollPosition, containerSize, headerSize, footerSize);
        float newScrollSize = measureScrollSize();
        _scroller->setScrollSize(std::max(_minScrollSize, newScrollSize));
        
        // TODO :  검증
        //        if (scrollPosition + _scroller->getWindowSize() > newScrollSize) {
        //            float delta = scrollPosition + _scroller->getWindowSize() - newScrollSize;
        //            _scroller->setScrollPosition(scrollPosition-delta);
        //        }
        
        if (!_justAtLast && _canExactScrollSize) {
            // 최초로 마지막에 도달했을 때 잘못된 오버스크롤 방지
            _justAtLast = true;
            _scroller->justAtLast();
        }
        
        scrollPosition = _scroller->getScrollPosition();
    }
    
    positionChildren(scrollPosition, containerSize, headerSize, footerSize);
    
    clippingChildren(scrollPosition, containerSize, headerSize, footerSize);
    
    if (fillList(adapterItemCount, scrollPosition, containerSize, headerSize, footerSize)) {
        _scroller->setScrollSize(std::max(_minScrollSize, measureScrollSize()));
        
        if (!_justAtLast && _canExactScrollSize) {
            // 최초로 마지막에 도달했을 때 잘못된 오버스크롤 방지
            _justAtLast = true;
            _scroller->justAtLast();
        }
        
        if (adapterItemCount > 0) {
            if (!_fillWithCellsFirstTime) {
                _fillWithCellsFirstTime = true;
                if (onInitFillWithCells) {
                    onInitFillWithCells(this);
                }
                if (_initRefreshEnable && _refreshView != nullptr) {
                    _initRefreshEnable = false;
                    if (_scroller->getNewScrollPosition() <= 0) {
                        _scroller->setScrollPosition(-_refreshTriggerSize);
                    }
                    _scroller->setHangSize(_refreshTriggerSize);
                    _refreshState = RefreshState::REFRESHING;
                    positionChildren(-_refreshTriggerSize, containerSize, headerSize, footerSize);
                    
                    if (onRefreshDataCallback) {
                        onRefreshDataCallback(_refreshView, _refreshState, _refreshTriggerSize);
                    }
                    
                    updated = true;
                }
            }
        }
    }
    
    _needUpdate = true;
    if (_lastScrollPosition != scrollPosition) {
        float distance = (scrollPosition - _lastScrollPosition);
        onScrollChanged(scrollPosition, distance);
        
    } else if (!updated) {
        _needUpdate = false;
        unscheduleScrollUpdate();
    }
    
    _deltaScroll = _lastScrollPosition - scrollPosition;
    _lastScrollPosition = scrollPosition;
    
    
    
    return;
}
void SMTableView::onScrollChanged(float position, float distance) {
    if (onScrollCallback) {
        onScrollCallback(position, distance);
    }
}

void SMTableView::scrollFling(const float velocity) {
    _scroller->onTouchFling(velocity);
}


int SMTableView::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    
    if (_forceJumpPage)
        return TOUCH_TRUE;
    
    if (_tableRect && action == MotionEvent::ACTION_DOWN && !_tableRect->containsPoint(*point)) {
        if (_lockScroll)
            return TOUCH_TRUE;
    } else {
        if (_lockScroll) {
            return BaseTableView::dispatchTouchEvent(action, touch, point, event);
        }
        
        if (!_inScrollEvent && _scroller->isTouchable()) {
            if (action == MotionEvent::ACTION_DOWN && _scroller->getState() != SMScroller::State::STOP) {
                stop();
                scheduleScrollUpdate();
            }
            int ret = BaseTableView::dispatchTouchEvent(action, touch, point, event);
            if (ret == TOUCH_INTERCEPT) {
                return TOUCH_INTERCEPT;
            }
        }
    }
    
    float x = point->x;
    float y = point->y;
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
            _inScrollEvent = false;
            _lastMotionX = x;
            _lastMotionY = y;
            _firstMotionTime = _director->getGlobalTime();
            
            if (_accelScrollEnable) {
                if (_scroller->getScrollSpeed() > 2000) {
                    _accelCount++;
                } else {
                    _accelCount = 0;
                }
            }
            
            _scroller->onTouchDown();
            
            if (_scrollRect != nullptr && !_scrollRect->containsPoint(*point)) {
                _touchFocused = false;
                return TOUCH_FALSE;
            }
            
            _touchFocused = true;
            _velocityTracker->addMovement(event);
            
            
            break;
            
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            _touchFocused = false;
            
            if (_inScrollEvent) {
                _inScrollEvent = false;
                
                float vx, vy;
                _velocityTracker->getVelocity(0, &vx, &vy);
                
                // Velocity tracker에서 계산되지 않았을때 보정..
                if (vx == 0 && vy == 0) {
                    auto dt = _director->getGlobalTime() - _firstMotionTime;
                    if (dt > 0) {
                        auto p1 = touch->getStartLocationInView();
                        auto p2 = touch->getLocationInView();
                        vx = (p2.x - p1.x) / dt;
                        vy = -(p2.y - p1.y) / dt;
                    }
                }
                
                // Accelate scroll
                float maxVelocity = _maxVelocicy;
                if (_accelScrollEnable) {
                    auto dt = _director->getGlobalTime() - _firstMotionTime;
                    
                    if (dt < 0.15 && _accelCount > 3) {
                        maxVelocity *= (_accelCount-2);
                    }
                }
                
                //                int dir = ViewUtil::getDirection(vx, vy);
                
                if (isVertical()) {
                    if (/*(dir == ViewUtil::UP || dir == ViewUtil::DOWN) &&*/ std::abs(vy) > SMViewConstValue::Config::MIN_VELOCITY) {
                        if (std::abs(vy) > maxVelocity) {
                            vy = ViewUtil::signum(vy) * maxVelocity;
                        }
                        scrollFling(-vy);
                    } else {
                        _scroller->onTouchUp();
                    }
                    
                    _lastVelocityY = vy;
                    _lastVelocityX = 0;
                    _lastFlingTime = _director->getGlobalTime();
                    
                } else {
                    if (/*(dir == ViewUtil::LEFT || dir == ViewUtil::RIGHT) &&*/ std::abs(vx) > SMViewConstValue::Config::MIN_VELOCITY) {
                        if (std::abs(vx) > maxVelocity) {
                            vx = ViewUtil::signum(vx) * maxVelocity;
                        }
                        scrollFling(vx);
                    } else {
                        _scroller->onTouchUp();
                    }
                    
                    _lastVelocityX = vx;
                    _lastVelocityY = 0;
                    _lastFlingTime = _director->getGlobalTime();
                }
                
                scheduleScrollUpdate();
            } else {
                _scroller->onTouchUp();
                scheduleScrollUpdate();
                _lastVelocityX = 0;
                _lastVelocityY = 0;
            }
            
            _velocityTracker->clear();
            
            // 터치로 놓을 때 refreshView 처리
            if (_refreshView && _refreshState != RefreshState::NONE) {
                //                float size = -_scroller->getScrollPosition();
                
                switch (_refreshState) {
                        // 준비상태에서 터치 release
                        //                    case RefreshState::READY:
                        //                        if (size >= _refreshTriggerSize) {
                        //                            // 충분한 사이즈일태 BEGIN REFRESH
                        //                            _refreshState = RefreshState::REFRESHING;
                        //                        } else {
                        //                            // 충분한 사이즈 아니면 취소
                        //                            _refreshState = RefreshState::EXIT;
                        //                        }
                        //                        scheduleScrollUpdate();
                        //                        break;
                    case RefreshState::ENTER:
                        _refreshState = RefreshState::EXIT;
                        scheduleScrollUpdate();
                        break;
                        
                    default:
                        break;
                }
            }
            
            break;
        case MotionEvent::ACTION_MOVE:
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
            
            // 터치로 당길 때 refreshView 처리
            if (_refreshView && !getActionByTag(SMViewConstValue::Tag::ACTION_LIST_HIDE_REFRESH)) {
                float size = -_scroller->getScrollPosition();
                
                switch (_refreshState) {
                        // 아무상태 아님
                    case RefreshState::NONE:
                        canRefreshData= nullptr;
                        if (canRefreshData != nullptr && !canRefreshData)
                            break;
                        
                        if (size > 0) {
                            // 최상단에서 아래로 당길때 RefreshView 추가
                            _refreshState = RefreshState::ENTER;
                            scheduleScrollUpdate();
                        }
                        break;
                        
                        // ENTER 상태
                    case RefreshState::ENTER:
                        if (size < 0) {
                            // refresh 시작 전 사이즈 0 이하면 취소
                            _refreshState = RefreshState::NONE;
                        } else if (size >= _refreshTriggerSize) {
                            // 발동 사이즈 이상이면 준비 상태로 전환 (삭제)
                            // _refreshState = RefreshState::READY;
                            
                            // 충분히 당겨지면 바로 발동하는 것으로 변경함 (추가)
                            _refreshState = RefreshState::REFRESHING;
                        }
                        scheduleScrollUpdate();
                        break;
                        
                        // 준비 상태
                    case RefreshState::READY:
                        if (size < 0) {
                            // refresh 시작 전 사이즈 0 이하면 취소
                            _refreshState = RefreshState::NONE;
                        } else if (size < _refreshTriggerSize) {
                            // 발동 사이즈 이하로 내려가면 ENTER 상태로 전환
                            _refreshState = RefreshState::ENTER;
                        }
                        scheduleScrollUpdate();
                        break;
                        
                    default:
                        break;
                }
            }
            
            if (_touchFocused && !_inScrollEvent) {
                float ax = x - _lastMotionX;
                float ay = y - _lastMotionY;
                // 첫번째 스크롤 이벤트에서만 체크한다
                int dir = ViewUtil::getDirection(ax, ay);
                
                if (isVertical()) {
                    if ((dir == ViewUtil::UP || dir == ViewUtil::DOWN) && std::abs(ay) > SMViewConstValue::Config::SCROLL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
                } else {
                    if ((dir == ViewUtil::LEFT || dir == ViewUtil::RIGHT) && std::abs(ax) > SMViewConstValue::Config::SCROLL_HORIZONTAL_TOLERANCE) {
                        _inScrollEvent = true;
                    }
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
                if (isVertical()) {
                    _scroller->onTouchScroll(-deltaY);
                } else {
                    _scroller->onTouchScroll(+deltaX);
                }
                _lastMotionX = x;
                _lastMotionY = y;
                scheduleScrollUpdate();
            }
            
            break;
    }
    
    if (_inScrollEvent) {
        return TOUCH_INTERCEPT;
    }
    
    return TOUCH_TRUE;
}

bool SMTableView::findCursorForIndexPath(IndexPath& indexPath, Cursor& outCursor) {
    CCASSERT(indexPath.getIndex() >= 0 && indexPath.getIndex() <= numberOfRowsInSection(0), "index out of bounds");
    
    int index = indexPath.getIndex();
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        ColumnInfo* info = _column + col;
        Cursor begin, end;
        
        begin = info->getFirstCursor();
        end = info->getLastCursor();
        
        if (!info->getViewFirstCursor().isEnd()) {
            if (index < info->getViewFirstCursor().getIndexPath().getIndex()) {
                end = ++info->getViewFirstCursor();
            } else if (!info->getViewLastCursor().isEnd()) {
                if (index < info->getViewLastCursor().getIndexPath().getIndex()) {
                    begin = info->getViewFirstCursor();
                    end = info->getViewLastCursor();
                } else {
                    begin = info->getViewLastCursor();
                }
            }
        }
        
        Cursor& cursor = begin;
        int count = end - begin;
        
        for (; count > 0; ++cursor, --count) {
            if (!cursor.getItem().isDeleted()) {
                if (cursor.getIndexPath().getIndex() == index) {
                    indexPath = IndexPath(0, (int)col, index);
                    outCursor = cursor;
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool SMTableView::findCursorForChild(cocos2d::Node* child, SMTableView::Cursor& outCursor) {
    CCASSERT(child != nullptr, "child is null");
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        
        ssize_t numChild = getChildrenCount(col);
        for (int i = 0; i < numChild; i++) {
            if (child == getChildAt(col, i)) {
                ColumnInfo* info = _column + col;
                outCursor = info->getViewFirstCursor(i);
                
                if (!(outCursor.getItem()._flags & ITEM_FLAG_DELETE)) {
                    return true;
                }
                break;
            }
        }
    }
    
    return false; // not found
}

cocos2d::Node* SMTableView::findChildForIndexPath(IndexPath& indexPath, Cursor& outCursor) {
    
    if (!findCursorForIndexPath(indexPath, outCursor)) {
        return nullptr;
    }
    
    ColumnInfo* info = _column + indexPath.getColumn();
    if (outCursor < info->getViewFirstCursor() || outCursor >= info->getViewLastCursor())
        return nullptr;
    
    int offset = outCursor - info->getViewFirstCursor();
    
    if (offset < 0 || offset >= getChildrenCount(indexPath.getColumn())) {
        return nullptr;
    }
    
    return getChildAt(indexPath.getColumn(), outCursor - info->getViewFirstCursor());
}



void SMTableView::stopAndCompleteChildAction(int tag) {
    if (tag <= 0)
        return;
    
    cocos2d::Action* action = getActionByTag(tag);
    if (action) {
        _DelaySequence* seq = dynamic_cast<_DelaySequence*>(action);
        _BaseAction* a = nullptr;
        if (seq) {
            a = seq->getAction();
        } else {
            a = dynamic_cast<_BaseAction*>(action);
        }
        
        if (a) {
            a->complete();
            stopActionByTag(tag);
        }
    }
}

void SMTableView::deleteCursor(const int column, Cursor cursor, bool cleanup) {
    
    ColumnInfo* info = _column + column;
    
    // 화면에 표시중이면 child 삭제
    if (cursor >= info->getViewFirstCursor() && cursor < info->getViewLastCursor()) {
        int position = cursor - info->getViewFirstCursor();
        cocos2d::Node* child = getChildAt(column, position);
        Item& item = cursor.getItem();
        
        if (onCellDeleteCallback) {
            onCellDeleteCallback(child, 1);
        }
        
        if (dynamic_cast<_DeletedNode*>(child) || item._dontReuse) {
            removeChild(column, child, cleanup);
        } else {
            removeChildAndReuseScrap(column, item._reuseType, child, CLEANUP_FALG);
        }
    }
    
    // column에서 cursor 삭제
    _column[column].deleteCursor(cursor);
    
    // 삭제 후 child reorder
    if (_column[column]._data.size() > 0) {
        ssize_t numChild = getChildrenCount(column);
        
        if (numChild > 0) {
            Cursor c = _column[column].getViewFirstCursor();
            for (ssize_t i = 0; i < numChild; i++) {
                
                cocos2d::Node* child = getChildAt(column, i);
                if (child) {
                    child->setLocalZOrder(c.getPosition());
                }
                c++;
            }
            sortAllChildren(column);
        }
    }
}

bool SMTableView::performDelete(Cursor& cursor, cocos2d::Node* child, float duration, float delay, bool cleanup) {
    
    if (cursor.getItem().isDeleted())
        return false;
    
    int column = cursor.getIndexPath().getColumn();
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        _column[col].markDeleteCursor(cursor);
    }
    
    Item& item = cursor.getItem();
    if (item._tag > 0) {
        cocos2d::Action* a = getActionByTag(item._tag);
        if (a) {
            stopAction(a);
        }
    }
    
    _column[column]._resizeReserveSize = -item._size;
    
    if (duration > 0 || delay > 0) {
        // 애니메이션
        _BaseAction* deleteAction = _DeleteAction::create(this, column, cursor);
        deleteAction->setDuration(duration);
        cocos2d::Action* action;
        if (delay > 0) {
            action = _DelaySequence::create(delay, deleteAction);
        } else {
            action = deleteAction;
        }
        
        item._flags |= ITEM_FLAG_RESIZE;
        item._tag = _internalActionTag++;
        action->setTag(item._tag);
        
        runAction(action);
    } else {
        item._newSize = 0;
        _column[column].resizeCursor(cursor);
        deleteCursor(column, cursor, cleanup);
    }
    
    scheduleScrollUpdate();
    
    return true;
}

bool SMTableView::performResize(Cursor& cursor, cocos2d::Node* child, const float newSize, float duration, float delay) {
    Item& item = cursor.getItem();
    
    if (item._tag > 0) {
        stopAndCompleteChildAction(item._tag);
    }
    
    IndexPath& indexPath = cursor.getIndexPath();
    _column[indexPath.getColumn()]._resizeReserveSize += newSize - item._size;
    
    if (duration > 0 || delay > 0) {
        // 애니메이션
        _BaseAction* resizeAction = _ResizeAction::create(this, indexPath.getColumn(), cursor, newSize);
        resizeAction->setDuration(duration);
        cocos2d::Action* action;
        if (delay > 0) {
            action = _DelaySequence::create(delay, resizeAction);
        } else {
            action = resizeAction;
        }
        
        item._tag = _internalActionTag++;
        action->setTag(item._tag);
        
        runAction(action);
    } else {
        item._newSize = newSize;
        _column[indexPath.getColumn()].resizeCursor(cursor);
        _animationDirty = true;
    }
    
    scheduleScrollUpdate();
    return true;
}

void SMTableView::setMinScrollSize(const float minScrollSize) {
    _ScrollProtocol::setMinScrollSize(minScrollSize);
    _scroller->setScrollSize(std::max(_minScrollSize, _scroller->getScrollSize()));
}

void SMTableView::setScrollPosition(const float position) {
    _scroller->setScrollPosition(position);
}

void SMTableView::reloadData(bool bExceptHeader) {

    _reloadExceptHeader = bExceptHeader;

    _reloadFlag = true;
    scheduleScrollUpdate();
}

void SMTableView::updateData() {
    _animationDirty = true;
    scheduleScrollUpdate();
}

void SMTableView::notifyScrollUpdate() {
    scheduleScrollUpdate();
}


void SMTableView::reloadRowsAtIndexPath(const IndexPath& indexPath) {
    Cursor cursor;
    
    IndexPath ip = indexPath;
    
    if (!findCursorForIndexPath(ip, cursor))
        return;
    
    cursor.getItem()._reload = true;
    
    scheduleScrollUpdate();
}


cocos2d::Node* SMTableView::getCellForIndexPath(const IndexPath& indexPath) {
    Cursor cursor;
    
    IndexPath ip = indexPath;
    
    cocos2d::Node* cell = findChildForIndexPath(ip, cursor);
    
    if (!cell || dynamic_cast<_DeletedNode*>(cell)) {
        // 해당 cell 찾지 못함 or 이미 지워짐
        return nullptr;
    }
    
    return cell;
}

IndexPath* SMTableView::getIndexPathForCell(cocos2d::Node* cell) {
    Cursor cursor;
    
    if (!findCursorForChild(cell, cursor)) {
        // 해당 item 찾지 못함 or 이미 지워짐
        return nullptr;
    }
    
    return &cursor.getIndexPath();
}

bool SMTableView::insertRowAtIndexPath(const IndexPath& indexPath, float estimateSize, float duration, float delay, bool immediate) {
    CCASSERT(indexPath.getIndex() >= 0 && indexPath.getIndex() <= numberOfRowsInSection(0), "index out of bounds");
    
    //    if (!_adapter->adapterOnInsertItem(this, indexPath.getIndex()))
    //        return false;
    
    ColumnInfo* info;
    
    // 추가할 column 확인
    ssize_t column = -1;
    float lastLocation = FLT_MAX;
    int lastIndex = INT_MIN;
    for (ssize_t col = _numContainer-1; col >= 0; col--) {
        info = _column + col;
        
        lastIndex = std::max(lastIndex, info->_lastIndexPath.getIndex());
        
        if (info->_data.size() == 0) {
            column = col;
            lastLocation = 0;
        } else {
            if (info->getLastCursor().getLocation() + info->_resizeReserveSize < lastLocation) {
                column = col;
                lastLocation = info->getLastCursor().getLocation() + info->_resizeReserveSize;
            }
        }
    }
    
    if (indexPath.getIndex() > lastIndex+1) {
        // 아직 자라지 않은 Item이면 지금 추가할 필요 없다.
        return true;
    }
    
    Cursor cursor;
    
    for (ssize_t col = 0; col < _numContainer; col++) {
        if (col == column) {
            cursor = _column[col].insertItem(IndexPath(0, (int)column, indexPath.getIndex()), -1, estimateSize);
        } else {
            _column[col].markInsertItem(IndexPath(0, (int)column, indexPath.getIndex()));
        }
    }
    
    Item& item = cursor.getItem();
    info = _column + column;
    
    bool needChild = cursor >= info->getViewFirstCursor() && cursor < info->getViewLastCursor();
    bool addChildOnTop = false;
    if (immediate) {
        // TODO: 임시 땜빵 (수정해야 한다..)
        if (info->getViewFirstCursor() > info->getFirstCursor() && cursor == info->getViewFirstCursor(-1)) {
            addChildOnTop = true;
            needChild = true;
        } else if (cursor == info->getViewLastCursor(+1)) {
            needChild = true;
        }
    }
    
    if (needChild && cursor == info->getViewLastCursor(-1)) {
        float headerSize = 0;
        if (_headerView != nullptr) {
            headerSize = isVertical()?_headerView->getContentSize().height:_headerView->getContentSize().width;
        }
        // View 내 위치
        float location = headerSize + _firstMargin - _scroller->getScrollPosition() + cursor.getLocation();
        float containerSize = isVertical()?_contentSize.height:_contentSize.width;
        
        if (location > containerSize) {
            info->retreatViewLast();
            needChild = false;
        }
    }
    
    bool needAnimation = (duration > 0 || delay > 0) && (needChild || cursor < info->getViewFirstCursor());
    
    if (!needAnimation) {
        item._newSize = estimateSize;
        info->resizeCursor(cursor);
    }
    
    if (needChild) {
        // 화면에 즉시 보여야 한다.
        int childIndex = 0;
        
        ssize_t numChild = getChildrenCount(column);
        
        // 1) 현재 children reorder
        for (Cursor c = info->getViewFirstCursor(); childIndex < numChild && c < info->getViewLastCursor(); ++c) {
            if (c == cursor)
                continue;
            
            cocos2d::Node* child = getChildAt(column, childIndex++);
            if (child) {
                child->setLocalZOrder(c.getPosition());
            }
        }
        
        if (addChildOnTop) {
            info->advanceViewFirst();
        }
        
        // 2) child 추가
        _reuseScrapper->_internalReuseType = -1;
        _reuseScrapper->_internalReuseNode = nullptr;
        cocos2d::Node* child = cellForRowAtIndexPath(item._indexPath);
        if (_reuseScrapper->_internalReuseType >= 0) {
            item._reuseType = _reuseScrapper->_internalReuseType;
        }
        
        if (!child) {
            CC_ASSERT(0);
        }
        
        // order
        child->setLocalZOrder(cursor.getPosition());
        
        float headerSize = 0;
        if (_headerView != nullptr) {
            headerSize = isVertical()?_headerView->getContentSize().height:_headerView->getContentSize().width;
        }
        
        // View 내 위치
        float locationInView = headerSize + _firstMargin - _scroller->getScrollPosition();
        if (isVertical()) {
            onPositionCell(child, _contentSize.height - (cursor.getLastLocation() + locationInView), true);
        } else {
            onPositionCell(child, cursor.getLocation() + locationInView, true);
        }
        
        addChild(column, child);
        if (_reuseScrapper->_internalReuseNode && _reuseScrapper->_internalReuseNode == child) {
            _reuseScrapper->popBack(_reuseScrapper->_internalReuseType);
        }
        _reuseScrapper->_internalReuseType = -1;
        _reuseScrapper->_internalReuseNode = nullptr;
        
        if (!needAnimation) {
            item._newSize = estimateSize;
            info->resizeCursor(cursor);
        }
        
        if (onCellResizeCallback) {
            onCellResizeCallback(child, item._newSize);
        }
        
        if (!needAnimation && onCellInsertCallback) {
            onCellInsertCallback(child, 1);
        }
        
        sortAllChildren(column);
    }
    
    if (needAnimation) {
        // 애니메이션
        _BaseAction* insertAction = _InsertAction::create(this, (int)column, cursor, estimateSize);
        insertAction->setDuration(duration);
        cocos2d::Action* action;
        if (delay > 0) {
            action = _DelaySequence::create(delay, insertAction);
        } else {
            action = insertAction;
        }
        
        item._flags = ITEM_FLAG_RESIZE | ITEM_FLAG_INSERT;
        item._tag = _internalActionTag++;
        action->setTag(item._tag);
        
        runAction(action);
    } else {
        info->resizeCursor(cursor);
        
        if (!needChild) {
            // child도 없고 animation도 없으면 flags에 표시만 해둔다.
            // => 추가되는 순간 onCellInsertCallback 호출됨
            cursor.getItem()._flags = ITEM_FLAG_INSERT;
        }
    }
    
    scheduleScrollUpdate();
    return false;
}

bool SMTableView::deleteRowForCell(cocos2d::Node* child, float duration, float delay) {
    CCASSERT(child != nullptr, "child is null");
    
    Cursor cursor;
    
    if (!findCursorForChild(child, cursor)) {
        // child가 없거나 delete중임
        return false;
    }
    
    if (cursor >= _column[cursor.getIndexPath().getColumn()].getViewLastCursor()) {
        // 화면 밖 뒤에 있으면 즉시 삭제 (애니메이션 필요 없음)
        duration = 0;
        delay = 0;
    }
    
    return performDelete(cursor, child, duration, delay);
}

cocos2d::Node* SMTableView::popCell(cocos2d::Node* cell) {
    CCASSERT(cell != nullptr, "child is null");
    
    Cursor cursor;
    
    if (!findCursorForChild(cell, cursor)) {
        // child가 없거나 delete중임
        return nullptr;
    }
    
    if (cursor <  _column[cursor.getIndexPath().getColumn()].getViewFirstCursor() || cursor >= _column[cursor.getIndexPath().getColumn()].getViewLastCursor()) {
        return nullptr;
    }
    
    cursor.getItem()._dontReuse = true;
    
    if (performDelete(cursor, cell, 0, 0, false)) {
        return cell;
    }
    
    return nullptr;
}




bool SMTableView::isDeleteCell(cocos2d::Node* child) {
    Cursor cursor;
    
    if (!findCursorForChild(child, cursor)) {
        // child가 없거나 delete중임
        return true;
    }
    
    return false;
    
}

bool SMTableView::deleteRowAtIndexPath(const IndexPath& indexPath, float duration, float delay) {
    CCASSERT(indexPath.getIndex() >= 0 && indexPath.getIndex() <= numberOfRowsInSection(0), "index out of bounds");
    
    int lastIndex = INT_MIN;
    for (ssize_t col = 0; col < _numContainer; col++) {
        lastIndex = std::max(lastIndex, _column[col].getLastIndexPath().getIndex());
    }
    
    if (indexPath.getIndex() > lastIndex) {
        // 아직 생성되지 않은 Item이면 바로 지운다.
        //        return _adapter->adapterOnDeleteItem(this, indexPath.getIndex());
        return true;
    }
    
    Cursor cursor;
    IndexPath searchIndexPath = indexPath;
    cocos2d::Node* child = findChildForIndexPath(searchIndexPath, cursor);
    if (child == nullptr) {
        // 해당 item 찾지 못함 or 이미 지워짐
//        return false;
        searchIndexPath = indexPath;
        findCursorForIndexPath(searchIndexPath, cursor);
    }
    
    if (cursor >= _column[indexPath.getColumn()].getViewLastCursor()) {
        // 화면 밖 뒤에 있으면 즉시 삭제 (애니메이션 필요 없음)
        duration = 0;
        delay = 0;
    }
    
    return performDelete(cursor, child, duration, delay);
}

bool SMTableView::resizeRowForCell(cocos2d::Node* child, float newSize, float duration, float delay) {
    CCASSERT(child != nullptr, "child is null");
    
    Cursor cursor;
    
    if (!findCursorForChild(child, cursor)) {
        // 해당 item 찾지 못함 or 이미 지워짐
        return false;
    }
    
    if (cursor >= _column[cursor.getIndexPath().getColumn()].getViewLastCursor()) {
        // 화면 밖 뒤에 있으면 즉시 삭제 (애니메이션 필요 없음)
        duration = 0;
        delay = 0;
    }
    
    return performResize(cursor, child, newSize, duration, delay);
}

bool SMTableView::resizeRowForIndexPath(IndexPath& indexPath, float newSize, float duration, float delay) {
    Cursor cursor;
    
    cocos2d::Node* child = findChildForIndexPath(indexPath, cursor);
    if (!child) {
        // 해당 item 찾지 못함 or 이미 지워짐
        return false;
    }
    
    if (cursor >= _column[cursor.getIndexPath().getColumn()].getViewLastCursor()) {
        // 화면 밖 뒤에 있으면 즉시 삭제 (애니메이션 필요 없음)
        duration = 0;
        delay = 0;
    }
    
    return performResize(cursor, child, newSize, duration, delay);
}

cocos2d::Vector<cocos2d::Node*>& SMTableView::getVisibleCells(const int column) {
    CCASSERT(column >= 0 && column < _numContainer, "column out of bounds");
    
    return _container[column]->getChildren();
}

float SMTableView::getScrollPosition() {
    return _scroller->getScrollPosition();
}

float SMTableView::getScrollSize() {
    
    if (_scroller->getScrollSize() <= 0 && numberOfRowsInSection) {
        measureScrollSize();
    }
    
    return _scroller->getScrollSize();
}

void SMTableView::scrollTo(float position) {
    auto scroller = static_cast<FlexibleScroller*>(_scroller);
    
    if (position < 0) {
        position = 0;
    } else {
        if (measureScrollSize()) {
            float scrollSize = scroller->getScrollSize();
            if (position > scrollSize) {
                position = scrollSize;
            }
        }
    }
    
    scroller->scrollTo(position);
    
    scheduleScrollUpdate();
}

void SMTableView::scrollToWithDuration(float position, float duration) {
    auto scroller = dynamic_cast<FlexibleScroller*>(_scroller);
    if (scroller) {
        //        if (position < 0) {
        //            position = 0;
        //        } else {
        //            if (measureScrollSize()) {
        //                float scrollSize = scroller->getScrollSize();
        //                if (position > scrollSize) {
        //                    position = scrollSize;
        //                }
        //            }
        //        }
        
        scroller->scrollToWithDuration(position, duration);
        
        scheduleScrollUpdate();
    }
}


void SMTableView::scrollBy(float offset) {
    scrollTo(_scroller->getScrollPosition() + offset);
}

void SMTableView::scrollByWithDuration(float offset, float duration) {
    scrollToWithDuration(_scroller->getScrollPosition() + offset, duration);
}

void SMTableView::setRefreshDataView(cocos2d::Node* node, float triggerSize, float minSize) {
    if (_refreshView != nullptr && _refreshView != node) {
        cocos2d::Node::removeChild(_refreshView, true);
        CC_SAFE_RELEASE(_refreshView);
    }
    
    _refreshView = node;
    _refreshTriggerSize = triggerSize;
    if (minSize < 0) {
        _refreshMinSize = _refreshTriggerSize;
    } else {
        _refreshMinSize = minSize;
    }
    
    if (_refreshView != nullptr) {
        _refreshView->retain();
    }
    
    scheduleScrollUpdate();
}

void SMTableView::endRefreshData() {
    if (!_refreshView || _refreshState != RefreshState::REFRESHING)
        return;
    
    _refreshState = RefreshState::EXIT;
    
    _scroller->setHangSize(0);
    _scroller->onTouchUp();
    
    scheduleScrollUpdate();
}

void SMTableView::endLoadData(bool bNeedMore) {
    
    if (!bNeedMore) {
        // 더이상 로드할 데이터가 없음
        //        setFooterView(nullptr);
        if (onLoadDataCallback) {
            onLoadDataCallbackTemp = onLoadDataCallback;
            onLoadDataCallback = nullptr;
        }
        
        if (_footerView && _isFooterInList) {
            cocos2d::Node::removeChild(_footerView,  true);
        }
        _isFooterInList = false;
        _justAtLast = false;
    }
    _progressLoading = false;
    updateData();
}


void SMTableView::setMaxScrollVelocity(const float maxVelocity) {
    float v = maxVelocity;
    if (v < SMViewConstValue::Config::MIN_VELOCITY) {
        v = SMViewConstValue::Config::MIN_VELOCITY+1;
    }
    _maxVelocicy = v;
}

void SMTableView::enableAccelerateScroll(bool enable) {
    _accelScrollEnable = enable;
}

void SMTableView::enableInitRefresh(bool enable) {
    _initRefreshEnable = true;
}

void SMTableView::setHeaderView(cocos2d::Node* node) {
    BaseTableView::setHeaderView(node);
    scheduleScrollUpdate();
}

void SMTableView::setFooterView(cocos2d::Node* node) {
    BaseTableView::setFooterView(node);
    scheduleScrollUpdate();
}

float SMTableView::getScrollSpeed() {
    return _scroller->getScrollSpeed();
}

float SMTableView::getTotalHeightInSection(int section) {
    return _column[section].getLastCursor().getLocation();
}

void SMTableView::fakeAdvanceLast(int index, float size) {
    _column[0].advanceViewLast(IndexPath(index), 0, size);
    _column[0].retreatViewFirst();
}

void SMTableView::fakeAdvanceLast2(int index, float size) {
    _column[0].advanceViewLast(IndexPath(index), 0, size);
}







