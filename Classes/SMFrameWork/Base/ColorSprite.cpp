//
//  ColorSprite.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "ColorSprite.h"
#include <cocos2d.h>
#include <string>

ColorSprite* ColorSprite::createWithSprite(cocos2d::Sprite* srcSprite) {
    auto sprite = new (std::nothrow)ColorSprite();
    
    if (sprite && sprite->initWithSprite(srcSprite)) {
        sprite->autorelease();
        sprite->_initColor.brig = 0;
        sprite->_initColor.cont = 0;
        sprite->_initColor.satu = 0;
        sprite->_initColor.temp = 0;
    } else {
        CC_SAFE_DELETE(sprite);
        sprite = nullptr;
    }
    
    return sprite;
}

ColorSprite::ColorSprite() : _editMode(false)
{
    
}

ColorSprite::~ColorSprite()
{
    
}

void ColorSprite::setColorShader(cocos2d::Sprite *sprite)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    const std::string shaderKey("mobileShader/SpriteAdjustColor.fsh");
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram("mobileShader/SpriteAdjustColor.fsh");
#else
    const std::string shaderKey("shader/SpriteAdjustColor.fsh");
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram("shader/SpriteAdjustColor.fsh");
#endif
    
    if (program == nullptr) {
        auto fileUtils = cocos2d::FileUtils::getInstance();
        
        auto fragmentFilePath = fileUtils->fullPathForFilename(shaderKey);
        auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
        
        program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
        
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderKey);
        
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
        program->link();
        program->updateUniforms();
    }
    
    auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    sprite->setGLProgramState(state);
    sprite->getGLProgram()->use();
}

void ColorSprite::setColorValue(cocos2d::Sprite *sprite, const ColorSprite::Color &color)
{
    auto state = sprite->getGLProgramState();
    
    if (state) {
        float value;
        
        // brightness
        value = color.brig * 0.01 * 0.5;
        state->setUniformFloat("brightness", value);
        
        // contrast
        value = 1.0 + color.cont * 0.01 * 0.5;
        state->setUniformFloat("contrast", value);
        
        // saturate
        value = 1.0 + color.satu * 0.01;
        state->setUniformFloat("saturate", value);
        
        // temprature
        value = (color.temp * 0.01 * 0.5) * 2500.0;
        if (value < 0) {
            value = 0.0004f * value;
        } else {
            value = 0.0003f * value;
        }
        state->setUniformFloat("temperature", value);
    }
}

void ColorSprite::setBrightness(const int brightness)
{
    if (_editColor.brig==brightness) {
        return;
    }
    _editColor.brig = brightness;
    setColorValue(this, _editColor);
}

void ColorSprite::setContrast(const int contrast)
{
    if (_editColor.cont==contrast) {
        return;
    }
    _editColor.cont = contrast;
    setColorValue(this, _editColor);
}

void ColorSprite::setSaturate(const int saturate)
{
    if (_editColor.satu==saturate) {
        return;
    }
    _editColor.satu = saturate;
    setColorValue(this, _editColor);
}

void ColorSprite::setTemperature(const int temperature)
{
    if (_editColor.temp==temperature) {
        return;
    }
    _editColor.temp = temperature;
    setColorValue(this, _editColor);
}

void ColorSprite::startEditMode()
{
    if (_editMode) {
        return;
    }
    
    _editMode = true;
    
    _editColor = _initColor;

//    _rawSprite = cocos2d::Sprite::createWithTexture(getTexture());
//    _rawSize =  getTexture()->getContentSizeInPixels();
    
    setColorShader(this);
    setColorValue(this, _initColor);
}

bool ColorSprite::isColorModified(ColorSprite::Color &value)
{
    Color zero;
    return 0 != memcpy(&value, &zero, sizeof(Color));
}

void ColorSprite::restoreColorValue(const ColorSprite::Color &color)
{
    memcpy(&_editColor, &color, sizeof(Color));
    setColorValue(this, color);
}

void ColorSprite::finishEditMode(const bool apply)
{
    if (!_editMode) {
        return;
    }
    _editMode = false;
    
    if (apply) {
        _initColor = _editColor;
    } else {
        restoreColorValue(_initColor);
        _editColor = _initColor;
    }
    
    if (isColorModified(_initColor)) {
        // capture current sprite
        CCLOG("[[[[[ Cpature current Sprite");
    } else {
//        if (_rawSprite) {
//            setTexture(_rawSprite->getTexture());
//        }
    }
    
//    CC_SAFE_RELEASE_NULL(_rawSprite);
    
//    convertToRect();
    
//    setGLProgramState(cocos2d::GLProgramState::getOrCreateWithGLProgramName(cocos2d::GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
}
