//
//  CaptureView.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 3..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef CaptureView_h
#define CaptureView_h

// Still image 또는 video를 capture한다.
// preview layer를 구현하고
// still camera contol (shutter, flash, front/back, focusing)
// video control (record/stop, rewind, forward, flash (level),
// 일단 still image capture만 하자.
// video는 다음에 시간나면 하는 걸로...

/*
 viewer : container, previewer, control layer,
 interface : preview start/stop, capture(still/video), stop(video), auto focus, flash(on/off/level), front/back, capture image/video output listener
 */

#include "../Base/SMView.h"
#include "../Base/ShaderNode.h"
#include "../DeviceAsset/DeviceCamera.h"
#include "../Base/SMImageView.h"
#include <string>
#include <cocos2d.h>

// 이런게 있다고 치고... interface만 있고 구현은 각각의 os에서(cpp는 Android, mm은 ios)
class DeviceCamera;

// shutter, front/back, flash, record/stop, rewind/forward,
class CameraControlLayer;
class FocusingView;

// pageView에 넣기위해
class SMPageView;

class Intent;


class CameraInterface {
public:
    enum class STATUS {
        SHUTTER_PRESSED,
        SHUTTER_RELEASED,
        SHUTTER_CLICKED,
        FRONTBACK_CLICKED,
        FLASH_CLICKED,
        RECORD_CLICKED,
        STOP_CLICKED,
        REWIND_CLICKED,
        FORWARD_CLICKED,
    } ;
public:
    virtual void onCameraInterface(SMView * view, STATUS status) = 0;
};


class OnFocusingListener {
public:
    virtual bool onFocusingInLayer(const cocos2d::Vec2& touchPoint, cocos2d::Vec2& focusPoint) = 0;
};


class OnCaptureListener {
public:
    virtual void onStillImageCaptured(cocos2d::Sprite * captureImage, float x, float y, const std::string tempUrl) = 0;
    virtual void onVideoCaptured(cocos2d::Data* capturedData, std::string tempUrl) = 0;
    virtual void onDetectedScanCode(int type, std::string codeString) = 0;
};


class CaptureView : public SMView,
                                public CameraDevice,
                                public CameraInterface,
                                public OnFocusingListener,
                                public OnCodeScannerListener
{
public:
    static void showAccessDeniedPrompt();
    static CaptureView* createForQRBarCode();
    static CaptureView* create(bool cropSquare = true);
    void setOnCaptureListener(OnCaptureListener * l);
    void attachPageView(SMPageView * pageView);
    
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void cleanup() override;
    
    virtual void initCameraDevice() override;

    virtual void onDetectedCode(std::vector<cocos2d::Vec2> pts, int type, std::string result) override;
    
    void startCamera(int cameraId=0);
    void stopCamera();

protected:
    virtual void onCameraNotify(CameraDeviceConst::Notify notify, Intent* intent) override;
    
    // camera interface
    virtual void onCameraInterface(SMView * view, STATUS status) override;
    
    // focusing listener
    virtual bool onFocusingInLayer(const cocos2d::Vec2& touchPoint, cocos2d::Vec2& focusPoint) override;
    
    virtual void onFrameRendered(VideoFrame * frame) override;
    

private:
    void onTextureUpdate(float dt);
    
private:
    class CrossFadeAction;
    
    CaptureView();
    virtual ~CaptureView();
    
    bool initWithCrop(bool cropSquare);
    
    // 2장으로 돌린다.
    cocos2d::Sprite * _previewSprite[2];
    
    cocos2d::Texture2D * _previewTexture[2];
    
    cocos2d::Node * _previewStub;
    
    CrossFadeAction * _crossFadeAction;
    
    void cameraFirstFrameReceived(int camerId, const cocos2d::Size& cropSize, const cocos2d::Size& fullSize);
    
    void captureStillImageComplete(cocos2d::Image* image, const std::string& cacheFilePath);
    
private:
    // camera id (front, back, , , , )
    int _cameraId;
    bool _cameraHasFlash;
    bool _cameraHasFrontFacing;
    bool _cameraSupportFocus;
    cocos2d::Rect _rectPreview;
    
    CameraControlLayer * _controlLayer;
    FocusingView * _focusView;
    SMPageView * _pageView;
    int _currentPreview;
    // on/off
    int _flashState[2];
    int _saveCameraId;
    
    OnCaptureListener * _listener;
    bool _cropSquare;
    
    cocos2d::Label * _tempCode;
    
    ShapeRoundLine * _ltrt;
    ShapeRoundLine * _rtrb;
    ShapeRoundLine * _rblb;
    ShapeRoundLine * _lblt;
    
    SMImageView * _urlContentView;
};





#endif /* CaptureView_h */
