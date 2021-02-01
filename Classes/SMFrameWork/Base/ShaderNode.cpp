//
//  ShaderNode.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "ShaderNode.h"

#include <cmath>
#include <string>
#include <algorithm>

static const GLfloat _texCoordStripConst[1+8][8] = {
    {  0,  0,   1,  0,   0,  1,   1,  1 }, // ALL
    {  0,  0,  .5,  0,   0,  1,  .5,  1 }, // LEFT_HALF
    { .5,  0,   1,  0,  .5,  1,   1,  1 }, // RIGHT_HALF
    {  0, .5,   1, .5,   0,  1,   1,  1 }, // TOP_HALF
    {  0,  0,   1,  0,   0, .5,   1, .5 }, // BOTTOM_HALF
    {  0, .5,  .5, .5,   0,  1,  .5,  1 }, // LEFT_TOP
    {  0,  0,  .5,  0,   0, .5,  .5, .5 }, // LEFT_BOTTOM
    { .5, .5,   1, .5,  .5,  1,   1,  1 }, // RIGHT_TOP
    { .5,  0,   1,  0,  .5, .5,   1, .5 }, // RIGHT_BOTTOM
};

float ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH = 1.5f;

enum ShaderKey {
    kShader_Unknown = 0,
    
    // hollow
    kShader_ShapeRect,
    kShader_ShapeCircle,
    kShader_ShapeRoundedRect,
    kShader_ShapeArcRing,
    
    // solid
    kShader_ShapeSolidRect,
    kShader_ShapeSolidCircle,
    kShader_ShapeCircleHole,
    kShader_ShapeInvertSolidCircle,
    kShader_ShapeSolidRoundRect,
    kShader_ShapeSolidTriangle,
    
    // effect
    kShader_ShapeVignetteRect,
    kShader_ShapeGradientRect,
    kShader_ShapeGaussianBlurRect,
    
};

static bool initFileName(const int key, std::string& outCacheKey, std::string& outVert, std::string& outFrag) {
    
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    switch (key) {
        case kShader_ShapeRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeRect.fsh";
            break;
        case kShader_ShapeCircle:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeCircle.fsh";
            break;
        case kShader_ShapeRoundedRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeRoundRect.fsh";
            break;
        case kShader_ShapeSolidRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeSolidRect.fsh";
            break;
        case kShader_ShapeSolidCircle:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeSolidCircle.fsh";
            break;
        case kShader_ShapeCircleHole:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeCircleHole.fsh";
            break;
        case kShader_ShapeSolidRoundRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeSolidRoundRect.fsh";
            break;
        case kShader_ShapeSolidTriangle:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeSolidTriangle.fsh";
            break;
        case kShader_ShapeArcRing:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeArcRing.fsh";
            break;
        case kShader_ShapeVignetteRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeVignetteRect.fsh";
            break;
        case kShader_ShapeGradientRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/ShapeGradientRect.fsh";
            break;
        case kShader_ShapeGaussianBlurRect:
            outVert = "mobileShader/PositionTexture_uColor.vsh";
            outFrag = "mobileShader/SpriteGaussianBlur.fsh";
            break;
        default:
            return false;
    }
#else 
    switch (key) {
        case kShader_ShapeRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeRect.fsh";
            break;
        case kShader_ShapeCircle:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeCircle.fsh";
            break;
        case kShader_ShapeRoundedRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeRoundRect.fsh";
            break;
        case kShader_ShapeSolidRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeSolidRect.fsh";
            break;
        case kShader_ShapeSolidCircle:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeSolidCircle.fsh";
            break;
        case kShader_ShapeCircleHole:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeCircleHole.fsh";
            break;
        case kShader_ShapeSolidRoundRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeSolidRoundRect.fsh";
            break;
        case kShader_ShapeSolidTriangle:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeSolidTriangle.fsh";
            break;
        case kShader_ShapeArcRing:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeArcRing.fsh";
            break;
        case kShader_ShapeVignetteRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeVignetteRect.fsh";
            break;
        case kShader_ShapeGradientRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/ShapeGradientRect.fsh";
            break;
        case kShader_ShapeGaussianBlurRect:
            outVert = "shader/PositionTexture_uColor.vsh";
            outFrag = "shader/SpriteGaussianBlur.fsh";
            break;
        default:
            return false;
    }
    
#endif
    
    outCacheKey = outFrag+outVert;
    
    return true;
}

ShaderNode::ShaderNode() :
_shaderKey(kShader_Unknown),
_quadrant(0)
{
    _blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
}

ShaderNode::~ShaderNode() {
    _eventDispatcher->removeEventListenersForTarget(this);
}

bool ShaderNode::initWithShaderKey(const int shaderKey) {
    _shaderKey = shaderKey;
    
    if (!setupShader(shaderKey)) {
        return false;
    }
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    auto listener = cocos2d::EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](cocos2d::EventCustom* event) {
        setGLProgramState(nullptr);
        setupShader(_shaderKey);
    });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
    
    return true;
}

bool ShaderNode::setupShader(const int shaderKey) {
    std::string cacheKey;
    std::string vert;
    std::string frag;
    
    if (!initFileName(shaderKey, cacheKey, vert, frag)) {
        return false;
    }
    return loadShaderVertex(cacheKey, vert, frag);
}

bool ShaderNode::loadShaderVertex(const std::string &cacheKey, const std::string &vert, const std::string &frag) {
    
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(cacheKey);
    
    if (program == nullptr) {
        auto fileUtils = cocos2d::FileUtils::getInstance();
        
        auto vertexFilePath = fileUtils->fullPathForFilename(vert);
        auto vertSource = fileUtils->getStringFromFile(vertexFilePath);
        
        auto fragmentFilePath = fileUtils->fullPathForFilename(frag);
        auto fragSource = fileUtils->getStringFromFile(fragmentFilePath);
        
        program = cocos2d::GLProgram::createWithByteArrays(vertSource.c_str(), fragSource.c_str());
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, cacheKey);
    }
    
    auto programState = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    setGLProgramState(programState);
    
    if (programState != nullptr) {
        onInitShaderParams(programState);
    }
    
    return programState;
}

void ShaderNode::onInitShaderParams(cocos2d::GLProgramState* programState) {
    auto p = programState->getGLProgram();
    _uniformColor = p->getUniformLocation("u_color");
}


void ShaderNode::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4& transform, uint32_t flags) {
    if (_displayedOpacity == 0)
        return;
    
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(ShaderNode::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}


void ShaderNode::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    
    const float a = _displayedOpacity/255.0;
    const cocos2d::Vec4 color = cocos2d::Vec4(a * _displayedColor.r/255.0,
                                              a * _displayedColor.g/255.0,
                                              a * _displayedColor.b/255.0,
                                              a);
    
    const float w = _contentSize.width;
    const float h = _contentSize.height;
    
    GLfloat vertices[8] = { 0, 0,  w, 0,  0, h, w, h };
    
    auto programState = getGLProgramState();
    
    programState->setUniformVec4(_uniformColor, color);
    programState->setVertexAttribPointer(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    programState->setVertexAttribPointer(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(_texCoordStripConst[_quadrant]));
    programState->apply(transform);
    
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,4);
}

void ShaderNode::setColor4F(const cocos2d::Color4F& color) {
    setColor(cocos2d::Color3B((GLubyte)(color.r*255), (GLubyte)(color.g*255), (GLubyte)(color.b*255)));
    setOpacity((GLubyte)(color.a*255));
}

cocos2d::Color4F ShaderNode::getColor4F()
{
    cocos2d::Color3B color3b = getColor();
    cocos2d::Color4F color4f = cocos2d::Color4F(color3b.r/255.0f, color3b.g/255.0f, color3b.b/255.0f, getOpacity()/255.0f);
    return color4f;
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Abstract Shape Node
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeNode::ShapeNode() : _cornerRadius(0.0f), _aaWidth(DEFAULT_ANTI_ALIAS_WIDTH), _lineWidth(10.0f) {
}

ShapeNode::~ShapeNode() {
}

void ShapeNode::setCornerRadius(float cornerRadius) {
    _cornerRadius = cornerRadius;
}

void ShapeNode::setAntiAliasWidth(float aaWidth) {
    _aaWidth = aaWidth;
}

void ShapeNode::setLineWidth(float lineWidth) {
    _lineWidth = lineWidth;
}

void ShapeNode::setCornerQuadrant(Quadrant quadrant) {
    _quadrant = (int)quadrant;
}

cocos2d::Vec2 ShapeNode::getQuadDimen(int quadrant, const cocos2d::Size& size) {
    if (!quadrant) {
        return size;
    }
    
    switch (quadrant) {
        default:
            return size;
            
        case LEFT_HALF:
        case RIGHT_HALF:
            return cocos2d::Vec2(2 * size.width, size.height);
            
        case TOP_HALF:
        case BOTTOM_HALF:
            return cocos2d::Vec2(size.width, 2 * size.height);
            
        case LEFT_TOP:
        case LEFT_BOTTOM:
        case RIGHT_TOP:
        case RIGHT_BOTTOM:
            return cocos2d::Vec2(2 * size.width, 2 * size.height);
    }
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeSolidRect::ShapeSolidRect() {
}

ShapeSolidRect::~ShapeSolidRect() {
}

bool ShapeSolidRect::init() {
    return initWithShaderKey(kShader_ShapeSolidRect);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeRect::ShapeRect() {
}

ShapeRect::~ShapeRect() {
}

bool ShapeRect::init() {
    return initWithShaderKey(kShader_ShapeRect);
}

void ShapeRect::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformDimension = p->getUniformLocation("u_dimension");
    _uniformLineWidth = p->getUniformLocation("u_lineWidth");
}

void ShapeRect::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    auto s = getGLProgramState();
    
    s->setUniformVec2(_uniformDimension, _contentSize);
    s->setUniformFloat(_uniformLineWidth, _lineWidth);
    
    ShapeNode::onDraw(transform, flags);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid RoundRect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeSolidRoundRect::ShapeSolidRoundRect() {
}

ShapeSolidRoundRect::~ShapeSolidRoundRect() {
}

bool ShapeSolidRoundRect::init() {
    if (!initWithShaderKey(kShader_ShapeSolidRoundRect))
        return false;
    return true;
}

void ShapeSolidRoundRect::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformDimension = p->getUniformLocation("u_dimension");
    _uniformCornerRadius = p->getUniformLocation("u_radius");
    _uniformAAWidth = p->getUniformLocation("u_aaWidth");
}

void ShapeSolidRoundRect::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    auto s = getGLProgramState();
    
    float cornerRadius = _cornerRadius;
    if (cornerRadius < _aaWidth) {
        cornerRadius = _aaWidth - _cornerRadius;
    }
    
    cornerRadius = std::max(cornerRadius, 0.0f);
    float aaWidth = std::min(_aaWidth, cornerRadius);
    
    s->setUniformVec2(_uniformDimension, getQuadDimen(_quadrant, _contentSize));
    s->setUniformFloat(_uniformCornerRadius, cornerRadius);
    s->setUniformFloat(_uniformAAWidth, aaWidth);
    
    ShapeNode::onDraw(transform, flags);
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape RoundRect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeRoundedRect::ShapeRoundedRect() {
}

ShapeRoundedRect::~ShapeRoundedRect() {
}

bool ShapeRoundedRect::init() {
    return initWithShaderKey(kShader_ShapeRoundedRect);
}

void ShapeRoundedRect::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShapeSolidRoundRect::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformLineWidth = p->getUniformLocation("u_lineWidth");
}

void ShapeRoundedRect::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    auto s = getGLProgramState();
    
    //    float lineWidth = std::min(_lineWidth, _cornerRadius);
    //    if (lineWidth < _cornerRadius) {
    //        lineWidth = _aaWidth - lineWidth;
    //    }
    
    float lineWidth = std::max(_lineWidth, 0.0f);
    
    
    s->setUniformFloat(_uniformLineWidth, lineWidth);
    
    ShapeSolidRoundRect::onDraw(transform, flags);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Circle
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeSolidCircle::ShapeSolidCircle() : _circleAnchor(cocos2d::Vec2::ANCHOR_MIDDLE) {
}

ShapeSolidCircle::~ShapeSolidCircle() {
}

bool ShapeSolidCircle::init() {
    return initWithShaderKey(kShader_ShapeSolidCircle);
}

void ShapeSolidCircle::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformRadius = p->getUniformLocation("u_radius");
    _uniformAAWidth = p->getUniformLocation("u_aaWidth");
    _uniformCircleAnchor = p->getUniformLocation("u_anchor");
}

void ShapeSolidCircle::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    float radius = std::min(_contentSize.width, _contentSize.height)*0.5;
    if (radius <= 0)
        return;
    
    auto s = getGLProgramState();
    
    s->setUniformFloat(_uniformRadius, radius);
    s->setUniformFloat(_uniformAAWidth, _aaWidth);
    s->setUniformVec2(_uniformCircleAnchor, _circleAnchor);
    
    ShapeNode::onDraw(transform, flags);
}

void ShapeSolidCircle::setCircleAnchorPoint(const cocos2d::Vec2& circleAnchor) {
    _circleAnchor = circleAnchor;
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Circle Hole
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeCircleHole::ShapeCircleHole() {
}

ShapeCircleHole::~ShapeCircleHole() {
}

bool ShapeCircleHole::init() {
    return initWithShaderKey(kShader_ShapeCircleHole);
}

void ShapeCircleHole::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformRadius = p->getUniformLocation("u_radius");
    _uniformAAWidth = p->getUniformLocation("u_aaWidth");
    _uniformCenter = p->getUniformLocation("u_center");
    _uniformDimension = p->getUniformLocation("u_dimension");
}

void ShapeCircleHole::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    float radius = _radius - _aaWidth;
    if (radius <= 0)
        return;
    
    auto s = getGLProgramState();
    
    s->setUniformFloat(_uniformRadius, radius);
    s->setUniformFloat(_uniformAAWidth, _aaWidth);
    s->setUniformVec2(_uniformDimension, _contentSize);
    s->setUniformVec2(_uniformCenter, cocos2d::Vec2(_centerX, _centerY));
    
    ShapeNode::onDraw(transform, flags);
}

void ShapeCircleHole::setCenterPoint(float centerX, float centerY) {
    _centerX = centerX;
    _centerY = centerY;
}

void ShapeCircleHole::setRadius(float radius) {
    _radius = radius;
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Circle (Ring)
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeCircle::ShapeCircle() {
}

ShapeCircle::~ShapeCircle() {
}

bool ShapeCircle::init() {
    return initWithShaderKey(kShader_ShapeCircle);
}

void ShapeCircle::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShapeNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformRadius = p->getUniformLocation("u_radius");
    _uniformAAWidth = p->getUniformLocation("u_aaWidth");
    _uniformLineWidth = p->getUniformLocation("u_lineWidth");
}

void ShapeCircle::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    float radius = std::min(_contentSize.width, _contentSize.height)*0.5;
    if (radius <= 0)
        return;
    
    auto s = getGLProgramState();
    
    s->setUniformFloat(_uniformRadius, radius);
    s->setUniformFloat(_uniformAAWidth, _aaWidth);
    s->setUniformFloat(_uniformLineWidth, _lineWidth);
    
    ShapeNode::onDraw(transform, flags);
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Arc Ring
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeArcRing::ShapeArcRing() : _theta(0.0) {
}

ShapeArcRing::~ShapeArcRing() {
}

bool ShapeArcRing::init() {
    return initWithShaderKey(kShader_ShapeArcRing);
}

void ShapeArcRing::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShapeCircle::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformTheta = p->getUniformLocation("u_theta");
}

void ShapeArcRing::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    auto s = getGLProgramState();
    
    s->setUniformFloat(_uniformTheta, _theta);
    
    ShapeCircle::onDraw(transform, flags);
}

void ShapeArcRing::setArcRatio(double ratio) {
    _theta = 2 * M_PI * ratio;
}

void ShapeArcRing::draw(const float startAngle, const float sweepAngle) {
    float sweep;
    if (sweepAngle < 0) {
        sweep = std::abs(std::max(sweepAngle, -360.0f));
        setRotation(startAngle - sweepAngle - 90.0);
    } else {
        sweep = std::min(sweepAngle, 360.0f);
        setRotation(startAngle - 90.0);
    }
    
    setArcRatio(sweep / 360.0);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Triangle
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeSolidTriangle::ShapeSolidTriangle() : _dirty(false), _p0(cocos2d::Vec2::ZERO), _p1(cocos2d::Vec2::ZERO), _p2(cocos2d::Vec2::ZERO) {
}

ShapeSolidTriangle::~ShapeSolidTriangle() {
}

bool ShapeSolidTriangle::init() {
    if (!initWithShaderKey(kShader_ShapeSolidTriangle))
        return false;
    
    return true;
}

void ShapeSolidTriangle::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformP0 = p->getUniformLocation("u_p0");
    _uniformP1 = p->getUniformLocation("u_p1");
    _uniformP2 = p->getUniformLocation("u_p2");
}

void ShapeSolidTriangle::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    if (_contentSize.width * _contentSize.height == 0)
        return;
    
    //    if (_dirty) {
    _dirty = false;
    
    cocos2d::Vec2 p0 = cocos2d::Vec2(_p0.x / _contentSize.width, _p0.y / _contentSize.height);
    cocos2d::Vec2 p1 = cocos2d::Vec2(_p1.x / _contentSize.width, _p1.y / _contentSize.height);
    cocos2d::Vec2 p2 = cocos2d::Vec2(_p2.x / _contentSize.width, _p2.y / _contentSize.height);
    
    auto s = getGLProgramState();
    
    s->setUniformVec2(_uniformP0, p0);
    s->setUniformVec2(_uniformP1, p1);
    s->setUniformVec2(_uniformP2, p2);
    //    }
    
    ShapeNode::onDraw(transform, flags);
}

void ShapeSolidTriangle::setTriangle(cocos2d::Vec2 p0, cocos2d::Vec2 p1, cocos2d::Vec2 p2) {
    if (_p0 != p0 || _p1 != p1 || _p2 != p2) {
        _p0 = p0;
        _p1 = p1;
        _p2 = p2;
        _dirty = true;
    }
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Round Line
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeRoundLine::ShapeRoundLine() : _dirty(false), _x1(0), _x2(0), _y1(0), _y2(0), _lineScale(1.0) {
}

ShapeRoundLine::~ShapeRoundLine() {
}

void ShapeRoundLine::moveTo(float x, float y) {
    if (_dirty) updateLineShape();
    
    if (x != _x1 || y != _y1) {
        float dx = x - _x1;
        float dy = y + _y1;
        
        _x1 = x;
        _y1 = y;
        _x2 += dx;
        _y2 += dy;
        
        cocos2d::Node::setPosition(_x1, _y1);
    }
}

void ShapeRoundLine::moveBy(float dx, float dy) {
    if (_dirty) updateLineShape();
    
    if (dx != 0 || dy != 0) {
        _x1 += dx;
        _y1 += dy;
        _x2 += dx;
        _y2 += dy;
        
        cocos2d::Node::setPosition(_x1, _y1);
    }
}

void ShapeRoundLine::setLengthScale(float lineScale) {
    if (lineScale != _lineScale) {
        _lineScale = lineScale;
        _dirty = true;
    }
}

void ShapeRoundLine::updateLineShape() {
    float dx = _x2 - _x1;
    float dy = _y2 - _y1;
    float length = std::sqrt(dx*dx + dy*dy) * _lineScale;
    float degrees = -(float)(std::atan2(dy, dx) * 180.0 / M_PI); // to degrees
    
    setCornerRadius(_lineWidth/2);
    cocos2d::Node::setContentSize(cocos2d::Size(length + _lineWidth, _lineWidth));
    cocos2d::Node::setAnchorPoint(cocos2d::Vec2(0.5 * _lineWidth / _contentSize.width, 0.5 * _lineWidth / _contentSize.height));
    cocos2d::Node::setPosition(_x1, _y1);
    cocos2d::Node::setRotation(degrees);
    
    _dirty = false;
}

void ShapeRoundLine::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    if (_dirty) {
        updateLineShape();
    }
    
    if (_lineWidth == 0)
        return;
    
    ShapeSolidRoundRect::onDraw(transform, flags);
}

void ShapeRoundLine::line(cocos2d::Vec2 from, cocos2d::Vec2 to) {
    line(from.x, from.y, to.x, to.y);
}

void ShapeRoundLine::line(float x1, float y1, float x2, float y2) {
    if (_x1 != x1 || _x2 != x2 || _y1 != y1 || _y2 != y2) {
        
        _x1 = x1;
        _x2 = x2;
        _y1 = y1;
        _y2 = y2;
        
        updateLineShape();
    }
}

void ShapeRoundLine::setLineWidth(float lineWidth) {
    if (lineWidth != _lineWidth) {
        ShapeNode::setLineWidth(lineWidth);
        updateLineShape();
    }
}

// hide
void ShapeRoundLine::setCornerRadius(float cornerRadius){
    ShapeNode::setCornerRadius(cornerRadius);
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Guassian Blur Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeGaussianBlurRect::ShapeGaussianBlurRect()
{
    
}

ShapeGaussianBlurRect::~ShapeGaussianBlurRect()
{
    
}

bool ShapeGaussianBlurRect::init()
{
    if (!initWithShaderKey(kShader_ShapeGaussianBlurRect)) {
        return false;
    }
    
    return true;
}

void ShapeGaussianBlurRect::onInitShaderParams(cocos2d::GLProgramState *programState)
{
    ShaderNode::onInitShaderParams(programState);
    
//    auto p = programState->getGLProgram();
    
//    _uniformTexelWidth = p->getUniformLocation("u_texelWidth");
//    _uniformTexelHeight = p->getUniformLocation("u_texelHeight");
}

void ShapeGaussianBlurRect::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
//    auto s = getGLProgramState();
//    s->setUniformFloat(_uniformTexelWidth, _contentSize.width);
//    s->setUniformFloat(_uniformTexelHeight, _contentSize.height);
    
//    const float a = _displayedOpacity/255.0;
//    const cocos2d::Vec4 color = cocos2d::Vec4(a * _displayedColor.r/255.0,
//                                              a * _displayedColor.g/255.0,
//                                              a * _displayedColor.b/255.0,
//                                              a);
//    
//    const float w = _contentSize.width;
//    const float h = _contentSize.height;
//    
//    GLfloat vertices[8] = { 0, 0,  w, 0,  0, h, w, h };
//    
//    auto programState = getGLProgramState();
//    
//    programState->setUniformVec4(_uniformColor, color);
//    programState->setVertexAttribPointer("a_position", 2, GL_FLOAT, GL_FALSE, 0, vertices);
//    programState->setVertexAttribPointer("a_texCoord", 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(_texCoordStripConst[_quadrant]));
//    programState->setVertexAttribPointer("a_color", 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(_texCoordStripConst[_quadrant]));
//    programState->apply(transform);
//    
//    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
//    
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    
//    CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,4);
    
    ShaderNode::onDraw(transform, flags);
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Vignette Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeVignetteRect::ShapeVignetteRect() : _strength(0.25), _centerX(0.5), _centerY(0.5) {
}

ShapeVignetteRect::~ShapeVignetteRect() {
}

bool ShapeVignetteRect::init() {
    if (!initWithShaderKey(kShader_ShapeVignetteRect))
        return false;
    
    return true;
}

void ShapeVignetteRect::onInitShaderParams(cocos2d::GLProgramState* programState) {
    ShaderNode::onInitShaderParams(programState);
    
    auto p = programState->getGLProgram();
    
    _uniformAspect = p->getUniformLocation("u_aspect");
    _uniformStrength = p->getUniformLocation("u_strength");
    _uniformCenter = p->getUniformLocation("u_center");
}

void ShapeVignetteRect::onDraw(const cocos2d::Mat4& transform, uint32_t flags) {
    auto s = getGLProgramState();
    
    s->setUniformVec2(_uniformCenter, cocos2d::Vec2(_centerX, _centerY));
    s->setUniformFloat(_uniformAspect, 2.0 * _contentSize.width / _contentSize.height);
    s->setUniformFloat(_uniformStrength, _strength);
    
    ShapeNode::onDraw(transform, flags);
}

void ShapeVignetteRect::setCenterPoint(float centerX, float centerY) {
    _centerX = centerX;
    _centerY = centerY;
}

void ShapeVignetteRect::setStrength(float strength) {
    _strength = strength;
}



//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Gradient Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
ShapeGradientRect::ShapeGradientRect() {
}

ShapeGradientRect::~ShapeGradientRect() {
}

bool ShapeGradientRect::init() {
    if (!initWithShaderKey(kShader_ShapeGradientRect))
        return false;
    
    return true;
}

