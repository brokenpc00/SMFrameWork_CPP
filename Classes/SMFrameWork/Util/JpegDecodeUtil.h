//
//  JpegDecodeUtil.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 8..
//
//

#ifndef JpegDecodeUtil_h
#define JpegDecodeUtil_h
#include <cocos2d.h>

class JpegDecodeUtil {
public:
    static bool decodeDimension(uint8_t* data, size_t dataLen, int& outWidth, int& outHeight);
    
    static bool decode(uint8_t* data, size_t dataLen, uint8_t** outData, size_t& outDataLen, int& outWidth, int& outHeight, const int inSampleSize = 0);
};



#endif /* JpegDecodeUtil_h */
