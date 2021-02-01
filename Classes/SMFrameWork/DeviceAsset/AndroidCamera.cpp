//
//  AndroidCamera.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "AndroidCamera.h"
#include <algorithm>
#include <map>
#include <cocos2d.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include <platform/android/jni/JniHelper.h>
#include <base/CCDirector.h>
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#define BUFFER_COUNT 3

// 일단 껍데기만 만들어 놓자
/*
** 필요사항
    - init
        : frameReceive = 0, cameraId = back
        : iOS에서 global async queue를 위한게 있었는데 android에서는 asyncTask로 처리하자.
        : mutex 설정(idleQueue, decodeQueue 접근을 위한 mutex)
        : frame buffer 2장(또는 3장?)을 만들어 idle queue에 넣는다.
 
    - destructor
        : idle queue, decode queue에서 frame 삭제
        : CameraPlayer가 shared_ptr이므로 reset
 
    - startCamera
        : CameraPlayer에서 호출 된다.
        : 이게 화면이 나올때마다 매번 호출되므로 한번만 필요한 init은 여기에 두지 않거나 flag처리해야한다.

    - readyCamea
        : 한번만 필요한 camera init method
        : camera id setting
        : camera flash setting
        : camera auto focusing setting
 
    - stopCamera
        : CameraPlayer에서 호출 된다.
        : 화면이 없어질때마다 또는 사진을 찍을때마다 매번 호출되므로 카메라를 완전히 release하면 안된다.
 
    - switchCamera
        : CameraPlayer에서 전환 버튼이 눌릴때 호출된다.
        : 현재 cameraId를 얻어와서
        : stopCamera를 호출하고
        : 다시 startCamera를 호출한다.
 
    - cleanup
        : Camera의 완전 초기화
        : stopCamera호출
        : CameraPlayer가 shared_ptr이므로 reset
 
    - dequeueIdleFrame
        : idle queue에서 frame을 하나 꺼내온다.
 
    - queueIdelFrame
        : idle queue에 frame을 하나 추가한다.
 
    - dequeueDecodeFrame
        : decode queue에서 frame을 하나 꺼내온다.

    - queueDecodeFrame
        : decode queue에 frame을 하나 추가한다.
 
    - onFrameReceiveFromJNI(uint8_t *buffer, int length).... JNI에서 frame을 전달받는 method 만들어야함.
 
    - copyVideoFrame
        : 전달 받은 preview frame(binary data)를 queue에 넣는 method
        : idle queue에서 frame을 꺼내와서 frame을 만들고 decode queue에 넣는다.
        : 이때 frameReceived가 0이면 onCameraFirstFrameReceived를 통해 전달한다. (java class에서 할 필요 없다.)
        : frameReceived count를 증가시킨다.
 
    - captureImage
        : shutter button이 눌렸을때 호출
        : JNI를 통해 onPictureTaken이 호출 되도록 한다.

    - updateTexture
        : Camera Preview UI 갱신 method. CameraPlayer에서 호출됨
        : decode queue에서 frame을 하나 꺼내고 
        : texture size대로 frame을 이용하여 cocos2d::Image를 만든다.
        : 사용한 frame은 idle queue에 넣는다.
 
    - autoFocusAtPoint
        : preview screen touch시 호출된다.
        : camera device에 focusing되는 point를 전달한다.

 */

AndroidCamera::AndroidCamera() : _orientation(90), _forQRBarCode(false)
{
    
}

AndroidCamera::~AndroidCamera()
{
    /*
     - destructor
     : idle queue, decode queue에서 frame 삭제
     : CameraPlayer가 shared_ptr이므로 reset
     
     */

    LOGD("[[[[[ android camera DESTRUCTOR!!!!!!!!!!!");

    while (auto frame = dequeueIdleFrame()) {
        if (frame->buffer) {
            free(frame->buffer);
        }
        delete frame;
    }
    
    while (auto frame = dequeueDecodeFrame()) {
        if (frame->buffer) {
            free(frame->buffer);
        }
        delete frame;
    }
    
    _cameraPlayer.reset();
}

AndroidCamera* AndroidCamera::create(std::shared_ptr<CameraPlayer> player, bool bScanner)
{
    AndroidCamera * camera = new (std::nothrow)AndroidCamera();
    if (camera!=nullptr) {
        camera->_forQRBarCode = bScanner;
        if (camera->initWithCamera(player)) {
        //camera->autorelease();
    } else {
        CC_SAFE_DELETE(camera);
    }
    }
    
    return camera;
}

bool AndroidCamera::initWithCamera(std::shared_ptr<CameraPlayer> player)
{
    if (player==nullptr) {
        return false;
    }
    
    _cameraPlayer = player;

    // 이것 저것 초기화
/*
 - init
 : frameReceive = 0, cameraId = back
 : iOS에서 global async queue를 위한게 있었는데 android에서는 asyncTask로 처리하자.
 : mutex 설정(idleQueue, decodeQueue 접근을 위한 mutex)
 : frame buffer 2장(또는 3장?)을 만들어 idle queue에 넣는다.
 
 */
    _frameReceived = 0;
    _cameraId = CameraDeviceConst::Facing::BACK;
    
    //LOGD("[[[[[ android camera initWithCamera   !!!!!!!!!!!");
    for (int i=0; i<BUFFER_COUNT; i++) {
        queueIdleFrame(new VideoFrame());
    }

    return true;
}

bool AndroidCamera::startCamera(int cameraId)
{
    /*
     - startCamera
     : CameraPlayer에서 호출 된다.
     : 이게 화면이 나올때마다 매번 호출되므로 한번만 필요한 init은 여기에 두지 않거나 flag처리해야한다.
     
     */
     LOGD("[[[[[ android camera startCamera   !!!!!!!!!!!");
    _cameraId = cameraId;
    
    int reqCameraId;
    if (cameraId == CameraDeviceConst::Facing::FRONT) {
        reqCameraId = CameraDeviceConst::Facing::FRONT;
    } else {
        reqCameraId = CameraDeviceConst::Facing::BACK;
    }
    
    cocos2d::JniMethodInfo initCamera;
    if (!cocos2d::JniHelper::getStaticMethodInfo(initCamera, "org/cocos2dx/cpp/AppActivity", "initCamera", "(I)V")) {
        LOGD("[[[[[ failed to get static method.... cameraPreviewStart....");
        return false;
    }
    
    initCamera.env->CallStaticVoidMethod(initCamera.classID, initCamera.methodID, reqCameraId);
    
    initCamera.env->DeleteLocalRef(initCamera.classID);
    
    // set barcode scan mode
    cocos2d::JniHelper::callStaticVoidMethod("org/cocos2dx/cpp/AppActivity", "setCodeScanSupport", _forQRBarCode);
    
    
    cocos2d::JniMethodInfo cameraPreviewStart;
    if (!cocos2d::JniHelper::getStaticMethodInfo(cameraPreviewStart, "org/cocos2dx/cpp/AppActivity", "cameraPreviewStart", "()I")) {
        LOGD("[[[[[ failed to get static method.... cameraPreviewStart....");
        return false;
    }

    jint jOrientation = cameraPreviewStart.env->CallStaticIntMethod(cameraPreviewStart.classID, cameraPreviewStart.methodID);

    _orientation = jOrientation;
    LOGD("[[[[[ start camera orientation : %d", _orientation);
    //cameraPreviewStart.env->CallStaticVoidMethod(cameraPreviewStart.classID, cameraPreviewStart.methodID);
    
    cameraPreviewStart.env->DeleteLocalRef(cameraPreviewStart.classID);



    return true;
}

bool AndroidCamera::switchCamera()
{

/*
    : CameraPlayer에서 전환 버튼이 눌릴때 호출된다.
    : 현재 cameraId를 얻어와서
    : stopCamera를 호출하고
    : 다시 startCamera를 호출한다.
 */
    int newCameraId;
    if (_cameraId != CameraDeviceConst::Facing::BACK) {
        newCameraId = CameraDeviceConst::Facing::BACK;
    } else {
        newCameraId = CameraDeviceConst::Facing::FRONT;
    }

    LOGD("[[[[[ switch Camera!!! : %d == %d", _cameraId, newCameraId);

    stopCamera();

    cocos2d::JniMethodInfo switchCamera;
    if (!cocos2d::JniHelper::getStaticMethodInfo(switchCamera, "org/cocos2dx/cpp/AppActivity", "switchCamera", "()I")) {
        LOGD("[[[[[ failed to get static method.... switchCamera....");
        return false;
    }

    _cameraId = newCameraId;

    jint jOrientation = switchCamera.env->CallStaticIntMethod(switchCamera.classID, switchCamera.methodID);

    _orientation = jOrientation;

    LOGD("[[[[[ switch camera orientation : %d", _orientation);

    //switchCamera.env->CallStaticVoidMethod(switchCamera.classID, switchCamera.methodID);

    switchCamera.env->DeleteLocalRef(switchCamera.classID);

    //startCamera(newCameraId);
    
    return true;
}

bool AndroidCamera::isFrontFacingCameraPresent()
{
    LOGD("[[[[[ is front facing camera!!!");
    return true;
}

bool AndroidCamera::isFlashPresent()
{
    LOGD("[[[[[ is Flash present!!!");
    return true;
}

bool AndroidCamera::isFocusModeSupport()
{
    return true;
}

void AndroidCamera::stopCamera()
{
    /*
     : CameraPlayer에서 호출 된다.
     : 화면이 없어질때마다 또는 사진을 찍을때마다 매번 호출되므로 카메라를 완전히 release하면 안된다.
     */
    
    
    cocos2d::JniMethodInfo stopCamera;
    if (!cocos2d::JniHelper::getStaticMethodInfo(stopCamera, "org/cocos2dx/cpp/AppActivity", "stopCamera", "()V")) {
        LOGD("[[[[[ failed to get static method.... stopCamera....");
        return;
    }

    stopCamera.env->CallStaticVoidMethod(stopCamera.classID, stopCamera.methodID);

    stopCamera.env->DeleteLocalRef(stopCamera.classID);
    
    if (_cameraPlayer) {
        _cameraPlayer->onCameraStopped();
    }
    
    _frameReceived = 0;

    auto frame = dequeueDecodeFrame();
    while (frame) {
        queueIdleFrame(frame);
        frame = dequeueDecodeFrame();
    }

    LOGD("[[[[[ stop camera ");
}

void AndroidCamera::captureImage(int flashState, bool stopAfterCapture)
{
    //capture
    cocos2d::JniMethodInfo capture;

    if (!cocos2d::JniHelper::getStaticMethodInfo(capture, "org/cocos2dx/cpp/AppActivity", "capture", "()V")) {
        LOGD("[[[[[ failed to get static method.... capture....");
        return;
    }

    capture.env->CallStaticVoidMethod(capture.classID, capture.methodID);

    capture.env->DeleteLocalRef(capture.classID);
}

#define USING_BUFFER 0

bool AndroidCamera::updateTexture(cocos2d::Texture2D *texture)
{

    auto frame = dequeueDecodeFrame();

    if (!frame) {
        // 저장된 frame 없음.
        //LOGD("[[[[[ update texture but...... decode frame is empty!!!!");
        return false;
    }
    
    if (_cameraPlayer) {
        _cameraPlayer->onFrameRendered(frame);
    }

    cocos2d::Size size = texture->getContentSizeInPixels();

    int length = frame->length;
    int width = frame->width;
    int height = frame->height;

    if (width<=0 || height<=0 || frame->buffer==nullptr) {
        LOGD("[[[[[ something is wrong!!!!!!");
        return false;
    }
    


#if USING_BUFFER


    uint8_t* data = (uint8_t*)malloc(length);
    memcpy(data, frame->buffer, length);

    queueIdleFrame(frame);

    if (size.width != width || size.height != height) {
        // 기존과 크기가 다르면 texure 초기화
        texture->initWithData(data, length, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
    } else {
        texture->updateWithData(data, 0, 0, width, height);
        //LOGD("[[[[[ update frame for received buffer : %d, %d, %d", length, width, height);
    }

    free(data);
#else

    if (size.width != width || size.height != height) {
        // 기존과 크기가 다르면 texure 초기화
        texture->initWithData(frame->buffer, length, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height));
    } else {
        texture->updateWithData(frame->buffer, 0, 0, width, height);
        //LOGD("[[[[[ update frame for received buffer : %d, %d, %d", length, width, height);
    }
    
    // 여기에서 frame buffer를 이용해서 이미지 처리 해야함... detect 같은거... 상위로 보낼까??? cature view로????

    
    queueIdleFrame(frame);

#endif
    return true;

}

bool AndroidCamera::isFrameReceived()
{
    return true;
}

void AndroidCamera::queueIdleFrame(VideoFrame *frame)
{
    std::unique_lock<std::mutex> mt(_queueMutex);
    {
        _idleQueue.push(frame);
    }

    //LOGD("[[[[[ android camera queue idle frame : %d", _idleQueue.size());
}

VideoFrame* AndroidCamera::dequeueIdleFrame()
{
    VideoFrame* frame = NULL;
    
    std::unique_lock<std::mutex> mt(_queueMutex);
    {
        if (!_idleQueue.empty()) {
            frame = _idleQueue.front();
            _idleQueue.pop();
        }
    }
    
    return frame;
}

void AndroidCamera::queueDecodeFrame(VideoFrame *frame)
{
    std::unique_lock<std::mutex> mt(_queueMutex);
    {
        _decodeQueue.push(frame);
    }

    //LOGD("[[[[[ android camera queue decode frame : %d", _decodeQueue.size());
}

VideoFrame* AndroidCamera::dequeueDecodeFrame()
{
    VideoFrame* frame = NULL;
    
    std::unique_lock<std::mutex> mt(_queueMutex);
    {
        if (!_decodeQueue.empty()) {
            frame = _decodeQueue.front();
            _decodeQueue.pop();
        }
    }
    
    return frame;

}

bool AndroidCamera::autoFocusAtPoint(cocos2d::Vec2 point)
{
    return true;
}

void AndroidCamera::cleanup()
{
    /*
     : Camera의 완전 초기화
     : stopCamera호출
     : CameraPlayer가 shared_ptr이므로 reset
     */
    stopCamera();
    _cameraPlayer.reset();
}

void AndroidCamera::onFrameReceiveFromJNIEnd()
{

}

void AndroidCamera::onPictureTakenFromJNI(uint8_t *buffer, int length, int width, int height)
{
    LOGD("[[[[[ AndroidCamera::onPictureTakenFromJNI 1 !!!!!!!!!");
    //stopCamera();
    LOGD("[[[[[ AndroidCamera::onPictureTakenFromJNI 2 !!!!!!!!! length : %d", length);
    _cameraPlayer->onCameraCaptureComplete(buffer, length, width, height);
    LOGD("[[[[[ AndroidCamera::onPictureTakenFromJNI 3 !!!!!!!!!");
}

void AndroidCamera::onFrameReceiveFromJNI(uint8_t *buffer, int length, int preWidth, int preHeight)
{
    // 넣는 부분 스레딩 처리 하자..
    //cocos2d::AsyncTaskPool::getInstance()->enqueue(cocos2d::AsyncTaskPool::TaskType::TASK_IO, CC_CALLBACK_0(AndroidCamera::onFrameReceiveFromJNIEnd, this), nullptr, [this, buffer, length, preWidth, preHeight] {
        //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1");


        auto frame = dequeueIdleFrame();
        if (!frame) {
            // 대기중인 버퍼 없음.
            //LOGD("[[[[[ onFrameReceiveFromJNI but...... Idle frame is empty!!!!");
            return;
        }

        if (buffer==nullptr) {
            //LOGD("[[[[[[[[[[[[[[[[[[[[[[[[[ frame buffer is null !!!!!!!!!!!!!!!!!!!!!!!!");
            return;
        }

        bool isFront = _cameraId== CameraDeviceConst::Facing::FRONT;

        if (isFront) {
            //LOGD("[[[[[ camera is front mode!!!");
        } else {
            //LOGD("[[[[[ camera is back mode!!!");
        }


        // front면 hFlip true,

        //uint8_t* sampler = (uint8_t*)malloc(length);
        //memcpy(sampler, buffer, length);

        cv::Mat yuv(preHeight+preHeight/2, preWidth, CV_8UC1, buffer);
        cv::Mat rgb(preHeight, preWidth, CV_8UC4, preHeight*preHeight);
        cv::cvtColor(yuv, rgb, cv::COLOR_YUV2BGRA_NV12);

        //cv::Mat yuv(preHeight+preHeight/2, preWidth, CV_8UC1, (uchar *)buffer);
        //cv::Mat rgb(preHeight, preWidth, CV_8UC4);
        //cv::cvtColor(yuv, rgb, cv::COLOR_YUV2RGBA_NV21);

        //if (isFront) {
        //    cv::flip(rgb,rgb,1);
        //}

        int bufferSize = rgb.elemSize()*rgb.total();
        //uint8_t* sampler = (uint8_t*)malloc(bufferSize);
        //memcpy(sampler, rgb.data, bufferSize);
        //rgb.release();
        yuv.release();
        //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2");

        //auto image = new cocos2d::Image;
        //image->initWithRawData((unsigned char*)buffer, length, preWidth, preHeight, 32);
        //image->initWithImageData((unsigned char *)buffer, length);
        int width = preWidth;
        int height = preHeight;

        size_t cropWidth = (4 * height / 3);
        size_t cropLength = cropWidth * height * 4;

        bool needCrop = false;
        if (cropWidth != width) {
            // 4 : 3 크롭
            needCrop = true;
        }

        if (cropLength>bufferSize) {
            needCrop = false;
            cropLength = bufferSize;
        }

        //needCrop = false;

        //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1 : (%d, %d), %d, cropLength : %d", width, height, length, cropLength);

        if (frame->buffer == nullptr) {
            //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2");
            // 버퍼 생성
            if (needCrop) {
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1");
                frame->buffer = (uint8_t*)malloc(cropLength);
                frame->length = cropLength;
                frame->width = (int)cropWidth;
                frame->height = (int)height;
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-2");

                // copy to frame buffer
                size_t offset = (width - cropWidth)/2;
                size_t rowBytes = cropWidth * 4;

                uint32_t* src = (uint32_t*)rgb.data + offset;
                uint32_t* dst = (uint32_t*)frame->buffer;

                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-3");
                for (int i = 0; i < height; i++) {
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-3-1(%d) : %d + %d => %d==%d(%d) ", i, rowBytes*i, rowBytes, (rowBytes*i+rowBytes), rowBytes*(i+1), cropLength);
                    memcpy(dst, src, rowBytes);
                    //memcpy(dst, src, 1);
                    if (cropLength<rowBytes*(i+1)) {
                        LOGD("[[[[[ android camera onFrameReceiveFromJNI what the Fxxx ????? 1 : %d, %d", cropLength, rowBytes*(i+1));
                    }
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-3-2");
                    dst += cropWidth;
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-3-3");
                    src += width;
                }
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-1-4");
            } else {
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 2-2");
                frame->buffer = (uint8_t*)malloc(bufferSize);
                frame->length = length;
                frame->width = (int)width;
                frame->height = (int)height;

                // copy to buffer
                memcpy(frame->buffer, rgb.data, bufferSize);
            }
        } else {
            //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-1");
            if (needCrop) {
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-2-1");
                if (frame->length != cropLength) {
                    // 버퍼 크기가 다르면 realloc
                    frame->buffer = (uint8_t*)realloc((void*)frame->buffer, cropLength);
                    frame->length = cropLength;
                    frame->width = (int)cropWidth;
                    frame->height = (int)height;
                }

                // copy to frame buffer
                size_t offset = (width - cropWidth)/2;
                size_t rowBytes = cropWidth * 4;

                uint32_t* src = (uint32_t*)rgb.data + offset;
                uint32_t* dst = (uint32_t*)frame->buffer;

                for (int i = 0; i < height; i++) {
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-1-3-1(%d) : %d + %d => %d==%d(%d) ", i, rowBytes*i, rowBytes, (rowBytes*i+rowBytes), rowBytes*(i+1), cropLength);
                    memcpy(dst, src, rowBytes);
                    if (cropLength<rowBytes*(i+1)) {
                        LOGD("[[[[[ android camera onFrameReceiveFromJNI what the Fxxx 2 ????? : %d, %d", cropLength, rowBytes*(i+1));
                    }
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-1-3-2");
                    dst += cropWidth;
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-1-3-3");
                    src += width;
                    //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-1-3-4");
                }

            } else {
                //LOGD("[[[[[ android camera onFrameReceiveFromJNI 1-2-2");
                if (frame->length != bufferSize) {
                    // 버퍼 크기가 다르면 realloc
                    frame->buffer = (uint8_t*)realloc((void*)frame->buffer, bufferSize);
                    frame->length = bufferSize;
                    frame->width = (int)width;
                    frame->height = (int)height;
                }
                memcpy(frame->buffer, rgb.data, bufferSize);
            }
        }

        //free(sampler);
        rgb.release();


        queueDecodeFrame(frame);

        if (_frameReceived == 0 && _cameraPlayer) {
            LOGD("[[[[[ android camera first frame received");
            _cameraPlayer->onCameraFirstFrameReceived(_cameraId, (int)cropWidth, (int)height, (int)width, (int)height);
        }

        _frameReceived++;

        //LOGD("[[[[[ android camera onFrameReceiveFromJNI 3 : length : %d, size(%d, %d)", frame->length, frame->width, frame->height);
    //});


}

void AndroidCamera::onCodeDetectedFromJNI(std::vector<float> pts, int type, std::string result)
{
    _cameraPlayer->detectedScanCode(pts, type, result);
}

#endif
