//
//  SMImagePickerScene.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 26..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImagePickerScene_h
#define SMImagePickerScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/Util/ImageFetcher.h"
#include "../../SMFrameWork/UI/ImagePickerView.h"
#include "../../SMFrameWork/UI/CaptureView.h"
#include "../../SMFrameWork/Util/ImageDownloader.h"
#include "../../SMFrameWork/Util/DownloadProtocol.h"
#include <cocos2d.h>

class SMPageView;
class SMButton;

class OnImageSelectedListener {
public:
    virtual void onImageSelected(int callIndex, cocos2d::Sprite *image) = 0;
};

class OnQRBarCodeListener {
public:
    virtual void onQRBarCodeResult(std::string result) = 0;
};

class SMImagePickerScene : public SMScene,
                                                public OnImagePickerListener,
                                                public OnCaptureListener,
                                                public OnClickListener,
                                                public DownloadProtocol
{
public:
    enum Mode {
        // Profile 만들기 1:1 Crop
        PROFILE_PHOTO,
        // 일반 Image 선택
        IMAGE_SELECT,
        
    };
    
    enum StartWith {
        ALBUM,
        CAMERA,
        ALBUM_ONLY,
        CAMERA_ONLY,
        EDIT_ONLY,
    };
    
    static SMImagePickerScene * createForQRBarCode(OnQRBarCodeListener * l=nullptr, Intent* sceneParam=nullptr, int index=-1);

    static SMImagePickerScene * createForEdit(Mode mode, StartWith startWith, SwipeType type=SwipeType::DISMISS, Intent* sceneProgram=nullptr);
    
    static SMImagePickerScene * create(OnImageSelectedListener * l, int index, SwipeType type=SwipeType::DISMISS, Intent* sceneProgram=nullptr);

    void setQRBarCode(bool enable) {_forQRBarCode=enable;};
    
    void setOnQRBarCodeListener(OnQRBarCodeListener* l) {_qrListener = l;}
    
    void closeImagePicker();

    virtual void onSceneResult(SMScene* fromScene, Intent* result) override;

protected:
    
    StartWith getStartWithTypeFromSceneParam(Intent* sceneParam=nullptr);

    bool canSwipe(const cocos2d::Vec2& worldPoint);
    
    bool initWithMode(Mode mode, StartWith startWith);
    
    
    
    virtual void onClick(SMView* view) override;
    
    virtual void finishScene(Intent* result=nullptr) override;
    
    // capture view listener
    virtual void onStillImageCaptured(cocos2d::Sprite * captureImage, float x, float y, const std::string tempUrl) override;
    virtual void onVideoCaptured(cocos2d::Data* capturedData, std::string tempUrl) override ;
    virtual void onDetectedScanCode(int type, std::string codeResult) override;
    
    // image picker listener
    virtual void onImageItemClick(SMView * view, const ImageItemInfo& item) override;
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    virtual void onGroupSelect(const ImageGroupInfo* group, bool immediate) override;
    #else
    virtual void onGroupSelect(const std::string& name, bool immediate) override;
    #endif
    virtual void onGroupSelectViewOpen(bool bOpen) override;

    virtual void onTransitionStart(const Transition t, const int flag) override;
    virtual void onTransitionComplete(const Transition t, const int flag) override;
    
    virtual void onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct) override;
    
protected:
    SMImagePickerScene();
    virtual ~SMImagePickerScene();
    
private:
    cocos2d::Node * cellForRowAtIndexPath(const IndexPath& indexPath);
    int numberOfRowsInSection(int section);
    
    void onPageScroll(float position, float distance);
    void onPagenChanged(int page);
    
    void startProfileCropScene(SMView * view, const std::string& imagePath, int width, int height, int orient, int from);
    void startImageSelectScene(SMView* view, const std::string& imagePath, int with, int height, int orient, int from);
    
    void goImageEditor(cocos2d::Sprite * sendSprite);
    
    void goDrawDirect(float t);
private:
    SMView * _topMenuView;
    SMButton * _closeButton; // close, back, menu from swipe type
    SMButton * _cameraButton;
    SMButton * _albumButton;
    
    SMButton * _titleButton;    
    SMButton * _groupButton;
    
    SMPageView * _pageView;
    
    ImagePickerView * _imagePickerView;
    CaptureView * _captureView;
    
    bool _canSwipe;
    int _currentPage;
    
    SMView * _deliverCell;
    
    Mode _mode;

    // for Test
    SMView * _pageA;
    SMView * _pageB;
    StartWith _startWith;
    
    OnImageSelectedListener * _listener;
    bool _forQRBarCode;
    bool _forPicker;
    int _callIndex;
    
    OnQRBarCodeListener * _qrListener;
    bool _sendQRBarCodeResult;
    std::string _strQRCodeResult;

    std::string _pickerType;
    bool _needDraw;
};



#endif /* SMImagePickerScene_h */
