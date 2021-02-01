//
//  NanoDrawNode.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#include "NanoDrawNode.h"
#include "nanovg_core.h"


// draw shader
static const std::string NANODRAW_SHADER_KEY = "NANOVG_SHADER_KEY";

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// vertex shader
static const char* sNanoVS = R"(
    precision mediump float;
    attribute vec2 a_position;
    attribute vec2 a_texCoord;

    varying mediump vec2 v_texCoord;

    void main() {
        gl_Position = CC_MVPMatrix * vec4(a_position.x, a_position.y, 0.0, 1.0);
        v_texCoord = a_texCoord;
    }
)";

// fragment shader
static const char* sNanoFS = R"(
    precision mediump float;
    uniform float u_stroke;
    uniform vec4 u_color;

    varying vec2 v_texCoord;

    float strokeMask() {
        return min(1.0, (1.0-abs(v_texCoord.x*2.0-1.0))*u_stroke) * min(1.0, v_texCoord.y);
    }

    void main() {
        float strokeAlpha = strokeMask();
        gl_FragColor =  u_color * strokeAlpha;
    }
)";
#else
// vertex shader
static const char* sNanoVS = R"(
attribute vec2 a_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;

void main() {
    gl_Position = CC_MVPMatrix * vec4(a_position.x, a_position.y, 0.0, 1.0);
    v_texCoord = a_texCoord;
}
)";

// fragment shader
static const char* sNanoFS = R"(
uniform float u_stroke;
uniform vec4 u_color;

varying vec2 v_texCoord;

float strokeMask() {
    return min(1.0, (1.0-abs(v_texCoord.x*2.0-1.0))*u_stroke) * min(1.0, v_texCoord.y);
}

void main() {
    float strokeAlpha = strokeMask();
    gl_FragColor =  u_color * strokeAlpha;
}
)";
#endif



// class method

NanoDrawNode::NanoDrawNode() : _vao(0)
, _vbo(0)
, _dirty(false)
, _nanoCore(nullptr)
{
    _blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
}

NanoDrawNode::~NanoDrawNode()
{
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    
    if (cocos2d::Configuration::getInstance()->supportsShareableVAO()) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
    
    NanoVG::destroy(_nanoCore);
}

NanoDrawNode* NanoDrawNode::create()
{
    auto node = new (std::nothrow)NanoDrawNode();
    if (node) {
        if (node->init()) {
            node->autorelease();
        } else {
            CC_SAFE_DELETE(node);
        }
    }
    
    return node;
}

bool NanoDrawNode::init()
{
    _nanoCore = NanoVG::create();
    if (_nanoCore==nullptr) {
        return false;
    }
    
    _blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
    
    _nanoCore->nvgBeginFrame(1.0);
    _nanoCore->nvgLineJoin(NanoVG::NVG_ROUND);
    _nanoCore->nvgLineCap(NanoVG::NVG_ROUND);
    
    loadShaderVertex();
    
    if (cocos2d::Configuration::getInstance()->supportsShareableVAO()) {
        glGenVertexArrays(1, &_vao);
        cocos2d::GL::bindVAO(_vao);
        
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        
        // vertex
        glEnableVertexAttribArray(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
        glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(NanoVG::NVGvertex), (GLvoid *)(size_t)0);
        
        // texcoord
        glEnableVertexAttribArray(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
        glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(NanoVG::NVGvertex), (GLvoid *)(0 + 2*sizeof(float)));
        
        cocos2d::GL::bindVAO(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        
        // pass~
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
// for android
#if CC_ENABLE_CACHE_TEXTURE_DATA
    auto listener = cocos2d::EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](cocos2d::EventCustom* event){
        this->init();
    });
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
    
    return true;
}

bool NanoDrawNode::loadShaderVertex()
{
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(NANODRAW_SHADER_KEY);
    // 캐시에 없으면 만듬.
    if (program==nullptr) {
        program = cocos2d::GLProgram::createWithByteArrays(sNanoVS, sNanoFS);
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, NANODRAW_SHADER_KEY);
    }
    
    auto state = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    setGLProgramState(state);
    
    // vertex to "a_position" binding
    program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
    // texcoord to "a_texCoord" binding
    program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
    
    _uniformColor = program->getUniformLocation("u_color");
    _uniformStroke = program->getUniformLocation("u_stroke");
    
    return true;
}

void NanoDrawNode::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    auto context = _nanoCore->getContext();
    if (context->cache->npaths==0) {
        // 그릴게 없으면 나간다.
        return;
    }
    
    // 지금 안그리고 onDraw에서 그림
    _customCommand.init(_globalZOrder, transform, flags);
    _customCommand.func = CC_CALLBACK_0(NanoDrawNode::onDraw, this, transform, flags);
    renderer->addCommand(&_customCommand);
}

void NanoDrawNode::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
    auto context = _nanoCore->getContext();
    if (context->cache->npaths==0) {
        // 그릴게 없으면 나간다.
        return;
    }
    
    NanoVG::NVGpath& p = context->cache->paths[0];
    if (p.nstroke==0) {
        return;
    }
    
    getGLProgramState()->apply(transform);
    
    cocos2d::GL::blendFunc(_blendFunc.src, _blendFunc.dst);
    
    const float a = _displayedOpacity/255.0;
    const cocos2d::Vec4 color = cocos2d::Vec4(
                                              a * _displayedColor.r/255.0,
                                              a * _displayedColor.g/255.0,
                                              a * _displayedColor.b/255.0,
                                              a);
    // 적용할 값을 gl shader에 전달
    auto state = getGLProgramState();
    state->setUniformVec4(_uniformColor, color);
    state->setUniformFloat(_uniformStroke, 0.5*_strokeWidth-0.5);
    state->apply(transform);
    
    if (_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        // GL_STREAM_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW 각각 테스트 해야함.
        if (p.nstroke > 0) {
            // 그리냐 ?
            // GL_STREAM_DRAW이 맘에 든다.
            glBufferData(GL_ARRAY_BUFFER, sizeof(NanoVG::NVGvertex)*p.nstroke, p.stroke, GL_STREAM_DRAW);
        } else if (p.nfill) {
            // 칠하냐 ?
            glBufferData(GL_ARRAY_BUFFER, sizeof(NanoVG::NVGvertex)*p.nfill, p.fill, GL_STREAM_DRAW);
        }
        
        _dirty = false;
    }
    
    if (cocos2d::Configuration::getInstance()->supportsShareableVAO()) {
        cocos2d::GL::bindVAO(_vao);
    } else {
        cocos2d::GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_COLOR);
        
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        //vertex
        glVertexAttribPointer(cocos2d::GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(NanoVG::NVGvertex), (GLvoid *)(size_t)0);
        
        // pass
    }
    
    if (p.nstroke>0) {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, p.nstroke);
        /*
        do {
            auto renderer = cocos2d::Director::getInstance()->getRenderer();
            renderer->addDrawnBatches(1);
            renderer->addDrawnVertices(p.nstroke);
        } while(0);
         */
        
        // 위에꺼 대신에 이거 쓰자.. 매크로로 있네...
        CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, p.nstroke);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if (cocos2d::Configuration::getInstance()->supportsShareableVAO()) {
        cocos2d::GL::bindVAO(0);
    }
    
    CHECK_GL_ERROR_DEBUG();
}

void NanoDrawNode::clear()
{
    _nanoCore->nvgReset();
    _dirty = true;
}

const cocos2d::BlendFunc& NanoDrawNode::getBlendFunc() const {
    return _blendFunc;
}

void NanoDrawNode::setBlendFunc(const cocos2d::BlendFunc &blendFunc)
{
    _blendFunc = blendFunc;
}

void NanoDrawNode::drawPath(const std::vector<cocos2d::Vec2> &pointArray, float strokeWidth, bool close)
{
    int numPoints = (int)pointArray.size();
    if (numPoints<2) {
        return;
    }
    
    _strokeWidth = strokeWidth;
    
    _nanoCore->nvgBeginPath();
    _nanoCore->nvgStrokeWidth(_strokeWidth);
    
    const cocos2d::Vec2* ptA = &pointArray[0];
    _nanoCore->nvgMoveTo(ptA->x, ptA->y);
    
    for (int i=1; i<numPoints; i++) {
        ptA = &pointArray[i];
        _nanoCore->nvgLineTo(ptA->x, ptA->y);
    }
    
    if (close) {
        _nanoCore->nvgClosePath();
    }
    
    _nanoCore->nvgStroke();
    
    _dirty = true;
}
