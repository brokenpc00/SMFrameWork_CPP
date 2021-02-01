//
//  NanoWrapper.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#ifndef NanoWrapper_h
#define NanoWrapper_h

#include <base/CCRef.h>
#include <base/ccTypes.h>
#include <math/CCGeometry.h>

class NanoWrapper : public cocos2d::Ref
{
public:
    enum LineCap {
        BUTT,
        ROUND,
        SQUARE,
        BEVEL,
        MITER,
    };
    
    static NanoWrapper * create(const cocos2d::Size& contentSize, float devicePixelRatio=1.0);
    
    void beginFrame();
    void endFrame();
    
    void stroke();
    void fill();
    
    void beginPath();
    void setLineCap(LineCap cap);
    void setLineJoin(LineCap join);
    void setStrokeColor(const cocos2d::Color4F& color);
    void setFillColor(const cocos2d::Color4F& color);
    void setStrokeWidth(float strokeWidth);
    void moveTo(const cocos2d::Vec2& pt);
    void lineTo(const cocos2d::Vec2& pt);
    void closePath();
    
protected:
    NanoWrapper();
    virtual ~NanoWrapper();
    
    bool init(const cocos2d::Size& contentSize, float devicePixelRatio);
    
private:
    void* _context;
    cocos2d::Size _contentSize;
    float _devicePixelRatio;
};



#endif /* NanoWrapper_h */
