//
//  ImageProcessFunction.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#include "ImageProcessFunction.h"
#include "../Base/Intent.h"
#include "../Const/SMViewConstValue.h"
#include <string>
#include <cocos2d.h>

ImageProcessFunction::ImageProcessFunction() :
_inputData(nullptr),
_outputData(nullptr),
_param(nullptr),
_renderTexture(nullptr),
_clearColor(SMViewConstValue::Const::COLOR4F_TRANSPARENT),
_isCaptureOnly(false),
_outputImage(nullptr)
{
    initParam();
}

ImageProcessFunction::~ImageProcessFunction()
{
    releaseInputData();
    releaseOutputData();
    CC_SAFE_RELEASE(_param);
}

void ImageProcessFunction::setTask(ImageProcessor::ImageProcessTask* task) {
    _task = task;
}

void ImageProcessFunction::initOutputBuffer(const int width, const int height, const int bpp)
{
    _outputData = new uint8_t[width*height*bpp];
    _outputSize = cocos2d::Size(width, height);
    _outputBpp = bpp;
}

bool ImageProcessFunction::onPreProcess(cocos2d::Node *node)
{
    if (node) {
    auto canvasSize = node->getContentSize();
    
    float x = canvasSize.width * 0.5f;
    float y = canvasSize.height * 0.5f;
    
    return startProcess(node, canvasSize, cocos2d::Vec2(x, y), cocos2d::Vec2::ANCHOR_MIDDLE, 1.0f, 1.0f);
    } else {
        return true;
    }
}

cocos2d::Sprite * ImageProcessFunction::onPostProcess()
{
    cocos2d::Texture2D* texture = nullptr;
    
    if (getOutputImage()) {
        texture = new cocos2d::Texture2D();
        
        texture->initWithImage(getOutputImage());
        releaseOutputData();
    } else if (getOutputData()) {
        cocos2d::Texture2D::PixelFormat pixelFormat;
        auto bpp = getOutputBpp();
        
        switch (bpp) {
            case 1:
            {
                pixelFormat = cocos2d::Texture2D::PixelFormat::A8;
            }
                break;
            case 3:
            {
                pixelFormat = cocos2d::Texture2D::PixelFormat::RGB888;
            }
                break;
            default:
            {
                pixelFormat = cocos2d::Texture2D::PixelFormat::RGBA8888;
            }
                break;
        }
        
        auto size = getOutputSize();
        texture = new cocos2d::Texture2D();
        
        texture->initWithData(getOutputData(), size.width*size.height*bpp, pixelFormat, size.width, size.height, size);
        releaseOutputData();
    }
    
    if (texture) {
        auto sprite = cocos2d::Sprite::createWithTexture(texture);
        CC_SAFE_RELEASE(texture);
        return sprite;
    }
    
    return nullptr;
}

bool ImageProcessFunction::startProcess(cocos2d::Node *node, const cocos2d::Size &canvasSize, const cocos2d::Vec2 &position, const cocos2d::Vec2 &anchorPoint, const float scaleX, const float scaleY)
{
    // backup sprite property
    auto old_position = node->getPosition();
    auto old_anchor = node->getAnchorPoint();
    auto old_scaleX = node->getScaleX();
    auto old_scaleY = node->getScaleY();
    auto old_rotate = node->getRotation();
    auto old_opacity = node->getOpacity();
    
    cocos2d::CustomCommand command;
    command.func = CC_CALLBACK_0(ImageProcessFunction::onReadPixelsCommand, this);
    
    _renderTexture = cocos2d::RenderTexture::create(canvasSize.width, canvasSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    auto renderer = cocos2d::Director::getInstance()->getRenderer();
    
    _renderTexture->beginWithClear(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    // begin
    node->setPosition(position);
    node->setAnchorPoint(anchorPoint);
    node->setScale(scaleX, scaleY);
    node->setRotation(0);
    node->setOpacity(0xff);
    
    // capture를 위해 한번 뿌림
    node->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
    renderer->addCommand(&command);
    // end
    _renderTexture->end();
    // capture!!!!
    renderer->render();
    
    
    // restore
    node->setPosition(old_position);
    node->setAnchorPoint(old_anchor);
    node->setScale(old_scaleX, old_scaleY);
    node->setRotation(old_rotate);
    node->setOpacity(old_opacity);
    
    // error check
    auto errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
        // error capture
        CCLOG("[[[[[ Catprue Error on ImageProcessFunction : 0x%x", (int)errorCode);
        return false;
    }
    
    onCaptureComplete(_renderTexture->getSprite()->getTexture());
    
    // current node capture success;
    return true;
}

void ImageProcessFunction::onReadPixelsCommand()
{
    if (isCaptureOnly()) {
        return;
    }
    
    auto texture = _renderTexture->getSprite()->getTexture();
    
    _inputSize = texture->getContentSizeInPixels();
    _inputBpp = 4;
    _inputData = (uint8_t*)malloc((size_t)(_inputSize.width*_inputSize.height*_inputBpp));
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, _inputSize.width, _inputSize.height, GL_RGBA, GL_UNSIGNED_BYTE, _inputData);
}

cocos2d::Texture2D* ImageProcessFunction::getCapturedTexture()
{
    if (_renderTexture) {
        return _renderTexture->getSprite()->getTexture();
    } else {
        return nullptr;
    }
}

void ImageProcessFunction::releaseInputData()
{
    CC_SAFE_FREE(_inputData);
}

void ImageProcessFunction::releaseOutputData()
{
    if (_outputData) {
        CC_SAFE_DELETE_ARRAY(_outputData);
        _outputData = nullptr;
    }
    
    if (_outputImage) {
        CC_SAFE_DELETE(_outputImage);
    }
}

Intent* ImageProcessFunction::initParam()
{
    if (_param==nullptr) {
        _param = Intent::create();
        _param->retain();
    }
    
    return _param;
}
