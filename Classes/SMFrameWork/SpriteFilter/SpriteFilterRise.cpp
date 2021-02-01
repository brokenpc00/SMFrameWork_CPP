//
//  SpriteFilterRise.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SpriteFilterRise.h"
#include "FilterShaderString.h"
#include <cocos2d.h>


SpriteFilterRise * SpriteFilterRise::createWithTexture(cocos2d::Texture2D *texture)
{
    auto sprite = new (std::nothrow)SpriteFilterRise();
    if (sprite && sprite->initWithTexture(texture)) {
        sprite->autorelease();
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

SpriteFilterRise::SpriteFilterRise() :
_ext1(nullptr)
, _ext2(nullptr)
, _ext3(nullptr)
{
    
}

SpriteFilterRise::~SpriteFilterRise()
{
    CC_SAFE_RELEASE_NULL(_ext1);
    CC_SAFE_RELEASE_NULL(_ext2);
    CC_SAFE_RELEASE_NULL(_ext3);
}

bool SpriteFilterRise::initWithTexture(cocos2d::Texture2D *texture)
{
    if (!cocos2d::Sprite::initWithTexture(texture)) {
        return false;
    }
    
    auto addedSprite = cocos2d::Sprite::create("LookupImage/blackboard1024.png");
    _ext1 = addedSprite->getTexture();
    _ext1->retain();
    auto addedSprite2 = cocos2d::Sprite::create("LookupImage/overlayMap.png");
    _ext2 = addedSprite2->getTexture();
    _ext2->retain();
    auto addedSprite3 = cocos2d::Sprite::create("LookupImage/riseMap.png");
    _ext3 = addedSprite3->getTexture();
    _ext3->retain();
    
    std::string shaderKey = "RiseShader";
    
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderKey);
    if (program==nullptr) {
        program = cocos2d::GLProgram::createWithByteArrays(FilterShaderString::FILTER_VS_BASE.c_str(), FilterShaderString::FILTER_FS_RISE.c_str());
        
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
    
    _uniformAddedTexture1 = program->getUniformLocation("filterTexture1");
    _uniformAddedTexture2 = program->getUniformLocation("filterTexture2");
    _uniformAddedTexture3 = program->getUniformLocation("filterTexture3");
    
    return true;
}

void SpriteFilterRise::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    cocos2d::Sprite::draw(renderer, transform, flags);
    
    if (_displayedOpacity==0) {
        // opacity가 0이면 그릴 필요 없음.
        return;
    }
    
    // on draw 호출
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(SpriteFilterRise::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void SpriteFilterRise::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
    auto state = getGLProgramState();
    
    state->setUniformTexture(_uniformAddedTexture1, _ext1);
    state->setUniformTexture(_uniformAddedTexture2, _ext2);
    state->setUniformTexture(_uniformAddedTexture3, _ext3);
    
    state->apply(transform);
    
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}




