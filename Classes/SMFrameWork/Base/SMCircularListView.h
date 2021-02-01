//
//  SMCircularListView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 11..
//
//

#ifndef SMCircularListView_h
#define SMCircularListView_h

#include "SMView.h"
#include <string>
#include <vector>

class SMScroller;
class SMCircularListView;
class VelocityTracker;

class CellProtocol
{
public:
    CellProtocol() : _deleted(false) {}
    int getCellIndex() {return _index;}
    
private:
    virtual float getCellPosition() {return _position;}
    const std::string& getCellIdentifier() {return _reuseIdentifier;}
    void markDelete() {_deleted = true;}
    void setCellIndex(const int index) {_index = index;}
    virtual void setCellPosition(const float position) {_position = position;}
    void setReuseIdentifier(const std::string& identifier) {_reuseIdentifier = identifier;}
    
private:
    int _index;
    bool _deleted;
    float _position;
    float _aniSrc, _aniDst;
    int _aniIndex;
    
    std::string _reuseIdentifier;
    friend class SMCircularListView;
};


class SMCircularListView : public SMView
{
public:
    enum Orientation {
        VERTICAL,
        HORIZONTAL
    };
    
    enum ScrollMode {
        BASIC = 0,
        PAGER,
        ALIGNED
    };
    
    struct Config {
        Orientation orient;
        bool circular;
        float cellSize;
        float windowSize;
        float anchorPosition;
        float preloadPadding;
        float maxVelocity;
        float minVelocity;
        ScrollMode scrollMode;
        Config() : orient(Orientation::HORIZONTAL), circular(true), anchorPosition(0), preloadPadding(0), maxVelocity(5000), minVelocity(0), scrollMode(ScrollMode::BASIC) {};
    };
    
    static SMCircularListView* create(const Config& config);
    
    cocos2d::Node* dequeueReusableCellWithIdentifier(const std::string& identifier);
    std::function<cocos2d::Node*(const int index)> cellForRowAtIndex;
    
    int getIndexForCell(cocos2d::Node* cell);

    std::function<void(cocos2d::Node* cell, float position, bool created)> positionCell;
    std::function<void()> initFillWithCells;
    std::function<void(float pagePosition)> pageScrollCallback;
    
    bool deleteCell(cocos2d::Node* target, float deleteDt, float deleteDelay, float positionDt, float positionDelay);
    bool deleteCell(int targetIndex, float deleteDt, float deleteDelay, float positionDt, float positionDelay);
    
    std::function<int(void)> numberOfRows;
    std::function<void(bool aligned, int index, bool force)> scrollAlignedCallback;
    std::function<void(cocos2d::Node* cell, float dt)> cellDeleteUpdate;
    
    cocos2d::Vector<cocos2d::Node*>& getVisibleCells();
    
    void stop(bool align=true);
    
    void setScrollPosition(float position);
    void scrollByWithDuration(float distance, float dt);
    void runFling(float velocity);
    int getAlignedIndex();
    void updateData();
    
public:
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
protected:
    SMCircularListView();
    virtual ~SMCircularListView();
    
    bool initWithConfig(const Config& config);
    void scheduleScrollUpdate();
    void unscheduleScrollUpdate();
    virtual void onUpdateOnVisit() override;
    inline bool isVertical() const {return _config.orient==Orientation::VERTICAL;}
    inline bool isHorizontal() const {return _config.orient==Orientation::HORIZONTAL;}
    
    int convertToIndex(int reallIndex, int numRows);
    void onCellAction(int tag, std::vector<cocos2d::Node*> cells, float dt, bool complete);
    
    virtual void onScrollAligned(bool aligned);
    
    float getLIstAnchorX() {return _config.anchorPosition;}
    
private:
    void positionChildren(float scrollPosition, int numRows);
    
    static bool sortFunc(CellProtocol* l, CellProtocol* r);
    
private:
    class CellsAction;
    class ReuseScrapper;
    ReuseScrapper* _reuseScrapper;
    VelocityTracker* _velocityTracker;
    SMScroller* _scroller;
    Config _config;
    
    float _lastScrollPosition;
    float _lastMotionX;
    float _lastMotionY;
    float _deltaScroll;
    int _deleteIndex;
    bool _inScrollEvent;
    bool _touchFocused;
    bool _needUpdate;
    bool _actionLock;
    bool _fillWithCellsFirstTime;
    int _currentPage;
    
    
    
};










#endif /* SMCircularListView_h */
