//
//  SMImagePickerScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 26..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImagePickerScene.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMPageView.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"

#include "../ImageEditor/SMImageEditorScene.h"
#include "../ImageEditor/SMImageEditorDrawScene.h"
#include "../ImageEditor/SMImageEditorRotateScene.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>
#include <base/CCDirector.h>
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

#define SCENE_TAG_PROFILE_CROP   (100)
#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())

SMImagePickerScene * SMImagePickerScene::createForQRBarCode(OnQRBarCodeListener * l, Intent * sceneParam, int index)
{
    SMImagePickerScene * scene = new (std::nothrow)SMImagePickerScene();
    
    if (scene!=nullptr) {
        scene->setOnQRBarCodeListener(l);
        scene->setQRBarCode(true);
        scene->_callIndex = index;
        Mode mode = IMAGE_SELECT;
        StartWith startWith = CAMERA_ONLY;
        SwipeType type = SwipeType::DISMISS;
        if (scene->initWithSceneParam(sceneParam, type) &&
            scene->initWithMode(mode, startWith)) {
            scene->autorelease();
        } else {
            CC_SAFE_DELETE(scene);
        }
    }
    
    return scene;
}

SMImagePickerScene * SMImagePickerScene::createForEdit(Mode mode, StartWith startWith, SwipeType type, Intent * sceneProgram)
{
    SMImagePickerScene * scene = new (std::nothrow)SMImagePickerScene();
    if (scene &&
        scene->initWithSceneParam(sceneProgram, type) &&
        scene->initWithMode(mode, startWith))
    {
        scene->autorelease();
    } else {
        CC_SAFE_DELETE(scene);
    }
    return scene;
}

SMImagePickerScene * SMImagePickerScene::create(OnImageSelectedListener * l, int index, SwipeType type, Intent * sceneProgram)
{
    SMImagePickerScene * scene = new (std::nothrow)SMImagePickerScene();
    if (scene!=nullptr) {
        scene->_callIndex = index;
        scene->_listener = l;
        scene->_forPicker = true;
        StartWith startWith = scene->getStartWithTypeFromSceneParam(sceneProgram);
        if (scene->initWithSceneParam(sceneProgram, type) && scene->initWithMode(IMAGE_SELECT, startWith)) {
            scene->autorelease();
        } else {
            CC_SAFE_DELETE(scene);
        }
    }
    
    return scene;
}

SMImagePickerScene::SMImagePickerScene() :
_imagePickerView(nullptr)
, _captureView(nullptr)
, _canSwipe(true)
, _deliverCell(nullptr)
, _forQRBarCode(false)
, _forPicker(false)
, _listener(nullptr)
, _callIndex(-1)
, _qrListener(nullptr)
, _sendQRBarCodeResult(false)
, _strQRCodeResult("")
, _pickerType("111")
, _needDraw(true)
{
    
}

SMImagePickerScene::~SMImagePickerScene()
{
    CC_SAFE_RELEASE(_imagePickerView);
    CC_SAFE_RELEASE(_captureView);
}

SMImagePickerScene::StartWith SMImagePickerScene::getStartWithTypeFromSceneParam(Intent *param)
{
    if (param==nullptr) {
        return ALBUM;
    }
    
    _pickerType = param->getString("PICKER_TYPE", "111");
    char cameraChar = _pickerType.at(0);
    char imageChar = _pickerType.at(1);
    char editChar = _pickerType.at(2);
    _needDraw = editChar=='1';
    
    if (cameraChar=='1' && imageChar=='1') {
        // all
        return ALBUM;
    } else if (cameraChar=='1' && imageChar=='0') {
        // camera only
        return CAMERA_ONLY;
    } else if (cameraChar=='0' && imageChar=='1') {
        // image only
        return ALBUM_ONLY;
    } else {
        // edit only
        _needDraw = true;
        return EDIT_ONLY;
    }
}

bool SMImagePickerScene::initWithMode(Mode mode, StartWith startWith)
{

    _startWith = startWith;
//    startWith = StartWith::ALBUM;
    if (_startWith==StartWith::ALBUM || _startWith==StartWith::ALBUM_ONLY) {
        // 앨범으로 시작
        if (!ImagePickerView::canAcceessible()) {
            ImagePickerView::showAccessDeniedPrompt();
            return false;
        }
        _currentPage = 0;
    } else {
        // 카메라로 시작
        if (!CaptureView::canAccessible()) {
            CaptureView::showAccessDeniedPrompt();
            return false;
        }
        if (_startWith==CAMERA_ONLY) {
            _currentPage = 0;
        } else {
        _currentPage = 1;
    }
    }

    if (!SMScene::init()) {
        return false;
    }
    
    auto param = getSceneParam();
    if (param) {
        
        // A : 카메라만, B : 이미지만, C : 그림판 없이, D : 그림판만, "" : 공백이면 Free
        // already set
//        _pickerType = param->getString("PICKER_TYPE", "111");
//        _needDraw = _pickerType.at(2)=='1';
    }

    _mode = mode;

    bool cameraCropSquare = false;

    if (mode==Mode::PROFILE_PHOTO) {
        cameraCropSquare = true;
    }

    auto s = _director->getWinSize();

    getRootView()->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    
    if (_forQRBarCode) {
        // no page view
        // no image picker
        // only camera
        _captureView = CaptureView::createForQRBarCode();
        _captureView->setOnCaptureListener(this);
        _captureView->attachPageView(nullptr);
        _captureView->setContentSize(s);
        _captureView->setPosition(cocos2d::Vec2::ZERO);
        _captureView->setAnchorPoint(cocos2d::Vec2::ZERO);
        addChild(_captureView);
    
        auto closeButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, s.width-400, s.height-200, 340, 120);
        closeButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        closeButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4);
        closeButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 0.0f));
        closeButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 0.2f));
        closeButton->setShapeCornerRadius(40);
        closeButton->setTextSystemFont("닫기", SMFontConst::SystemFontRegular, 50);
        closeButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        addChild(closeButton);
        closeButton->setLocalZOrder(90);
        closeButton->setOnClickCallback([&](SMView * view){
            _director->getScheduler()->performFunctionInCocosThread([&]{
                auto scene = SceneTransition::SlideOutToBottom::create(0.3f, _director->getPreviousScene());
                _director->popSceneWithTransition(scene);
            });
        });
    } else {
        if (_startWith==EDIT_ONLY) {
            if (!isScheduled(schedule_selector(SMImagePickerScene::goDrawDirect))) {
                schedule(schedule_selector(SMImagePickerScene::goDrawDirect), 0.5f);
            }
        } else {
    _topMenuView = SMView::create(0, 0, s.height-TOP_MENU_HEIGHT, s.width, TOP_MENU_HEIGHT);
    _topMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    addChild(_topMenuView);
    _topMenuView->setLocalZOrder(99);
        
    // image picker view
        if (_startWith!=CAMERA_ONLY) {
    _imagePickerView = ImagePickerView::create();
    _imagePickerView->setOnImagePickerListener(this);
    _imagePickerView->retain();
        }
    
    // camera
        if (_startWith!=ALBUM_ONLY) {
    _captureView = CaptureView::create(cameraCropSquare);
    _captureView->setOnCaptureListener(this);
    _captureView->retain();
        }
    
    _pageView = SMPageView::create(SMPageView::Orientation::HORIZONTAL, 0, 0, s.width, s.height);
    _pageView->cellForRowAtIndexPath = CC_CALLBACK_1(SMImagePickerScene::cellForRowAtIndexPath, this);
    _pageView->numberOfRowsInSection = CC_CALLBACK_1(SMImagePickerScene::numberOfRowsInSection, this);
    _pageView->onPageScrollCallback = CC_CALLBACK_2(SMImagePickerScene::onPageScroll, this);
    _pageView->onPageChangedCallback = CC_CALLBACK_1(SMImagePickerScene::onPagenChanged, this);
        if (_startWith==CAMERA_ONLY || _startWith==ALBUM_ONLY) {
            _pageView->setScrollLock(true);
        }
    
    addChild(_pageView);
    
        // for capgture view... not for page view
        if (_startWith==CAMERA || _startWith==CAMERA_ONLY) {
    _captureView->attachPageView(_pageView);
        }
    
    // siwpe type에 따라서 close, menu, back 구분 해야함.
    // 일단 close만 하자
    _closeButton = SMButton::create(0, SMButton::Style::SOLID_RECT, 10, 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    _closeButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xFFFFFF, 1.0f));
    _closeButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xADAFB3, 1.0f));
    _closeButton->setIcon(SMButton::State::NORMAL, "images/popup_close.png");
    _closeButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
        if (_forPicker) {
            _closeButton->setOnClickCallback([&](SMView * view){
                closeImagePicker();
            });
        } else {
    _closeButton->setOnClickListener(this);
        }
    _topMenuView->addChild(_closeButton);
    
        if (_startWith!=ALBUM_ONLY) {
    _cameraButton = SMButton::create(0, SMButton::Style::SOLID_RECT, s.width-TOP_MENU_HEIGHT+10, 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    _cameraButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xFFFFFF, 1.0f));
    _cameraButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xADAFB3, 1.0f));
    _cameraButton->setTextSystemFont("카", SMFontConst::SystemFontRegular, 40);
    _cameraButton->setOnClickListener(this);
    _topMenuView->addChild(_cameraButton);
        }
    
    
        if (_startWith!=CAMERA_ONLY) {
    _albumButton = SMButton::create(0, SMButton::Style::SOLID_RECT, s.width-TOP_MENU_HEIGHT+10, 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    _albumButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xFFFFFF, 1.0f));
    _albumButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xADAFB3, 1.0f));
    _albumButton->setTextSystemFont("앨", SMFontConst::SystemFontRegular, 40);
    _albumButton->setOnClickListener(this);
    _topMenuView->addChild(_albumButton);
    
    _groupButton = SMButton::create(0, SMButton::Style::DEFAULT, 80, 0, s.width-160, TOP_MENU_HEIGHT-20);
    _groupButton->setTextSystemFont("", SMFontConst::SystemFontRegular, 40);
    _groupButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    _groupButton->setOnClickListener(this);
            _groupButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _topMenuView->addChild(_groupButton);
        }
    
    _titleButton = SMButton::create(0, SMButton::Style::DEFAULT, 80, 0, s.width-160, TOP_MENU_HEIGHT-20);
    _titleButton->setTextSystemFont("CAMERA", SMFontConst::SystemFontRegular, 40);
    _titleButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
        _titleButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _topMenuView->addChild(_titleButton);
        _titleButton->setContentSize(cocos2d::Size(_titleButton->getTextLabel()->getContentSize().width+20, _titleButton->getContentSize().height));
        _titleButton->setPosition(cocos2d::Vec2(_topMenuView->getContentSize().width/2, TOP_MENU_HEIGHT/2-22));
            
    
    
    if (_currentPage==0) {
        // album button hidden
            if (_startWith==CAMERA_ONLY) {
                _cameraButton->setOpacity(0);
                _cameraButton->setEnabled(false);
            } else {
        _albumButton->setOpacity(0);
        _albumButton->setEnabled(false);
        _titleButton->setOpacity(0);
        _titleButton->setEnabled(false);
            }
    } else {
        // camera button hidden
        _cameraButton->setOpacity(0);
        _cameraButton->setEnabled(false);
        _groupButton->setOpacity(0);
        _groupButton->setEnabled(false);
    }
    
    _pageView->setScrollPosition(s.width*_currentPage);
    }
    }

    return true;
}

void SMImagePickerScene::onClick(SMView *view)
{
    if (view==_closeButton) {
        if (getSwipeType()==MENU) {
            
        } else if (getSwipeType()==BACK) {
            
        } else if (getSwipeType()==DISMISS) {
            
        }
        if (!_forPicker) {
        // 뭐.. 일단 닫자
        finishScene();
        }
    } else if (view==_cameraButton) {
        _pageView->jumpPage(1, _contentSize.width);
    } else if (view==_albumButton) {
        _pageView->jumpPage(0, _contentSize.width);
    } else if (view==_groupButton) {
        _imagePickerView->onGroupSelectButtonClick(view);
    }
}

void SMImagePickerScene::finishScene(Intent* intent)
{
    SMScene::finishScene();
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
        
    });    
}
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
void SMImagePickerScene::onGroupSelect(const ImageGroupInfo *group, bool immediate)
{
    if (group) {
        _groupButton->setText(group->name);
    }
}
#else
void SMImagePickerScene::onGroupSelect(const std::string& name, bool immediate)
{
    _groupButton->setText(name);
    _groupButton->setContentSize(cocos2d::Size(_groupButton->getTextLabel()->getContentSize().width+20, _groupButton->getContentSize().height));
    _groupButton->setPosition(cocos2d::Vec2(_topMenuView->getContentSize().width/2, TOP_MENU_HEIGHT/2-22));
//    float width = _groupButton->getTextLabel()->getContentSize().width;
//    auto s = cocos2d::Director::getInstance()->getWinSize();
//    _groupButton->setPosition(s.width/2-width/2, _groupButton->getPosition().y);
}
#endif

void SMImagePickerScene::onGroupSelectViewOpen(bool bOpen)
{
    if (_startWith!=ALBUM_ONLY) {
    if (bOpen) {
        // group select view closing animation
            
        _pageView->setScrollLock(true);
        _canSwipe = false;
    } else {
        // group select view opening animation
            
        _pageView->setScrollLock(false);
        _canSwipe = true;
    }
    }
}

cocos2d::Node* SMImagePickerScene::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    switch (_startWith) {
        case ALBUM:
        case CAMERA:
        {
    if (indexPath.getIndex()==0) {
        // album
        return _imagePickerView;
    } else {
        // camera
        return _captureView;
    }
        }
            break;
        case ALBUM_ONLY:
        {
            return _imagePickerView;
        }
            break;
        case CAMERA_ONLY:
        {
            return _captureView;
        }
            break;
    }
}

int SMImagePickerScene::numberOfRowsInSection(int section)
{
    // album & camera... totally 2
    if (_startWith==ALBUM_ONLY || _startWith==CAMERA_ONLY) {
        return 1;
    }
    return 2;
}

void SMImagePickerScene::onPageScroll(float position, float distance)
{
    if (position < 0) position = 0;
    else if (position > 1) position = 1;

    if (_startWith!=CAMERA_ONLY && _startWith!=ALBUM_ONLY) {
    _albumButton->setOpacity((GLubyte)(0xFF*(position)));
    _groupButton->setOpacity((GLubyte)(0xFF*(1-position)));
    _cameraButton->setOpacity((GLubyte)(0xFF*(1-position)));
    _titleButton->setOpacity((GLubyte)(0xFF*position));
    }
    
}

void SMImagePickerScene::onPagenChanged(int page)
{
    if (page==0) {
        // album page
        if (_startWith!=CAMERA_ONLY && _startWith!=ALBUM_ONLY) {
        // album button hidden
        _albumButton->setOpacity(0);
        _albumButton->setEnabled(false);
        // camera button visible
        _cameraButton->setOpacity(0xff);
        _cameraButton->setEnabled(true);
        // group button visible
        _groupButton->setOpacity(0xff);
        _groupButton->setEnabled(true);
        }
        // title button hidden
        _titleButton->setOpacity(0);
        _titleButton->setEnabled(false);

        
        _imagePickerView->askAccessPermission();
    } else {
        // camera page
        if (_startWith!=CAMERA_ONLY && _startWith!=ALBUM_ONLY) {
        // album button visible
        _albumButton->setOpacity(0xff);
        _albumButton->setEnabled(true);
        // camera button hidden
        _cameraButton->setOpacity(0);
        _cameraButton->setEnabled(false);
        // group button hidden
        _groupButton->setOpacity(0);
        _groupButton->setEnabled(false);
        }
        // title button visible
        _titleButton->setOpacity(0xff);
        _titleButton->setEnabled(true);

        _captureView->askAccessPermission();
    }
    
    _currentPage = page;
}

bool SMImagePickerScene::canSwipe(const cocos2d::Vec2 &worldPoint)
{
    return _canSwipe;
}

void SMImagePickerScene::onImageItemClick(SMView *view, const ImageItemInfo &item)
{
    // 받은 이미지로  SMImageEditorScene 호출...

    
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    CCLOG("[[[[[ img path : %s", item.url.c_str());
    // NSURL
//    ImageDownloader::getInstance().loadImageFromLocalIdentifier(this, item.url);
    ImageDownloader::getInstance().loadImagePHAsset(this, item.asset);
    
#else
    LOGD("[[[[[ img path : %s", item.url.c_str());
    // Android Local File Path
    ImageDownloader::getInstance().loadImageFromAndroidLocalFile(this, item.url);
#endif
    
    switch (_mode) {
        case Mode::PROFILE_PHOTO:
        {
            
        }
            break;
        case Mode::IMAGE_SELECT:
        {
            
        }
            break;
        default:
            break;
    }
    
}

void SMImagePickerScene::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    CCLOG("[[[[[ on image load complete");
    if (sprite) {
        goImageEditor(sprite);
    }
}

void SMImagePickerScene::goImageEditor(cocos2d::Sprite *sendSprite)
{

    
    if (_forPicker) {
    
        if (_needDraw) {
            // go draw scene;
    auto rt = cocos2d::RenderTexture::create(sendSprite->getContentSize().width, sendSprite->getContentSize().height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->beginWithClear(1, 1, 1, 1);
    sendSprite->setAnchorPoint(cocos2d::Vec2::ZERO);
    sendSprite->setPosition(cocos2d::Vec2::ZERO);
    sendSprite->visit();
    rt->end();
    
    _director->getRenderer()->render();
 
    
    auto sendImage = rt->newImage();

    Intent * intent = Intent::create();
            intent->putRef("EDIT_IMAGE", sendImage);
    intent->putBool("FROM_PICKER", true);
            intent->putInt("CALL_INDEX", _callIndex);
            
            // draw로 바로 가지말고 회전시켜야 한다.

//            auto scene = SMImageEditorDrawScene::create(intent, SMScene::SwipeType::NONE);
            auto scene = SMImageEditorRotateScene::create(intent, SMScene::SwipeType::NONE);
            scene->setOnImageSelectedListener(_listener);
            auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
            _director->replaceScene(pScene);
            CC_SAFE_RELEASE_NULL(sendImage);
        } else {
            if (_listener) {
                _listener->onImageSelected(_callIndex, sendSprite);
            }
            
            //        CC_SAFE_RELEASE_NULL(sendImage);
            
            closeImagePicker();
            
        }
        return;
    }

    CCLOG("[[[[[ go Image Editor!!!!!");
    auto rt = cocos2d::RenderTexture::create(sendSprite->getContentSize().width, sendSprite->getContentSize().height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->beginWithClear(1, 1, 1, 1);
    sendSprite->setAnchorPoint(cocos2d::Vec2::ZERO);
    sendSprite->setPosition(cocos2d::Vec2::ZERO);
    sendSprite->visit();
    rt->end();
    
    _director->getRenderer()->render();
 
    
    auto sendImage = rt->newImage();

    Intent * intent = Intent::create();
    intent->putRef("INTENT_IMAGE", sendImage);
    intent->putBool("FROM_PICKER", true);

    auto scene = SMImageEditorScene::create(intent, SMScene::SwipeType::BACK);
    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
    _director->pushScene(pScene);
    
    CC_SAFE_RELEASE_NULL(sendImage);
}

void SMImagePickerScene::onVideoCaptured(cocos2d::Data* capturedData, std::string tempUrl)
{
    // 아무것도 안함
    
    CCLOG("[[[[[ on video captured");
}

void SMImagePickerScene::onDetectedScanCode(int type, std::string codeResult)
{
//    CCLOG("[[[[[ SMImagePickerScene::onDetectedScanCode : %s", codeResult.c_str());
    if (_qrListener && _sendQRBarCodeResult==false) {
        _strQRCodeResult = codeResult;
        _sendQRBarCodeResult = true;
        _qrListener->onQRBarCodeResult(codeResult);
    }

    // 받은 넘이 닫게 하자
//    if (cocos2d::Director::getInstance()->getPreviousScene()) {
//        closeImagePicker();
//    }
}

void SMImagePickerScene::onStillImageCaptured(cocos2d::Sprite *captureImage, float x, float y, const std::string tempUrl)
{
    // 캡쳐된 이미지로 SMImageEditorScene 호출...
    goImageEditor(captureImage);
    
    switch (_mode) {
        case Mode::PROFILE_PHOTO:
        {
            
        }
            break;
        case Mode::IMAGE_SELECT:
        {
            
        }
            break;
        default:
            break;
    }

}

void SMImagePickerScene::startProfileCropScene(SMView *view, const std::string &imagePath, int width, int height, int orient, int from)
{
    // 선택된 이미지로 프로필 만들때...
    // 아직 구현 안함.
}

void SMImagePickerScene::startImageSelectScene(SMView *view, const std::string &imagePath, int with, int height, int orient, int from)
{
    // 선택된 이미지를 그냥 넘길때...
    // 아직 구현 안함.
}

void SMImagePickerScene::onTransitionStart(const SMScene::Transition t, const int flag)
{
    // 선택된 이미지로 scene 전환
    
}

void SMImagePickerScene::onTransitionComplete(const SMScene::Transition t, const int flag)
{
}

void SMImagePickerScene::onSceneResult(SMScene *fromScene, Intent *result)
{
}

void SMImagePickerScene::closeImagePicker()
{
    if (_startWith!=ALBUM_ONLY && _startWith!=EDIT_ONLY) {
    _captureView->stopCamera();
    }
 
    if (_sendQRBarCodeResult) {
        if (_sendQRBarCodeResult) {
            Intent * intent = Intent::create();
            intent->putString("QRBarCode", _strQRCodeResult);
            intent->putInt("CALLER_INDEX", _callIndex);
            this->setSceneResult(intent);
    }
        
        auto pScene = SceneTransition::SlideOutToBottom::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
        _director->popSceneWithTransition(pScene);
    } else {
        _director->getScheduler()->performFunctionInCocosThread([&]{
            auto pScene = SceneTransition::SlideOutToBottom::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
        });
    }
    
}

void SMImagePickerScene::goDrawDirect(float t)
{
    if (isScheduled(schedule_selector(SMImagePickerScene::goDrawDirect))) {
        unschedule(schedule_selector(SMImagePickerScene::goDrawDirect));
    }

    _director->getScheduler()->performFunctionInCocosThread([&]{
        auto rt = cocos2d::RenderTexture::create(800, 600, cocos2d::Texture2D::PixelFormat::RGBA8888);
        rt->beginWithClear(1, 1, 1, 1);
        rt->end();
        
        _director->getRenderer()->render();
        
        
        auto sendImage = rt->newImage();
        
        Intent * intent = Intent::create();
        intent->putRef("EDIT_IMAGE", sendImage);
        intent->putBool("FROM_PICKER", true);
        intent->putInt("CALL_INDEX", _callIndex);
        
//        auto prevScene = _director->getPreviousScene();
        auto scene = SMImageEditorDrawScene::create(intent, SMScene::SwipeType::NONE);
        scene->setOnImageSelectedListener(_listener);
        auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
        _director->replaceScene(pScene);

        CC_SAFE_RELEASE_NULL(sendImage);
    });
}
