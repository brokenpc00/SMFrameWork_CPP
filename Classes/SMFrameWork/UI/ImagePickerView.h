//
//  ImagePickerView.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ImagePickerView_h
#define ImagePickerView_h

#include "../Base/SMView.h"
#include "../Base/SMTableView.h"
#include "../Util/ImageFetcher.h"
#include "../Base/SMImageView.h"

class GroupSelectView;

class OnGroupSelectListener {
public:
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    virtual void onGroupSelect(const ImageGroupInfo& group) = 0;
#else
    virtual void onGroupSelect(int groupIndex) = 0;
#endif
};

class OnImagePickerListener {
public:
    virtual void onImageItemClick(SMView* view, const ImageItemInfo& item) = 0;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    virtual void onGroupSelect(const ImageGroupInfo* group, bool init) = 0;
#else
    virtual void onGroupSelect(const std::string& name, bool immediate) = 0;
#endif
    virtual void onGroupSelectViewOpen(bool bOpen) = 0;
};

class ImagePickerView : public SMView,
                                        public OnClickListener,
                                        public OnGroupSelectListener,
                                        public ImageFetcherClient
{
public:
    static void showAccessDeniedPrompt();
    
    static ImagePickerView * create();
    
    virtual bool init() override;
    
    void finish();
    
    void setOnImagePickerListener(OnImagePickerListener* l);
    
    // ImageFetcher
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    virtual void onImageGroupResult(const std::vector<ImageGroupInfo>& groupInfos) override;
    virtual void onImageItemResult(const std::vector<ImageItemInfo>& itemInfos) override;
    virtual void onImageGroupThumbResult(const std::vector<ImageGroupInfo>& groupInfos) override;
#else
    virtual void onImageFetcherInit(const std::vector<ImageGroupInfo>* groupInfos) override;
    virtual void onImageGroupResult(const std::vector<ImageGroupInfo>* groupInfos) override;
#endif
    
    // onclick
    virtual void onClick(SMView* view) override;
    
    // Group select listener
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    virtual void onGroupSelect(const ImageGroupInfo& groupInfo) override;
#else
    virtual void onGroupSelect(int groupIndex) override;
#endif
    
    void onGroupSelectButtonClick(SMView * view);
    
    virtual void onEnter() override;
    virtual void onExit() override;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    ImageGroupInfo& getCurrentGroup() {return _currentGroup;};
 #else
    int getCurrentGroupIndex() { return _currentGroupIndex; }
    
    std::string getCurrentGroupName() { return _currentGroupName; }
    virtual void onAlbumAccessAuthorized() override;
 #endif
    virtual void onAlbumAccessDenied() override;
 
protected:
    ImagePickerView();
    virtual ~ImagePickerView();
    
    
private:
    cocos2d::Node * cellForRowAtIndexPath(const IndexPath& indexPath);
    
    int numberOfRowsInSection(int section);
    
    void openGroupSelectView();
    
    void closeGroupSelectView();

    class ImagePickerCell : public SMView
    {
    public:
        ImagePickerCell(){};
        virtual ~ImagePickerCell(){};
        CREATE_VIEW(ImagePickerCell);
        
        SMImageView * _imageView;
    };


protected:
    GroupSelectView* _groupSelectView;
    
private:
    class OpenSelectAction;
    
    SMTableView * _tableView;
    
    SMView * _groupSelectViewStub;
    
    OpenSelectAction* _openSelectAction;
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    std::vector<ImageItemInfo> _itemInfos;
    std::vector<ImageGroupInfo> _groupInfos;
    
    ImageGroupInfo  _currentGroup;
    
    bool _initRequest;
#else
    int _currentGroupIndex;
    
    
    bool _initRequest;
    
    std::string _currentGroupName;

#endif
    OnImagePickerListener* _listener;
    // action bar 대신에
    bool _bGroupSelectViewOpen;
};

#endif /* ImagePickerView_h */
