//
//  SMDownloader.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 7..
//

#include "SMDownloader.h"
#include <cocos2d.h>

#define TIMEOUT_IN_SECOND   (10L)
#define LOW_SPEED_LIMIT     (1L)
#define LOW_SPEED_TIME      (5L)
#define MAX_REDIRS          (2L)
#define MAX_WIDTH       (160)

#define HTTP_SUCCESS 200

#define __CHECK_THREAD_INTERRUPTED__  { std::this_thread::sleep_for(std::chrono::milliseconds(1)); if (!getTask()->isRunning()) break; }

SMDownloader::SMDownloader() : _listener(nullptr)
{
    
}

SMDownloader::~SMDownloader()
{
    
}

void SMDownloader::addDownloadTask(std::string name, std::string url, FileManager::FileType fileType, bool bDiskCache, int tag)
{
    auto info = new (std::nothrow)DownloadInfo();
    info->downloadUrl = url;
    info->bSave = bDiskCache;
    info->name = name;
    info->fileType = fileType;
    WorkThread::getInstance().doWork(this, new SMDownloadFunction(tag, info), tag);
    CC_SAFE_RELEASE_NULL(info);
}

void SMDownloader::onJobFinish(const int tag, const bool success, Intent *intent)
{
    if (_listener) {
        _listener->onDownloadFinished(success, tag, intent);
    }
}

void SMDownloader::onJobProgress(const int tag, const int idx, const int total, const float progress, std::string desc)
{
    if (_listener) {
        _listener->onDownloadProgress(tag, idx, total, progress);
    }
}

bool SMDownloadFunction::onBeforeDoJob()
{
    if (_info->downloadUrl=="") {
        return false;
    }
    
    return true;
}

bool SMDownloadFunction::onJobInBackground()
{
    // 여기는 back ground thread이다.
    // 실제 네트웍으로 가져와야 한다.
    
    // local에 있는지???
    auto fileUtils = cocos2d::FileUtils::getInstance();
    auto downfolder = FileManager::getInstance()->getFullPath(_info->fileType);
    if (!fileUtils->isDirectoryExist(downfolder)) {
        fileUtils->createDirectory(downfolder);
    }
    
    auto fileName = downfolder + _info->name;
    if (!fileUtils->isFileExist(fileName)) {
        CURL * curl = nullptr;
        _bSuccess = false;
        do {
            __CHECK_THREAD_INTERRUPTED__;
            
            curl = curl_easy_init();
            
            if (curl==nullptr) {
                CCLOG("Failed to CURL Init()");
                break;
            }
            
            __CHECK_THREAD_INTERRUPTED__;
            
            // curl option setting
            initCurlHandleProc(curl, true);
            
            __CHECK_THREAD_INTERRUPTED__;
            
            CURLcode retCode = curl_easy_perform(curl);
            
            // curl error code check
            if (retCode != CURLE_OK) {
                CCLOG("Failed to curl download!!!!");
                break;
            }
            
            long responseCode = 0;
            
            retCode = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &responseCode);
            
            if (retCode != CURLE_OK || responseCode!=HTTP_SUCCESS) {
                CCLOG("Failed to COM error for curl download");
                break;
            }
            
            // 통신 완료... 잘 받아졌으면...
            
            // 사용한 curl 정리
            if (curl) {
                curl_easy_cleanup(curl);
            }
            
            if (_info->bSave) {
                cocos2d::Data fileData;
                fileData.fastSet((u_char*)_info->data.data(), _info->data.size());

                _bSuccess = cocos2d::FileUtils::getInstance()->writeDataToFile(fileData, fileName);
                if (!_bSuccess) {
                    break;
                }
                fileData.fastSet(nullptr, 0);

                _info->localSavedPath = fileName;
                // 저장하는 경우는 data를 지운다.
                _info->data.clear();
            }
            
            return true;
        } while (0);
        
        if (curl) {
            curl_easy_cleanup(curl);
        }
        
        return false;

    } else {
        
        _info->localSavedPath = fileName;
        return true;
    }
}

Intent * SMDownloadFunction::onAfterDoneJob()
{
    auto param = Intent::create();
    
    auto info = new (std::nothrow)DownloadInfo();
    info->autorelease();
    info->bSave = _info->bSave;
    info->downloadUrl = _info->downloadUrl;
    info->localSavedPath = _info->localSavedPath;
    info->name = _info->name;
    info->tag = _info->tag;
    info->data.clear();
    
    if (!_info->bSave) {
        // 저장하지 않는 경우 data를 넘겨줌.. 웬만하면 쓰지 말자
        info->data.insert(info->data.end(), _info->data.begin(), _info->data.end());
    }
    
    param->putRef("DOWNLOAD_INFO", info);
    param->putBool("SUCCESS", _bSuccess);
    param->putInt("TAG", _tag);
    
    return param;
}

int SMDownloadFunction::progressCallbackProc(void *callerPtr,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow)
{
    SMDownloadFunction * func = static_cast<SMDownloadFunction*>(callerPtr);
    if (func && dltotal>0) {
        float progress = (float)dlnow/(float)dltotal * 100.0f;
        if (progress>0.1f) {
            func->getTask()->onProgress(func->_tag, (int)dltotal, progress, "");
        }
    }
//    CCLOG("[[[[[ TotalToDownload : %d, NowDownloaded : %d, TotalToUpload : %d, NowUploaded : %d", dltotal, dlnow, ultotal, ulnow);
    return 0;
}

size_t SMDownloadFunction::writeDataProc(unsigned char* buffer, size_t size, size_t count)
{
    size_t readSize = size * count;
    if (readSize>0) {
        std::copy(buffer, buffer + readSize, std::back_inserter(_info->data));
    }
    return readSize;
}

size_t SMDownloadFunction::outputDataCallbackProc(void *buffer, size_t size, size_t count, void *callerPtr)
{
    SMDownloadFunction * func = static_cast<SMDownloadFunction*>(callerPtr);
    
    if (!func->getTask()->isRunning()) {
        return -1;
    }
    return func->writeDataProc((unsigned char*)buffer, size, count);
}

void SMDownloadFunction::initCurlHandleProc(CURL *handle, bool existBody)
{
    // CURL Handler...
    // CURL settting
    // thread callback func
    
    // url setting
    curl_easy_setopt(handle, CURLOPT_URL, _info->downloadUrl.c_str());

    // time out signal setting
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
    
    // write func settng
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, outputDataCallbackProc);
    
    // write data setting
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

    // progress func setting
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, progressCallbackProc);

    // progress setting
    curl_easy_setopt(handle, CURLOPT_XFERINFODATA, this);

    // fail on error settting
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);
    
    if (!existBody) {
        // only header
        curl_easy_setopt(handle, CURLOPT_HEADER, 1);
        // no body
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    }
    
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, TIMEOUT_IN_SECOND);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
    curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);

    if (MAX_REDIRS) {
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(handle, CURLOPT_MAXREDIRS, MAX_REDIRS);
    }

    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
}
