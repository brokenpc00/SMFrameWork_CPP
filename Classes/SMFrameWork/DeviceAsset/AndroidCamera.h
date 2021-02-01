//
//  AndroidCamera.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef AndroidCamera_h
#define AndroidCamera_h

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include "DeviceCamera.h"
#include <queue>
#include <renderer/CCTexture2D.h>
#include <base/CCRef.h>

// 일단 껍데기만 만들어 놓자.
class AndroidCamera : public cocos2d::Ref
{
public:
    static AndroidCamera* create(std::shared_ptr<CameraPlayer> player, bool bScanner=false);
    
    bool startCamera(int cameraId);
    bool isFrontFacingCameraPresent();
    bool isFlashPresent();
    bool isFocusModeSupport();
    
    void stopCamera();
    bool switchCamera();
    
    
    void captureImage(int flashState, bool stopAfterCapture);
    
    bool updateTexture(cocos2d::Texture2D * texture);
    bool isFrameReceived();
    void cleanup();
    bool autoFocusAtPoint(cocos2d::Vec2 point);

    void onFrameReceiveFromJNI(uint8_t *buffer, int length, int preWidth, int preHeight);

    void onPictureTakenFromJNI(uint8_t *buffer, int length, int width, int height);
    
    void onCodeDetectedFromJNI(std::vector<float> pts, int type, std::string result);

    int getOrientation() { return _orientation;}
    
protected:
    AndroidCamera();
    virtual ~AndroidCamera();
    
    bool initWithCamera(std::shared_ptr<CameraPlayer> player);
    
    
private:
    void queueIdleFrame(VideoFrame* frame);
    VideoFrame* dequeueIdleFrame();
    void queueDecodeFrame(VideoFrame* frame);
    VideoFrame* dequeueDecodeFrame();

    void onFrameReceiveFromJNIEnd();
    
private:
    int _cameraId;
    int _frameReceived;
    bool _stopCameraAfterCapture;
    int _orientation;
    
    std::shared_ptr<CameraPlayer> _cameraPlayer;
    std::queue<VideoFrame*> _idleQueue;
    std::queue<VideoFrame*> _decodeQueue;
    std::mutex _queueMutex;
    bool _forQRBarCode;
};


#endif

#endif /* AndroidCamera_h */
