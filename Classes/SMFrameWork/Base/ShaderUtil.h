//
//  ShaderUtil.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 8..
//
//

#ifndef ShaderUtil_h
#define ShaderUtil_h

#include <2d/CCSprite.h>
#include <platform/CCFileUtils.h>
#include <string>
#include "MeshSprite.h"

class ShaderUtil {
    
public:

    static void setBgr2RgbShader(cocos2d::Sprite * sprite) {
        #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        std::string shaderName = "mobileShader/bgr2rgb.fsh";
        #else
        std::string shaderName = "shader/bgr2rgb.fsh";
        #endif
        
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto fragmentFilePath = fileUtils->fullPathForFilename(shaderName);

            auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
            
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
    }
    
    static void setBilateralShader(cocos2d::Sprite* sprite, bool bBGR=true) {
        std::string shaderName;
        if (bBGR) {
            shaderName = "mobileShader/BilateralBlurBGR.fsh";
        } else {
            shaderName = "mobileShader/BilateralBlur.fsh";
        }
        
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto vertexFilePath = fileUtils->fullPathForFilename("mobileShader/BilateralBlur.vsh");
            auto vertexSource = fileUtils->getStringFromFile(vertexFilePath);
            
            auto fragmentShaderName = fileUtils->fullPathForFilename(shaderName);
            auto fragmentSource =fileUtils->getStringFromFile(fragmentShaderName);
            
            program = cocos2d::GLProgram::createWithByteArrays(vertexSource.c_str(), fragmentSource.c_str());
            
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
        state->setUniformFloat("texelWidthOffset", 0);
        state->setUniformFloat("texelHeightOffset", 2.0f/sprite->getContentSize().height);
        
        state->setUniformFloat("distanceNormalizationFactor", 4.0f);
        sprite->getGLProgram()->use();
    }
    
    static void setGaussianBlurShader(cocos2d::Sprite* sprite) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        std::string shaderName = "mobileShader/SpriteGaussianBlur.fsh";
#else
        std::string shaderName = "shader/SpriteGaussianBlur.fsh";
#endif
        
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto fragmentFilePath = fileUtils->fullPathForFilename(shaderName);
            
            auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
            
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
        state->setUniformVec2("resolution", sprite->getContentSize()/4);
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        state->setUniformFloat("blurRadius", 12);
        state->setUniformFloat("sampleNum", 8);
#else
//        state->setUniformFloat("blurRadius", 20);
//        state->setUniformFloat("sampleNum", 14);
        state->setUniformFloat("blurRadius", 17);
        state->setUniformFloat("sampleNum", 17);
#endif
        sprite->getGLProgram()->use();
    }
    
    
    static void setCircleSpriteShader(cocos2d::Sprite* sprite, float radius=0) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        std::string shaderName = "mobileShader/SpriteCircle.fsh";
#else
        std::string shaderName = "shader/SpriteCircle.fsh";
#endif
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto fragmentFilePath = fileUtils->fullPathForFilename(shaderName);

            auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
            
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
        
        state->setUniformFloat("u_aaWidth", 1.5f);
        if (radius > 0) {
            state->setUniformFloat("u_radius", radius);
        } else {
            state->setUniformFloat("u_radius", std::min(sprite->getContentSize().width, sprite->getContentSize().height)/2);
        }
        sprite->getGLProgram()->use();
    }
    
    static void setVignetteSpriteShader(cocos2d::Sprite* sprite) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        std::string shaderName = "mobileShader/SpriteVignette.fsh";
#else
        std::string shaderName = "shader/SpriteVignette.fsh";
#endif
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto fragmentFilePath = fileUtils->fullPathForFilename(shaderName);

            auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
            
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
        sprite->getGLProgram()->use();
    }
    
    static void setGenieEffectSpriteShader(cocos2d::Sprite* sprite) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        std::string shaderName = "mobileShader/GenieEffect.fsh";
#else
        std::string shaderName = "shader/GenieEffect.fsh";
#endif
        auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
        
        if (program == nullptr) {
            auto fileUtils = cocos2d::FileUtils::getInstance();
            
            auto fragmentFilePath = fileUtils->fullPathForFilename(shaderName);

            auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
            
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, fragSource.c_str());
            
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
            
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
            program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
            program->link();
            program->updateUniforms();
        }
        
        auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
        sprite->setGLProgramState(state);
        sprite->getGLProgram()->use();
    }
    
    
};



#endif /* ShaderUtil_h */
