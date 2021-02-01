//
//  BaseTableView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#include "BaseTableView.h"
#include "VelocityTracker.h"
#include "SMScroller.h"
#include <cocos2d.h>

#define CLEANUP_FLAG (true)

BaseTableView::InstantHolder::InstantHolder()
{
    
}

BaseTableView::InstantHolder::~InstantHolder()
{
    _data.clear();
}

// view에서 떼어내고 holder에 붙임.
bool BaseTableView::InstantHolder::insert(const int tag, cocos2d::Node *const parent, cocos2d::Node *const child, const bool cleanup)
{
    CC_ASSERT(parent!=nullptr && child!=nullptr);
    
    if (find(tag)) {
        return false;
    }
    
    child->retain();
    
    parent->removeChild(child, cleanup);
    _data.insert(tag, child);
    
    child->release();
    
    return true;
}

cocos2d::Node * BaseTableView::InstantHolder::find(const int tag)
{
    cocos2d::Map<int, cocos2d::Node*>::iterator iter = _data.find(tag);
    
    if (iter!=_data.end()) {
        return iter->second;
    }
    
    return nullptr;
}

void BaseTableView::InstantHolder::erase(const int tag)
{
    _data.erase(tag);
}

void BaseTableView::InstantHolder::clear()
{
    _data.clear();
}

// BaseTableView abstract class

BaseTableView::BaseTableView() :
_holder(nullptr),
_headerView(nullptr),
_footerView(nullptr),
_isHeaderInList(false),
_isFooterInList(false),
_lockScroll(false),
_reloadFlag(false)
{
    
}

BaseTableView::~BaseTableView()
{
    setHeaderView(nullptr);
    setFooterView(nullptr);
    
    // SMView bgNode 까지 clear
    SMView::removeAllChildrenWithCleanup(true);
    
    CC_SAFE_DELETE(_scroller);
    CC_SAFE_DELETE(_holder);
    CC_SAFE_DELETE_ARRAY(_container);
}

bool BaseTableView::initWithContainer(const ssize_t numContainer)
{
    CCASSERT(numContainer>0, "container must greater than zero");
    
    _numContainer = numContainer;
    
    _container = new SMView*[_numContainer];
    for (int i=0; i<_numContainer; i++) {
        _container[i] = SMView::create();
        SMView::addChild(_container[i]);
    }
    
    if (_headerView != nullptr && _isHeaderInList) {
        cocos2d::Node::removeChild(_headerView, CLEANUP_FLAG);
        _isHeaderInList = false;
    }
    
    if (_footerView != nullptr && _isFooterInList) {
        cocos2d::Node::removeChild(_footerView, CLEANUP_FLAG);
    }
    
    removeAllChildrenWithCleanup(true);
    
    return true;
}

void BaseTableView::stop()
{
    if (_scroller!=nullptr) {
        // last click;
        _scroller->onTouchDown();
        _scroller->onTouchUp();
    }
}

void BaseTableView::removeAllChildrenWithCleanup(bool cleanup)
{
    // container 들만 남기고 다 지움
    
    // container 떼어냄...
    if (_container!=nullptr) {
        for (int i=0; i<_numContainer; i++) {
            _container[i]->retain();
            SMView::removeChild(_container[i]);
        }
    }
    
    // 나머지 다 날리고
    SMView::removeAllChildrenWithCleanup(cleanup);
    
    // container들을 다시 붙임
    if (_container!=nullptr) {
        for (int i=0; i<_numContainer; i++) {
            SMView::addChild(_container[i]);
            // reference count 하나 올라갔으니 release
            _container[i]->release();
        }
    }
}

void BaseTableView::removeChildAndHold(const ssize_t container, const int tag, cocos2d::Node *child, const bool cleanup)
{
    // view에서 떼어내서 holder에 붙임
    getHolder()->insert(tag, _container[container], child);
}

cocos2d::Node * BaseTableView::findFromHolder(const int tag)
{
    if (_holder) {
        return getHolder()->find(tag);
    }
    
    return nullptr;
}

void BaseTableView::eraseFromHolder(const int tag)
{
    if (_holder) {
        getHolder()->erase(tag);
    }
}

// add child
void BaseTableView::addChild(const ssize_t container, cocos2d::Node *child)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->addChild(child);
}

void BaseTableView::addChild(const ssize_t container, cocos2d::Node *child, int localZOrder)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->addChild(child, localZOrder);
}

void BaseTableView::addChild(const ssize_t container, cocos2d::Node *child, int localZOrder, int tag)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->addChild(child, localZOrder, tag);
}

void BaseTableView::addChild(const ssize_t container, cocos2d::Node *child, int localZOrder, const std::string &name)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->addChild(child, localZOrder, name);
}


// remove child
void BaseTableView::removeChild(const ssize_t container, cocos2d::Node *child, bool cleanup)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->removeChild(child, cleanup);
}

void BaseTableView::removeChildByTag(const ssize_t container, int tag, bool cleanup)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->removeChildByTag(tag, cleanup);
}

void BaseTableView::removeChildByName(const ssize_t container, const std::string &name, bool cleanup)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->removeChildByName(name, cleanup);
}

// sort list
void BaseTableView::sortAllChildren(const ssize_t container)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    _container[container]->sortAllChildren();
}


// child node, count...
ssize_t BaseTableView::getChildrenCount(const ssize_t container) const
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    return _container[container]->getChildrenCount();
}

cocos2d::Node* BaseTableView::getChildAt(const ssize_t container, ssize_t index)
{
    CC_ASSERT(container>=0 && container<_numContainer);
    
    return _container[container]->getChildren().at(index);
}

cocos2d::Vector<cocos2d::Node*>& BaseTableView::getColumnChildren(const ssize_t column)
{
    CC_ASSERT(column>=0 && column<_numContainer);
    
    return _container[column]->getChildren();
}

// header
void BaseTableView::setHeaderView(cocos2d::Node *node)
{
    if (_headerView!=nullptr && _headerView!=node) {
        // exist header view
        // remove and release
        cocos2d::Node::removeChild(_headerView, true);
        CC_SAFE_RELEASE(_headerView);
    }
    
    _headerView = node;
    _isHeaderInList = false;
    
    if (_headerView!=nullptr) {
        _headerView->retain();
    }
}

void BaseTableView::setFooterView(cocos2d::Node *node)
{
    if (_footerView!=nullptr && _footerView!=node) {
        // exist footer view
        // remove and release
        cocos2d::Node::removeChild(_footerView, true);
        CC_SAFE_RELEASE(_footerView);
    }
    
    _footerView = node;
    _isFooterInList = false;
    
    if (_footerView!=nullptr) {
        _footerView->retain();
    }
}

BaseTableView::InstantHolder* BaseTableView::getHolder()
{
    if (_holder==nullptr) {
        _holder = new InstantHolder();
    }
    
    return _holder;
}

void BaseTableView::clearInstantHolder()
{
    if (_holder) {
        _holder->clear();
    }
}
