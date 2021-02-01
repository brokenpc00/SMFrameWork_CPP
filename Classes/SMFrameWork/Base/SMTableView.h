//
//  SMTableView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef SMTableView_h
#define SMTableView_h

#include "../Interface/BaseTableView.h"
#include <string>
#include <list>
#include <map>
#include <cocos2d.h>


//#warning( "IndexPath에서 section을 제거하자... 어차피 안씀.... 뭐하러 만들었는지 모르겠네... 내부에서 무조건 0으로 호출함." )

class Intent;

struct IndexPath {
private:
    int _section;
    int _column;
    int _index;
    
public:
    IndexPath();
    IndexPath(const int index);
    IndexPath(const int section, const int index);
    IndexPath(const int section, const int column, const int index);
    
    inline int getSection() const { return _section; }
    inline int getColumn() const { return _column; }
    inline int getIndex() const { return _index; }
    
    bool operator==(const IndexPath& rhs) const;
    bool operator!=(const IndexPath& rhs) const;
    bool operator<=(const IndexPath& rhs) const;
    bool operator>=(const IndexPath& rhs) const;
    bool operator<(const IndexPath& rhs) const;
    bool operator>(const IndexPath& rhs) const;
    
    IndexPath& operator=(const IndexPath& rhs);
    IndexPath& operator++();
    IndexPath& operator--();
    IndexPath& operator+=(int);
    IndexPath& operator-=(int);
    IndexPath operator++(int);
    IndexPath operator--(int);
    IndexPath operator+(int);
    IndexPath operator-(int);
};



class SMTableView : public BaseTableView
{
public:
    enum class Orientation {
        VERTICAL,
        HORIZONTAL
    };
    
    enum class RefreshState {
        NONE,
        ENTER,
        READY,
        REFRESHING,
        EXIT
    };

    // create simple (1 column)
    static SMTableView* create(Orientation orient);
    
    // create single column with dimension
    static SMTableView * create(Orientation orient, float x, float y, float width, float height, float anchorX=0, float anchorY=0);
    
    // create simple multicolumn
    static SMTableView * createMultiColumn(Orientation orient, int numOfColumn);
    
    // create multi column with dimension
    static SMTableView * createMultiColumn(Orientation orient, int numOfColumn, float x, float y, float width, float height, float anchorX=0, float anchorY=0);
    
    // update date.. 보통 아래 또는 위로 추가되었을경우
    void updateData();
    
    // 새로 고침
    void reloadData(bool bExceptHeader=false);
    
    // 고정사이즈일경우 같은 사이즈로 스크롤하기 위하여
    virtual void hintFixedCellSize(const float cellSize);
    
    // 스크롤 앞뒤 마진
    virtual void setScrollMarginSize(const float topMargin, const float bottomMargin);
    
    // 화면 밖에서 미리 생성해서 스크롤 되게 하자. 조금 빨라지도록
    virtual void setPreloadPaddingSize(const float paddingSize);
    
    // 특정 rows를 reload
    virtual void reloadRowsAtIndexPath(const IndexPath& indexPath);
    
    // 현재 보이는 셀 가져오기 (column을 지정하면 그놈만)
    cocos2d::Vector<cocos2d::Node*>& getVisibleCells(const int column=0);
    
    // 특정 cell가져옴
    cocos2d::Node* getCellForIndexPath(const IndexPath& indexPath);
    
    // cell의 indexPath 반환
    IndexPath* getIndexPathForCell(cocos2d::Node* cell);
    
    // cell 삽입 (immediate는 첫줄 앞이나 마지막 뒤에 삽입할때만 사용)
    virtual bool insertRowAtIndexPath(const IndexPath& indexPath, float estimateSize, float duration=0, float delay=0, bool immediate=false);
    
    // cell 삭제(삭제된 cell은 reuse되어서 다른 데이터가 있다면 사용된다. 데이터만 삭제된다고 보면 됨)
    virtual bool deleteRowForCell(cocos2d::Node* cell, float duration=0, float delay=0);
    
    // cell 뽑아냄 (삭제된 cell은 reuse되지 않고 Node를 뽑아냄. 데이터와 뷰가 동시에 삭제되길 원할때...또는 뽑아서 다른 뷰에 붙이고 싶을때...)
    virtual cocos2d::Node* popCell(cocos2d::Node* cell);
    
    // indexpath의 cell 삭제
    virtual bool deleteRowAtIndexPath(const IndexPath& indexPath, float duration=0, float delay=0);
    
    // cell resize
    virtual bool resizeRowForCell(cocos2d::Node* cell, float newSize, float duration=0, float delay=0);
    
    // indexpath의 cell resize
    virtual bool resizeRowForIndexPath(IndexPath& indexPath, float newSize, float duration=0, float delay=0);
    
    // reuse cell 반환
    cocos2d::Node* dequeueReusableCellWithIdentifier(const std::string& cellID);
    
    // DATASOURCE
    // cell 요청
    std::function<cocos2d::Node*(const IndexPath& indexPath)> cellForRowAtIndexPath;
    // row count 요청
    std::function<int(int section)> numberOfRowsInSection;
    
    // cell resize callback
    std::function<void(cocos2d::Node*cell, float newSize)> onCellResizeCallback;
    // cell resize complete callback
    std::function<void(cocos2d::Node*cell)> onCellResizeCompletionCallback;
    // cell insert callback
    std::function<void(cocos2d::Node*cell, float progress)> onCellInsertCallback;
    // cell delete callback
    std::function<void(cocos2d::Node*cell, float progress)> onCellDeleteCallback;
    // cell delete complete callback
    std::function<void(void)> onCellDeleteCompletionCallback;
    // scroll callback
    std::function<void(float position, float distance)> onScrollCallback;
    // scroll to
    virtual void scrollTo(float position);
    virtual void scrollToWithDuration(float position, float duration=0);
    // scroll by
    virtual void scrollBy(float position);
    virtual void scrollByWithDuration(float offset, float duration);
    // max scroll velocity
    void setMaxScrollVelocity(const float maxVelocity);
    // current scroll position
    virtual float getScrollPosition();
    // scroll size... 이거는 끝까지 가봐야 안다... 정확하지 않음. (fixed인경우 계산에 의해 뱉어낼 수 있음)
    virtual float getScrollSize();
    // refresh data view (당겨서 새로고침 할때, 새로고침 뷰... 로딩뷰를 여기에 넣으면 됨)
    void setRefreshDataView(cocos2d::Node* node, float triggerSize, float minSize = -1);
    // refresh 끝났음을 알려줘야한다. refresh data view가 들어가야하므로.
    void endRefreshData();
    // refresh callback
    std::function<void(cocos2d::Node* node, RefreshState state, float size)> onRefreshDataCallback;
    std::function<bool(void)> canRefreshData;
    
    // load more callback... callback이 세팅 되었을때 footer가 나타나면 호출된다. 여기서 통신등 페이지 더보기를 호출 하면 된다. 끝나면 endLoadData()를 호출 할 것.
    // 다시 호출 될일 이 없다면 callback을 nullptr로 세팅하거나 footer 자체를 nullptr로 세팅하면 된다.
    std::function<bool(cocos2d::Node* node)> onLoadDataCallback;
    // true이면 그만 부른다.
    void endLoadData(bool bMoreData);
    
    // cell이 처음 나타날때 애니메이션을 위한 callback (willDisplayCell...같은 역할)
    std::function<void(SMTableView* tableView)> onInitFillWithCells;
    
    // 일반 method
    bool isDeleteCell(cocos2d::Node* cell);
    virtual void onPositionCell(cocos2d::Node* cell, const float position, const bool isAdded);
    virtual void onPositionHeader(cocos2d::Node* headerView, const float position, const bool isAdded);
    virtual void onPositionFooter(cocos2d::Node* footerView, const float position, const bool isAdded);
    
    // virtual method from SMScroller
    virtual void setHeaderView(cocos2d::Node* node) override;
    virtual void setFooterView(cocos2d::Node* node) override;
    
    float getTotalHeightInSection(int section);
    
public:
    virtual void setContentSize(const cocos2d::Size& size) override;
    virtual bool isTouchEnable() const override {return true;}
    ssize_t getColumnCount() const {return getContainerCount();}
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    // page view에서
    bool jumpPage(const int pageNo, const float pageSize);
    virtual void setMinScrollSize(const float minScrollSize) override;
    void setScrollPosition(const float position);
    void enableAccelerateScroll(bool enable);
    void enableInitRefresh(bool enable);
    float getScrollSpeed();
    
    // 페이지 끝까지 scroll 해야 전체 사이즈를 알 수 있기 때문에 fake로 끝까지 가본것 처럼 한다.
    void fakeAdvanceLast(int index, float size);
    void fakeAdvanceLast2(int index, float size);
    
protected:
    SMTableView();
    virtual ~SMTableView();
    
    virtual bool initWithOrientAndColumns(Orientation orient, int numColumns);
    virtual void scheduleScrollUpdate();
    void unscheduleScrollUpdate();
    virtual void onUpdateOnVisit() override;
    virtual bool updateScrollInParentVisit(float& deltaScroll) override;
    virtual void notifyScrollUpdate() override;
    virtual SMScroller* initScroller();
    
    inline bool isVertical() const {return _orient==Orientation::VERTICAL;}
    inline bool isHorizontal() const {return _orient==Orientation::HORIZONTAL;}
    
    void initFixedColumnInfo(int numPages, float pageSize, int initPage);
    
private:
    // fill list
    virtual bool fillList(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, float footerSize);
    // fill backward
    virtual bool fillListBack(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, float footerSize);
    // fill forward
    virtual bool fillListFront(const ssize_t adapterItemCount, const float scrollPosition, const float containerSize, const float headerSize, float footerSize);
    // position childe view
    virtual void positionChildren(const float scrollPosition, const float containerSize, const float headerSize, const float footerSize);
    // view를 벗어난 cell를 화면에서 제거
    virtual void clippingChildren(const float scrollPosition, const float containerSize, const float headerSize, const float footerSize);
    // scroll size 계산
    virtual float measureScrollSize();
    void stopAndCompleteChildAction(int tag);
    virtual void scrollFling(const float velocity);
    
private:
    
    static const uint8_t ITEM_FLAG_DELETE;
    static const uint8_t ITEM_FLAG_RESIZE;
    static const uint8_t ITEM_FLAG_INSERT;
    
    class ReuseScrapper {
    private:
        int _numberOfTypes;
        
        std::map<std::string, int>_key;
        std::vector<cocos2d::Vector<cocos2d::Node*>>_data;
        
    public:
        int _internalReuseType;
        cocos2d::Node* _internalReuseNode;
        
    public:
        int getReuseType(const std::string& reuseIdentifier);
        void scrap(const int reuseType, cocos2d::Node* const parent, cocos2d::Node* const child, const bool cleanup = true);
        void popBack(const int reuseType);
        cocos2d::Node* back(const int reuseType);
        void clear();
        ReuseScrapper();
        ~ReuseScrapper();
    };
    
    class Item {
    public:
        IndexPath _indexPath;
        int _reuseType;
        int _tag;
        float _size, _newSize;
        int8_t _flags;
        bool _reload;
        bool _dontReuse;
    public:
        Item();
        Item(const IndexPath& indexPath, const int type, const float size);
        ~Item();
        
        Item& operator=(const Item&item);
        bool isDeleted() {return _flags & ITEM_FLAG_DELETE;}
    private:
        Intent* _state;
    };
    
    class Cursor {
    public:
        Cursor();
        Cursor& operator=(const Cursor &rhs);
        Cursor& operator++();
        Cursor& operator--();
        Cursor operator++(int);
        Cursor operator--(int);
        Cursor& advance(int offset);
 
        bool operator==(const Cursor& rhs) const;
        bool operator!=(const Cursor& rhs) const;
        bool operator>(const Cursor& rhs) const;
        bool operator<(const Cursor& rhs) const;
        bool operator>=(const Cursor& rhs) const;
        bool operator<=(const Cursor& rhs) const;
        int operator+(const Cursor& rhs) const;
        int operator-(const Cursor& rhs) const;

    public:
        void init(std::list<Item>& data);
        Item& getItem() { return *_iter; }
        IndexPath& getIndexPath() { return (*_iter)._indexPath; }

        float getLocation() { return _location; }
        float getLastLocation() { return _location + getItem()._size; }
        void offsetLocation(const float offset) { _location += offset; }

        int getPosition() { return _position; }
        void offsetPosition(const int offset) { _position += offset; }
        void setPosition(const int position) { _position = position; }

        void offset(const int position, const float location) { _position += position; _location += location; }
        
        std::list<Item>::iterator& getIterator() { return _iter; }
        void setIterator(std::list<Item>::iterator iter) { _iter = iter; }
        void incIterator() { ++_iter; }
        void decIterator() { --_iter; }
        void incPosition() { ++_position; }
        void decPosition() { --_position; }
        bool isBegin() const { return _data->begin() == _iter; }
        bool isEnd() const { return _data->end() == _iter; }
        
    private:
        std::list<Item>::iterator _iter;
        int _position;
        float _location;
        std::list<Item>* _data;
//        std::vector<Cursor*>* _buffer;
    };
    
    class ColumnInfo {
    public:
        ColumnInfo();
        virtual ~ColumnInfo();
        void init(SMTableView* parent, ssize_t column);
        Cursor advanceViewFirst();
        Cursor advanceViewLast(const IndexPath& indexPath, const int type, const float size, const int8_t flags = 0);

        Cursor retreatViewFirst();
        Cursor retreatViewLast();
        
        Cursor getFirstCursor();
        Cursor getLastCursor();
        Cursor getViewFirstCursor();
        Cursor getViewLastCursor();
        Cursor getFirstCursor(const int offset);
        Cursor getLastCursor(const int offset);
        Cursor getViewFirstCursor(const int offset);
        Cursor getViewLastCursor(const int offset);
        void rewindViewLastCursor();
        void setViewCursor(Cursor& cursor);
        void resizeCursor(Cursor& cursor);
        
        void markDeleteCursor(Cursor& cursor);
        void deleteCursor(Cursor& cursor);
        Cursor markInsertItem(const IndexPath& indexPath);
        Cursor insertItem(const IndexPath& indexPath, const int type, const float estimateSize);
        
        bool isAtFirst() { return _viewFirstCursor == _firstCursor; }
        bool isAtLast() { return _viewLastCursor == _lastCursor; }
        
        int getAliveItemCount() { return _numAliveItem; }
        IndexPath& getLastIndexPath() { return _lastIndexPath; }

        Cursor* obtainCursorBuffer(Cursor& cursor);
        void recycleCursorBuffer(Cursor* cursor);
        void resizeCursorBuffer(Cursor& targetCursor, const float deltaSize);
        void deleteCursorBuffer(Cursor& targetCursor);
        void insertCursorBuffer(Cursor& targetCursor);
    public:
        std::list<Item> _data;
        Cursor _firstCursor;
        Cursor _lastCursor;
        Cursor _viewFirstCursor;
        Cursor _viewLastCursor;
        int _numAliveItem;
        IndexPath _lastIndexPath;
        ssize_t _column;
        int _resizeReserveSize;
    private:
        SMTableView* _parent;
        std::vector<Cursor*> _buffer;
    };
    
    class _DeletedNode : public cocos2d::Node {
    public:
        static _DeletedNode* create();
    };
    
    class _BaseAction : public cocos2d::ActionInterval {
    protected:
        cocos2d::Node* getChild();
        virtual void startWithTarget(cocos2d::Node *target) override;
    public:
        virtual void complete() = 0;
    protected:
        int _col;
        float _startSize;
        Cursor* _cursor;
        SMTableView* _parent;
    };
    
    class _DeleteAction : public SMTableView::_BaseAction {
    public:
        static _DeleteAction* create(SMTableView* parent, int column, Cursor& cursor);
        virtual void update(float t) override;
        virtual void complete() override;
    };
    
    class _ResizeAction : public SMTableView::_BaseAction {
    public:
        static _ResizeAction* create(SMTableView* parent, int column, Cursor& cursor, float newSize);
        cocos2d::Node* updateResize(float t);
        virtual void update(float t) override;
        virtual void complete() override;
    protected:
        float _newSize;
        bool _insert;
    };
    
    class _InsertAction : public SMTableView::_ResizeAction {
    public:
        static _InsertAction* create(SMTableView* parent, int column, Cursor& cursor, float newSize);
    };
    
    class _DelaySequence : public cocos2d::Sequence {
    public:
        static _DelaySequence* create(float delay, _BaseAction* action);
        _BaseAction* getAction();
    private:
        _BaseAction* _action;
        _DelaySequence() {}
        virtual ~_DelaySequence() {}
    };

    class _PageJumpAction : public SMTableView::_BaseAction {
    public:
        static _PageJumpAction* create(SMTableView* parent, Cursor& cursor, float pageSize, int fromPage, int toPage, int direction);
        virtual void update(float t) override;
        virtual void complete() override;
    private:
        float _pageSize;
        int _fromPage;
        int _toPage;
        int _direction;
    };
    


    ColumnInfo* _column;
    Orientation _orient;
    ssize_t _lastItemCount;
    float _firstMargin;
    float _lastMargin;
    float _preloadPadding;
    float _lastScrollPosition;
    float _hintFixedChildSize;
    bool _hintIsFixedSize;
    bool _justAtLast;
    bool _canExactScrollSize;
    int _internalActionTag;
    ReuseScrapper* _reuseScrapper;
    cocos2d::Node* _refreshView;
    float _refreshTriggerSize, _refreshMinSize;
    float _refreshSize, _lastRefreshSize;
    RefreshState _refreshState, _lastRefreshState;
    bool _progressLoading;
    bool _animationDirty;
    float _firstMotionTime;
    float _lastMotionX;
    float _lastMotionY;
    float _deltaScroll;
    bool _needUpdate;
    bool _skipUpdateOnVisit;
    bool _forceJumpPage;
    bool _touchFocused;
    bool _fillWithCellsFirstTime;
    float _maxVelocicy;
    bool _accelScrollEnable;
    float _lastVelocityX;
    float _lastVelocityY;
    float _lastFlingTime;
    int _accelCount;
    bool _initRefreshEnable;
    bool _reloadExceptHeader;
    std::function<bool(cocos2d::Node* node)> onLoadDataCallbackTemp;
    
protected:
    int _currentPage;
    virtual void onScrollChanged(float position, float distance);

private:
    bool findCursorForIndexPath(IndexPath& indexPath, Cursor& outCursor);
    bool findCursorForChild(cocos2d::Node* child, Cursor& outCursor);
    cocos2d::Node* findChildForIndexPath(IndexPath& indexPath, Cursor& outCursor);
    bool performDelete(Cursor& cursor, cocos2d::Node* child, float duration, float delay, bool cleanup = true);
    bool performResize(Cursor& cursor, cocos2d::Node* child, const float newSize, float duration, float delay);
    void deleteCursor(const int column, Cursor cursor, bool cleanup = true);
    void removeChildAndReuseScrap(const ssize_t container, const int reuseType, cocos2d::Node* child, const bool cleanup);
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(SMTableView);
    
    friend class _ScrollProtocol;
    
};


#endif /* SMTableView_h */
