//
//  ImageFetcher.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 21..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ImageFetcher_h
#define ImageFetcher_h

#include "ImageItemInfo.h"
#include <memory>
#include <vector>


// Photo gallery를 위하여 만든 Fetcher
// Image group을 가져오고
// group 안에 image를 가져온다.
// iOS의 경우 Asset URL만 가져오고 실제 이미지는 안 가져온다.
// Android의 경우 file Path만 가져오고 실제 이미지는 안 가져온다.
// ImageItemInfo에 실제 이미지는 없다. URL만 있을 뿐....
// 실제 이미지는 URL 또는 Path를 통해 가져오는 DonwloadTask에서 가져온다.

class ImageFetcherHost;


class ImageFetcherClient {
public:
    // 접근 가능한지 (for iOS)
    static bool canAcceessible();
    // 접근 권한지 있는지
    static bool isAccessPermitted();
    
    void initImageFetcher();
    
    void releaseImageFetcher();
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    // 폴더 리스트를 가져왔을 때
    virtual void onImageGroupResult(const std::vector<ImageGroupInfo>& groupInfos) = 0;

    // 폴더 썸네일을 가져왔을 때
    virtual void onImageGroupThumbResult(const std::vector<ImageGroupInfo>& groupInfos) = 0;

    // 아이템 리스트를 가져왔을 때
    virtual void onImageItemResult(const std::vector<ImageItemInfo>& itemInfos) = 0;

    // 폴더에 접근 거부 되었을 때
    virtual void onAlbumAccessDenied() = 0;

    // 폴더 접근 권한 alert
    void askAccessPermission();

protected:
    std::shared_ptr<ImageFetcherHost> _fetcher;

    std::shared_ptr<bool> _alive;
#else
    // 폴더 정보 가져 왔을때
    virtual void onImageFetcherInit(const std::vector<ImageGroupInfo>* groupInfos) = 0;

    // 폴더 리스트를 가져왔을 때
    virtual void onImageGroupResult(const std::vector<ImageGroupInfo>* groupInfos) = 0;

    // 폴더에 접근 거부 되었을 때
    virtual void onAlbumAccessDenied() = 0;

    // 폴더에 접근 권한 있을 때
    virtual void onAlbumAccessAuthorized() = 0;

    // 폴더 접근 권한 alert
    void askAccessPermission();

protected:
    std::shared_ptr<ImageFetcherHost> _fetcher;

    std::shared_ptr<bool> _imageFetcherAliveFlag;

#endif
    friend class ImageFetcherHost;
};

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
class ImageFetcherHost : public std::enable_shared_from_this<ImageFetcherHost>
{
public:
    static std::shared_ptr<ImageFetcherHost> create(ImageFetcherClient* client);

    ImageFetcherHost();
    virtual ~ImageFetcherHost();

    // 폴더 리스트 주세요.
    void requestGroupList();

    // 폴더 리스트의 썸네일 주세요.
    void requestGroupThumbnail();

    // 폴더 안에 이미지 리스트 주세요.
    bool requestPhotoList(const ImageGroupInfo& info, int start, int count);

    void requestLoadGroupEnd();
    void requestLoadPhotoEnd();


    // 폴더 접근 거부 됨
    void onAlbumAccessDenied();

private:
    ImageFetcherClient * _client;
    std::vector<ImageGroupInfo> _groupInfos;
    std::vector<ImageItemInfo> _itemInfos;
    std::weak_ptr<bool> _alive;

    int _groupSequence;
    int _requestSeq;
    int _requestStart;
    int _requestCount;
};
#else


class ImageFetcherHost : public std::enable_shared_from_this<ImageFetcherHost>
{
public:
    static std::shared_ptr<ImageFetcherHost> create(ImageFetcherClient* client);

    ImageFetcherHost();
    virtual ~ImageFetcherHost();
    void onAlbumAccessDenied();

    void onAlbumAccessAuthorized();

    void requestAlbumList();

    void* fetchPhotoAssetObject(int groupIndex, int index);

    void* getAlbumPosterAssetObject(int groupIndex);

    ImageGroupInfo* getGroupInfo(int groupIndex);

    ImageItemInfo getImageItemInfo(int groupIndex, int index);

    int getGroupCount();

    void onInit();

private:
    void initCameraRoll();

    void initAlbumList();

    void* fetchPosterAssetObject(ImageGroupInfo& group);

private:

    bool _initAlbum;

    ImageFetcherClient* _imageFetcherClient;

    std::vector<ImageGroupInfo> _groups;

    int _groupSequence;

    // for android
    int _currentGroupIndex;

    std::weak_ptr<bool> _imageFetcherAliveFlag;

};

#endif
#endif /* ImageFetcher_h */
