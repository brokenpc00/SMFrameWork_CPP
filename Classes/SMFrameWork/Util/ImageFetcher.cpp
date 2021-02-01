//
//  ImageFetcher.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 21..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "ImageFetcher.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "ImageDownloader.h"
#include <algorithm>
#include <map>
#include <cocos2d.h>

#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)



void ImageFetcherClient::initImageFetcher()
{
    _alive = std::make_shared<bool>(true);
    _fetcher = ImageFetcherHost::create(this);
}

void ImageFetcherClient::releaseImageFetcher()
{
    _alive.reset();
}

bool ImageFetcherClient::canAcceessible()
{
    // 앨범에 접근할 수 있는지 체크
    // 처음 호출 된 것일 수 있어서
    // 아직 안물어 봤는지 또는 권한이 있는지 두가지 타입이 있다.
    // 일단 ALAssetsLibrary부터
    
    //    ALAuthorizationStatus status = [ALAssetsLibrary authorizationStatus];
    //
    //    bool bAcceesible = (status==ALAuthorizationStatusAuthorized || status==ALAuthorizationStatusNotDetermined);
    //
    //    return bAcceesible;
    return true;
}

bool ImageFetcherClient::isAccessPermitted()
{
    // 앨범에 접근할 수 있는지 체크
    // 한번이라도 호출된 적이 있기때문에
    // 결정안함...이 없이 권한이 있는지 없는지만 보면 된다.
    // 일단 ALAssetsLibrary부터
    
    //    ALAuthorizationStatus status = [ALAssetsLibrary authorizationStatus];
    //
    //    bool bAcceesible = status==ALAuthorizationStatusAuthorized;
    //
    //    return bAcceesible;
    return true;
}

void ImageFetcherClient::askAccessPermission()
{
    // 아직 안물어 본 상황이면 물어보고 결과가 deny 또는 restirct이면 access deny를 호출한다.
    // 이거 호출했을 때 처음이면 confirm 창 뜸.
    //    ALAuthorizationStatus status = [ALAssetsLibrary authorizationStatus];
    //    if (status==ALAuthorizationStatusDenied ||
    //        status==ALAuthorizationStatusRestricted) {
    //        onAlbumAccessDenied();
    //    }
}


// Image Fetcher Host
ImageFetcherHost::ImageFetcherHost()
{
    //    if (sAssetsLibrary==nil) {
    //        sAssetsLibrary = [[ALAssetsLibrary alloc] init];
    //    }
    //
    //    sGroupList.clear();
}

ImageFetcherHost::~ImageFetcherHost()
{
    //    for (auto group : sGroupList) {
    //        group = nil;
    //    }
    //
    //    sGroupList.clear();
    //
    //    if (sAssetsLibrary) {
    //        sAssetsLibrary = nil;
    //    }
}

std::shared_ptr<ImageFetcherHost> ImageFetcherHost::create(ImageFetcherClient *client)
{
    std::shared_ptr<ImageFetcherHost> fetcher = std::make_shared<ImageFetcherHost>();
    
    fetcher->_client = client;
    fetcher->_alive = client->_alive;
    
    return fetcher;
}

void ImageFetcherHost::onAlbumAccessDenied()
{
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([&]{
            if (!_alive.expired()) {
                _client->onAlbumAccessDenied();
            }
        });
    }
}

static bool sortFunc(ImageGroupInfo& l, ImageGroupInfo& r)
{
    return l.numPhotos > r.numPhotos;
}

void ImageFetcherHost::requestLoadGroupEnd()
{
    // 이거는 다시 메인

    if (!_alive.expired()) {   // 부른넘이 살아 있으면
        LOGD("[[[[[ requestLoadGroupEnd async end !!!!! : %d", _groupInfos.size());
        _client->onImageGroupResult(_groupInfos);
    } else {
        LOGD("[[[[[ request Load group end NOT ALIVE!!!!!!! !!!!!");
    }
}

void ImageFetcherHost::requestLoadPhotoEnd()
{
    if (!_alive.expired() && _alive.lock()) {
        LOGD("[[[[[ requestLoadPhotoEnd async end !!!!! : %d", _itemInfos.size());
        // 모아놓은 items를 넘겨준다.
        _client->onImageItemResult(_itemInfos);
    } else {
        LOGD("[[[[[ request Load photo end NOT ALIVE!!!!!!! !!!!!");
    }
}

void ImageFetcherHost::requestGroupList()
{

// 그룹 리스트 가져오기
    _groupInfos.clear();
    _groupSequence = 0;
// Activity로 부터 리스트를 가져온다
    

    // thread 처리 한다.
    cocos2d::AsyncTaskPool::getInstance()->enqueue(cocos2d::AsyncTaskPool::TaskType::TASK_IO, CC_CALLBACK_0(ImageFetcherHost::requestLoadGroupEnd, this), nullptr, [&] {
        LOGD("[[[[[ request group list async");
        // 먼저 group count를 가져온다
        cocos2d::JniMethodInfo getGroupCount;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getGroupCount, "org/cocos2dx/cpp/AppActivity", "getGroupCount", "()I")) {
            return;
        }
        
        jint jintGroupCount = (jint)getGroupCount.env->CallStaticIntMethod(getGroupCount.classID, getGroupCount.methodID);
        int groupCount = jintGroupCount;
        
        if (groupCount<=0) {
            return;
        }
        
        getGroupCount.env->DeleteLocalRef(getGroupCount.classID);
        
        
        cocos2d::JniMethodInfo getGroupCoverUrl;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getGroupCoverUrl, "org/cocos2dx/cpp/AppActivity", "getGroupCoverUrl", "(I)Ljava/lang/String;")) {
            return;
        }
        cocos2d::JniMethodInfo getGroupName;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getGroupName, "org/cocos2dx/cpp/AppActivity", "getGroupName", "(I)Ljava/lang/String;")) {
            return;;
        }

        cocos2d::JniMethodInfo getPhotoCount;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getPhotoCount, "org/cocos2dx/cpp/AppActivity", "getPhotoCount", "(I)I")) {
            return;;
        }

        for (int i=0; i<groupCount; i++) {
            if (!_alive.expired() && _alive.lock()) {   // 부른넘이 살아 있으면

                jstring jGroupCoverUrl = (jstring)getGroupCoverUrl.env->CallStaticObjectMethod(getGroupCoverUrl.classID, getGroupCoverUrl.methodID, i);
                std::string groupCoverUrl = cocos2d::JniHelper::jstring2string(jGroupCoverUrl);
                getGroupCoverUrl.env->DeleteLocalRef(jGroupCoverUrl);
                
                jstring jGroupName = (jstring)getGroupName.env->CallStaticObjectMethod(getGroupName.classID, getGroupName.methodID, i);
                std::string groupName = cocos2d::JniHelper::jstring2string(jGroupName);
                getGroupName.env->DeleteLocalRef(jGroupName);
                
                jint jPhotoCount = (jint)getPhotoCount.env->CallStaticIntMethod(getPhotoCount.classID, getPhotoCount.methodID, i);
                int photoCount = jPhotoCount;
                
                ImageGroupInfo info;
                info.seq = _groupSequence++;
                info.key = groupCoverUrl;
                info.thumb.url = groupCoverUrl;
                info.name = groupName;
                info.numPhotos = photoCount;
//                LOGD("[[[[[ photo in group(%d) : %d", info.seq, info.numPhotos);
                
                info.isDefault = (i==0);

                _groupInfos.push_back(info);
            }
        }
        getGroupCoverUrl.env->DeleteLocalRef(getGroupCoverUrl.classID);
        getGroupName.env->DeleteLocalRef(getGroupName.classID);
        getPhotoCount.env->DeleteLocalRef(getPhotoCount.classID);
    });
}

// 포토 리스트를 가져오자
bool ImageFetcherHost::requestPhotoList(const ImageGroupInfo &info, int start, int count)
{
    // 쓰레딩 처리 고민 해야 함.
    if (info.seq>=_groupInfos.size()) {
        // 잘못된 sequence
        return false;
    }

    _requestStart = start;
    _requestCount = count;
    _requestSeq = info.seq;

    //LOGD("[[[[[ requestPhotoList %d : %d", info.seq, info.numPhotos);

    _itemInfos.clear();
    
    // thread 처리 한다.
    cocos2d::AsyncTaskPool::getInstance()->enqueue(cocos2d::AsyncTaskPool::TaskType::TASK_IO, CC_CALLBACK_0(ImageFetcherHost::requestLoadPhotoEnd, this), nullptr, [this, info] {
        //LOGD("[[[[[ Get photo list 1");
        auto group = _groupInfos.at(_requestSeq);
        
        auto hold = shared_from_this();
        
        // get photo list from group index
        
        int photoCount = info.numPhotos;
        //LOGD("[[[[[ Get photo list 2 : (%d)%d", info.seq, info.numPhotos);


        cocos2d::JniMethodInfo getPhotoUrl;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getPhotoUrl, "org/cocos2dx/cpp/AppActivity", "getPhotoUrl", "(II)Ljava/lang/String;")) {
            //LOGD("[[[[[ Get photo list 2 error");
            return;
        }

        cocos2d::JniMethodInfo getPhotoOrientation;
        if (!cocos2d::JniHelper::getStaticMethodInfo(getPhotoOrientation, "org/cocos2dx/cpp/AppActivity", "getPhotoOrientation", "(II)I")) {
            //LOGD("[[[[[ Get photo list 2 error");
            return;
        }



        //LOGD("[[[[[ Get photo list 4");
        for (int i=0; i<info.numPhotos; i++) {
            if (_requestStart>0) {
                //LOGD("[[[[[ SKIP Photo count!!!!!!!!!!!!!!!!");
                _requestStart--;
            } else {
                //LOGD("[[[[[ Get photo list 5 loop 1");

                jstring jPhotoUrl = (jstring)getPhotoUrl.env->CallStaticObjectMethod(getPhotoUrl.classID, getPhotoUrl.methodID, info.seq, i);
                std::string photoUrl = cocos2d::JniHelper::jstring2string(jPhotoUrl);
                getPhotoUrl.env->DeleteLocalRef(jPhotoUrl);

                jint jPhotoOrientation = (jint)getPhotoOrientation.env->CallStaticIntMethod(getPhotoOrientation.classID, getPhotoOrientation.methodID, info.seq, i);
                int orientation = jPhotoOrientation;
                //LOGD("[[[[[ Get photo list 5 loop 2");

                if (!_alive.expired() && _alive.lock()) {
                    
                    //LOGD("[[[[[ Get photo list 5 loop 4");
                    // 안드로이드는 url만 있고 나머지 정보가 없으므로 url만 넣고 나머지는 기본값으로 처리한다.
                    ImageItemInfo photo;
                    photo.url = photoUrl;
                    photo.orient = orientation;
                    photo.width = 0;
                    photo.height = 0;

                    _itemInfos.push_back(photo);

                    //LOGD("[[[[[ rest image count : %d", _requestCount);
                    _requestCount--;
                    if (_requestCount <= 0) {   // 할거 다 했으면 마무리
                        break;
                    }
                }
            }
        }
        getPhotoOrientation.env->DeleteLocalRef(getPhotoOrientation.classID);
        getPhotoUrl.env->DeleteLocalRef(getPhotoUrl.classID);
        //LOGD("[[[[[ Get photo list 7");

    });
    
    return true;
}

// 그룹을 대표하는 썸네일을 가져오자
void ImageFetcherHost::requestGroupThumbnail()
{
    // 안드로이드는 대표 썸네일이 이미 있다.
    // 패스~
//    _client->onImageGroupThumbResult(_groupInfos);
    if (!_alive.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        // loop가 global thread이므로 넘겨 받는 넘은 gl main thread여야 한다. 안그럼 뒤짐.
        scheduler->performFunctionInCocosThread([this] {
            if (!_alive.expired()) {
                _client->onImageGroupThumbResult(_groupInfos);
                //LOGD("[[[[[ requestGroupThumbnail 5");
            }
        });
    }

}
#endif

