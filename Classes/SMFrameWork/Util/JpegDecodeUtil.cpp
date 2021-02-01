//
//  JpegDecodeUtil.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 8..
//
//

#include "JpegDecodeUtil.h"
#include <setjmp.h>
#include <platform/CCPlatformConfig.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <external/jpeg/include/ios/jpeglib.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <external/jpeg/include/android/jpeglib.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <external/jpeg/include/mac/jpeglib.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <external/jpeg/include/win32/jpeglib.h>
#endif

namespace
{
    struct MyErrorMgr {
        struct jpeg_error_mgr pub;
        jmp_buf setjmp_buffer;
    };
    
    typedef struct MyErrorMgr * MyErrorPtr;
    
    METHODDEF(void)
    myErrorExit(j_common_ptr cinfo) {
        MyErrorPtr myerr = (MyErrorPtr) cinfo->err;
        
        char buffer[JMSG_LENGTH_MAX];
        
        (*cinfo->err->format_message) (cinfo, buffer);
        
        CCLOG("jpeg error: %s", buffer);
        
        longjmp(myerr->setjmp_buffer, 1);
    }
}


bool JpegDecodeUtil::decodeDimension(uint8_t* data, size_t dataLen, int& outWidth, int& outHeight) {
    
    struct jpeg_decompress_struct cinfo;
    struct MyErrorMgr jerr;
    
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = myErrorExit;
    
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        
        return false;
    }
    
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, const_cast<unsigned char*>(data), dataLen);
    jpeg_read_header(&cinfo, TRUE);
    
    outWidth = cinfo.image_width;
    outHeight = cinfo.image_height;
    
    jpeg_destroy_decompress(&cinfo);
    
    return true;
}

bool JpegDecodeUtil::decode(uint8_t* data, size_t dataLen, uint8_t** outData, size_t& outDataLen, int& outWidth, int& outHeight, const int inSampleSize) {
    
    struct jpeg_decompress_struct cinfo;
    struct MyErrorMgr jerr;
    JSAMPROW row_pointer[1] = { 0 };
    unsigned long location = 0;
    
    bool ret = false;
    do {
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = myErrorExit;
        
        if (setjmp(jerr.setjmp_buffer)) {
            jpeg_destroy_decompress(&cinfo);
            break;
        }
        
        jpeg_create_decompress(&cinfo);
        
        jpeg_mem_src(&cinfo, const_cast<unsigned char*>(data), dataLen);
        jpeg_read_header(&cinfo, TRUE);
        
        if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
            jpeg_destroy_decompress(&cinfo);
            break;
        }
        cinfo.out_color_space = JCS_RGB;
        
        if (inSampleSize > 1 && inSampleSize <= 16) {
            cinfo.scale_num = 1;
            cinfo.scale_denom = inSampleSize;
        }
        
        jpeg_start_decompress(&cinfo);
        
        outWidth  = cinfo.output_width;
        outHeight = cinfo.output_height;
        
        outDataLen = outWidth * outHeight * cinfo.output_components;
        *outData = static_cast<unsigned char*>(malloc(outDataLen * sizeof(unsigned char)));
        if (*outData == nullptr) {
            jpeg_destroy_decompress(&cinfo);
            break;
        }
        
        while (cinfo.output_scanline < outHeight) {
            row_pointer[0] = (unsigned char*)(*outData + location);
            location += outWidth * cinfo.output_components;
            jpeg_read_scanlines(&cinfo, row_pointer, 1);
        }
        
        jpeg_destroy_decompress( &cinfo );
        ret = true;
    } while (0);
    
    return ret;
}




