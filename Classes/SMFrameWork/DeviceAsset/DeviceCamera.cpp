//
//  DeviceCamera.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include "DeviceCamera.h"
#include "../Util/JpegDecodeUtil.h"
#include "../Base/Intent.h"
#include "../Base/ShaderUtil.h"
#include "../Util/ViewUtil.h"
#include <cocos2d.h>
#define MAX_SIDE_LENGTH     (1280)

#include "opencv2/imgproc.hpp"
#include "opencv2/core/core.hpp"

#include "AndroidCamera.h"
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static AndroidCamera* sCameraIndicator = nullptr;

void CameraDevice::initCameraDevice()
{
    _alive = std::make_shared<bool>(true);
    _camera = CameraPlayer::create(this);
}

void CameraDevice::releaseCameraDevice()
{
    _camera->cleanup();
    _camera.reset();
    _alive.reset();
}

bool CameraDevice::canAccessible()
{
    return true;
}

void CameraDevice::askAccessPermission()
{
    if (sCameraIndicator==nullptr) {
        sCameraIndicator = AndroidCamera::create(_camera);
        //sCameraIndicator->retain();
    }
    //_camera->startCamera(_camera->getReqCameraId());
}

void CameraPlayer::detectedScanCode(std::vector<float> pts, int type, std::string result)
{
    if (_scannerListener) {
//        CCLOG("[[[[[ android detect point : %f/%f, %f/%f, %f/%f, %f/%f", pts[0], pts[1], pts[2], pts[3], pts[4], pts[5], pts[6], pts[7]);

        std::vector<cocos2d::Vec2> cocosPts;
        cocosPts.clear();
        cocosPts.push_back(cocos2d::Vec2(pts[0], pts[1]));
        cocosPts.push_back(cocos2d::Vec2(pts[2], pts[3]));
        cocosPts.push_back(cocos2d::Vec2(pts[4], pts[5]));
        cocosPts.push_back(cocos2d::Vec2(pts[6], pts[7]));

        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, cocosPts, type, result]{
            _scannerListener->onDetectedCode(cocosPts, type, result);
        });
    }
}

void CameraDevice::onFrameRendered(VideoFrame* frame)
{
    CCLOG("[[[[[ camera device on frame rendered ");
}

CameraPlayer::CameraPlayer() : _captureImage(nullptr)
, _frameListener(nullptr)
{
    
}

CameraPlayer::~CameraPlayer()
{
    if (sCameraIndicator) {
        LOGD("[[[[[ Camera Player DESTRUCTOR!!!!!!");
        CC_SAFE_RELEASE(sCameraIndicator);
        sCameraIndicator = nullptr;
    }
    
    CC_SAFE_RELEASE(_captureImage);
}

void CameraPlayer::cleanup()
{
    if (sCameraIndicator) {
        sCameraIndicator->cleanup();
    }
}

std::shared_ptr<CameraPlayer> CameraPlayer::create(CameraDevice* device)
{
    std::shared_ptr<CameraPlayer> player = std::make_shared<CameraPlayer>();
    
    if (sCameraIndicator==nullptr) {
        sCameraIndicator = AndroidCamera::create(player, device->_forQRBarCode);
    }
    
    player->_device = device;
    player->_alive = device->_alive;
    
    return player;
}

void CameraPlayer::startCamera(int cameraId)
{
    _reqCameraId = cameraId;
    if (sCameraIndicator) {
        sCameraIndicator->startCamera(cameraId);
    }
}

bool CameraPlayer::hasFrontFacingCamera()
{
    if (sCameraIndicator) {
        return sCameraIndicator->isFrontFacingCameraPresent();
    }
    return false;
}

bool CameraPlayer::hasFlash()
{
    if (sCameraIndicator) {
        return sCameraIndicator->isFlashPresent();
    }
    return false;
}

void CameraPlayer::stopCamera()
{
    if (sCameraIndicator) {
        sCameraIndicator->stopCamera();
    }
}

void CameraPlayer::onCameraAccessDenied()
{
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_alive.expired()) {
                _device->onCameraNotify(CameraDeviceConst::Notify::DENY, nullptr);
            }
        });
    }
}

bool CameraPlayer::switchCamera()
{
    if (sCameraIndicator) {
        sCameraIndicator->switchCamera();
        return true;
    }
    
    return false;
}

bool CameraPlayer::capture(int flashState, bool stopAfterCapture)
{
    if (sCameraIndicator) {
        sCameraIndicator->captureImage(flashState, stopAfterCapture);
        return true;
    }
    return false;
}

void CameraPlayer::onCameraStarted(int cameraId)
{
    _cameraId = cameraId;
    
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_alive.expired()) {
                Intent * intent = Intent::create();
                intent->putInt("CAMERA_ID", _cameraId);
                _device->onCameraNotify(CameraDeviceConst::Notify::START, intent);
            }
        });
    }
}

void CameraPlayer::onCameraStopped()
{
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_alive.expired()) {
                _device->onCameraNotify(CameraDeviceConst::Notify::STOP, nullptr);
            }
        });
    }
}

void CameraPlayer::onCameraFirstFrameReceived(int cameraId, int cropWidth, int cropHeight, int frameWidth, int frameHeight)
{
    LOGD("[[[[[[[[[[[[[ camera player first frame received :::::: : camera ID ::: %d", cameraId);
    _cameraId = cameraId;
    _cropWidth = cropWidth;
    _cropHeight = cropHeight;
    _width = frameWidth;
    _height = frameHeight;
    
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_alive.expired()) {
                // FIRST일 때 Intent : "CAMERA_ID" : int, "CROP_WIDTH" : float, "CROP_HEIGHT" : float, "FULL_WIDTH" : float, "FULL_HEIGHT" : float
                Intent * intent = Intent::create();
                intent->putInt("CAMERA_ID", _cameraId);
                intent->putFloat("CROP_WIDTH", _cropWidth);
                intent->putFloat("CROP_HEIGHT", _cropHeight);
                intent->putFloat("FULL_WIDTH", _width);
                intent->putFloat("FULL_HEIGHT", _height);
                _device->onCameraNotify(CameraDeviceConst::Notify::FIRST, intent);;
            }
        });
    }
}

int CameraPlayer::getOrientation()
{
    return sCameraIndicator->getOrientation();
}

void CameraPlayer::onFrameRendered(VideoFrame* frame)
{
    if (_frameListener) {
        _frameListener->onFrameRendered(frame);
    }
}

bool CameraPlayer::updateTexture(cocos2d::Texture2D* texture)
{
    //LOGD("[[[[[ camera player update Texture 1");
    if (texture == nullptr || sCameraIndicator == nullptr) {
        if (texture==nullptr) {
            LOGD("[[[[[ camera player update Texture..... but texture is nullptr");
        }
        if (sCameraIndicator==nullptr) {
            LOGD("[[[[[ camera player update Texture..... but cameraindicator is nullptr");
        }
        return false;
    }


    //LOGD("[[[[[ camera player update Texture 2");
    return sCameraIndicator->updateTexture(texture);
}

bool CameraPlayer::isFrameReceived()
{
    if (!sCameraIndicator) {
        return false;
    }
    return sCameraIndicator->isFrameReceived();
}

bool CameraPlayer::autoFocusAtPoint(const cocos2d::Vec2& point)
{
    if (!sCameraIndicator) {
        return false;
    }
    
    return sCameraIndicator->autoFocusAtPoint(point);
}

void CameraPlayer::setCropRatio(CameraDeviceConst::Crop cropRatio)
{
    _cropRatio = cropRatio;
}

void CameraPlayer::onPictureTakenFromJNIEnd()
{

}

void CameraPlayer::onCameraCaptureComplete(uint8_t* data, size_t length, int width, int height)
{
    if (!_alive.expired()) {
        auto image = decodeJpegData(data, length);
        if (image) {
            auto scheduler = cocos2d::Director::getInstance()->getScheduler();
            scheduler->performFunctionInCocosThread([this, image] {
                if (_cameraId == CameraDeviceConst::Facing::FRONT) {
                    auto texture = new cocos2d::Texture2D;
                    texture->initWithImage(image);
                    image->release();
                    auto tmpSprite =  cocos2d::Sprite::createWithTexture(texture);
                    texture->release();
                    ShaderUtil::setBilateralShader(tmpSprite, false);
                    int maxSize = 1080;
                    int spriteWidth = tmpSprite->getContentSize().width;
                    int spriteHeight = tmpSprite->getContentSize().height;
                    float widthRatio = (float)maxSize/(float)spriteWidth;
                    float heightRatio = (float)maxSize/(float)spriteHeight;
                    float ratio = MIN(widthRatio, heightRatio);
                    int ratioWidth = spriteWidth * ratio;
                    int ratioHeight = spriteHeight * ratio;
                    auto rt = cocos2d::RenderTexture::create(ratioWidth, ratioHeight, cocos2d::Texture2D::PixelFormat::RGBA8888);
                    auto renderer = cocos2d::Director::getInstance()->getRenderer();
                    rt->beginWithClear(0, 0, 0, 0);
                    tmpSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                    tmpSprite->setPosition(ratioWidth/2, ratioHeight/2);
                    tmpSprite->setRotation(180);
                    tmpSprite->setOpacity(0xff);
                    tmpSprite->setScale(ratio);
                    tmpSprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
                    rt->end();
                    renderer->render();

                    _captureImage = rt->newImage();

                } else {
                    _captureImage = image;

                }
                if (!_alive.expired()) {
                    // finally success capture
                    // SNAP일 때 intent : "IMAGE" : cocos2d::Image, "CACHE_FILE_PATH" : std::string
                    Intent * intent = Intent::create();
                    intent->putRef("IMAGE", _captureImage);
                    intent->putString("CACHE_FILE_PATH", "CACHED_PATH");
                    _device->onCameraNotify(CameraDeviceConst::Notify::SNAP, intent);
                }
                CC_SAFE_RELEASE_NULL(_captureImage);
            });
        } else {
            auto scheduler = cocos2d::Director::getInstance()->getScheduler();
            scheduler->performFunctionInCocosThread([this] {
                if (!_alive.expired()) {
                    // finally failed capture
                    _device->onCameraNotify(CameraDeviceConst::Notify::SNAP, nullptr);
                }
            });
        }
    }
}

cocos2d::Image* CameraPlayer::decodeJpegData(uint8_t* data, size_t dataLen)
{
    if (data == nullptr || dataLen <= 0)
        return nullptr;
    
    uint8_t* outData;
    size_t outDataLen;
    int outWidth, outHeight;
    
    int width, height;
    int inSampleSize = 1;


    // 헤더 읽음.
    if (!JpegDecodeUtil::decodeDimension(data, dataLen, outWidth, outHeight))
        return nullptr;


    if (outWidth * outHeight > 2000 * 2000) {
        // 대략 이정도 크면 downscaling
        inSampleSize = 2; // iOS Device에서는 3264x2448보다 더 큰 사진은 촬영되지 않음.
    } else if (outWidth * outHeight > 3000 * 3000) {
        inSampleSize = 3; // 보험.
    }


    // 디코딩
    if (!JpegDecodeUtil::decode(data, dataLen, &outData, outDataLen, outWidth, outHeight, inSampleSize))
        return nullptr;


    switch (_cropRatio) {
        case CameraDeviceConst::Crop::RATIO_4_3:
        {
            if (outHeight < outWidth) {
                width = (int)(4.0 * outHeight / 3.0);
                height = outHeight;
            } else {
                width = outWidth;
                height = (int)(outWidth * 3.0 / 4.0);
            }

        }
            break;
        default:
        {
            width = outWidth;
            height = outHeight;
        }
            break;
    }


    cv::Mat src(outHeight, outWidth, CV_8UC3, outData);
    cv::Mat dst;


    // 크롭
    int offset = (outWidth - width)/2;
    if (width > MAX_SIDE_LENGTH) {
        // 리사이즈
        float scale = MAX_SIDE_LENGTH / (float)width;
        
        int scaleWidth = (int)(width * scale);
        int scaleHeight = (int)(height * scale);


        cv::resize(src(cv::Rect(offset, 0, width, height)), dst, cv::Size(scaleWidth, scaleHeight), 0, 0, CV_INTER_LINEAR);

    } else {
        src(cv::Rect(offset, 0, width, height)).copyTo(dst);
    }
    
    src.release();
    free(outData);
    
    cv::Mat output;

    // 회전
    if (_cameraId == CameraDeviceConst::Facing::FRONT) {
        cv::transpose(dst, output);
    } else {
        cv::transpose(dst, output);
        cv::flip(output, output, 1);
    }
    dst.release();
    
    // result image
    auto image = new cocos2d::Image();
    if (!image->initWithRawData(output.data, output.cols * output.rows * 3, output.cols, output.rows, 24, true)) {
        return nullptr;
    }


    return image;
}

// jni를 통해 frame이 들어왔다.
void cocos_android_app_onFrameReceiveFromJNI(uint8_t *buffer, int length, int preWidth, int preHeight)
{
        if (sCameraIndicator) {
            //LOGD("[[[[[ onFrameReceiveFromJNI on CameraPlayer");
            sCameraIndicator->onFrameReceiveFromJNI(buffer, length, preWidth, preHeight);
        }
}

void cocos_android_app_onPictureTakenFromJNI(uint8_t *buffer, int length, int width, int height)
{

        if (sCameraIndicator) {
            //LOGD("[[[[[ onFrameReceiveFromJNI on CameraPlayer");
            sCameraIndicator->onPictureTakenFromJNI(buffer, length, width, height);
        }
}


void cocos_android_app_onCodeDetectedFromJNI(float pts[], int type, const char * result)
{
    std::string scanResult(result);
    if (sCameraIndicator) {
        std::vector<float> newPts;
        newPts.clear();
        for (int i=0; i<8; i++) {
            newPts.push_back(pts[i]);
        }
        sCameraIndicator->onCodeDetectedFromJNI(newPts, type, scanResult);
    }
//    CCLOG("[[[[[ DeviceCamera onCodeDetected from JNI : %s", result);
}


#endif
