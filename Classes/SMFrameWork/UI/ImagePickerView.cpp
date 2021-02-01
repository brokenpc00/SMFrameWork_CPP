//
//  ImagePickerView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "ImagePickerView.h"
#include "GroupSelectView.h"
#include "../Util/ViewUtil.h"
#include "../Util/OSUtil.h"
#include "../Base/ViewAction.h"
#include "../Base/ShaderNode.h"
#include <2d/CCTweenFunction.h>
#include "ThumbImageView.h"
#include "../Const/SMViewConstValue.h"

#define CELL_SIZE       (361)
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#define COLUMN_COUNT    (3)
#else
#define COLUMN_COUNT    (4)
#endif
#define INCELL_SIZE     (s.width/COLUMN_COUNT)

#define SCROLL_MARGIN   (100)
#define BOTTOM_PRELOAD  (200)

// 기동 속도 향상을 위해 처음 읽는 이미지 갯수
#define INIT_REQ_COUNT  (100)

#define ACTION_OPEN     (SMViewConstValue::Tag::USER+1)

#define THUMB_SIZE 256

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())


// open select view action
class ImagePickerView::OpenSelectAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(OpenSelectAction);
    
    virtual void onStart() override
    {
        auto scene = (ImagePickerView*)_target;
        auto target = scene->_groupSelectView;
        
        _from = target->getPositionY();
        
        if (_show) {
            _to = 0;
        } else {
            _to = -target->getContentSize().height;
        }
        
        target->setEnabled(false);
    }
    
    virtual void onUpdate(float t) override
    {
        auto scene = (ImagePickerView*)_target;
        auto target = scene->_groupSelectView;
        
        if (_show) {
            t = cocos2d::tweenfunc::cubicEaseOut(t);
        }
        
        target->setPositionY(ViewUtil::interpolation(_from, _to, t));
    }
    
    virtual void onEnd() override
    {
        auto scene = (ImagePickerView*)_target;
        auto target = scene->_groupSelectView;
        
        if (!_show) {
            target->removeFromParent();
            scene->_groupSelectView = nullptr;
        } else {
            target->setEnabled(true);
        }
    }
    
    void show(bool show)
    {
        _show = show;
        
        if (_show) {
            setTimeValue(0.3f, 0);
        } else {
            setTimeValue(0.2f, 0);
        }
    }
    
    bool _show;
    
    float _from, _to;
};


static DownloadConfig sThumbDownloadConfig;


ImagePickerView::ImagePickerView() :
_listener(nullptr)
, _groupSelectView(nullptr)
, _openSelectAction(nullptr)
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
, _currentGroupIndex(-1)
#endif
, _bGroupSelectViewOpen(false)
{
    
}

ImagePickerView::~ImagePickerView()
{
    releaseImageFetcher();
    CC_SAFE_RELEASE(_openSelectAction);
}

ImagePickerView *  ImagePickerView::create()
{
    ImagePickerView * view = new (std::nothrow)ImagePickerView();
    
    if (view!=nullptr) {
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

bool ImagePickerView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    auto s = _director->getWinSize();
    
    setContentSize(s);
    
    sThumbDownloadConfig.setResamplePolicy(DownloadConfig::ResamplePolicy::EXACT_CROP, THUMB_SIZE, THUMB_SIZE);
    //sThumbDownloadConfig.setCachePolicy(DownloadConfig::CachePolycy::MEMORY_ONLY);
    // tableView init
    _tableView = SMTableView::createMultiColumn(SMTableView::Orientation::VERTICAL, COLUMN_COUNT, 0, -BOTTOM_PRELOAD, s.width+4, s.height-TOP_MENU_HEIGHT+4+SCROLL_MARGIN+BOTTOM_PRELOAD);
    _tableView->setPreloadPaddingSize(300);
    _tableView->setScrollMarginSize(SCROLL_MARGIN, BOTTOM_PRELOAD);
//    _tableView->hintFixedCellSize(CELL_SIZE);
    _tableView->cellForRowAtIndexPath = CC_CALLBACK_1(ImagePickerView::cellForRowAtIndexPath, this);
    _tableView->numberOfRowsInSection = CC_CALLBACK_1(ImagePickerView::numberOfRowsInSection, this);
    _tableView->setScissorEnable(true);
    
    addChild(_tableView);;
    
    _groupSelectViewStub = SMView::create();
    _groupSelectViewStub->setContentSize(_contentSize);
    addChild(_groupSelectViewStub);
    
    _initRequest = false;
    
    if (isAccessPermitted()) {
        // access 권한이 있냐?... 그러면 미리 읽어둔다.
        _initRequest = true;
        _scheduler->performFunctionInCocosThread([&]{
            // 지금 말고 다음 프레임에...
            initImageFetcher();
            #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            _fetcher->requestGroupList();
            #else
            _fetcher->onInit();
            #endif
        });
    }
    
    return true;
}

void ImagePickerView::onEnter()
{
    SMView::onEnter();
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    if (!_initRequest) {
        _initRequest = true;
        initImageFetcher();
        _fetcher->requestGroupList();
    }
    #endif

}

void ImagePickerView::onExit()
{
    SMView::onExit();
}

void ImagePickerView::finish()
{
    releaseImageFetcher();
    
    ImageDownloader::getInstance().clearCache();
    
    _tableView->stop();
}

void ImagePickerView::setOnImagePickerListener(OnImagePickerListener *l)
{
    _listener = l;
}

void ImagePickerView::onGroupSelectButtonClick(SMView *view)
{
    auto action = getActionByTag(ACTION_OPEN);
    
    if (!action) {
        if (_bGroupSelectViewOpen) {
            closeGroupSelectView();
        } else {
        #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
            openGroupSelectView();
        #else
            _fetcher->requestAlbumList();
        #endif
        }
    }
}

void ImagePickerView::onClick(SMView *view)
{
    int index = view->getTag();
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    if (index >= 0 && index < (int)_itemInfos.size()) {
        auto item = _itemInfos.at(view->getTag());
        
        if (_listener) {
            _listener->onImageItemClick(view, item);
        }
    }    
    #else
    auto item = _fetcher->getImageItemInfo(_currentGroupIndex, index);
    
    if (_listener) {
        _listener->onImageItemClick(view, item);
    }
    #endif
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
void ImagePickerView::onGroupSelect(const ImageGroupInfo &groupInfo)
{
    closeGroupSelectView();

    
    //if (_currentGroup.key.compare(group.key) != 0) {
    if (_currentGroup.seq != groupInfo.seq) {
        // 다른 그룹이면 바꾼다.
        _itemInfos.clear();
        _tableView->reloadData();

        
        _currentGroup = groupInfo;
        _fetcher->requestPhotoList(groupInfo, 0, INIT_REQ_COUNT);
        
        if (_listener) {
            _listener->onGroupSelect(&groupInfo, false);
        }
    }
}

// 처음 그룹 정보 가져왔을 때
void ImagePickerView::onImageGroupResult(const std::vector<ImageGroupInfo> &groupInfos)
{
    for (auto groupInfo : groupInfos) {
        _groupInfos.push_back(groupInfo);
        if (groupInfo.isDefault) {
            _currentGroup = groupInfo;
            _fetcher->requestPhotoList(_currentGroup, 0, INIT_REQ_COUNT);
            
            if (_listener) {
                _listener->onGroupSelect(&groupInfo, true);
            }
        }
    }
}

void ImagePickerView::onImageItemResult(const std::vector<ImageItemInfo> &itemInfos)
{
    if (itemInfos.empty()) {    // 비어있으면 패스~
        return;
    }
    
    _itemInfos.insert(_itemInfos.end(), itemInfos.begin(), itemInfos.end());
    _tableView->updateData();
    
    // 다음꺼 읽어온다
    _fetcher->requestPhotoList(_currentGroup, (int)_itemInfos.size(), (int)(_currentGroup.numPhotos-_itemInfos.size()));
}

// thumbnail 다 읽었으면
void ImagePickerView::onImageGroupThumbResult(const std::vector<ImageGroupInfo> &groupInfos)
{
    _groupInfos = groupInfos;

    
    if (_groupSelectView) {
        _groupSelectView->onImageGroupThumbResult(_groupInfos);
    }
}

void ImagePickerView::openGroupSelectView()
{
    if (_groupSelectView==nullptr) {
        _groupSelectView = GroupSelectView::create(_groupInfos);
        _groupSelectView->setOnGroupSelectListener(this);
        _groupSelectView->setPositionY(-_groupSelectView->getContentSize().height);
        _groupSelectViewStub->addChild(_groupSelectView);
        _fetcher->requestGroupThumbnail();
    }

    auto action = getActionByTag(ACTION_OPEN);
    if (action) {
        stopAction(action);
    }

    if (_openSelectAction==nullptr) {
        _openSelectAction = OpenSelectAction::create(false);
        _openSelectAction->setTag(ACTION_OPEN);
    }

    _openSelectAction->show(true);
    runAction(cocos2d::Sequence::create(_openSelectAction, cocos2d::CallFunc::create([&]{
        _bGroupSelectViewOpen = true;
    }), NULL));
    
    if (_listener) {
        _listener->onGroupSelectViewOpen(true);
    }
}

void ImagePickerView::closeGroupSelectView()
{
    auto action = getActionByTag(ACTION_OPEN);
    if (action) {
        stopAction(action);
    }
    
    _openSelectAction->show(false);
    runAction(cocos2d::Sequence::create(_openSelectAction, cocos2d::CallFunc::create([&]{
        _bGroupSelectViewOpen = false;
    }), NULL));
    
    if (_listener) {
        _listener->onGroupSelectViewOpen(false);
    }
}

static const cocos2d::Color3B COLOR_ZERO = cocos2d::Color3B(0, 0, 0);

cocos2d::Node * ImagePickerView::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    auto s = cocos2d::Director::getInstance()->getWinSize();
    ImagePickerCell * cell=nullptr;
    cocos2d::Node * convertView = _tableView->dequeueReusableCellWithIdentifier("PICKER_CELL");
    if (convertView!=nullptr) {
        cell = (ImagePickerCell*)convertView;
    } else {
        cell = ImagePickerCell::create(0, 0, 0, INCELL_SIZE+4, INCELL_SIZE+4);

        cell->_imageView = SMImageView::create();
        cell->addChild(cell->_imageView);
        cell->setOnClickListener(this);
        cell->_imageView->setContentSize(cocos2d::Size(INCELL_SIZE, INCELL_SIZE));
        cell->_imageView->setAnchorPoint(cocos2d::Vec2::ZERO);
        cell->_imageView->setPosition(cocos2d::Vec2(2, 2));
//        cell->setContentSize(cocos2d::Size(INCELL_SIZE-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2, INCELL_SIZE-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));

        cell->_imageView->setLoadingIconColor(COLOR_ZERO);
        cell->_imageView->setOnClickListener(this);
        cell->_imageView->setDownloadConfig(sThumbDownloadConfig);
//        cell->setAnchorPoint(cocos2d::Vec2::ZERO);
//        cell->setPosition(cocos2d::Vec2(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH));
//        auto bottomLine = SMView::create(0, 0, 0, INCELL_SIZE, 2);
//        bottomLine->setBackgroundColor4F(cocos2d::Color4F::BLACK);
//        cell->addChild(bottomLine);
    }

    int index = indexPath.getIndex();
    cell->_imageView->setTag(index);
    cell->setTag(index);


    std::string thumbPath = _itemInfos.at(indexPath.getIndex()).url;

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    cell->_imageView->setNSUrlPathForIOS(thumbPath, _isHighSpeed?true:false);
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    cell->_imageView->setImageFilePath(thumbPath, true);
#endif

    cell->_imageView->setScaleType(SMImageView::ScaleType::CENTER_CROP);
    cell->_imageView->setScissorEnable(true);
    cell->setScissorEnable(true);
//    cell->setBackgroundColor4F(cocos2d::Color4F(1, 0, 0, 0.3f));
    
//    cell->setNSUrlPathForIOS(_itemInfos.at(indexPath.getIndex()).url, _isHighSpeed?true:false);
    
    return cell;
}

int ImagePickerView::numberOfRowsInSection(int section)
{
    return (int)_itemInfos.size();
}


#include "AlertView.h"

void ImagePickerView::showAccessDeniedPrompt()
{
    // alert view call
    AlertView::showConfirm("앨범 접근 권한 없음", "앨범 접근 권한이 필요합니다.", "닫기", "설정하기", [&]{
        OSUtil::openDeviceSettings();
    }, [&]{
        
    });

}

void ImagePickerView::onAlbumAccessDenied()
{
    showAccessDeniedPrompt();
}

#else
void ImagePickerView::onGroupSelect(int groupIndex)
{
    closeGroupSelectView();

    if (groupIndex >= 0 && _currentGroupIndex != groupIndex) {
        // 그룹 교체
        _currentGroupIndex = groupIndex;
        _currentGroupName = _fetcher->getGroupInfo(groupIndex)->name;
        _tableView->reloadData();
        
        if (_listener) {
            _listener->onGroupSelect(_currentGroupName, true);
        }
    }
}

// 최초 진입 시 그룹 정보를 가져왔을 때
void ImagePickerView::onImageFetcherInit(const std::vector<ImageGroupInfo>* groupInfos)
{
    if (groupInfos == nullptr || groupInfos->empty()) {
        // 아무것도 없다..
        return;
    }
    
    // 첫번째 그룹 (Camera Roll 선택)
    onGroupSelect(0);
}

// 폴더 변경 클릭
void ImagePickerView::onImageGroupResult(const std::vector<ImageGroupInfo>* groupInfos)
{
    _scheduler->performFunctionInCocosThread([groupInfos, this]{
        if (_groupSelectView == nullptr) {
            _groupSelectView = GroupSelectView::create(_fetcher.get());
            _groupSelectView->setOnGroupSelectListener(this);
            _groupSelectView->setPositionY(-_groupSelectView->getContentSize().height);
            _groupSelectViewStub->addChild(_groupSelectView);
        }
        
        auto action = getActionByTag(ACTION_OPEN);
        if (action) {
            stopAction(action);
        }
        
        if (_openSelectAction == nullptr) {
            _openSelectAction = OpenSelectAction::create(false);
            _openSelectAction->setTag(ACTION_OPEN);
        }
        _openSelectAction->show(true);
        runAction(cocos2d::Sequence::create(_openSelectAction, cocos2d::CallFunc::create([&]{
            _bGroupSelectViewOpen = true;
        }), NULL));

        if (_listener) {
            _listener->onGroupSelectViewOpen(true);
        }
    });

}

void ImagePickerView::openGroupSelectView()
{
//    if (_groupSelectView==nullptr) {
//        _groupSelectView = GroupSelectView::create(_groups);
//        _groupSelectView->setOnGroupSelectListener(this);
//        _groupSelectView->setPositionY(-_groupSelectView->getContentSize().height);
//        _groupSelectViewStub->addChild(_groupSelectView);
//        _fetcher->requestGroupThumbnail();
//    }
//
//    auto action = getActionByTag(ACTION_OPEN);
//    if (action) {
//        stopAction(action);
//    }
//
//    if (_openSelectAction==nullptr) {
//        _openSelectAction = OpenSelectAction::create(false);
//        _openSelectAction->setTag(ACTION_OPEN);
//    }
//
//    _openSelectAction->show(true);
//    runAction(cocos2d::Sequence::create(_openSelectAction, cocos2d::CallFunc::create([&]{
//        _bGroupSelectViewOpen = true;
//    }), NULL));
//
//    _bGroupSelectViewOpen = true;
//
//    if (_listener) {
//        _listener->onGroupSelectViewOpen(true);
//    }
}

void ImagePickerView::closeGroupSelectView()
{
    if (_groupSelectView) {
        auto action = getActionByTag(ACTION_OPEN);
        if (action) {
            stopAction(action);
        }
    
        _openSelectAction->show(false);
        runAction(cocos2d::Sequence::create(_openSelectAction, cocos2d::CallFunc::create([&]{
            _bGroupSelectViewOpen = false;
        }), NULL));

        if (_listener) {
            _listener->onGroupSelectViewOpen(false);
        }
    }
    
}

static const cocos2d::Color3B COLOR_ZERO = cocos2d::Color3B(0, 0, 0);

cocos2d::Node * ImagePickerView::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    auto s = cocos2d::Director::getInstance()->getWinSize();
    ImagePickerCell * cell=nullptr;
    cocos2d::Node * convertView = _tableView->dequeueReusableCellWithIdentifier("PICKER_CELL");
    if (convertView!=nullptr) {
        cell = (ImagePickerCell*)convertView;
    } else {
        cell = ImagePickerCell::create(0, 0, 0, INCELL_SIZE+4, INCELL_SIZE+4);

        cell->_imageView = SMImageView::create();
        cell->addChild(cell->_imageView);
        cell->setOnClickListener(this);
        cell->_imageView->setContentSize(cocos2d::Size(INCELL_SIZE, INCELL_SIZE));
        cell->_imageView->setAnchorPoint(cocos2d::Vec2::ZERO);
        cell->_imageView->setPosition(cocos2d::Vec2(2, 2));
//        cell->setContentSize(cocos2d::Size(INCELL_SIZE-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2, INCELL_SIZE-ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2));

        cell->_imageView->setLoadingIconColor(COLOR_ZERO);
        cell->_imageView->setOnClickListener(this);
        cell->_imageView->setDownloadConfig(sThumbDownloadConfig);
//        cell->setAnchorPoint(cocos2d::Vec2::ZERO);
//        cell->setPosition(cocos2d::Vec2(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH, ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH));
//        auto bottomLine = SMView::create(0, 0, 0, INCELL_SIZE, 2);
//        bottomLine->setBackgroundColor4F(cocos2d::Color4F::BLACK);
//        cell->addChild(bottomLine);
    }

    int index = indexPath.getIndex();
    cell->_imageView->setTag(index);
    cell->setTag(index);
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    auto phAssetObject = _fetcher->fetchPhotoAssetObject(_currentGroupIndex, indexPath.getIndex());
    cell->_imageView->setPHAssetObject(phAssetObject, true);
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    auto itemInfo = _fetcher->getImageItemInfo(_currentGroupIndex, indexPath.getIndex());
    std::string thumbPath = itemInfo.url;
    cell->_imageView->setImageFilePath(thumbPath, true);
#endif

    cell->_imageView->setScaleType(SMImageView::ScaleType::CENTER_CROP);
    cell->_imageView->setScissorEnable(true);
    cell->setScissorEnable(true);
//    cell->setBackgroundColor4F(cocos2d::Color4F(1, 0, 0, 0.3f));
    
//    cell->setNSUrlPathForIOS(_itemInfos.at(indexPath.getIndex()).url, _isHighSpeed?true:false);
    
    return cell;
}

int ImagePickerView::numberOfRowsInSection(int section)
{
//    return (int)_itemInfos.size();
    if (_currentGroupIndex >= 0) {
        return _fetcher->getGroupInfo(_currentGroupIndex)->numPhotos;
    }
    return 0;
}


#include "AlertView.h"

void ImagePickerView::showAccessDeniedPrompt()
{
    
    AlertView::showConfirm("앨범 접근 권한 없음", "앨범 접근 권한이 필요합니다.", "닫기", "설정하기", [&]{
        OSUtil::openDeviceSettings();
    }, [&]{
        
    });

}

void ImagePickerView::onAlbumAccessDenied()
{
    showAccessDeniedPrompt();
}

void ImagePickerView::onAlbumAccessAuthorized()
{
    
}
#endif
