//
//  DropDownView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 13..
//

#include "DropDownView.h"
#include "../Popup/PopupManager.h"

#define DROP_DOWN_VIEW    (0xFFFFE0)

DropDownView::DropDownView() : _contentView(nullptr)
, _itemListView(nullptr)
, _listener(nullptr)
{
    
}

DropDownView::~DropDownView()
{
    
}

DropDownView * DropDownView::getInstance()
{
    static DropDownView * instance = nullptr;
    if (instance==nullptr) {
        instance = new (std::nothrow)DropDownView();
    }
    
    return instance;
}

void DropDownView::openDropDown(DropDownConfig &config, DropDownListener *l)
{
    DropDownView * view = DropDownView::findByID(DROP_DOWN_VIEW);
    if (view==nullptr) {
        view = DropDownView::getInstance();
    }

    std::sort(config._items.begin(), config._items.end(), [&](DropDownItem& a, DropDownItem& b)->bool{
        return a.index < b.index;
    });

    view->_config = config;
    view->_listener = l;
    if(view->init()) {
        auto bg = cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
        view->setTag(DROP_DOWN_VIEW);
        bg->addChild(view);
    }
}

DropDownView * DropDownView::findByID(int tag)
{
    auto director = cocos2d::Director::getInstance();
    auto layer = director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);

    DropDownView * view = nullptr;
    auto children = layer->getChildren();
    for (auto child : children) {
        view = dynamic_cast<DropDownView*>(child);
        if (view && view->getTag()==tag) {
            return view;
        }
    }
    
    return view;
}

void DropDownView::close()
{
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
        DropDownView * view = DropDownView::findByID(DROP_DOWN_VIEW);
        view->removeFromParent();
        view = nullptr;
    });

}

bool DropDownView::isOpen()
{
    return DropDownView::findByID(DROP_DOWN_VIEW)!=nullptr;
}

void DropDownView::setDropDownListener(DropDownListener *l)
{
    DropDownView::getInstance()->_listener = l;
}

bool DropDownView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setBackgroundColor4F(MAKE_COLOR4F(0x222222, 0.3f));
    
    auto s = _director->getWinSize();
    setPosition(cocos2d::Vec2::ZERO);
    setContentSize(s);
    
    float itemHeight = _config._itemHeight;
    if (itemHeight==0) {
        return false;
    }
    
    float itemCount = _config._items.size();
    if (itemHeight==0) {
        return false;
    }
    
    float viewHeight = itemHeight*itemCount;
    
    float viewWidth = _config._itemWidth;
    if (viewWidth==0) {
        return false;
    }
    
    auto pos = _config._startPos;
    pos.y -= viewHeight;
    
    if (_itemListView!=nullptr) {
        _itemListView->removeFromParent();
        _itemListView = nullptr;
    }

    if (_contentView!=nullptr) {
        _contentView->removeFromParent();
        _contentView = nullptr;
    }
    
    _contentView = SMView::create(0, pos.x, pos.y, viewWidth, viewHeight);
    _contentView->setBackgroundColor4F(_config._backColor);
    addChild(_contentView);
    
    _contentView->setOnClickCallback([this](SMView * view){
        DropDownView::close();
    });
    
    
    _itemListView = SMTableView::createMultiColumn(SMTableView::Orientation::VERTICAL, 1, 0, 0, _contentView->getContentSize().width, _contentView->getContentSize().height);
    _contentView->addChild(_itemListView);
    
    _itemListView->cellForRowAtIndexPath = [this] (const IndexPath& indexPath) -> cocos2d::Node *{
        int index = indexPath.getIndex();
        std::string cellID = cocos2d::StringUtils::format("CELLID%d", index);
        cocos2d::Node * convertView = _itemListView->dequeueReusableCellWithIdentifier(cellID);
        SMButton * cell = nullptr;
        DropDownItem item = _config._items[index];
        if (convertView) {
            cell = (SMButton*)convertView;
        } else {
            cell = SMButton::create(index, SMButton::Style::SOLID_ROUNDEDRECT, 0, 0, _config._itemWidth, _config._itemHeight);
            cell->setButtonColor(SMButton::State::NORMAL, item._backColor_N);
            cell->setButtonColor(SMButton::State::PRESSED, item._backColor_P);
            cell->setOutlineColor(SMButton::State::NORMAL, item._lineColor_N);
            cell->setOutlineColor(SMButton::State::PRESSED, item._lineColor_P);
            cell->setTextSystemFont(item.title, SMFontConst::SystemFontRegular, item.fontSize);
            cell->setTextColor(SMButton::State::NORMAL, item._textColor_N);
            cell->setTextColor(SMButton::State::PRESSED, item._textColor_P);
            cell->setShapeCornerRadius(item.cornerRadius);
            cell->setOutlineWidth(item.lineWidth);
            cell->setOnClickCallback([this](SMView * view){
                int index = view->getTag();
                if (_listener) {
                    DropDownItem item = _config._items[index];
                    _listener->OnDropdownSelected(this, &item);
                }
                DropDownView::close();
            });
        }
        
        cell->setTag(index);
        
        return cell;
    };
    
    _itemListView->numberOfRowsInSection = [&] (int section) -> int {
        return (int)_config._items.size();
    };
    
    return true;
}

int DropDownView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    int ret = SMView::dispatchTouchEvent(action, touch, point, event);
    const SMView * target = getMotionTarget();
    
    if (target!=nullptr && target==_contentView) {
        
        //        CCLOG("[[[[[ content view");
        return TOUCH_TRUE;
    }
    
    return TOUCH_INTERCEPT;
}
