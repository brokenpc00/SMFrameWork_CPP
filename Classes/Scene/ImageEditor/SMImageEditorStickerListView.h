//
//  SMImageEditorStickerListView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStickerListView_h
#define SMImageEditorStickerListView_h

#include "../../SMFrameWork/Base/SMTableView.h"
#include <cocos2d.h>
#include "ItemInfo.h"
#include <vector>

class ItemListView;
class SMImageEditorItemThumbView;
class SMSlider;

class OnItemClickListener {
public:
    virtual void onItemClick(ItemListView* sender, SMImageEditorItemThumbView * view) = 0;
};

class ItemListView : public SMTableView, public OnClickListener
{
public:
    static const std::string ITEMS;
    static const std::string THUMB;
    static const std::string NAME;
    static const std::string IMG_EXTEND;
    static const std::string IMAGE;
    static const std::string LAYOUT;
    
    virtual void show();
    virtual void hide();
    virtual void onEnter() override;
    virtual void onExit() override;
    
    void setOnItemClickListener(OnItemClickListener * l){_listener=l;};
    virtual void onClick(SMView * view) override;
    std::string getResourceRootPath() {return _resourceRootPath;};
    virtual void setVisible(bool visible) override;
protected:
    ItemListView();
    virtual ~ItemListView();
    virtual bool initLayout();
    virtual bool initLoadItemList();
    
protected:
    bool _initLoaded;
    cocos2d::ValueMap _dict;
    OnItemClickListener * _listener;
    std::string _resourceRootPath;
    int _itemSize;
};

class SMImageEditorStickerItemListView : public ItemListView
{
public:
    static SMImageEditorStickerItemListView* create();
    virtual void show() override;
    StickerItem* findItem(const std::string& name);
    StickerItem* getItem(const int index);
    
protected:
    SMImageEditorStickerItemListView();
    virtual ~SMImageEditorStickerItemListView();
    
    virtual bool initLayout() override;
    virtual bool initLoadItemList() override;

    cocos2d::Node * getView(const IndexPath& indexPath);
    int getItemCount(int section);
    bool parseStickerItem(StickerItem& item, int index);
    std::vector<StickerItem> _items;
};

#endif /* SMImageEditorStickerListView_h */
