//
//  ImageProcessFunction.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
// 여러가지 Image process 함수들...

#ifndef ImageProcessFunction_h
#define ImageProcessFunction_h

#include "ImageProcessor.h"
#include <platform/CCImage.h>
#include <2d/CCSprite.h>
#include <base/ccTypes.h>

class Intent;

class ImageProcessFunction
{
public:
    ImageProcessFunction();
    virtual ~ImageProcessFunction();
    
    virtual bool onPreProcess(cocos2d::Node* node);
    
    virtual bool onProcessInBackground() = 0;
    
    virtual cocos2d::Sprite* onPostProcess();
    
    Intent* getParam() {return _param;}
    
    virtual void onCaptureComplete(cocos2d::Texture2D* texture) {}
    
protected:
    void setTask(ImageProcessor::ImageProcessTask* task);

    void onProgress(float progress);

    void setCaptureOnly() {_isCaptureOnly = true;}
    
    bool isCaptureOnly() {return _isCaptureOnly;}
    
    void initOutputBuffer(const int width, const int height, const int bpp);
    
    void setOutputImage(cocos2d::Image* image) {_outputImage = image;}
    
    cocos2d::Size getOutputSize() {return _outputSize;}
    cocos2d::Size getInputSize() {return _inputSize;}
    
    int getInputBpp() {return _inputBpp;}
    int getOutputBpp() {return _outputBpp;}
    
    cocos2d::Image* getOutputImage() {return _outputImage;}
    
    uint8_t* getInputData() {return _inputData;}
    uint8_t* getOutputData() {return _outputData;}

    size_t getInputDataLength() { return (size_t)(_inputSize.width*_inputSize.height*_inputBpp); }

    void releaseInputData();
    void releaseOutputData();
    
    Intent* initParam();
    
    cocos2d::Texture2D* getCapturedTexture();
    
    void setClearColor(const cocos2d::Color4F& clearColor) {_clearColor = clearColor;}
    
protected:
    virtual bool startProcess(cocos2d::Node* node, const cocos2d::Size& canvasSize, const cocos2d::Vec2& position, const cocos2d::Vec2& anchorPoint, const float scaleX, const float scaleY);
    
    void interrupt() {_interrupt = true;}
    bool isInterrupt() {return _interrupt;}

    void onReadPixelsCommand();
    
protected:
    cocos2d::RenderTexture* _renderTexture;
    
    cocos2d::Size _inputSize;
    
    uint8_t* _inputData;
    
    int _inputBpp;
private:
    Intent* _param;
    cocos2d::Size _outputSize;
    
    uint8_t* _outputData;
    
    cocos2d::Image* _outputImage;
    
    int _outputBpp;
    
    cocos2d::Color4F _clearColor;

    bool _interrupt;
    bool _isCaptureOnly;
    
    ImageProcessor::ImageProcessTask* _task;
    
    friend class ImageProcessor;
    
};


#endif /* ImageProcessFunction_h */
