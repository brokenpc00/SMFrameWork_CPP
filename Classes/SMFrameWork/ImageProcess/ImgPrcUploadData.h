//
//  ImgPrcUploadData.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 9..
//

#ifndef ImgPrcUploadData_h
#define ImgPrcUploadData_h

#include "ImageProcessFunction.h"
#include "../Network/JsonWrapper.h"
#include <vector>

class ImgPrcUploadData : public ImageProcessFunction
{
public:
    virtual bool onPreProcess(cocos2d::Node* node) override = 0;
    
    virtual bool onProcessInBackground() override;
    
    virtual cocos2d::Sprite* onPostProcess() override;

protected:
    struct DestInfo {
        std::string bucketName;
        std::string filePath;
    };
    
    struct MemoryStruct {
        uint8_t* ptr;
        size_t remainBytes;
        size_t totalBytes;
        
        MemoryStruct() {}
        
        MemoryStruct(uint8_t* buffer, size_t size) : ptr(buffer), remainBytes(size), totalBytes(size) {}
    };
    
    struct UploadInfo {
        
        std::vector<DestInfo> dest;
        
        static bool parseFunc(JsonWrapper<UploadInfo>& p, UploadInfo& item);
    };

    // 어디로 올릴지 물어보고
//    requestUploadInfo();
    
    // upload 한다.
//    bool processUploadData(const UploadInfo& info, int seq, uint8_t* data, size_t size);

    static size_t outputStringCallbackProc(void* buffer, size_t size, size_t count, void* userdata);

    static size_t uploadDataCallbackProc(void* buffer, size_t size, size_t count, void* userdata);

    static size_t writeMemoryCallbackProc(void* buffer, size_t size, size_t count, void* userdata);

public:
    ImgPrcUploadData(float scaleFactor);
    virtual ~ImgPrcUploadData();

private:
    std::string _response;
    MemoryStruct _memory;
    
protected:
    float _scaleFactor;
};


#endif /* ImgPrcUploadData_h */
