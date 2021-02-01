//
//  BaseTableView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef BaseTableView_h
#define BaseTableView_h

#include "../Base/SMView.h"
#include <base/CCVector.h>
#include <base/CCMap.h>
#include "SMScroller.h"

class VelocityTracker;


class BaseTableView : public SMView, public _ScrollProtocol
{
public:
    virtual void removeAllChildrenWithCleanup(bool cleanup) override;

    ssize_t getContainerCount() const { return _numContainer; }

    // header view set
    virtual void setHeaderView(cocos2d::Node* node);
    
    // footer view set
    virtual void setFooterView(cocos2d::Node* node);
    
    void stop();
    
    cocos2d::Vector<cocos2d::Node*>& getColumnChildren(const ssize_t column);
    
    bool isHeaderInList() { return _isHeaderInList; }
    
    bool isFooterInList() { return _isFooterInList; }
    
    void setScrollLock(bool bLock) {_lockScroll = bLock;};
    
    const cocos2d::Node* getHeaderView(){return _headerView;};
    
protected:
    BaseTableView();
    ~BaseTableView();
    
    virtual bool initWithContainer(const ssize_t numContainer);
    
    // list에서 remove holder에 add
    void removeChildAndHold(const ssize_t container, const int tag, cocos2d::Node* child, const bool cleanup = true);
    
    // holder에서 찾음
    cocos2d::Node* findFromHolder(const int tag);
    
    // hodler에서 삭제
    void eraseFromHolder(const int tag);
    
    // instant holder 삭제
    void clearInstantHolder();
    
    // add child
    virtual void addChild(const ssize_t container, cocos2d::Node* child);
    virtual void addChild(const ssize_t container, cocos2d::Node* child, int localZOrder);
    virtual void addChild(const ssize_t container, cocos2d::Node* child, int localZOrder, int tag);
    virtual void addChild(const ssize_t container, cocos2d::Node* child, int localZOrder, const std::string &name);

    // remove child
    virtual void removeChild(const ssize_t container, cocos2d::Node* child, bool cleanup = true);
    virtual void removeChildByTag(const ssize_t container, int tag, bool cleanup = true);
    virtual void removeChildByName(const ssize_t container, const std::string &name, bool cleanup = true);

    // children sort
    virtual void sortAllChildren(const ssize_t container);
    virtual ssize_t getChildrenCount(const ssize_t container) const;
    virtual cocos2d::Node* getChildAt(const ssize_t container, ssize_t index);
    
protected:
    SMView** _container;    // main container
    
    ssize_t _numContainer;
    
    // footer
    cocos2d::Node* _footerView;
    bool _isFooterInList = false;
    
    // header
    cocos2d::Node* _headerView;
    bool _isHeaderInList = false;
    
    bool _lockScroll;
    bool _reloadFlag;
    
private:
    class InstantHolder {
        
    public:
        bool insert(const int tag, cocos2d::Node* const parent, cocos2d::Node* const child, const bool cleanup = true);
        
        cocos2d::Node* find(const int tag);
        void erase(const int tag);
        
        InstantHolder();
        ~InstantHolder();
        
        void clear();
        
    private:
        cocos2d::Map<int, cocos2d::Node*> _data;
    };
    
    // instant view holder
    InstantHolder* _holder;
    
    InstantHolder* getHolder();
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(BaseTableView);
};

#endif /* BaseTableView_hpp */
