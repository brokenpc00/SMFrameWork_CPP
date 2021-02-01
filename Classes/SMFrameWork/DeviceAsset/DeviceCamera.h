//
//  DeviceCamera.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//  실제 Device의 Camera를 Control하는 class
//  Android용 -> cpp
// iOS용 -> mm
// header에는 interface만 정의한다.

#ifndef DeviceCamera_h
#define DeviceCamera_h

#include <platform/CCImage.h>
#include <renderer/CCTexture2D.h>
#include <memory>
#include <vector>
#include "CameraConstant.h"
#include "../Base/Intent.h"


// 카메라를 컨트롤 하기 위한 클래스...
class CameraPlayer;

class OnFrameRenderListener {
public:
    virtual void onFrameRendered(VideoFrame* frame) = 0;
};

class OnCodeScannerListener {
public:
    virtual void onDetectedCode(std::vector<cocos2d::Vec2> pts, int type, std::string result) = 0;
};

// 카메라 장치 & 이벤트 리스너 클래스
class CameraDevice : public OnFrameRenderListener
{
public:
    // 카메라 동작을 알리는 리스너 함수
    // START일 때 intent : "CAMERA_ID" : int
    // SNAP일 때 intent : "IMAGE" : cocos2d::Image, "CACHE_FILE_PATH" : std::string
    // FIRST일 때 Intent : "CAMERA_ID" : int, "CROP_WIDTH" : float, "CROP_HEIGHT" : float, "FULL_WIDTH" : float, "FULL_HEIGHT" : float
    virtual void onCameraNotify(CameraDeviceConst::Notify notify, Intent * intent) = 0;
    
    virtual void onFrameRendered(VideoFrame* frame) override;
    
public:
    CameraDevice() : _forQRBarCode(false) {};
    virtual ~CameraDevice(){};
    // 카메라 접근이 가능한지...
    static bool canAccessible();
    
    // 카메라 접근 권한을 물어본다.
    void askAccessPermission();
    
    // 카메라 초기화
    virtual void initCameraDevice();
    
    // 카메라 종료
    void releaseCameraDevice();
    
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    // jni를 통해 frame이 들어왔다.
    void onFrameReceiveFromJNI(uint8_t *buffer, int length, int preWidth, int preHeight);
    void onPictureTakenFromJNI(uint8_t *buffer, int length, int width, int height);
    #endif
    
protected:
    std::shared_ptr<CameraPlayer> _camera;
    std::shared_ptr<bool> _alive;
    friend class CameraPlayer;
    bool _forQRBarCode;
};

// 카메라를 컨트롤 하기 위한 클래스...
class CameraPlayer : public std::enable_shared_from_this<CameraPlayer>
{
public:
    static std::shared_ptr<CameraPlayer> create(CameraDevice* client);
    
    CameraPlayer();
    virtual ~CameraPlayer();
    
    void startCamera(int cameraId = CameraDeviceConst::Facing::BACK);
    void stopCamera();
    bool switchCamera();
    
    
    bool capture(int flashState, bool stopAfterCapture=true);
    bool autoFocusAtPoint(const cocos2d::Vec2& point);
    bool isFrameReceived();
    bool updateTexture(cocos2d::Texture2D* texture);
    
    void onCameraStarted(int cameraId);
    void onCameraStopped();
    void onCameraAccessDenied();
    void onCameraFirstFrameReceived(int cameraId, int cropWidth, int cropHeight, int frameWidth, int frameHeight);
    void onCameraCaptureComplete(uint8_t* data, size_t length, int width=0, int height=0);
    
    void setCropRatio(CameraDeviceConst::Crop cropRatio);
    bool hasFrontFacingCamera();
    bool hasFlash();
    void enumFlashMode(const std::vector<int>& flashModes);
    void cleanup();
    bool askAccessPermission();
    int getReqCameraId() { return _reqCameraId; }
    
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    // jni를 통해 frame이 들어왔다.
    int getOrientation();
    void onPictureTakenFromJNIEnd();
    #else
    int getOrientation() { return 90; }
    #endif
    
    void setOnFrameRenderListener(OnFrameRenderListener * l) {_frameListener = l;}
    void onFrameRendered(VideoFrame* frame);
    
    void setOnCodeScannerListener(OnCodeScannerListener * l) {_scannerListener = l;}
    void detectedScanCode(std::vector<float> pts, int type, std::string result);
private:
    cocos2d::Image* decodeJpegData(uint8_t* data, size_t dataLen);
    
private:
    CameraDevice* _device;
    std::weak_ptr<bool> _alive;
    
    int _lastFrameNumber;
    int _reqCameraId;   // 요청한 카메라 ID,  대부분 FRONT, BACK 순서로 되어 있음.... 듀얼은 각각 access가 가능한가??? 나중에 해봐야 알 것 같음
    int _cameraId;  // 현재 카메라 ID
    int _cropWidth;
    int _cropHeight;
    int _width;
    int _height;
    
    CameraDeviceConst::Crop _cropRatio;
    cocos2d::Image* _captureImage;
    friend class CameraDevice;
    OnFrameRenderListener * _frameListener;
    OnCodeScannerListener * _scannerListener;
};



#endif /* DeviceCamera_h */
