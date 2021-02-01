//
//  NanoWrapper.c
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#include "NanoWrapper.h"


#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"


NanoWrapper::NanoWrapper() : _context(nullptr)
{
    
}

NanoWrapper::~NanoWrapper()
{
    if (_context) {
        auto ctx = (NVGcontext*)_context;
        nvgDeleteGLES2(ctx);
    }
}

NanoWrapper * NanoWrapper::create(const cocos2d::Size &contentSize, float devicePixelRatio)
{
    auto node = new (std::nothrow)NanoWrapper();
    if (node!=nullptr) {
        if (node->init(contentSize, devicePixelRatio)) {
            node->autorelease();
        } else {
            CC_SAFE_DELETE(node);
        }
    }
    
    return node;
}

bool NanoWrapper::init(const cocos2d::Size &contentSize, float devicePixelRatio)
{
//    _context = (void*)nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    _context = (void*)nvgCreateGLES2(NVG_ANTIALIAS);
    _contentSize = contentSize;
    _devicePixelRatio = devicePixelRatio;
    return true;
}


// util fuction
static NVGlineCap convertLineCap(NanoWrapper::LineCap cap)
{
    NVGlineCap nvgValue;
    
    switch (cap) {
        case NanoWrapper::LineCap::BUTT:    nvgValue = NVGlineCap::NVG_BUTT; break;
        case NanoWrapper::LineCap::ROUND:   nvgValue = NVGlineCap::NVG_ROUND; break;
        case NanoWrapper::LineCap::SQUARE:  nvgValue = NVGlineCap::NVG_SQUARE; break;
        case NanoWrapper::LineCap::BEVEL:   nvgValue = NVGlineCap::NVG_BEVEL; break;
        case NanoWrapper::LineCap::MITER:   nvgValue = NVGlineCap::NVG_MITER; break;
    }
    
    return nvgValue;
}

static NVGcolor convertColor(const cocos2d::Color4F& color)
{
    NVGcolor nvgValue;
    nvgValue.r = color.r;
    nvgValue.g = color.g;
    nvgValue.b = color.b;
    nvgValue.a = color.a;
    
    return nvgValue;
}



// wrapper function
void NanoWrapper::beginFrame()
{
    auto ctx = (NVGcontext*)_context;
    nvgBeginFrame(ctx, (int)_contentSize.width, (int)_contentSize.height, _devicePixelRatio);
}

void NanoWrapper::endFrame()
{
    auto ctx = (NVGcontext*)_context;
    nvgEndFrame(ctx);
}

void NanoWrapper::stroke()
{
    auto ctx = (NVGcontext*)_context;
    nvgStroke(ctx);
}

void NanoWrapper::fill()
{
    auto ctx = (NVGcontext*)_context;
    nvgFill(ctx);
}

void NanoWrapper::beginPath()
{
    auto ctx = (NVGcontext*)_context;
    nvgBeginPath(ctx);
}

void NanoWrapper::setLineCap(LineCap cap)
{
    auto ctx = (NVGcontext*)_context;
    nvgLineCap(ctx, convertLineCap(cap));
}

void NanoWrapper::setLineJoin(LineCap join)
{
    auto ctx = (NVGcontext*)_context;
    nvgLineJoin(ctx, convertLineCap(join));
}

void NanoWrapper::setStrokeColor(const cocos2d::Color4F &color)
{
    auto ctx = (NVGcontext*)_context;
    nvgStrokeColor(ctx, convertColor(color));
}

void NanoWrapper::setFillColor(const cocos2d::Color4F &color)
{
    auto ctx = (NVGcontext*)_context;
    nvgFillColor(ctx, convertColor(color));
}

void NanoWrapper::setStrokeWidth(float strokeWidth)
{
    auto ctx = (NVGcontext*)_context;
    nvgStrokeWidth(ctx, strokeWidth);
}

void NanoWrapper::moveTo(const cocos2d::Vec2 &pt)
{
    auto ctx = (NVGcontext*)_context;
    nvgMoveTo(ctx, pt.x, pt.y);
}

void NanoWrapper::lineTo(const cocos2d::Vec2 &pt)
{
    auto ctx = (NVGcontext*)_context;
    nvgLineTo(ctx, pt.x, pt.y);
}

void NanoWrapper::closePath()
{
    auto ctx = (NVGcontext*)_context;
    nvgClosePath(ctx);
}


