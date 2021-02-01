//
//  SpriteFilterLordKelvin.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SpriteFilterLordKelvin.h"
#include "FilterShaderString.h"
#include <cocos2d.h>


SpriteFilterLordKelvin * SpriteFilterLordKelvin::createWithTexture(cocos2d::Texture2D *texture)
{
    auto sprite = new (std::nothrow)SpriteFilterLordKelvin();
    if (sprite && sprite->initWithTexture(texture)) {
        sprite->autorelease();
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

SpriteFilterLordKelvin::SpriteFilterLordKelvin() : _ext1(nullptr)
{
    
}

SpriteFilterLordKelvin::~SpriteFilterLordKelvin()
{
    CC_SAFE_RELEASE_NULL(_ext1);
}

bool SpriteFilterLordKelvin::initWithTexture(cocos2d::Texture2D *texture)
{
    if (!cocos2d::Sprite::initWithTexture(texture)) {
        return false;
    }
    
    auto addedSprite = cocos2d::Sprite::create("LookupImage/kelvinMap.png");
    _ext1 = addedSprite->getTexture();
    _ext1->retain();
    
    std::string shaderKey = "LordKelvinShader";
    
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderKey);
    if (program==nullptr) {
        program = cocos2d::GLProgram::createWithByteArrays(FilterShaderString::FILTER_VS_BASE.c_str(), FilterShaderString::FILTER_FS_KEVLVIN.c_str());
        
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderKey);
        
        // base
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
        program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
        
        program->link();
        program->updateUniforms();
    }
    
    auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    setGLProgramState(state);
    getGLProgram()->use();
    
    _uniformAddedTexture1 = program->getUniformLocation("filterTexture");
    
    return true;
}

void SpriteFilterLordKelvin::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    cocos2d::Sprite::draw(renderer, transform, flags);
    
    if (_displayedOpacity==0) {
        // opacity가 0이면 그릴 필요 없음.
        return;
    }
    
    // on draw 호출
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(SpriteFilterLordKelvin::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void SpriteFilterLordKelvin::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
    auto state = getGLProgramState();

    state->setUniformTexture(_uniformAddedTexture1, _ext1);
    
    state->apply(transform);
    
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}
