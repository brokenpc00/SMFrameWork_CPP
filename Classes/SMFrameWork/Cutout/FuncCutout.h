//
//  FuncCutout.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#ifndef FuncCutout_h
#define FuncCutout_h

#include "../ImageProcess/ImageProcessFunction.h"
#include "CutoutProcessor.h"
#include <zlib.h>

class DataHolder : public cocos2d::Ref
{
public:
    DataHolder(uint8_t* data, size_t dataLen) : _data(data), _dataLen(dataLen)
    {
        
    }
    
    uint8_t* getData() {return _data;}
    size_t getDataLen() {return _dataLen;}
protected:
    virtual ~DataHolder()
    {
        CC_SAFE_FREE(_data);
    }
private:
    uint8_t* _data;
    size_t _dataLen;
};

class FuncCutout : public ImageProcessFunction
{
public:
    FuncCutout(uint8_t* pixelData, int width, int height, int bpp, uint8_t* maskData, int maskPixelsWide) : _holder(nullptr)
    {
        _processor = new CutoutProcessor(pixelData, width, height, bpp, maskData, maskPixelsWide);
        _maskPixelsWide = maskPixelsWide;
        _maskPixelsHigh = height;
    }
    
protected:
    virtual ~FuncCutout()
    {
        CC_SAFE_DELETE(_processor);
        CC_SAFE_RELEASE(_holder);
    }
    
    virtual bool onProcessInBackground() override
    {
        auto maskData = _processor->cutout();
        
        if (maskData) {
            _holder = new DataHolder(maskData, (size_t)(_maskPixelsWide * _maskPixelsHigh));
            auto param = getParam();
            param->putRef("NO_COMPRESS_DATA", _holder);
            return true;
            
            
            
//            uint8_t* compressedData = nullptr;
//            size_t compressedSize;
//
//            compressData(maskData, (size_t)(_maskPixelsWide * _maskPixelsHigh), &compressedData, compressedSize);
//            CC_SAFE_FREE(maskData);
//
//            if (compressedData && compressedSize > 0) {
//                _holder = new DataHolder(compressedData, compressedSize);
//                auto param = getParam();
//                param->putRef("DATA", _holder);
//                return true;
//            }
        }

        return false;
    }
    
private:
    bool compressData(const uint8_t* inData, size_t inSize, uint8_t** outData, size_t& outSize) {
        // 데이터가 크지 않으므로 한번에 compress 한다.
        outSize = (unsigned long)((1.001 * (inSize + 12) + 1)); // 1.001은 zlib의 magic number
        *outData = (uint8_t*)malloc(outSize);
        
        int ret = compress((Bytef*)*outData, (uLongf*)&outSize, (const Bytef*)inData, (uLong)inSize);
        if (ret != Z_OK) {
            CC_SAFE_FREE(*outData);
            
            CCLOG("[[[[[ PRELOADER >>> zlib compress failed : %d", ret);
            return false;
        }
        
        *outData = (uint8_t*)realloc(*outData, outSize);
        
        return true;
    }
    
private:
    CutoutProcessor* _processor;
    int _maskPixelsWide;
    int _maskPixelsHigh;
    DataHolder* _holder;
};

#endif /* FuncCutout_h */
