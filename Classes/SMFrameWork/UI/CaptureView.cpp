//
//  CaptureView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 3..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "CaptureView.h"
#include "CameraControlLayer.h"
#include "FocusingView.h"
#include "../Base/SMPageView.h"
#include "../Base/SMButton.h"
#include "../Base/ShaderNode.h"
#include "../Base/ShaderUtil.h"
#include "../Util/ViewUtil.h"
#include "../Util/StringUtil.h"
#include "../Util/OSUtil.h"
#include "../Base/ViewAction.h"
#include "../Base/Intent.h"
#include "../Const/SMFontColor.h"
#include "../UI/AlertView.h"
#include <base/CCUserDefault.h>
#include <cmath>
#include "../Util/ImageDownloader.h"
#include "../Util/cvImageUtil.h"
#include "../Const/SMViewConstValue.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define ACTION_TAG_CROSSFADE (SMViewConstValue::Tag::USER+1)

#define USERDEFAULT_KEY_CAMERA_0_FLASH  ("camera_0_flash")
#define USERDEFAULT_KEY_CAMERA_1_FLASH  ("camera_1_flash")
#define USERDEFAULT_KEY_CAMERA_ID       ("camera_id")

#define CONTROL_LAYER_MIN_HEIGHT    (230)
#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define FRAME_SPEED (30.0f)

class CaptureView::CrossFadeAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(CrossFadeAction);
    
    virtual void onStart() override {
        if (_inSprite) {
            _inSprite->setOpacity(0);
        }
        if (_outSprite) {
            _from = _outSprite->getOpacity();
        }
    }
    
    virtual void onUpdate(float t) override {
        if (_inSprite) {
            _inSprite->setOpacity((GLubyte)(0xFF*std::sin(M_PI_2*t)));
        }
        if (_outSprite) {
            float opacity = ViewUtil::interpolation(_from, 0, t);
            _outSprite->setOpacity((GLubyte)opacity);
        }
    };
    
    virtual void onEnd() override {
        auto view = (CaptureView*)_target;
        if (_outSprite) {
            view->_previewStub->removeChild(_outSprite);
            view->_previewSprite[1-_index] = nullptr;
            view->_previewTexture[1-_index] = nullptr;
        }
    }
    
    void setValue(int index, cocos2d::Sprite* inSprite, cocos2d::Sprite* outSprite) {
        setTimeValue(0.25, 0);
        _inSprite = inSprite;
        _outSprite = outSprite;
        _index = index;
    }
    
private:
    cocos2d::Sprite* _inSprite;
    cocos2d::Sprite* _outSprite;
    float _from;
    int _index;
};

CaptureView::CaptureView() :
_pageView(nullptr)
, _crossFadeAction(nullptr)
, _cropSquare(false)
, _listener(nullptr)
, _tempCode(nullptr)
, _ltrt(nullptr)
, _rtrb(nullptr)
, _rblb(nullptr)
, _lblt(nullptr)
, _urlContentView(nullptr)
{
    
}

CaptureView::~CaptureView()
{
    releaseCameraDevice();
    CC_SAFE_RELEASE(_crossFadeAction);
}

CaptureView* CaptureView::createForQRBarCode()
{
    CaptureView * view = new (std::nothrow)CaptureView();
    if (view!=nullptr) {
        view->_forQRBarCode = true;
        if (view->initWithCrop(false)) {
            
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

CaptureView* CaptureView::create(bool cropSquare)
{
    CaptureView * view = new (std::nothrow)CaptureView();
    if (view!=nullptr) {
        if (view->initWithCrop(cropSquare)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    return view;
}

void CaptureView::initCameraDevice()
{
    CameraDevice::initCameraDevice();
    
    if (_forQRBarCode) {
        _camera->setOnCodeScannerListener(this);
    }
}

bool CaptureView::initWithCrop(bool cropSquare)
{
    if (!SMView::init()) {
        return false;
    }
    
    _cropSquare = cropSquare;
    
    // Camera view는 일단 전체화면
    auto s = _director->getWinSize();
    setContentSize(s);
    
    // buffering을 할 sprite, texture를 초기화
    _previewSprite[0] = nullptr;
    _previewSprite[1] = nullptr;
    _previewTexture[0] = nullptr;
    _previewTexture[1] = nullptr;
    _currentPreview = 0;
    
    // device camera를 초기화
    initCameraDevice();
    
    float layerHeight;
    bool needCropMask = false;
    
    if (_forQRBarCode) {
        _rectPreview = cocos2d::Rect(0, 0, s.width, s.height);
        _camera->setCropRatio(CameraDeviceConst::Crop::NONE);
        
    } else {
    // preview를 일단 4:3 비율에 맞춘다.
    if (s.height - (CONTROL_LAYER_MIN_HEIGHT+TOP_MENU_HEIGHT) > 4 * s.width / 3) {
        // 4:3 비율보다 큰거면 4:3에 맞게
        float previewWidth = s.width;
        float previewHeight = 4*s.width/3;
        layerHeight = s.height - (previewHeight + TOP_MENU_HEIGHT);
        
        _rectPreview = cocos2d::Rect(0, layerHeight, previewWidth, previewHeight);
        
        //if (_cropSquare) {
            // 정사각형이면 1:1로
            _camera->setCropRatio(CameraDeviceConst::Crop::RATIO_1_1);
            needCropMask = true;
        //} else {
        //    _camera->setCropRatio(CameraDeviceConst::Crop::RATIO_4_3);
        //}
    } else {
        // 아이패드 같이 애매한 사이즈면 1:1 사이즈로...
        float previewWidth = s.width;
        float previewHeight = s.width;  // 1:1
        layerHeight = s.height - (previewHeight+TOP_MENU_HEIGHT);
        
        _rectPreview = cocos2d::Rect(0, previewHeight, previewWidth, previewHeight);
        _camera->setCropRatio(CameraDeviceConst::Crop::RATIO_1_1);
    }

    }
    

    needCropMask = false;
    
    auto bg = ShapeSolidRect::create();
    bg->setContentSize(cocos2d::Size(_rectPreview.size.width*1.3, _rectPreview.size.height));
    bg->setPosition(_rectPreview.origin);
    bg->setColor4F(SMColorConst::COLOR_F_222222);
    addChild(bg);

    _previewStub = cocos2d::Node::create();
    addChild(_previewStub);

    _focusView = FocusingView::create(this);
    _focusView->setVisible(false);
    addChild(_focusView);
    
    // 정사각형인 경우 위 아래를 덮자.
    if (needCropMask) {
        float maskHeight = (s.height - (TOP_MENU_HEIGHT + layerHeight + _rectPreview.size.width))/2;
        // 위
        auto mask = ShapeSolidRect::create();
        mask->setContentSize(cocos2d::Size(s.width*1.3, maskHeight));
        mask->setColor4F(MAKE_COLOR4F(0, 0.8));
        mask->setPosition(0, s.height-TOP_MENU_HEIGHT-maskHeight);
        addChild(mask);
        
        // 아래
        mask = ShapeSolidRect::create();
        mask->setContentSize(cocos2d::Size(s.width*1.3, maskHeight));
        mask->setColor4F(MAKE_COLOR4F(0, 0.8));
        mask->setPosition(0, layerHeight);
        addChild(mask);
    }

    if (_forQRBarCode) {
        layerHeight = 0;
    } else {
    _controlLayer = CameraControlLayer::create(cocos2d::Size(s.width, layerHeight), this);
    addChild(_controlLayer);
    }

    
    
    _cameraId = CameraDeviceConst::Facing::UNSPECIFIED;
    _cameraHasFlash = false;
    _cameraHasFrontFacing = false;
    
    // camera 상태값을 읽어온다... user default를 쓰자...
    auto userDefault = cocos2d::UserDefault::getInstance();
    _saveCameraId = userDefault->getIntegerForKey(USERDEFAULT_KEY_CAMERA_ID, CameraDeviceConst::Facing::UNSPECIFIED);
    _flashState[CameraDeviceConst::Facing::BACK] = userDefault->getIntegerForKey(USERDEFAULT_KEY_CAMERA_0_FLASH, CameraDeviceConst::Flash::AUTO);
    _flashState[CameraDeviceConst::Facing::FRONT] = userDefault->getIntegerForKey(USERDEFAULT_KEY_CAMERA_1_FLASH, CameraDeviceConst::Flash::AUTO);
    
    return true;
}

void CaptureView::cleanup()
{
    SMView::cleanup();
    
    auto userDefault = cocos2d::UserDefault::getInstance();
    userDefault->setIntegerForKey(USERDEFAULT_KEY_CAMERA_ID, _cameraId);
    userDefault->setIntegerForKey(USERDEFAULT_KEY_CAMERA_0_FLASH, _flashState[CameraDeviceConst::Facing::BACK]);
    userDefault->setIntegerForKey(USERDEFAULT_KEY_CAMERA_1_FLASH, _flashState[CameraDeviceConst::Facing::FRONT]);
    userDefault->flush();
}

void CaptureView::setOnCaptureListener(OnCaptureListener *l)
{
    _listener = l;
}

void CaptureView::attachPageView(SMPageView *pageView)
{
    _pageView = pageView;
}

void CaptureView::onEnter()
{
    SMView::onEnter();
    
    int cameraId = _cameraId;
    
    if (cameraId != CameraDeviceConst::Facing::BACK && cameraId != CameraDeviceConst::Facing::FRONT) {
        cameraId = _saveCameraId;
    }
    
    if (cameraId != CameraDeviceConst::Facing::BACK && cameraId != CameraDeviceConst::Facing::FRONT) {
        cameraId = CameraDeviceConst::Facing::BACK;
    }
    
    if (cameraId == CameraDeviceConst::Facing::FRONT && !_camera->hasFrontFacingCamera()) {
        cameraId = CameraDeviceConst::Facing::BACK;
    }
    
    if (_forQRBarCode) {
        cameraId = _saveCameraId = CameraDeviceConst::Facing::BACK;
    }
    
    // 화면에 나타날때면 해당 camera를 시작한다.
    startCamera(cameraId);
}

void CaptureView::onExit()
{
    SMView::onExit();

    stopCamera();
}

// camera에서 noti가 왔음.
void CaptureView::onCameraNotify(CameraDeviceConst::Notify notify, Intent *intent)
{
    switch (notify) {
        case CameraDeviceConst::Notify::START :
        {
            // START일 때 intent : "CAMERA_ID" : int
            CCLOG("[[[[[ camera start");
        }
            break;
        case CameraDeviceConst::Notify::STOP:
        {
            CCLOG("[[[[[ camera stop");
            // 업데이트 하던거 멈춤
            if (isScheduled(schedule_selector(CaptureView::onTextureUpdate))) {
                unschedule(schedule_selector(CaptureView::onTextureUpdate));
            }
        }
            break;
        case CameraDeviceConst::Notify::SNAP:
        {
            // 찰칵~~
            if (intent) {
                // SNAP일 때 intent : "IMAGE" : cocos2d::Image, "CACHE_FILE_PATH" : std::string
                cocos2d::Image * image = (cocos2d::Image*)intent->getRef("IMAGE");
                std::string cachePath = intent->getString("CACHE_FILE_PATH");
                captureStillImageComplete(image, cachePath);
            }
        }
            break;
        case CameraDeviceConst::Notify::FIRST:
        {
            // 처음 preview 한 장
            int cameraID = intent->getInt("CAMERA_ID");
            cocos2d::Size cropSize(intent->getFloat("CROP_WIDTH"), intent->getFloat("CROP_HEIGHT"));
            cocos2d::Size fullSize(intent->getFloat("FULL_WIDTH"), intent->getFloat("FULL_HEIGHT"));
            cameraFirstFrameReceived(cameraID, cropSize, fullSize);
        }
            break;
        case CameraDeviceConst::Notify::DENY:
        {
            // camera 접근이 거부됨
            // prompt 보여줘야지...
            CCLOG("[[[[[ camera access deny");
            showAccessDeniedPrompt();
        }
            break;
        default:
            break;
    }
}

void CaptureView::cameraFirstFrameReceived(int camerId, const cocos2d::Size &cropSize, const cocos2d::Size &fullSize)
{
    // preivew 처음 한장이 들어왔을때 (첫 기동시 또는 앞/뒤 전환시)
    bool isNewCamera = false;
    if (_cameraId!=camerId) {
        // 바뀐 카메라냐???
        _cameraId = camerId;
        _cameraHasFlash = _camera->hasFlash();
        _cameraSupportFocus = true;
        _cameraHasFrontFacing = _camera->hasFrontFacingCamera();
        if (!_forQRBarCode) {
        _controlLayer->setDeviceInfo(_cameraId, _cameraHasFlash, _cameraHasFrontFacing);
        }
        
        isNewCamera = true;
    }
    
    _currentPreview = (_currentPreview+1) % 2; // 0이면 1, 1이면 0 (앞/뒤 camera 구별)
    
    if (_previewSprite[_currentPreview]) {
        _previewStub->removeChild(_previewSprite[_currentPreview]);
        _previewSprite[_currentPreview] = nullptr;
        _previewTexture[_currentPreview] = nullptr;
    }
    
    _previewTexture[_currentPreview] = new cocos2d::Texture2D;
    _camera->updateTexture(_previewTexture[_currentPreview]);
    _previewSprite[_currentPreview] = cocos2d::Sprite::createWithTexture(_previewTexture[_currentPreview]);
    CC_SAFE_RELEASE(_previewTexture[_currentPreview]);
    
    auto sprite = _previewSprite[_currentPreview];
    // ios의 경우 bgr로 들어오니 rgb로 바꾼다. ... android는 확인 해야함.
    float scale = _contentSize.width / sprite->getContentSize().height;
    float rotate;
    bool isMirror;

    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    if (_cameraId == 1) {
        rotate = 90;
        isMirror = true;
        ShaderUtil::setBilateralShader(sprite, false);
    } else {
        rotate = 90;
        isMirror = false;
    }
    #else
    if (_cameraId == 1) {
        rotate = 270;
        isMirror = true;
        ShaderUtil::setBilateralShader(sprite, true);
    } else {
        rotate = 90;
        isMirror = false;
        ShaderUtil::setBgr2RgbShader(sprite);
    }
    #endif

    sprite->setScale(isMirror?-scale:scale, scale);
    sprite->setRotation(rotate);
    sprite->setOpacity(0);
    sprite->setPosition(_rectPreview.getMidX(), _rectPreview.getMidY());
    _previewStub->addChild(sprite);

    if (isNewCamera) {
        // 바뀐 camera면 focusing 한번 해준다.
        if (_cameraSupportFocus) {
            // 물론 지원되는거만...
            _focusView->setFocusLayer(_rectPreview.getMidX(), _rectPreview.getMidY(), scale, isMirror, fullSize);
            _focusView->setVisible(true);
        } else {
            // focus를 지원하지 않네..
            _focusView->setVisible(false);
        }
    }
    
    // 스르륵 나타나게
    auto action = getActionByTag(ACTION_TAG_CROSSFADE);
    if (action) {
        // 뭔가 action 중이었다면 일단 멈추고
        stopAction(action);
    }
    
    if (_crossFadeAction==nullptr) {
        _crossFadeAction = CrossFadeAction::create(false);
        _crossFadeAction->setTag(ACTION_TAG_CROSSFADE);
    }
    
    _crossFadeAction->setValue(_currentPreview, _previewSprite[_currentPreview], _previewSprite[1-_currentPreview]);
    runAction(_crossFadeAction);

    // 화면 갱신 시작
    schedule(schedule_selector(CaptureView::onTextureUpdate), 1.0/FRAME_SPEED); // 일단 30 frame
    
    // focusing 가능 상태
    if (!_forQRBarCode) {
    _controlLayer->enableControl(true, false);
    }
    _focusView->setEnableFocus(true);
}

// 찰칵 했냐???
void CaptureView::captureStillImageComplete(cocos2d::Image *image, const std::string &cacheFilePath)
{
    if (image!=nullptr) {
        if (_listener) {
            // 전달할 넘이 있을 때만
            auto texture = new cocos2d::Texture2D();
            
            if (texture->initWithImage(image)) {
                auto sprite = cocos2d::Sprite::createWithTexture(texture);
                sprite->setPosition(_rectPreview.getMidX(), _rectPreview.getMidY());
                // sprite 전달
                _listener->onStillImageCaptured(sprite, _rectPreview.getMidX(), _rectPreview.getMidY(), "");
                
                stopCamera();
            }
            
            CC_SAFE_RELEASE(texture);
        }
    }
    
    // focusing 가능 상태
    if (!_forQRBarCode) {
    _controlLayer->enableControl(true, false);
    }
    _focusView->setEnableFocus(true);
}

void CaptureView::onTextureUpdate(float dt)
{
    if (_previewTexture[_currentPreview]) {
        auto texture = _previewTexture[_currentPreview];
        // texture를 업데이트 한다.
        _camera->updateTexture(texture);
    }
}

void CaptureView::onCameraInterface(SMView *view, CameraInterface::STATUS status)
{
    switch (status) {
        case CameraInterface::STATUS::SHUTTER_PRESSED:
        {
            // shutter가 눌리면 page view를 잠궈~~~ 물론 있을때만
            if (_pageView) {
                _pageView->setScrollLock(true);
            }
        }
            break;
        case CameraInterface::STATUS::SHUTTER_RELEASED:
        {
            // shutter가 release 되면 page view를 푼다~~~ 물론 있을때만
            if (_pageView) {
                _pageView->setScrollLock(false);
            }
        }
            break;
        case CameraInterface::STATUS::SHUTTER_CLICKED:
        {
            // 찰칵~~~
            if (_cameraId != CameraDeviceConst::Facing::BACK && _cameraId != CameraDeviceConst::Facing::FRONT) {
                // camera id가 이상하다....넘어가자.
                return;
            }
            
            // 현재 flash 상태로 capture를 진행한다. camera 멈춤은 이미지처리 다 끝내고 할거니까 여기서는 false
            _camera->capture(_flashState[_cameraId], false);
            
            // focusing이 불가능한 상태로
            _controlLayer->enableControl(false, false);
            _focusView->setEnableFocus(false);
        }
            break;
        case CameraInterface::STATUS::FRONTBACK_CLICKED:
        {
            // camera 앞/뒤 switching
            _camera->switchCamera();
            // 바뀌자 마자는 focusing 불가능
            _controlLayer->enableControl(false, false);
            _focusView->setEnableFocus(false);
        }
            break;
        case CameraInterface::STATUS::FLASH_CLICKED:
        {
            // flash 변경
            if (!_cameraHasFlash) {
                // flash 없으면 패스~
                return;
            }
            
            if (_cameraId!=CameraDeviceConst::Facing::BACK && _cameraId!=CameraDeviceConst::Facing::FRONT) {
                // camera id가 이상하다...
                return;
            }
            
            // flash 상태 변경 순서 auto -> on -> off -> auto
            int newFlashState = CameraDeviceConst::Flash::OFF;
            switch (_flashState[_cameraId]) {
                case CameraDeviceConst::Flash::AUTO:
                {
                    // auto면 on으로
                    newFlashState = CameraDeviceConst::Flash::ON;
                }
                    break;
                case CameraDeviceConst::Flash::ON:
                {
                    // on이면 off로
                    newFlashState = CameraDeviceConst::Flash::OFF;
                }
                    break;
                case CameraDeviceConst::Flash::OFF:
                {
                    // off면 auto로
                    newFlashState = CameraDeviceConst::Flash::AUTO;
                }
                    break;
            }
            // 새로운 flash state 값을 설정하고
            _flashState[_cameraId] = newFlashState;
            // control layer의 flash icon 변경
            _controlLayer->setFlashState(newFlashState);
        }
            break;
            
        default:
        {
            // video 관련은 나중에 구현
        }
            break;
    }
}

bool CaptureView::onFocusingInLayer(const cocos2d::Vec2 &touchPoint, cocos2d::Vec2 &focusPoint)
{
    if (_cameraId!=CameraDeviceConst::Facing::BACK && _cameraId!=CameraDeviceConst::Facing::FRONT) {
        // camera id 가 이상하다.
        return false;
    }
    
    // layer에서 touch가 들어오면 focusing  한다.
    return _camera->autoFocusAtPoint(focusPoint);
}

void CaptureView::showAccessDeniedPrompt()
{
    // iOS의 경우 설정으로 이동
    // Android도... 이동???
    AlertView::showConfirm("카메라 접근 권한 없음", "카메라 접근 권한이 필요합니다.", "닫기", "설정하기", [&]{
        OSUtil::openDeviceSettings();
    }, [&]{
        
    });
    CCLOG("[[[[[ Access deny");
}

void CaptureView::onDetectedCode(std::vector<cocos2d::Vec2> pts, int type, std::string result)
{
    if (_forQRBarCode) {
        
        if (_cameraId==CameraDeviceConst::Facing::FRONT) {
            return;
        }

        // for test
        if (_tempCode==nullptr) {
            _tempCode = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontRegular, 70);
            _tempCode->setTextColor(MAKE_COLOR4B(0xffffffff));
            addChild(_tempCode);
            _tempCode->setLocalZOrder(999);
        }
        
        if (_ltrt==nullptr) {
            _ltrt = ShapeRoundLine::create();
            _ltrt->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4.0f);
            _ltrt->setColor4F(MAKE_COLOR4F(0x33ff99, 1.0f));
            _previewSprite[1]->addChild(_ltrt);
            
            _rtrb = ShapeRoundLine::create();
            _rtrb->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4.0f);
            _rtrb->setColor4F(MAKE_COLOR4F(0x33ff99, 1.0f));
            _previewSprite[1]->addChild(_rtrb);

            _rblb = ShapeRoundLine::create();
            _rblb->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4.0f);
            _rblb->setColor4F(MAKE_COLOR4F(0x33ff99, 1.0f));
            _previewSprite[1]->addChild(_rblb);

            _lblt = ShapeRoundLine::create();
            _lblt->setLineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*4.0f);
            _lblt->setColor4F(MAKE_COLOR4F(0x33ff99, 1.0f));
            _previewSprite[1]->addChild(_lblt);

        }

        cocos2d::Vec2 lt = pts[0];
        cocos2d::Vec2 rt = pts[1];
        cocos2d::Vec2 rb = pts[2];
        cocos2d::Vec2 lb = pts[3];
        
        
        _ltrt->line(lt, rt);

        _rtrb->line(rt, rb);

        _rblb->line(rb, lb);

        _lblt->line(lb, lt);


        std::string title = cocos2d::StringUtils::format("[%d]%s", type, result.c_str());
        _tempCode->setString(title);
        _tempCode->setPosition(getContentSize()/2);

        auto s = cocos2d::Director::getInstance()->getWinSize();
        std::string fileExtension = cocos2d::FileUtils::getInstance()->getFileExtension(result);
        
        fileExtension = StringUtil::toLower(fileExtension);
        
        bool bVisibleImage = false;
        if (fileExtension==".jpg" || fileExtension==".png") {
            bVisibleImage = true;
        }

        if (_urlContentView==nullptr) {
            _urlContentView = SMImageView::createWithDownloadImage(result, 0, s.height-200, 200, 200);
            _urlContentView->setAnchorPoint(cocos2d::Vec2::ZERO);
            addChild(_urlContentView);
        }

        _urlContentView->setImagePath(bVisibleImage?result:"");
        _urlContentView->setVisible(bVisibleImage);

        if (_listener) {
            std::string codeResult = result;
            _listener->onDetectedScanCode(type, codeResult);
        }
        
        

    }
}

void CaptureView::onFrameRendered(VideoFrame* frame)
{
    return;
    if (_forQRBarCode) {
//        CCLOG("[[[[[ capture view frame rendered >>>> length : %d, width : %d, height : %d", (int)frame->length, frame->width, frame->height);
//        frame = nullptr;
//        frame->length = 0;
        
        cv::Mat img = cvImageUtil::createCvMatFromRaw(frame->buffer, frame->width, frame->height, 4);
        
        free(frame->buffer);
//        CCLOG("[[[[[ on frame update opencv image : %d, %d", img.rows, img.cols);
        
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_RGBA2BGRA);
        img.release();
        
//        cv::cvtColor(gray, gray, cv::COLOR_GRAY2RGBA);
        auto ccImage = cvImageUtil::cvMat2ccImage(gray);
        gray.release();
        
        frame->buffer = (uint8_t*)malloc(ccImage->getDataLen());
        frame->length = ccImage->getDataLen();
        frame->width = (int)ccImage->getWidth();
        frame->height = (int)ccImage->getHeight();

        memcpy(frame->buffer, ccImage->getData(), ccImage->getDataLen());

        // ccImage is already autorelease
    }
}

void CaptureView::startCamera(int cameraId)
{
    if (_camera) {
        _camera->startCamera(cameraId);
    }
}

void CaptureView::stopCamera()
{
    if (_camera) {
        // 화면에서 없어질때라면 camera를 꺼야지..
        _camera->stopCamera();
        
        // 업데이트 하던거 멈춘다.
        if (isScheduled(schedule_selector(CaptureView::onTextureUpdate))) {
            unschedule(schedule_selector(CaptureView::onTextureUpdate));
        }
    }
}
