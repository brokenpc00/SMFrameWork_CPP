//
//  DropDownView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 13..
//

#ifndef DropDownView_h
#define DropDownView_h

#include "../Base/SMView.h"
#include "../Base/ShaderNode.h"
#include "../Base/SMTableView.h"
#include "../Base/SMImageView.h"
#include "../Base/SMButton.h"
#include "../Util/ViewUtil.h"
#include "../Const/SMFontColor.h"


class DropDownItem
{
public:
    DropDownItem() : index(-1), title(""), iconPath(""), cornerRadius(0), _textColor_N(MAKE_COLOR4F(0x222222, 1.0f)), _textColor_P(MAKE_COLOR4F(0x222222, 1.0f))
    , _backColor_N(MAKE_COLOR4F(0xffffff, 1.0f)), _backColor_P(MAKE_COLOR4F(0xffffff, 1.0f)), _lineColor_N(MAKE_COLOR4F(0x222222, 1.0f)), _lineColor_P(MAKE_COLOR4F(0x222222, 1.0f))
    , fontSize(50), lineWidth(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2.0f)
    {
        
    }
    
    virtual ~DropDownItem(){}
    
    int index;
    std::string title;
    int fontSize;
    std::string iconPath;
    float cornerRadius;
    float lineWidth;
    
    cocos2d::Color4F _textColor_N;
    cocos2d::Color4F _textColor_P;
    cocos2d::Color4F _backColor_N;
    cocos2d::Color4F _backColor_P;
    cocos2d::Color4F _lineColor_N;
    cocos2d::Color4F _lineColor_P;

    int nValue;
    float fValue;
    std::string strValue;
    
    std::vector<int> nValues;
    std::vector<float> fValues;
    std::vector<std::string> strValues;
};

class DropDownConfig
{
public:
    cocos2d::Color4F _backColor;
    float _itemWidth;
    float _itemHeight;
    cocos2d::Vec2 _startPos;
    std::vector<DropDownItem> _items;
};


class DropDownView;

class DropDownListener
{
public:
    virtual void OnDropdownSelected(DropDownView * view, DropDownItem * item) = 0;
};

class DropDownView : public SMView
{
public:
    static void openDropDown(DropDownConfig& config, DropDownListener * l);
    static void close();
    static bool isOpen();
    static void setDropDownListener(DropDownListener* l);
    
protected:
    DropDownView();
    virtual ~DropDownView();
    
    virtual bool init() override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
//    class ItemCell : public SMView, public OnClickListener
//    {
//    public:
//        CREATE_VIEW(ItemCell);
//        ItemCell(){}
//        ~ItemCell(){}
//        
//        virtual void onClick(SMView* view) override;
//        virtual bool init() override;
//        
//        SMView * _contentView;
//        
//    };
    
    cocos2d::Node * cellForRowsAtIndexPath(const IndexPath & indexPath);
    
    static DropDownView * getInstance();
    
    static DropDownView * findByID(int tag);
    
private:
    SMView * _contentView;
    DropDownConfig _config;
    SMTableView * _itemListView;
    DropDownListener* _listener;
};


#endif /* DropDownView_h */
