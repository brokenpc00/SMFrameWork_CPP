//
//  CircleCropSprite.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#include "CircleCropSprite.h"
#include "../Base/ShaderNode.h"

static const GLfloat _texCoordStripConst[8] = { 0, 1,  1, 1, 0, 0, 1, 0 };

CircleCropSprite* CircleCropSprite::createWithTexture(cocos2d::Texture2D *texture, bool crop)
{
    auto cropSprite = new (std::nothrow)CircleCropSprite();
    if (cropSprite && cropSprite->initWithCropTexture(texture, crop)) {
        cropSprite->autorelease();
    } else {
        CC_SAFE_DELETE(cropSprite);
    }
    
    return cropSprite;
}

CircleCropSprite::CircleCropSprite() :
_aaWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH)
{
    
}

CircleCropSprite::~CircleCropSprite()
{
    
}

bool CircleCropSprite::initWithCropTexture(cocos2d::Texture2D *texture, bool crop)
{
    if (!cocos2d::Sprite::initWithTexture(texture)) {
        return false;
    }
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    const std::string shaderName = "mobileShader/SpriteCropCircle.fsh";
#else
    const std::string shaderName = "shader/SpriteCropCircle.fsh";
#endif
    
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);
    if (program==nullptr) {
        auto fileUtils = cocos2d::FileUtils::getInstance();
        
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        auto vertexFilePath = fileUtils->fullPathForFilename("mobileShader/PositionTexture_uColor.vsh");
#else
        auto vertexFilePath = fileUtils->fullPathForFilename("shader/PositionTexture_uColor.vsh");
#endif
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
    setGLProgramState(state);
    getGLProgram()->use();
    
    _uniformDimension = program->getUniformLocation("u_dimension");
    _uniformAAWidth = program->getUniformLocation("u_aaWidth");
    _uniformRadius = program->getUniformLocation("u_radius");
    _uniformColor = program->getUniformLocation("u_color");
    _uniformTexture = program->getUniformLocation("u_texture");
    
    
    // 실제 pixel단위 사이즈
    _size = texture->getContentSizeInPixels();
    if (crop) {
        _radius = 0.5*std::min(_size.x, _size.y);
    } else {
        _radius = 0.5*std::max(_size.x, _size.y);
    }
    
    return true;
}

void CircleCropSprite::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    if (_displayedOpacity==0) {
        // opacity가 0이면 그릴 필요 없음.
        return;
    }
    
    // on draw 호출
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(CircleCropSprite::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void CircleCropSprite::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
    const float alpha = _displayedOpacity/255.0f;
    const cocos2d::Vec4 color = cocos2d::Vec4( alpha * _displayedColor.r/255.0f,
                                                                            alpha * _displayedColor.g/255.0f,
                                                                            alpha * _displayedColor.b/255.0f,
                                                                            alpha);
    const float width = _size.x;
    const float height = _size.y;
    
    GLfloat vertices[8] = {0, 0, width, 0,
                                        0, height, width, height};
    
    auto state = getGLProgramState();
    
    state->setUniformVec2(_uniformDimension, _size);
    state->setUniformVec4(_uniformColor, color);
    state->setUniformFloat(_uniformRadius, _radius);
    state->setUniformFloat(_uniformAAWidth, _aaWidth);
    state->setUniformTexture(_uniformTexture, _texture);
    
    state->setVertexAttribPointer("a_position", 2, GL_FLOAT, GL_FALSE, 0, vertices);
    state->setVertexAttribPointer("a_texCoord", 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)_texCoordStripConst);
    
    state->apply(transform);
    
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}

void CircleCropSprite::setAntiAliasWidth(float aaWidth)
{
    _aaWidth = aaWidth;
}

void CircleCropSprite::setRadius(float radius)
{
    _radius = radius;
}
