//
//  DeviceCamera.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "DeviceCamera.h"
#include "../Base/SMImageView.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include "DeviceCamera.h"
#include "../Util/JpegDecodeUtil.h"
#include "../Base/Intent.h"
#include "../Base/ShaderUtil.h"

#import <AVFoundation/AVFoundation.h>
#import <AVFoundation/AVCaptureDevice.h> // For access to the camera
#import "AppleCamera.h"

#define MAX_SIDE_LENGTH     (1280)
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>


static AppleCamera* sCameraIndicator = nullptr;



void CameraDevice::initCameraDevice()
{
    _alive = std::make_shared<bool>(true);
    _camera = CameraPlayer::create(this);
    _camera->setOnFrameRenderListener(this);
}

void CameraDevice::releaseCameraDevice()
{
    _camera->cleanup();
    _camera.reset();
    _alive.reset();
}

bool CameraDevice::canAccessible()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
    
    return (status == AVAuthorizationStatusAuthorized || status == AVAuthorizationStatusNotDetermined);
#else
    // mac인 경우
    return true;
#endif
}


void CameraDevice::askAccessPermission()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
    
    if (status == AVAuthorizationStatusDenied ||
        status == AVAuthorizationStatusRestricted) {
        // notify listener
        onCameraNotify(CameraDeviceConst::Notify::DENY, nullptr);
    } else if(status == AVAuthorizationStatusNotDetermined) {
        [ AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler: ^(BOOL granted) {
             if (granted) {
                 if (sCameraIndicator == nil) {
                     sCameraIndicator = [ [ AppleCamera alloc] initWithHost: _camera withCodeScanner:NO];
                 }
                 _camera->startCamera(_camera->getReqCameraId());
             } else {
                 _camera->onCameraAccessDenied();
             }
         } ];
    }
#else
    // mac인 경우
    if (sCameraIndicator==nil) {
        sCameraIndicator = [[AppleCamera alloc] initWithHost:_camera withCodeScanner:NO];
    }
    _camera->startCamera(_camera->getReqCameraId());
#endif
}

void CameraDevice::onFrameRendered(VideoFrame* frame)
{
    // maybe not called... override high depth
    CCLOG("[[[[[ camera device on frame rendered ");
}

CameraPlayer::CameraPlayer() : _captureImage(nullptr)
, _frameListener(nullptr)
, _scannerListener(nullptr)
{
}

CameraPlayer::~CameraPlayer()
{
    if (sCameraIndicator) {
        sCameraIndicator = nil;
    }
    
    CC_SAFE_RELEASE(_captureImage);
}

void CameraPlayer::cleanup()
{
    // TODO : 이 로직을 ImageFetcher에 적용해야함.
    if (sCameraIndicator) {
        [sCameraIndicator cleanup];
    }
}

std::shared_ptr<CameraPlayer> CameraPlayer::create(CameraDevice* device)
{
    std::shared_ptr<CameraPlayer> player = std::make_shared<CameraPlayer>();
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
    
    if (status == AVAuthorizationStatusAuthorized) {
        if (sCameraIndicator == nil) {
            sCameraIndicator = [[AppleCamera alloc] initWithHost:player withCodeScanner:device->_forQRBarCode?YES:NO];
        }
    }
#else
    // mac인 경우
    if (sCameraIndicator == nil) {
        sCameraIndicator = [[AppleCamera alloc] initWithHost:player withCodeScanner:NO];
    }
#endif
    
    player->_device = device;
    player->_alive = device->_alive;
    
    return player;
}

void CameraPlayer::startCamera(int cameraId)
{
    _reqCameraId = cameraId;
    if (sCameraIndicator) {
        [sCameraIndicator startCamera:cameraId];
    }
}

bool CameraPlayer::hasFrontFacingCamera()
{
    if (sCameraIndicator) {
        return [sCameraIndicator isFrontFacingCameraPresent];
    }
    return false;
}

bool CameraPlayer::hasFlash()
{
    if (sCameraIndicator) {
        return [sCameraIndicator isFlashPresent];
    }
    return false;
}

void CameraPlayer::stopCamera()
{
    if (sCameraIndicator) {
        [sCameraIndicator stopCamera];
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
        [sCameraIndicator switchCamera];
    }
    
    return true;
}

bool CameraPlayer::capture(int flashState, bool stopAfterCapture)
{
    if (sCameraIndicator) {
        [sCameraIndicator captureImage: flashState flag: stopAfterCapture];
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

void CameraPlayer::detectedScanCode(std::vector<float> pts, int type, std::string result)
{
    if (_scannerListener) {
        std::vector<cocos2d::Vec2> cocosPts;
        cocosPts.clear();

        // apple은 시계 반대 방향이므로 lt 부터 나머지는 역순..
        cocosPts.push_back(cocos2d::Vec2(pts[0], pts[1]));
        cocosPts.push_back(cocos2d::Vec2(pts[6], pts[7]));
        cocosPts.push_back(cocos2d::Vec2(pts[4], pts[5]));
        cocosPts.push_back(cocos2d::Vec2(pts[2], pts[3]));

        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([this, cocosPts, type, result]{
            _scannerListener->onDetectedCode(cocosPts, type, result);
        });
    }
}

void CameraPlayer::onFrameRendered(VideoFrame* frame)
{
    if (_frameListener) {
        _frameListener->onFrameRendered(frame);
    }
}

bool CameraPlayer::updateTexture(cocos2d::Texture2D* texture)
{
    if (texture == nullptr || sCameraIndicator == nullptr)
        return false;

    return (bool)[sCameraIndicator updateTexture:texture];
}

bool CameraPlayer::isFrameReceived()
{
    if (!sCameraIndicator) {
        return false;
    }

    return (bool)[ sCameraIndicator isFrameReceived];
}

bool CameraPlayer::autoFocusAtPoint(const cocos2d::Vec2& point)
{
    if (!sCameraIndicator) {
        return false;
    }
    auto focusPoint = CGPointMake(point.x, point.y);
    return (bool)[ sCameraIndicator autoFocusAtPoint: focusPoint ];
}

void CameraPlayer::setCropRatio(CameraDeviceConst::Crop cropRatio)
{
    _cropRatio = cropRatio;
}

void CameraPlayer::onCameraCaptureComplete(uint8_t* data, size_t length, int width, int height)
{
    if (!_alive.expired()) {
        auto image = decodeJpegData(data, length);
        if (image) {
            if (_cameraId == CameraDeviceConst::Facing::FRONT) {
                // apply bilarteral
                auto texture = new cocos2d::Texture2D();
                texture->initWithImage(image);
                auto tmpSprite = cocos2d::Sprite::createWithTexture(texture);
                ShaderUtil::setBilateralShader(tmpSprite, false);
                
                auto rt = cocos2d::RenderTexture::create(image->getWidth(), image->getHeight(), cocos2d::Texture2D::PixelFormat::RGBA8888);
                auto renderer = cocos2d::Director::getInstance()->getRenderer();
                
                rt->beginWithClear(0, 0, 0, 0);
                // begin
                tmpSprite->setPosition(image->getWidth()/2, image->getHeight()/2);
                tmpSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
//                tmpSprite->setScale(1.0f, -1.0f);
                tmpSprite->setRotation(0);
                tmpSprite->setOpacity(0xff);
                
                // capture를 위해 한번 뿌림
                tmpSprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
                // end
                rt->end();
                // capture!!!!
                renderer->render();
                
                _captureImage = rt->newImage();
            } else {
            _captureImage = image;
            }
            
            auto scheduler = cocos2d::Director::getInstance()->getScheduler();
            scheduler->performFunctionInCocosThread([this] {
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
    // https://developer.apple.com/library/ios/technotes/tn2409/_index.html Figure.1 참조
    // setHighResolutionStillImageOutputEnabled = YES 상태에서
    // iOS Device의 후면 카메라는 3264x2448 까지 촬영된다.
    //
    // 3264x2448 = 24MByte, 3264x1836=18MByte
    // 디코딩시 2배 더 필요할수 있으므로.. 최대 48MByte 필요해서 과도한 메모리를 소모함.
    // 이런경우 libjpeg에서 디코딩할때 부터 downscale하여 읽어야 함.

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
        case CameraDeviceConst::Crop::RATIO_1_1:
        {
            width = outHeight;
            height = outHeight;
        }
            break;
        case CameraDeviceConst::Crop::RATIO_4_3:
        {
            width = (int)(4.0 * outHeight / 3.0);
            height = outHeight;
        }
            break;
        default:
            width = outWidth;
            height = outHeight;
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

#endif
