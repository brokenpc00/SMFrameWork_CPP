//
//  MaskSprite.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#include "MaskSprite.h"
#include "../Util/nanovgGLDraw/NanoWrapper.h"


#define FILL_SIZE (1.0)
#define IMAGE_LIMIT_SIZE (1080)

#define STROKE_COLOR cocos2d::Color4F::WHITE
#define FILL_COLOR cocos2d::Color4F::WHITE

/*
 static bool compressData(const uint8_t* inData, unsigned long inSize, uint8_t** outData, unsigned long& outSize)
 {
 // 데이터가 크지 않으므로 한번에 compress 한다.
 outSize = (unsigned long)((1.001 * (inSize + 12) + 1)); // 1.001은 zlib의 magic number
 *outData = (uint8_t*)malloc(outSize);
 
 int ret = compress((Bytef*)*outData, (uLongf*)&outSize, (const Bytef*)inData, (uLong)inSize);
 if (ret != Z_OK) {
 CC_SAFE_FREE(*outData);
 
 CCLOG("[PRELOADER] zlib compress failed : %d", ret);
 return false;
 }
 
 return true;
 }
 */


// shader
static const std::string DRAW_ON_SHADER_KEY  = "DRAW_ON_SHADER_KEY";
static const std::string DRAW_OFF_SHADER_KEY = "DRAW_OFF_SHADER_KEY";

static const char* sDrawOnFrag = R"(
    uniform float u_operation;
    uniform vec2 u_align;
    uniform sampler2D u_maskTexture;
    uniform sampler2D u_drawTexture;
    varying vec4 v_fragmentColor;
    varying vec2 v_texCoord;

    void main()
    {
        vec4  c = texture2D(CC_Texture0, v_texCoord);
        float m = texture2D(u_maskTexture, v_texCoord * u_align).z;
        if (m < 0.0) m = 0.0;
        else if (m > 1.0) m = 1.0;
        
        
        float d = texture2D(u_drawTexture, v_texCoord).a;
        if (d < 0.0) d = 0.0;
        else if (d > 1.0) d = 1.0;
        
        
        float a = clamp(m + u_operation * d, 0.0, 1.0);
        if (a < 0.0) a = 0.0;
        else if (a > 1.0) a = 1.0;
        
        gl_FragColor = a * v_fragmentColor * c;
    }
)";

static const char* sDrawOffFrag = R"(
    uniform vec2 u_align;
    uniform sampler2D u_maskTexture;
    varying vec4 v_fragmentColor;
    varying vec2 v_texCoord;

    void main()
    {
        vec4  c = texture2D(CC_Texture0, v_texCoord);
        float a = texture2D(u_maskTexture, v_texCoord * u_align).z;
        gl_FragColor = vec4(v_fragmentColor.rgb * c.rgb, a * c.a * v_fragmentColor.a);
    }
)";


MaskSprite::MaskSprite() : _operation(0)
, _dirtyDraw(false)
, _flushDraw(false)
, _maskData(nullptr)
, _nano(nullptr)
, _maskTexture(nullptr)
, _renderTexture(nullptr)
, _renderCommand(nullptr)
, _strokeColor(STROKE_COLOR)
, _fillColor(FILL_COLOR)
{
    
}

MaskSprite::~MaskSprite()
{
    CC_SAFE_FREE(_maskData);
    CC_SAFE_RELEASE(_maskTexture);
    
    releaseRenderTexture();
}

MaskSprite* MaskSprite::createWithTexture(cocos2d::Texture2D *texture, uint8_t* maskData, int maskStirde)
{
    auto sprite = new (std::nothrow)MaskSprite();
    
    if (sprite!=nullptr) {
        if (sprite->initWithMask(texture, maskData, maskStirde)) {
            sprite->autorelease();
        } else {
            CC_SAFE_DELETE(sprite);
        }
    }
    
    return sprite;
}

bool MaskSprite::initWithMask(cocos2d::Texture2D *texture, uint8_t *maskData, int maskStride)
{
    if (!initWithTexture(texture)) {
        return false;
    }
    
    setOpacityModifyRGB(true);
    setBlendFunc(cocos2d::BlendFunc::ALPHA_PREMULTIPLIED);
    
    auto size = texture->getContentSizeInPixels();
    setContentSize(size);
    
    _maskPixelsWide = (int)size.width;
    _maskPixelsHigh = (int)size.height;
    
    // 가로는 8 배수여야 함.
    if (_maskPixelsWide%8 != 0) {
        // 강제로 맞춘다.
        _maskPixelsWide = (_maskPixelsWide/8 + 1)*8;
    }
    
    size_t length = (size_t)(_maskPixelsWide * _maskPixelsHigh);
    _maskData = (uint8_t*)malloc(length);
    
    if (maskData == nullptr) {
        memset(_maskData, 0xFF, length);
    } else {
        if (maskStride <= 0) {
            maskStride = (int)size.width;
        }
        
        if (maskStride == _maskPixelsWide) {
            size_t length = (size_t)(_maskPixelsWide * _maskPixelsHigh);
            memcpy(_maskData, maskData, length);
        } else {
            int width = (int)size.width;
            for (int yy = 0; yy < _maskPixelsHigh; yy++) {
                auto dst = _maskData + yy * _maskPixelsWide;
                auto src = maskData + yy * maskStride;
                memcpy(dst, src, width);
                memset(dst+width, 0, _maskPixelsWide-width);
            }
        }
    }
    
    _maskTexture = new cocos2d::Texture2D();
    _maskTexture->initWithData(_maskData, length, cocos2d::Texture2D::PixelFormat::I8, _maskPixelsWide, _maskPixelsHigh, size);
    CC_SAFE_RETAIN(_maskTexture);

    _maskAlign = cocos2d::Vec2(size.width/_maskPixelsWide, 1.0);
    enableMaskDraw(false);

    return true;
}

void MaskSprite::initRenderTexture()
{
    if (_renderTexture == nullptr) {
        auto textureSize = getTexture()->getContentSizeInPixels();
        
        _nano = NanoWrapper::create(textureSize, 1.0);
        CC_SAFE_RETAIN(_nano);
        
        _renderTexture = cocos2d::RenderTexture::create((int)textureSize.width, (int)textureSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
        CC_SAFE_RETAIN(_renderTexture);
        
        _renderCommand = new cocos2d::CustomCommand();
        _renderCommand->func = CC_CALLBACK_0(MaskSprite::onDrawMask, this);
    }
}

void MaskSprite::releaseRenderTexture()
{
    if (_renderTexture == nullptr)
    {
        return;
    }
    
    CC_SAFE_RELEASE_NULL(_nano);
    CC_SAFE_RELEASE_NULL(_renderTexture);
    CC_SAFE_DELETE(_renderCommand);
}

uint8_t* MaskSprite::getMaskData()
{
    return _maskData;
}

uint8_t* MaskSprite::cloneMaskData()
{
    auto maskData = (uint8_t*)malloc(_maskPixelsWide * _maskPixelsHigh);
    memcpy(maskData, _maskData, _maskPixelsWide * _maskPixelsHigh);
    return maskData;
}

int MaskSprite::getMaskPixelsWide()
{
    return _maskPixelsWide;
}

void MaskSprite::updateMask(uint8_t* maskData)
{
    if (maskData) {
        memcpy(_maskData, maskData, _maskPixelsWide * _maskPixelsHigh);
    }
    
    _maskTexture->updateWithData(_maskData, 0, 0, _maskPixelsWide, _maskPixelsHigh);
}

void MaskSprite::enableMaskDraw(bool enabled)
{
    cocos2d::GLProgram* program = nullptr;
    
    if (enabled) {
        initRenderTexture();
        
        // Init Fragment Shader
        program = cocos2d::GLProgramCache::getInstance()->getGLProgram(DRAW_ON_SHADER_KEY);
        if (program == nullptr) {
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, sDrawOnFrag);
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, DRAW_ON_SHADER_KEY);
        }
        _uniformDrawTexture = program->getUniformLocation("u_drawTexture");
        _uniformMaskTexture = program->getUniformLocation("u_maskTexture");
        _uniformMaskAlign   = program->getUniformLocation("u_align");
        _uniformOperation   = program->getUniformLocation("u_operation");
    } else {
        releaseRenderTexture();
        
        // Init Fragment Shader
        program = cocos2d::GLProgramCache::getInstance()->getGLProgram(DRAW_OFF_SHADER_KEY);
        if (program == nullptr) {
            program = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, sDrawOffFrag);
            cocos2d::GLProgramCache::getInstance()->addGLProgram(program, DRAW_OFF_SHADER_KEY);
        }
        
        _uniformMaskTexture = program->getUniformLocation("u_maskTexture");
        _uniformMaskAlign   = program->getUniformLocation("u_align");
    }
    
    auto state = cocos2d::GLProgramState::create(program);
    setGLProgramState(state);
}

void MaskSprite::clearMask(GLubyte clearValue)
{
    _dirtyDraw = false;
    _flushDraw = false;
    _commandQueue.clear();
    
    int width = (int)getTexture()->getContentSizeInPixels().width;
    if (width == _maskPixelsWide || clearValue == 0) {
        size_t length = (size_t)(_maskPixelsWide * _maskPixelsHigh);
        memset(_maskData, clearValue, length);
    } else {
        for (int yy = 0; yy < _maskPixelsHigh; yy++) {
            auto dst = _maskData + yy * _maskPixelsWide;
            memset(dst, clearValue, width);
            memset(dst+width, 0, _maskPixelsWide-width);
        }
    }
    
    _maskTexture->updateWithData(_maskData, 0, 0, _maskPixelsWide, _maskPixelsHigh);
    if (_renderTexture) {
        _renderTexture->clear(0, 0, 0, 0);
    }
}

void MaskSprite::flush()
{
    if (_renderTexture) {
        _dirtyDraw = true;
        _flushDraw = true;
    }
}

void MaskSprite::addFill(const std::vector<cocos2d::Vec2>& points)
{
    if (_renderTexture && points.size() > 2) {
        _commandQueue.push_back(CommandItem(Command::ADD_FILL, points, FILL_SIZE));
        _dirtyDraw = true;
    }
}

void MaskSprite::subFill(const std::vector<cocos2d::Vec2> &points)
{
    if (_renderTexture && points.size() > 2) {
        _commandQueue.push_back(CommandItem(Command::SUB_FILL, points, FILL_SIZE));
        _dirtyDraw = true;
    }
}

void MaskSprite::addStroke(const std::vector<cocos2d::Vec2> &points, float strokeSize)
{
    if (_renderTexture && points.size() > 1) {
        _commandQueue.push_back(CommandItem(Command::ADD_STROKE, points, strokeSize));
        _dirtyDraw = true;
    }
}

void MaskSprite::subStroke(const std::vector<cocos2d::Vec2> &points, float strokeSize)
{
    if (_renderTexture && points.size() > 1) {
        _commandQueue.push_back(CommandItem(Command::SUB_STROKE, points, strokeSize));
        _dirtyDraw = true;
    }
}

void MaskSprite::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    auto state = getGLProgramState();
    
    if (_renderTexture) {
        if (_dirtyDraw) {
            _dirtyDraw = false;
            
            _renderTexture->beginWithClear(0, 0, 0, 0);
            _renderCommand->init(_globalZOrder);
            cocos2d::Director::getInstance()->getRenderer()->addCommand(_renderCommand);
            _renderTexture->end();
        }
        state->setUniformTexture(_uniformDrawTexture, _renderTexture->getSprite()->getTexture());
    }
    state->setUniformTexture(_uniformMaskTexture, _maskTexture);
    state->setUniformVec2(_uniformMaskAlign, _maskAlign);
    
    cocos2d::Sprite::draw(renderer, transform, flags);
}

void MaskSprite::onDrawMask()
{
    if (_commandQueue.empty()) {
        if (_flushDraw) {
            _flushDraw = false;
            glClearColor(0, 0, 0, 0);
            glClearStencil(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            _operation = 0;
            
            _maskTexture->updateWithData(_maskData, 0, 0, _maskPixelsWide, _maskPixelsHigh);
        }
    } else {
        int operation = 0, prevOperation = 0;
        int drawCount = 0;
        uint8_t* pixelData = nullptr;
        bool makeUpdated = false;
        
        cocos2d::GL::bindTexture2D((GLuint)0);
        
        _nano->beginFrame();
        {
            // draw mask
            for (auto& item : _commandQueue) {
                if (item.command==ADD_FILL || item.command==ADD_STROKE) {
                    operation = +1;
                } else {
                    operation = -1;
                }
                bool isStroke = (item.command==ADD_STROKE || item.command==SUB_STROKE);
                
                if (operation != prevOperation) {
                    if (drawCount>0) {
                        if (_flushDraw) {
                            // 잠시 멈추고
                            _nano->endFrame();
                            {
                                onUpdateMask(&pixelData, prevOperation);
                                glClearColor(0, 0, 0, 0);
                                glClearStencil(0);
                                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                                makeUpdated = true;
                            }
                            // 다시 시작
                            _nano->beginFrame();
                        }
                        drawCount = 0;
                    }
                    prevOperation = operation;
                    _operation = operation;
                }
                
                _nano->beginPath();
                _nano->setLineCap(NanoWrapper::LineCap::ROUND);
                _nano->setLineJoin(NanoWrapper::LineCap::ROUND);
                _nano->setStrokeColor(_strokeColor);
                _nano->setFillColor(_fillColor);
                
                if (isStroke) {
                    _nano->setStrokeWidth(item.strokeSize);
                } else {
                    _nano->setStrokeWidth(0);
                }
                
                auto* pt = &item.points[0];
                _nano->moveTo(*pt);
                for (int i=1; i<(int)item.points.size(); i++) {
                    pt = &item.points[i];
                    _nano->lineTo(*pt);
                }
                
                if (isStroke) {
                    _nano->stroke();
                } else {
                    _nano->closePath();
                    _nano->fill();
                }
                
                drawCount++;
            }
        }
        _nano->endFrame();
        
        if (drawCount > 0) {
            if (_flushDraw) {
                onUpdateMask(&pixelData, operation);
                glClearColor(0, 0, 0, 0);
                glClearStencil(0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                makeUpdated = true;
                _operation = 0;
            }
        }
        
        if (pixelData) {
            CC_SAFE_FREE(pixelData);
        }
        
        
        cocos2d::GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_NONE);
        cocos2d::GL::bindTexture2D((GLuint)0);
        cocos2d::GL::useProgram((GLuint)0);

        
        // 다 그렸으면 클리어
        _commandQueue.clear();
        _flushDraw = false;

        if (makeUpdated) {
            _maskTexture->updateWithData(_maskData, 0, 0, _maskPixelsWide, _maskPixelsHigh);
        }
    }
    
    // operation 전달 (mask draw enable 일 때만)
    auto state = getGLProgramState();
    state->setUniformFloat(_uniformOperation, _operation);
}

void MaskSprite::onUpdateMask(uint8_t **pixelData, int operation)
{
    if (_renderTexture == nullptr) {
        return;
    }
    
    auto texture = _renderTexture->getSprite()->getTexture();
    auto size = texture->getContentSizeInPixels();
    auto length = (size_t)(size.width * size.height * 4);
    
    if (*pixelData == nullptr) {
        *pixelData = (uint8_t*)malloc(length);
    }
    
    memchr(*pixelData, 0, length);
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, (GLsizei)size.width, (GLsizei)size.height, GL_RGBA, GL_UNSIGNED_BYTE, *pixelData);
    
    float ww = (int)size.width;
    float hh = (int)size.height;
    
    auto src = (*pixelData) + 3;
    if (operation == 1) {
        for (int yy=0; yy<hh; yy++) {
            auto dst = (_maskData + yy*_maskPixelsWide);
            for (int xx=0; xx<ww; xx++) {
                if (*src) {
                    if (*src==0xff) {
                        *dst = 0xff;
                    } else {
                        *dst = (GLubyte)std::min((int)0xff, (int)*dst + (int)*src);
                    }
                }
                src += 4;
                dst++;
            }
        }
    } else if (operation == -1) {
        for (int yy = 0; yy < hh; yy++) {
            auto dst = (_maskData + yy * _maskPixelsWide);
            for (int xx = 0; xx < ww; xx++) {
                if (*src) {
                    if (*src == 0xFF) {
                        *dst = 0;
                    } else {
                        *dst = (GLubyte)std::max((int)0, (int)*dst - (int)*src);
                    }
                }
                src += 4;
                dst++;
            }
        }
    }
}

void MaskSprite::readPixels(uint8_t **pixelData)
{
    auto size = getTexture()->getContentSizeInPixels();
    int ww = size.width;
    int hh = size.height;
    
    auto dataLength = (size_t)(ww * hh * 4);
    *pixelData = (uint8_t*)malloc(dataLength);
    
    auto sprite = cocos2d::Sprite::createWithTexture(getTexture());
    sprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    sprite->setScaleY(-1);
    
    cocos2d::CustomCommand command;
    command.func = [ww, hh, pixelData]() {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, (GLsizei)ww, (GLsizei)hh, GL_RGBA, GL_UNSIGNED_BYTE, *pixelData);
    };
    
    auto renderTexture = cocos2d::RenderTexture::create(ww, hh, cocos2d::Texture2D::PixelFormat::RGBA8888);
    auto renderer = _director->getRenderer();
    renderTexture->beginWithClear(0, 0, 0, 0);
    {
        sprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
        renderer->addCommand(&command);
    }
    renderTexture->end();
    renderer->render();
    
//    // 2) 알파 마스크 합성
//    auto src = *pixelData + 3;
//    for (int yy = 0; yy < hh; yy++) {
//        auto mask = (_maskData + yy * _maskPixelsWide);
//        for (int xx = 0; xx < ww; xx++) {
//            *src = std::min(*src, *mask);
//            src += 4;
//            mask++;
//        }
//    }
}

bool MaskSprite::cropData(const cocos2d::Rect &cropRect, uint8_t **outPixelData, uint8_t **outMaskData, cocos2d::Rect &outRect)
{
    auto size = getTexture()->getContentSizeInPixels();
    int l = std::max(0, (int)cropRect.getMinX());
    int r = std::min((int)size.width, (int)cropRect.getMaxX());
    int t = std::max(0, (int)cropRect.getMinY());
    int b = std::min((int)size.height, (int)cropRect.getMaxY());
    int ww = r-l;
    int hh = b-t;
    cocos2d::Rect rect(l, t, ww, hh);
    
    if (ww <= 0 || hh <= 0)
        return false;
    
    auto dataLength = (size_t)(ww * hh);
    *outPixelData = (uint8_t*)malloc(dataLength*4);
    *outMaskData  = (uint8_t*)malloc(dataLength);
    
    // 1) 이미지 캡쳐
    auto sprite = cocos2d::Sprite::createWithTexture(getTexture());
    sprite->setTextureRect(rect);
    sprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    sprite->setScaleY(-1);
    
    cocos2d::CustomCommand command;
    command.func = [ww, hh, outPixelData]() {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, (GLsizei)ww, (GLsizei)hh, GL_RGBA, GL_UNSIGNED_BYTE, *outPixelData);
    };
    
    auto renderTexture = cocos2d::RenderTexture::create(ww, hh, cocos2d::Texture2D::PixelFormat::RGBA8888);
    auto renderer = _director->getRenderer();
    renderTexture->beginWithClear(0, 0, 0, 0);
    {
        sprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
        renderer->addCommand(&command);
    }
    renderTexture->end();
    renderer->render();
    
    // 2) 알파 마스크 합성
    auto src = *outPixelData;
    auto dst = *outMaskData;
    for (int yy = 0; yy < hh; yy++) {
        auto mask = (_maskData + (t+yy) * _maskPixelsWide) + l;
        for (int xx = 0; xx < ww; xx++) {
            // Premultiplied-Alpha
            *dst = (GLubyte)std::min(*(src+3), *mask);
            src += 4;
            mask++;
            dst++;
        }
    }
    
    outRect.setRect(l, size.height-b, ww, hh);
    
    return true;
}

cocos2d::Sprite* MaskSprite::cropSprite(const cocos2d::Rect& cropRect, cocos2d::Rect& outRect)
{
    auto size = getTexture()->getContentSizeInPixels();
    int l = std::max(0, (int)cropRect.getMinX());
    int r = std::min((int)size.width, (int)cropRect.getMaxX());
    int t = std::max(0, (int)cropRect.getMinY());
    int b = std::min((int)size.height, (int)cropRect.getMaxY());
    int ww = r-l;
    int hh = b-t;
    cocos2d::Rect rect(l, t, ww, hh);
    
    auto dataLength = (size_t)(ww * hh * 4);
    auto pixelData = (uint8_t*)malloc(dataLength);
    
    // 1) 이미지 캡쳐
    auto sprite = cocos2d::Sprite::createWithTexture(getTexture());
    sprite->setTextureRect(rect);
    sprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    sprite->setScaleY(-1);
    
    cocos2d::CustomCommand command;
    command.func = [ww, hh, pixelData]() {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, (GLsizei)ww, (GLsizei)hh, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    };
    auto renderTexture = cocos2d::RenderTexture::create(ww, hh, cocos2d::Texture2D::PixelFormat::RGBA8888);
    auto renderer = _director->getRenderer();
    
    renderTexture->beginWithClear(0, 0, 0, 0);
    {
        sprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
        renderer->addCommand(&command);
    }
    renderTexture->end();
    renderer->render();
    
    // 2) 알파 마스크 합성
    /*
    auto src = pixelData + 3;
    for (int yy = 0; yy < hh; yy++) {
        auto mask = (_maskData + (t+yy) * _maskPixelsWide) + l;
        for (int xx = 0; xx < ww; xx++) {
            *src = (GLubyte)std::min(*src, *mask);
            src += 4;
            mask++;
        }
    }
    */
    
    auto src = pixelData;
    for (int yy = 0; yy < hh; yy++) {
        auto mask = (_maskData + (t+yy) * _maskPixelsWide) + l;
        for (int xx = 0; xx < ww; xx++) {
            // Premultiplied-Alpha
            auto a = (GLubyte)std::min(*(src+3), *mask);
            *(src+0) = (*(src+0) * a) >> 8;
            *(src+1) = (*(src+1) * a) >> 8;
            *(src+2) = (*(src+2) * a) >> 8;
            *(src+3) = a;
            src += 4;
            mask++;
        }
    }
    //*/
    
    // Sprite
    auto texture = new cocos2d::Texture2D();
    texture->initWithData(pixelData, dataLength, cocos2d::Texture2D::PixelFormat::RGBA8888, ww, hh, cocos2d::Size(ww, hh));
    CC_SAFE_FREE(pixelData);
    
    auto result = cocos2d::Sprite::createWithTexture(texture);
    result->setOpacityModifyRGB(true);
    result->setBlendFunc(cocos2d::BlendFunc::ALPHA_PREMULTIPLIED);
    
    CC_SAFE_RELEASE(texture);
    
    outRect.setRect(l, size.height-b, ww, hh);
    
    return result;
}

#include "../Util/use_opencv.h"

cocos2d::Sprite* MaskSprite::extractSprite(const cocos2d::Rect& cropRect, cocos2d::Rect& outRect, float& outScale, int padding)
{
    auto size = getTexture()->getContentSizeInPixels();
    int l = std::max(0, (int)cropRect.getMinX());
    int r = std::min((int)size.width, (int)cropRect.getMaxX());
    int t = std::max(0, (int)cropRect.getMinY());
    int b = std::min((int)size.height, (int)cropRect.getMaxY());
    int w0 = r-l;
    int h0 = b-t;
    cocos2d::Rect rect(l, t, w0, h0);
    
    auto dataLength = (size_t)(w0 * h0 * 4);
    auto pixelData = (uint8_t*)malloc(dataLength);
    
    // 1) 이미지 캡쳐
    auto sprite = cocos2d::Sprite::createWithTexture(getTexture());
    sprite->setTextureRect(rect);
    sprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    sprite->setScaleY(-1);
    
    cocos2d::CustomCommand command;
    command.func = [w0, h0, pixelData]() {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, (GLsizei)w0, (GLsizei)h0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    };
    auto renderTexture = cocos2d::RenderTexture::create(w0, h0, cocos2d::Texture2D::PixelFormat::RGBA8888);
    auto renderer = _director->getRenderer();
    
    renderTexture->beginWithClear(0, 0, 0, 0);
    {
        sprite->visit(renderer, cocos2d::Mat4::IDENTITY, 0);
        renderer->addCommand(&command);
    }
    renderTexture->end();
    renderer->render();
    
    // 2) 알파 마스크 합성
    int ll = r;
    int rr = l;
    int tt = b;
    int bb = t;
    
    auto src = pixelData;
    for (int yy = 0; yy < h0; yy++) {
        auto mask = (_maskData + (t+yy) * _maskPixelsWide) + l;
        for (int xx = 0; xx < w0; xx++) {
            // Premultiplied-Alpha
            auto a = (GLubyte)std::min(*(src+3), *mask);
            
            if (a) {
                if (a > 100) {
                    ll = std::min(xx, ll);
                    rr = std::max(xx, rr);
                    tt = std::min(yy, tt);
                    bb = std::max(yy, bb);
                }
                
                *(src+0) = (*(src+0) * a) >> 8;
                *(src+1) = (*(src+1) * a) >> 8;
                *(src+2) = (*(src+2) * a) >> 8;
                *(src+3) = a;
            } else {
                *((uint32_t*)src) = 0;
            }
            
            src += 4;
            mask++;
        }
    }
    if (rr-ll < 10 || bb-tt < 10) {
        // 너무 작은 스프라이트
        CC_SAFE_FREE(pixelData);
        return nullptr;
    }
    
    int ww = w0;
    int hh = h0;
    if (ll != l || rr != r || tt != t || bb != b) {
        ww = rr-ll;
        hh = bb-tt;
        
        dataLength = (size_t)((ww+padding*2) * (hh+padding*2) * 4);
        auto tmpBuffer = (uint8_t*)malloc(dataLength);
        memset(tmpBuffer, 0, dataLength);
        
        for (int yy = 0; yy < hh; yy++) {
            auto src = pixelData + (w0 * (yy+tt) + ll)*4;
            auto dst = tmpBuffer + ((ww+padding*2) * (yy+padding) + padding)*4;
            memcpy(dst, src, ww * 4);
        }
        
        CC_SAFE_FREE(pixelData);
        pixelData = tmpBuffer;
        
        if (padding > 0) {
            ll -= padding;
            rr += padding;
            tt -= padding;
            bb += padding;
        }
        
        ww += padding*2;
        hh += padding*2;
    }
    
    outRect.setRect(ll, size.height-bb, ww, hh);
    outScale = 1.0;
    
    if (ww > IMAGE_LIMIT_SIZE || hh > IMAGE_LIMIT_SIZE) {
        // 너무 크면 줄인다.
        cv::Mat src(hh, ww, CV_8UC4, pixelData);
        
        outScale = std::min(1000.0/ww, 1000.0/hh);
        ww = (int)(outScale * ww);
        hh = (int)(outScale * hh);
        
        dataLength = (size_t)(ww * hh * 4);
        auto tmpBuffer = (uint8_t*)malloc(dataLength);
        
        cv::Mat dst(hh, ww, CV_8UC4, tmpBuffer);
        cv::resize(src, dst, cv::Size(ww, hh));
        
        CC_SAFE_FREE(pixelData);
        pixelData = tmpBuffer;
    }
    
    // Sprite
    auto texture = new cocos2d::Texture2D();
    texture->initWithData(pixelData, dataLength, cocos2d::Texture2D::PixelFormat::RGBA8888, ww, hh, cocos2d::Size(ww, hh));
    CC_SAFE_FREE(pixelData);
    
    auto result = cocos2d::Sprite::createWithTexture(texture);
    result->setOpacityModifyRGB(true);
    result->setBlendFunc(cocos2d::BlendFunc::ALPHA_PREMULTIPLIED);
    CC_SAFE_RELEASE(texture);
    
    
    return result;
}

void MaskSprite::setDrawColor(const cocos2d::Color4F &strokeColor, const cocos2d::Color4F &fillColor)
{
    _strokeColor = strokeColor;
    _fillColor = fillColor;
}






