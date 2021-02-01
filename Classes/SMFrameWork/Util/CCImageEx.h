//
//  CCImageEx.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 14..
//

#ifndef CCImageEx_h
#define CCImageEx_h

#include <platform/CCImage.h>

class CCImageEx : public cocos2d::Image
{
public:
    bool initWithRawDataEx(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti);
};

#endif /* CCImageEx_h */
