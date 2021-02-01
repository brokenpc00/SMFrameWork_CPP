//
//  SMImageEditorStickerListView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStickerListView.h"
#include "SMImageEditorItemThumbView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include <base/ccMacros.h>
#include <string>
#include <base/CCNS.h>
#include <stdlib.h>


#define PANEL_HEIGHT    (160.0f)
#define CELL_WIDTH      (204.0+30.0)
//#define ITEMS   "items"


const std::string ItemListView::ITEMS = "items";
const std::string ItemListView::THUMB = "thumb";
const std::string ItemListView::NAME = "name";
//const std::string ItemListView::IMG_EXTEND = ".webp";
const std::string ItemListView::IMG_EXTEND = ".png";
const std::string ItemListView::IMAGE = "image";
const std::string ItemListView::LAYOUT = "layout";

ItemListView::ItemListView() :
_listener(nullptr)
, _initLoaded(false)
{
    
}

ItemListView::~ItemListView()
{
    
}

void ItemListView::setVisible(bool visible)
{
    if (visible != _visible) {
        if (visible) {
            show();
        } else {
            hide();
        }
    }
    SMView::setVisible(visible);
}

void ItemListView::show()
{
    initLoadItemList();
}

void ItemListView::hide()
{
    stop();
}

void ItemListView::onEnter()
{
    SMView::onEnter();
}

void ItemListView::onExit()
{
    SMView::onExit();
}

bool ItemListView::initLayout()
{
    setContentSize(cocos2d::Size(_director->getWinSize().width, PANEL_HEIGHT));
    setMaxScrollVelocity(10000);
    setPreloadPaddingSize(200);
    
    setBackgroundColor4F(MAKE_COLOR4F(0x767678, 1.0f));
    
    SMView::setVisible(false);
    
    return true;
}

void ItemListView::onClick(SMView *view)
{
    if (_listener) {
        if (dynamic_cast<SMImageEditorItemThumbView*>(view)) {
            _listener->onItemClick(this, (SMImageEditorItemThumbView *)view);
        }
    }
}

bool ItemListView::initLoadItemList()
{
    if (!_initLoaded) {
        // xml loading
        std::string plist = _resourceRootPath + ITEMS + ".xml";
        std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(plist);
        
        CCASSERT(fullPath.size()>0, "items xml file not found");
        
        _dict = cocos2d::FileUtils::getInstance()->getValueMapFromFile(fullPath);
        _itemSize = (int)_dict[ITEMS].asValueVector().size();
        
        _initLoaded = true;
    }
    
    return true;
}

SMImageEditorStickerItemListView * SMImageEditorStickerItemListView::create()
{
    auto view = new (std::nothrow)SMImageEditorStickerItemListView();
    if (view && view->initWithOrientAndColumns(SMTableView::Orientation::HORIZONTAL, 1)) {
        view->initLayout();
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

SMImageEditorStickerItemListView::SMImageEditorStickerItemListView()
{
    _resourceRootPath = "sticker/";
}

SMImageEditorStickerItemListView::~SMImageEditorStickerItemListView()
{
    
}

bool SMImageEditorStickerItemListView::initLayout()
{
    if (!ItemListView::initLayout()) {
        return false;
    }
    
    cellForRowAtIndexPath = CC_CALLBACK_1(SMImageEditorStickerItemListView::getView, this);
    numberOfRowsInSection = CC_CALLBACK_1(SMImageEditorStickerItemListView::getItemCount, this);
    
    setScrollMarginSize(15, 15);
    hintFixedCellSize(CELL_WIDTH);
    
    return true;
}

bool SMImageEditorStickerItemListView::initLoadItemList()
{
    if (!_initLoaded) {
        if (ItemListView::initLoadItemList()) {
            auto items = _dict[ITEMS].asValueVector();
            
            _items.resize(items.size());
            for (auto& item : _items) {
                item.decoded = false;
            }
        }
    }
    
    return _initLoaded;
}

StickerItem * SMImageEditorStickerItemListView::findItem(const std::string &name)
{
    if (name.empty()) {
        return nullptr;
    }
    
    initLoadItemList();
    
    auto array = _dict[ITEMS].asValueVector();
    for (int index=0; _itemSize; index++) {
        
        if (_items[index].decoded) {
            if (_items[index]._name.compare(name)==0) {
                return &_items[index];
            }
        } else {
            auto m = array.at(index).asValueMap();
            if (name.compare(m[NAME].asString())==0) {
                parseStickerItem(_items[index], index);
                return &_items[index];
            }
        }
    }

    return nullptr;
}

StickerItem * SMImageEditorStickerItemListView::getItem(const int index)
{
    if (index<0) {
        return nullptr;
    }
    
    initLoadItemList();
    
    if (index>=_itemSize) {
        return nullptr;
    }
    
    if (_items[index].decoded) {
        return &_items[index];
    }
    
    parseStickerItem(_items[index], index);
    return &_items[index];
}

cocos2d::Node * SMImageEditorStickerItemListView::getView(const IndexPath &indexPath)
{
    SMImageEditorItemThumbView * thumb = nullptr;
    
    int index = indexPath.getIndex();
    
    if (index==0) {
        // clear all sticker (no image)
        thumb = (SMImageEditorItemThumbView*)dequeueReusableCellWithIdentifier("NOIMAGE");
    } else {
        thumb = (SMImageEditorItemThumbView*)dequeueReusableCellWithIdentifier("STICKER_CELL");
    }
    
    if (thumb==nullptr) {
        thumb = SMImageEditorItemThumbView::create(&ImageDownloader::DC_NO_DISK, &ImageDownloader::DC_CACHE_ONLY);
        thumb->setContentSize(cocos2d::Size(CELL_WIDTH, PANEL_HEIGHT));
        thumb->setOnClickListener(this);
        
        if (index==0) {
            // clear all sticker (no image)
            auto text = cocos2d::Label::createWithSystemFont("CLEAR\nALL", SMFontConst::SystemFontRegular, 30);
            text->setAlignment(cocos2d::TextHAlignment::CENTER);
            text->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            text->setPosition(thumb->getImageView()->getContentSize()/2);
            thumb->getImageView()->addChild(text);
        }
    }
    
    auto m = _dict[ITEMS].asValueVector().at(index).asValueMap();
    thumb->setTag(index);
    std::string thumbPath = _resourceRootPath+THUMB + "/" + m[NAME].asString() + IMG_EXTEND;
    thumb->setImagePath(thumbPath);
    
    return thumb;
}

int SMImageEditorStickerItemListView::getItemCount(int section)
{
    return _itemSize;
}

void SMImageEditorStickerItemListView::show()
{
    ItemListView::show();
    
    auto cells = getVisibleCells();
    for (auto cell : cells) {
        auto thumb = dynamic_cast<SMImageEditorItemThumbView*>(cell);
        if (thumb) {
            // 흔들 흔들
            thumb->startShowAction();
        }
    }
}
