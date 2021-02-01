//
//  SMWheelPicker.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 15..
//

#include "SMWheelPicker.h"
#include "../Base/SMCircularListView.h"
#include "../Base/SMButton.h"
#include "../Util/ViewUtil.h"
#include "../Const/SMFontColor.h"
#include "../Base/ViewAction.h"
#include <string>

#define PANEL_HEIGHT    (588.0)

#define ACTION_TAG_SHOW (100)
#define CELL_SIZE       (140.0)
#define ANCHOR_POSITION  (172)
#define TITLE_HEIGHT    (40+52+40)
#define PADDING  (CELL_SIZE)
class SMWheelPicker::ItemCell : public SMButton, public CellProtocol {
public:
    CREATE_VIEW(ItemCell);
    
protected:
    virtual bool init() override {
        SMButton::initWithStyle(SMButton::Style::DEFAULT);
        auto s = _director->getWinSize();
        setContentSize(cocos2d::Size(s.width, CELL_SIZE));
        
        setTextSystemFont("", SMFontConst::SystemFontRegular, 48);
        setTextColor(SMView::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
        setTextColor(SMView::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
        
        setPushDownScale(0.97);
        
        return true;
    }
    
protected:
    ItemCell() : _text(nullptr) {
    }
    
    virtual ~ItemCell() {
    }
    
private:
    cocos2d::Label* _text;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Contexgt Wheel Picker Class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
SMWheelPicker::SMWheelPicker() :
_listener(nullptr),
_isFinishing(false)
{
}

SMWheelPicker::~SMWheelPicker()
{
}

SMWheelPicker* SMWheelPicker::open(int tag, const std::string& title, const std::vector<std::string>& items, int currentIndex, OnPickerListener* l) {
    
    auto viewStub = cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    auto children = viewStub->getChildren();
    
    for (auto child : children) {
        auto layer = dynamic_cast<SMWheelPicker*>(child);
        if (layer) {
            return nullptr;
        }
    }
    
    SMWheelPicker* view = new (std::nothrow)SMWheelPicker();
    
    if (view != nullptr) {
        if (view->initWithItems(title, items, currentIndex, l)) {
            view->setTag(tag);
            view->autorelease();
            viewStub->addChild(view);
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

void SMWheelPicker::close() {
    auto viewStub = cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
    auto children = viewStub->getChildren();
    
    for (auto child : children) {
        auto layer = dynamic_cast<SMWheelPicker*>(child);
        if (layer) {
            layer->onCloseStart();
            break;
        }
    }
}

void SMWheelPicker::onCloseStart() {
    if (_isFinishing)
        return;
    
    _isFinishing = true;
    
    auto a = _panel->getActionByTag(ACTION_TAG_SHOW);
    if (a) {
        _panel->stopAction(a);
    }
    
    auto action = ViewAction::TransformAction::create();
    action->toPosition(cocos2d::Vec2(0, -PANEL_HEIGHT)).setTweenFunc(cocos2d::tweenfunc::Cubic_EaseOut);
    action->setTimeValue(0.20, 0);
    _panel->runAction(action);
    
    setBackgroundColor4F(MAKE_COLOR4F(0, 0.0), 0.25);
    schedule(schedule_selector(SMWheelPicker::onCloseComplete), 0.3);
}

void SMWheelPicker::onCloseComplete(float dt) {
    // 부모에서 제거
    removeFromParent();
}

bool SMWheelPicker::initWithItems(const std::string& title, const std::vector<std::string>& items, int currentIndex, OnPickerListener* l) {
    if (!SMView::init())
        return false;
    
    _items = items;
    _listener = l;
    
    if (currentIndex >= getItemCount()) {
        currentIndex = getItemCount()-1;
    }
    if (currentIndex < 0) {
        currentIndex = 0;
    }
    _currentIndex = currentIndex;
    
    auto s = _director->getWinSize();
    setContentSize(s);
    
    // 패널
    _panel = SMView::create();
    _panel->setContentSize(cocos2d::Size(s.width, PANEL_HEIGHT));
    _panel->setPositionY(-PANEL_HEIGHT);
    _panel->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    //    _panel->setScissorEnable(true);
    addChild(_panel);
    
    // 리스트뷰
    SMCircularListView::Config config;
    config.orient = SMCircularListView::Orientation::VERTICAL;
    config.circular = false;
    config.scrollMode = SMCircularListView::ScrollMode::ALIGNED;
    config.cellSize = CELL_SIZE;
    config.windowSize = (PANEL_HEIGHT+PADDING);
    config.anchorPosition = ANCHOR_POSITION;
    config.maxVelocity = 2000;
    
    _listView = SMCircularListView::create(config);
    _listView->setContentSize(cocos2d::Size(_contentSize.width, PANEL_HEIGHT+PADDING));
    _listView->setAnchorPoint(cocos2d::Vec2(0.5, (PADDING+ANCHOR_POSITION)/(PANEL_HEIGHT+PADDING)));
    _listView->setPosition(_contentSize.width/2, ANCHOR_POSITION);
    _listView->cellForRowAtIndex = CC_CALLBACK_1(SMWheelPicker::getView, this);
    _listView->numberOfRows = CC_CALLBACK_0(SMWheelPicker::getItemCount, this);
    _listView->positionCell = CC_CALLBACK_2(SMWheelPicker::onPositionCell, this);
    _listView->initFillWithCells = CC_CALLBACK_0(SMWheelPicker::onInitFillWithCells, this);
    _listView->scrollAlignedCallback = CC_CALLBACK_3(SMWheelPicker::onScrollAligned, this);
    _listView->setScrollPosition(CELL_SIZE*_currentIndex);
    _panel->addChild(_listView);
    
    // 상단 Gradient
    auto gradientT = ShapeGradientRect::create();
    gradientT->setContentSize(cocos2d::Size(_contentSize.width, 41));
    gradientT->setPositionY(PANEL_HEIGHT-ANCHOR_POSITION+40);
    gradientT->setColor4F(cocos2d::Color4F::WHITE);
    gradientT->setScaleY(-1);
    _panel->addChild(gradientT);
    
    // 하단 Gradient
    auto gradientB = ShapeGradientRect::create();
    gradientB->setContentSize(cocos2d::Size(_contentSize.width, PANEL_HEIGHT-(ANCHOR_POSITION+CELL_SIZE)));
    gradientB->setColor4F(MAKE_COLOR4F(0xFFFFFF, .95));
    _panel->addChild(gradientB);
    
    auto r2 = ShapeSolidRect::create();
    r2->setContentSize(cocos2d::Size(_contentSize.width, PANEL_HEIGHT-(ANCHOR_POSITION+CELL_SIZE)));
    r2->setColor4F(MAKE_COLOR4F(0xFFFFFF, 0.5));
    _panel->addChild(r2);
    
    // 타이틀바 영역 덮개
    auto rect = ShapeSolidRect::create();
    rect->setContentSize(cocos2d::Size(_contentSize.width, TITLE_HEIGHT));
    rect->setPositionY(PANEL_HEIGHT-TITLE_HEIGHT);
    _panel->addChild(rect);
    
    // 타이틑
    auto titleLabel = cocos2d::Label::createWithSystemFont(title, SMFontConst::SystemFontRegular, 40);
    titleLabel->setColor(MAKE_COLOR3B(0x222222));
    titleLabel->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_LEFT);
    titleLabel->setPosition(40, PANEL_HEIGHT-TITLE_HEIGHT/2);
    _panel->addChild(titleLabel);
    
    // 완료 버튼
    auto button = SMButton::create(-1, SMButton::Style::DEFAULT);
    button->setTextSystemFont("완료", SMFontConst::SystemFontRegular, 38);
    auto size = button->getTextLabel()->getContentSize();
    button->setContentSize(cocos2d::Size(size.width+80, 120));
    button->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    size = button->getContentSize();
    button->setPosition(_contentSize.width-size.width/2, PANEL_HEIGHT-TITLE_HEIGHT/2);
    button->setOnClickListener(this);
    button->setTextColor(SMView::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    button->setTextColor(SMView::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
    button->setPushDownScale(0.95);
    _panel->addChild(button);
    
    // 1px line
    auto line = ShapeSolidRect::create();
    line->setColor4F(MAKE_COLOR4F(0xdbdcdf, 1.0f));
    line->setContentSize(cocos2d::Size(_contentSize.width, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH));
    line->setPositionY(PANEL_HEIGHT-ANCHOR_POSITION);
    _panel->addChild(line);
    
    line = ShapeSolidRect::create();
    line->setColor4F(MAKE_COLOR4F(0xdbdcdf, 1.0f));
    line->setContentSize(cocos2d::Size(_contentSize.width, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH));
    line->setPositionY(PANEL_HEIGHT-(ANCHOR_POSITION+CELL_SIZE));
    _panel->addChild(line);
    
    
    // show action
    auto action = ViewAction::TransformAction::create();
    action->setTag(ACTION_TAG_SHOW);
    action->toPosition(cocos2d::Vec2(0, 0)).setTweenFunc(cocos2d::tweenfunc::Cubic_EaseOut).enableOnFinish();
    action->setTimeValue(0.25, 0);
    _panel->runAction(action);
    
    setBackgroundColor4F(MAKE_COLOR4F(0, 0.65), 0.3);
    
    return true;
}

void SMWheelPicker::onInitFillWithCells() {
    onScrollAligned(true, _currentIndex, true);
}


void SMWheelPicker::onScrollAligned(bool aligned, int index, bool force) {
    if (aligned) {
        _currentIndex = index;
    }
    
    auto cells = _listView->getVisibleCells();
    for (auto cell : cells) {
        auto protocol = dynamic_cast<CellProtocol*>(cell);
        if (protocol->getCellIndex() == _currentIndex) {
            ((ItemCell*)cell)->setEnabled(aligned);
        } else {
            ((ItemCell*)cell)->setEnabled(false);
        }
    }
}

void SMWheelPicker::onPositionCell(cocos2d::Node* cell, float position) {
    auto view = dynamic_cast<ItemCell*>(cell);
    if (view == nullptr)
        return;
    position = (position-ANCHOR_POSITION)/2 + ANCHOR_POSITION;
    
    auto size = view->getContentSize();
    
    float dist = position - ANCHOR_POSITION;
    float t = dist / CELL_SIZE;
    
    float at = std::max(-3.0f, std::min(t, 3.0f));
    float sa = std::sin((at/3) * M_PI);
    float ca = 1.0 - std::cos((at/3) * M_PI);
    
    float offset = 80 * sa;
    float scale = 1.0 - 0.4 * std::abs(ca);
    
    float xx = size.width/2;
    float yy = (PANEL_HEIGHT+PADDING)-(position+CELL_SIZE/2+offset);
    
    view->setPosition(xx, yy);
    view->setScaleY(scale);
}

cocos2d::Node* SMWheelPicker::getView(const int index) {
    
    ItemCell* cell = (ItemCell*)_listView->dequeueReusableCellWithIdentifier("CELL");
    
    if (cell == nullptr) {
        cell = ItemCell::create();
        cell->setContentSize(cocos2d::Size(_contentSize.width, CELL_SIZE));
        cell->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        cell->setOnClickListener(this);
    }
    cell->setText(_items.at(index));
    
    return cell;
}

int SMWheelPicker::getItemCount() {
    return (int)_items.size();
}


void SMWheelPicker::onClick(SMView* view) {
    if (_listener) {
        _listener->onPickerSelected(getTag(), _currentIndex);
    }
    onCloseStart();
}

int SMWheelPicker::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    if (_isFinishing)
        return TOUCH_TRUE;
    
    int ret = SMView::dispatchTouchEvent(action, touch, point, event);
    if (ret == TOUCH_FALSE && action == MotionEvent::ACTION_DOWN) {
        auto pt = _panel->convertToNodeSpace(*point);
        if (!_panel->containsPoint(pt)) {
            onCloseStart();
            if (_listener) {
                _listener->onPickerSelected(getTag(), -1);
            }
        }
    }
    
    return TOUCH_TRUE;
}





