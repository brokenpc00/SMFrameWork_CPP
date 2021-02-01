//
//  CCImageEx.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 14..
//

#include "CCImageEx.h"
#include <renderer/CCTexture2D.h>

bool CCImageEx::initWithRawDataEx(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti)
{
    bool ret = false;
    do
    {
        CC_BREAK_IF(0 == width || 0 == height);
        
        _height   = height;
        _width    = width;
        _hasPremultipliedAlpha = preMulti;
        
        int bytesPerComponent;
        if (bitsPerComponent == 24) {
            _renderFormat = cocos2d::Texture2D::PixelFormat::RGB888;
            bytesPerComponent = 3;
        } else {
            _renderFormat = cocos2d::Texture2D::PixelFormat::RGBA8888;
            bytesPerComponent = 4;
        }
        
        _dataLen = height * width * bytesPerComponent;
        _data = static_cast<unsigned char*>(malloc(_dataLen * sizeof(unsigned char)));
        CC_BREAK_IF(! _data);
        memcpy(_data, data, _dataLen);
        
        ret = true;
    } while (0);
    
    return ret;
}
