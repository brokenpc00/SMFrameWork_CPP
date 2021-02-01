//
//  ImgPrcSnapShotToLocal.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 9..
//

#ifndef ImgPrcSnapShotToLocal_h
#define ImgPrcSnapShotToLocal_h

#include "ImageProcessFunction.h"
#include "../Util/use_opencv.h"
#include "../Util/FileManager.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <external/webp/include/ios/encode.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <external/webp/include/android/encode.h>
#endif

#include <stdio.h>

class ImgPrcSnapShotToLocal : public ImageProcessFunction {
public:
    ImgPrcSnapShotToLocal(const std::string& fileName) : _fileName(fileName), _sprite(nullptr) {
    }
    
    virtual ~ImgPrcSnapShotToLocal() {
        CC_SAFE_RELEASE(_sprite);
    }
    
    virtual bool onPreProcess(cocos2d::Node* node) override {
//        auto zoomView = (CgZoomView*)node;
        
        auto size = node->getContentSize();
        auto point = node->getPosition();
        auto rect = cocos2d::Rect(point.x,
                                  point.y,
                                  size.width,
                                  size.height);
        
//        auto canvasSize = cocos2d::Size(SNAPSHOT_WIDTH, SNAPSHOT_HEIGHT);
        auto canvasSize = size;
        auto anchor = cocos2d::Vec2(rect.getMidX() / size.width, rect.getMidY() / size.height);
//        float scale = std::max(SNAPSHOT_WIDTH / rect.size.width, SNAPSHOT_HEIGHT / rect.size.height);
        float scale = 1.0f;
        
        auto param = initParam();
        param->putFloat("SCALE", scale);
        
        return startProcess(node, canvasSize, canvasSize/2, anchor, scale, scale);
    }
    
    virtual void onCaptureComplete(cocos2d::Texture2D* texture) override {
        _sprite = cocos2d::Sprite::createWithTexture(texture);
        _sprite->retain();
    }

    virtual bool onProcessInBackground() override {
        std::string fullPath = FileManager::getInstance()->getFullFilePath(FileManager::FileType::SnapShot, _fileName);
        std::string localPath = FileManager::getInstance()->getLocalFilePath(FileManager::FileType::SnapShot, _fileName);
        
        auto param = getParam();
        param->putString("SNAPSHOT_PATH", localPath);
        param->putString("FILE_NAME", _fileName);
        
        // webp encode
        uint8_t* output;
        
        uint8_t* inputData = getInputData();
        int bpp = getInputBpp();
        cocos2d::Size inputSize = getInputSize();
        size_t size = WebPEncodeRGBA(inputData, (int)inputSize.width, (int)inputSize.height, (int)(bpp*inputSize.width), 90, &output);
        FileManager::getInstance()->writeToFile(FileManager::FileType::SnapShot, _fileName, output, size);
        
        free(output);
        
        return true;
    }
    
    virtual cocos2d::Sprite* onPostProcess() override {
        return _sprite;
    }
    
private:
    std::string _fileName;
    
    cocos2d::Sprite* _sprite;

};




#endif /* ImgPrcSnapShotToLocal_h */
