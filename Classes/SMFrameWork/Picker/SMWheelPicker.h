//
//  SMWheelPicker.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 15..
//

#ifndef SMWheelPicker_h
#define SMWheelPicker_h

#include "../Base/SMView.h"
#include <string>
#include <vector>
#include <cocos2d.h>

class SMCircularListView;

class OnPickerListener {
public:
    virtual void onPickerSelected(int tag, int index) = 0;
};

class SMWheelPicker : public SMView, public OnClickListener
{
public:
    static SMWheelPicker* open(int tag, const std::string& title, const std::vector<std::string>& items, int currentIndex=0, OnPickerListener* l=nullptr);
    
    static void close();
    
    //    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
    
protected:
    SMWheelPicker();
    
    virtual ~SMWheelPicker();
    
    bool initWithItems(const std::string& title, const std::vector<std::string>& items, int currentIndex, OnPickerListener* l);
    
    virtual void onClick(SMView* view) override;
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
private:
    void onCloseStart();
    
    void onCloseComplete(float dt);
    
    void onScrollAligned(bool aligned, int index, bool force);
    
    void onInitFillWithCells();
    
    void onPositionCell(cocos2d::Node* cell, float position);
    
    cocos2d::Node* getView(const int index);
    
    int getItemCount();
    
    
private:
    class ItemCell;
    
    SMView* _panel;
    
    SMCircularListView* _listView;
    
    OnPickerListener* _listener;
    
    int _currentIndex;
    
    bool _isFinishing;
    
    std::vector<std::string> _items;
};



#endif /* SMWheelPicker_h */
