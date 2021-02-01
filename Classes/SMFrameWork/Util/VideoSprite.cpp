//
//  VideoSprite.cpp
//  LiveOn
//
//  Created by SteveKim on 03/07/2019.
//

#include "VideoSprite.h"
#include "VideoDecoder.h"
#include "VideoFrame.h"
#include <cocos2d.h>
//#include <SimpleAudioEngine.h>

#define CROP_PADDING    (2)

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
static const std::string CONVERT_SHADER_NAME = "mobileShader/yuv420p2rgb.fsh";
//static const std::string CONVERT_SHADER_NAME = "mobileShader/bgr2rgb.fsh";
#else
static const std::string CONVERT_SHADER_NAME = "shader/yuv420p2rgb.fsh";
#endif


VideoSprite* VideoSprite::createWithImageView(const std::string& srcName, SMImageView * imageView)
{
    VideoSprite* sprite = new VideoSprite();
    
    if (sprite && sprite->initWithVideo(srcName, imageView)) {
        sprite->autorelease();
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

VideoSprite::VideoSprite() :
_texturePlanarU(nullptr),
_texturePlanarV(nullptr),
_updateDeltaTime(0.033),
_displayView(nullptr),
_initialized(false)
{
}

VideoSprite::~VideoSprite()
{
    CC_SAFE_DELETE(_texturePlanarU);
    CC_SAFE_DELETE(_texturePlanarV);
}

void VideoSprite::onEnter()
{
    cocos2d::Sprite::onEnter();
    
    if (_initialized) {
        play();
    }
}

void VideoSprite::onExit()
{
    if (_initialized) {
        stop();
    }
    
    cocos2d::Sprite::onExit();
}

bool VideoSprite::initWithVideo(const std::string &videoName, SMImageView * imageView)
{
    if (initVideoDecoder(videoName)) {
        setTexture(nullptr); // hack
        initWithTexture(getTexture());
        setContentSize(cocos2d::Size(100, 100));
        _displayView = imageView;
        _displayView->setSprite(this);
//        CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(videoName.c_str());
        return true;
    }
    
    return false;
}

void VideoSprite::initShader()
{
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(CONVERT_SHADER_NAME);
    
    if (program == nullptr) {
        auto fileUtils = cocos2d::FileUtils::getInstance();
        
        auto fragmentFilePath = fileUtils->fullPathForFilename(CONVERT_SHADER_NAME);
        auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
        
        program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
        
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, CONVERT_SHADER_NAME);
        
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
        program->link();
        program->updateUniforms();
    }
    
    auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    setGLProgramState(state);
}

void VideoSprite::onVideoInitResult(bool success, int width, int height, double frameRate, double durationTime)
{
    if (success) {
        _videoWidth = width;
        _videoHeight = height;
        _initialized = true;

        play();
    }
}

void VideoSprite::onFrameUpdate(float dt)
{
    auto frame = getVideoFrame();
    if (frame == nullptr) {
        // 디코딩된 프레임 없음
        return;
    }
    
    int w = frame->getWidth();
    int h = frame->getHeight();
    int r_padding = 0;
    int hw = w / 2;
    int hh = h / 2;
    
    if (w > _videoWidth) {
        r_padding = w - _videoWidth;
    }
    
    float sw = frame->getWidth()-(CROP_PADDING*2+r_padding);
    float sh = frame->getHeight()-CROP_PADDING*2;
    if (sw <= 0 && sh <= 0) {
        recycleVideoFrame(frame);
        return;
    }
    
    
    // Y Plane
    auto texturePlanarY = getTexture();
    if (texturePlanarY) {
        int pixelWide = texturePlanarY->getPixelsWide();
        int pixelHigh = texturePlanarY->getPixelsHigh();
//        CCLOG("[[[[[ wide(%d) : w(%d), high(%d) : h(%d)", pixelWide, w, pixelHigh, h);
    }

    if (texturePlanarY != nullptr && (
                                      texturePlanarY->getPixelsWide() != w ||
                                      texturePlanarY->getPixelsHigh() != h)) {
        texturePlanarY = nullptr;
    }
    
    if (texturePlanarY == nullptr) {
        texturePlanarY = new cocos2d::Texture2D();
        texturePlanarY->initWithData(frame->getData(), w * h, cocos2d::Texture2D::PixelFormat::I8,
                                     w, h, cocos2d::Size(w, h));
        setTexture(texturePlanarY);
        initShader();
        CC_SAFE_RELEASE(texturePlanarY);
    } else {
        texturePlanarY->updateWithData(frame->getData(), 0, 0, frame->getWidth(), frame->getHeight());
    }
    
    setTextureRect(cocos2d::Rect(CROP_PADDING, CROP_PADDING, sw, sh));
    if (_displayView) {
        _displayView->updateData();
    }
    
    // U Plane
    if (_texturePlanarU == nullptr) {
        _texturePlanarU = new cocos2d::Texture2D();
        _texturePlanarU->initWithData(frame->getDataU(), hw * hh, cocos2d::Texture2D::PixelFormat::I8,
                                      hw, hh, cocos2d::Size(hw, hh));
        
        getGLProgramState()->setUniformTexture("u_textureU", _texturePlanarU);
        
    } else {
        _texturePlanarU->updateWithData(frame->getDataU(), 0, 0, hw, hh);
    }
    
    // V Plane
    if (_texturePlanarV == nullptr) {
        _texturePlanarV = new cocos2d::Texture2D();
        _texturePlanarV->initWithData(frame->getDataV(), hw * hh, cocos2d::Texture2D::PixelFormat::I8,
                                      hw, hh, cocos2d::Size(hw, hh));
        
        getGLProgramState()->setUniformTexture("u_textureV", _texturePlanarV);
    } else {
        _texturePlanarV->updateWithData(frame->getDataV(), 0, 0, hw, hh);
    }
    
    recycleVideoFrame(frame);
}

void VideoSprite::play()
{
    stop();
    
    // decoder
    playVideo();
    schedule(schedule_selector(VideoSprite::onFrameUpdate), _updateDeltaTime);
}

void VideoSprite::pause()
{
    stop();
}

void VideoSprite::resume()
{
    play();
}

void VideoSprite::stop()
{
    if (isScheduled(schedule_selector(VideoSprite::onFrameUpdate))) {
        // decoder
        stopVideo();
        unschedule(schedule_selector(VideoSprite::onFrameUpdate));
    }
}
