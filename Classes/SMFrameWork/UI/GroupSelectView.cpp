//
//  GroupSelectView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "GroupSelectView.h"
#include "ImagePickerView.h"
#include "../Util/ViewUtil.h"
#include "../Const/SMFontColor.h"
#include "../Base/SMImageView.h"
#include "../Const/SMViewConstValue.h"
#include <string>
#include <sstream>


template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

#define CELL_DEFAULT_SIZE 210

#define CELL_EXTEND_SIZE (CELL_DEFAULT_SIZE+60)

#define TOP_HEIGHT SMViewConstValue::Size::TOP_MENU_HEIGHT

static std::string numberToCommaString(int value)
{
    std::string str = to_string(value);
    
    int p = (int)str.length() - 3;
    
    while (p > 0) {
        str.insert(p, ",");
        p -= 3;
    }
    
    return str;
}

class GroupCell : public SMView
{
public:
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    static GroupCell * create() {
        auto view = new (std::nothrow)GroupCell();
        if (view!=nullptr) {
            if (view->init()) {
                view->autorelease();
            } else {
                CC_SAFE_DELETE(view);
            }
        }
        return view;
    }
    
    virtual bool init() override {    
    #else
    static GroupCell * create(ImageFetcherHost* fetcher) {
        auto view = new (std::nothrow)GroupCell();
        if (view!=nullptr) {
            if (view->initWithImageFetcher(fetcher)) {
                view->autorelease();
            } else {
                CC_SAFE_DELETE(view);
            }
        }
        return view;
    }
    
    bool initWithImageFetcher(ImageFetcherHost* fetcher) {
        _fetcher = fetcher;
    #endif
        auto s = _director->getWinSize();
        setContentSize(cocos2d::Size(s.width, CELL_EXTEND_SIZE));
        setBackgroundColor4F(MAKE_COLOR4F(0xeeeff1, 0.0f));
        _image = SMImageView::create();
        _image->setContentSize(cocos2d::Size(CELL_DEFAULT_SIZE, CELL_DEFAULT_SIZE));
        _image->setPosition(40, 30);
        addChild(_image);

        auto arrow = cocos2d::Sprite::create("images/list_arrow.png");
        arrow->setColor(SMColorConst::COLOR_B_ADAFB3);
        arrow->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_RIGHT);
        arrow->setPosition(s.width-40, CELL_EXTEND_SIZE/2);
        addChild(arrow);

        _name = nullptr;
        _count = nullptr;
        return true;
    }
    
    virtual void onStateChangeNormalToPress() override {
        setBackgroundColor4F(SMColorConst::COLOR_F_EEEFF1, 0.05f);
    }
    
    virtual void onStateChangePressToNormal() override {
        setBackgroundColor4F(SMColorConst::COLOR_F_EEEFF1, 0.1f);
    }
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    void setGroupItem(int groupIndex) {
        auto info = _fetcher->getGroupInfo(groupIndex);
        if (!_name) {
            _name = cocos2d::Label::createWithSystemFont(info->name, SMFontConst::SystemFontRegular, 40);
            _name->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
            _name->setPosition(CELL_DEFAULT_SIZE+40+50, CELL_EXTEND_SIZE/2+15);
            _name->setColor(SMColorConst::COLOR_B_222222);
            addChild(_name);
        } else {
            _name->setString(info->name);
        }
        if (!_count) {
            _count = cocos2d::Label::createWithSystemFont(numberToCommaString(info->numPhotos), SMFontConst::SystemFontLight, 34);
            _count->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
            _count->setPosition(CELL_DEFAULT_SIZE+40+50, CELL_EXTEND_SIZE/2-15);
            _count->setColor(MAKE_COLOR3B(0x494949));
            addChild(_count);
        } else {
            _count->setString(numberToCommaString(info->numPhotos));
        }

        auto posterAsset = _fetcher->getAlbumPosterAssetObject(groupIndex);
        if (posterAsset) {
            _image->setPHAssetObject(posterAsset, true);
        } else {
            _image->setSprite(nullptr);
        }
    }
    
    const ImageGroupInfo& getGroupInfo() {
        return *_info;
    }
    #else
    void setGroupItem(const ImageGroupInfo& info) {
        if (!_name) {
            _name = cocos2d::Label::createWithSystemFont(info.name, SMFontConst::SystemFontRegular, 40);
            _name->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
            _name->setPosition(CELL_DEFAULT_SIZE+40+50, CELL_EXTEND_SIZE/2+15);
            _name->setColor(SMColorConst::COLOR_B_222222);
            addChild(_name);
        } else {
            _name->setString(info.name);
        }
        if (!_count) {
            _count = cocos2d::Label::createWithSystemFont(numberToCommaString(info.numPhotos), SMFontConst::SystemFontLight, 34);
            _count->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
            _count->setPosition(CELL_DEFAULT_SIZE+40+50, CELL_EXTEND_SIZE/2-15);
            _count->setColor(MAKE_COLOR3B(0x494949));
            addChild(_count);
        } else {
            _count->setString(numberToCommaString(info.numPhotos));
        }
        if (!info.thumb.url.empty()) {
            _image->setImageFilePath(info.thumb.url, true);
        } else {
            _image->setSprite(nullptr);
        }
        _info = info;
    }
    
    const ImageGroupInfo& getGroupInfo() {
        return _info;
    }    
    #endif
    
private:
    SMImageView * _image;
    cocos2d::Label * _name;
    cocos2d::Label * _count;
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    ImageGroupInfo * _info;
    ImageFetcherHost* _fetcher;
#else
    ImageGroupInfo _info;
#endif
};


GroupSelectView::GroupSelectView() : _listener(nullptr)
{

}

GroupSelectView::~GroupSelectView()
{
    
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

GroupSelectView* GroupSelectView::create(ImageFetcherHost* fetcher)
{
    GroupSelectView * view = new (std::nothrow)GroupSelectView();
    if (view!=nullptr) {
        if (view->initWithGroupInfo(fetcher)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    return view;
}

bool GroupSelectView::initWithGroupInfo(ImageFetcherHost* fetcher)
{
    if (!SMView::init()) {
        return false;
    }
    auto s = _director->getWinSize();
    
    setContentSize(cocos2d::Size(s.width, s.height-TOP_HEIGHT));
    setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _fetcher = fetcher;
    // table view init
    _tableView = SMTableView::createMultiColumn(SMTableView::Orientation::VERTICAL, 1, 0, 0, _contentSize.width, _contentSize.height);
    _tableView->setScrollMarginSize(10, 10);
    _tableView->setPreloadPaddingSize(300);
    _tableView->hintFixedCellSize(CELL_EXTEND_SIZE);
    _tableView->cellForRowAtIndexPath = CC_CALLBACK_1(GroupSelectView::cellForRowAtIndexPath, this);
    _tableView->numberOfRowsInSection = CC_CALLBACK_1(GroupSelectView::numberOfRowsInSection, this);
    _tableView->setScissorEnable(true);
    
    addChild(_tableView);
    setScissorEnable(true);

    return true;
}


void GroupSelectView::setOnGroupSelectListener(OnGroupSelectListener *l)
{
    _listener = l;
}

void GroupSelectView::onClick(SMView *view)
{
    if (_listener) {
        _listener->onGroupSelect(view->getTag());
    }
}

//void GroupSelectView::onImageGroupThumbResult(const std::vector<ImageGroupInfo> &groups)
//{
//    auto cells = _tableView->getVisibleCells();
//    for (auto child : cells) {
//        auto cell = dynamic_cast<GroupCell*>(child);
//        if (cell) {
//            cell->setGroupItem(groups.at(cell->getTag()));
//        }
//    }
//
//    _groups = groups;
//}

cocos2d::Node * GroupSelectView::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    auto cell = (GroupCell*)_tableView->dequeueReusableCellWithIdentifier("GROUP_CELL");
    if (cell==nullptr) {
        cell = GroupCell::create(_fetcher);
        cell->setOnClickListener(this);
    }

    cell->setTag(indexPath.getIndex());
    cell->setGroupItem(indexPath.getIndex());

    return cell;
}

int GroupSelectView::numberOfRowsInSection(int section)
{
    return (int)_fetcher->getGroupCount();
}

int GroupSelectView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    return TOUCH_TRUE;
}

#else
GroupSelectView* GroupSelectView::create(const std::vector<ImageGroupInfo> &groups)
{
    GroupSelectView * view = new (std::nothrow)GroupSelectView();
    if (view!=nullptr) {
        if (view->initWithGroupInfo(groups)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    return view;
}

bool GroupSelectView::initWithGroupInfo(const std::vector<ImageGroupInfo> &groups)
{
    if (!SMView::init()) {
        return false;
    }
    auto s = _director->getWinSize();
    
    setContentSize(cocos2d::Size(s.width, s.height-TOP_HEIGHT));
    setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _groups = groups;
    // table view init
    _tableView = SMTableView::createMultiColumn(SMTableView::Orientation::VERTICAL, 1, 0, 0, _contentSize.width, _contentSize.height);
    _tableView->setScrollMarginSize(10, 10);
    _tableView->setPreloadPaddingSize(300);
    _tableView->hintFixedCellSize(CELL_EXTEND_SIZE);
    _tableView->cellForRowAtIndexPath = CC_CALLBACK_1(GroupSelectView::cellForRowAtIndexPath, this);
    _tableView->numberOfRowsInSection = CC_CALLBACK_1(GroupSelectView::numberOfRowsInSection, this);
    _tableView->setScissorEnable(true);
    
    addChild(_tableView);
    setScissorEnable(true);

    return true;
}


void GroupSelectView::setOnGroupSelectListener(OnGroupSelectListener *l)
{
    _listener = l;
}

void GroupSelectView::onClick(SMView *view)
{
    auto item = _groups.at(view->getTag());
    
    if (_listener) {
        _listener->onGroupSelect(item);
    }
}

void GroupSelectView::onImageGroupThumbResult(const std::vector<ImageGroupInfo> &groups)
{
    auto cells = _tableView->getVisibleCells();
    for (auto child : cells) {
        auto cell = dynamic_cast<GroupCell*>(child);
        if (cell) {
            cell->setGroupItem(groups.at(cell->getTag()));
        }
    }

    _groups = groups;
}

cocos2d::Node * GroupSelectView::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    auto cell = (GroupCell*)_tableView->dequeueReusableCellWithIdentifier("GROUP_CELL");
    if (cell==nullptr) {
        cell = GroupCell::create();
        cell->setOnClickListener(this);
    }

    auto item = _groups.at(indexPath.getIndex());

    cell->setTag(indexPath.getIndex());
    cell->setGroupItem(item);

    return cell;
}

int GroupSelectView::numberOfRowsInSection(int section)
{
    return (int)_groups.size();
}

int GroupSelectView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    return TOUCH_TRUE;
}

#endif