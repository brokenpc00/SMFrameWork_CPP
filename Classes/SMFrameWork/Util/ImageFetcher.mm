//
//  ImageFetcher.m
//  iPet
//
//  Created by KimSteve on 2017. 6. 21..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "ImageFetcher.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#include "ImageDownloader.h"
#include <Photos/Photos.h>
#include <algorithm>
#include <map>

void ImageFetcherClient::initImageFetcher()
{
    _imageFetcherAliveFlag = std::make_shared<bool>(true);
    _fetcher = ImageFetcherHost::create(this);
}

void ImageFetcherClient::releaseImageFetcher()
{
    _imageFetcherAliveFlag.reset();
}

bool ImageFetcherClient::canAcceessible()
{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    return (status == PHAuthorizationStatusAuthorized || status == PHAuthorizationStatusNotDetermined);
}

bool ImageFetcherClient::isAccessPermitted()
{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    return (status == PHAuthorizationStatusAuthorized);
}

void ImageFetcherClient::askAccessPermission()
{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    if (status == PHAuthorizationStatusDenied || status == PHAuthorizationStatusRestricted) {
        onAlbumAccessDenied();
    } else if (status == PHAuthorizationStatusNotDetermined) {
        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
            switch (status) {
                case PHAuthorizationStatusAuthorized:
                    onAlbumAccessAuthorized();
                    break;
                default:
                case PHAuthorizationStatusRestricted:
                case PHAuthorizationStatusDenied:
        onAlbumAccessDenied();
                    break;
            }
        }];
    }
}

ImageFetcherHost::ImageFetcherHost() : _initAlbum(false)
{
}

ImageFetcherHost::~ImageFetcherHost()
{
    
    for (auto group : _groups) {
        if (group.collection) {
            auto collection = static_cast<PHAssetCollection*>(group.collection);
            if (collection) {
                [collection release];
            }
            group.collection = nullptr;
    }
    
        if (group.posterAsset) {
            auto posterAsset = static_cast<PHAsset*>(group.posterAsset);
            if (posterAsset) {
                [posterAsset release];
            }
            group.posterAsset = nullptr;
        }
        
        if (group.assets) {
            auto assets = static_cast<PHFetchResult*>(group.assets);
            if (assets) {
                [assets release];
            }
            group.assets = nullptr;
    }
    }
    
    _groups.clear();
}

void ImageFetcherHost::onInit()
{
    initCameraRoll();
}

std::shared_ptr<ImageFetcherHost> ImageFetcherHost::create(ImageFetcherClient* client)
{
    std::shared_ptr<ImageFetcherHost> fetcher = std::make_shared<ImageFetcherHost>();
    
    fetcher->_imageFetcherClient = client;
    fetcher->_imageFetcherAliveFlag = client->_imageFetcherAliveFlag;
    
    return fetcher;
}

void ImageFetcherHost::onAlbumAccessDenied()
{
    if (!_imageFetcherAliveFlag.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_imageFetcherAliveFlag.expired()) {
                _imageFetcherClient->onAlbumAccessDenied();
            }
        });
    }
}

void ImageFetcherHost::onAlbumAccessAuthorized()
{
    if (!_imageFetcherAliveFlag.expired()) {
        auto scheduler = cocos2d::Director::getInstance()->getScheduler();
        scheduler->performFunctionInCocosThread([this] {
            if (!_imageFetcherAliveFlag.expired()) {
                _imageFetcherClient->onAlbumAccessAuthorized();
            }
        });
    }
}

static bool sortFunc(ImageGroupInfo& l, ImageGroupInfo& r)
{
    return l.numPhotos > r.numPhotos;
}

void ImageFetcherHost::initCameraRoll()
{
    @autoreleasepool {
        PHFetchResult* smartAlbums = [PHAssetCollection fetchAssetCollectionsWithType: PHAssetCollectionTypeSmartAlbum
                                                                              subtype: PHAssetCollectionSubtypeSmartAlbumUserLibrary
                                                                              options: nullptr];
    
        PHFetchOptions* opts = [PHFetchOptions new];
        opts.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey: @"creationDate" ascending: NO]];
        opts.predicate = [NSPredicate predicateWithFormat:@"mediaType = %d", PHAssetMediaTypeImage];
                        
        for (int i = 0; i < (int)smartAlbums.count; i++) {
            PHAssetCollection* collection = smartAlbums[i];
            PHFetchResult* fetchResult = [PHAsset fetchAssetsInAssetCollection: collection
                                                                       options: opts];
                        
            [collection retain];
                        
            ImageGroupInfo info;
            info.isDefault = true;
            info.name = std::string([collection.localizedTitle UTF8String]);
            info.numPhotos = (int)fetchResult.count;
            info.collection = collection;
            info.seq = i;
                        
            _groups.push_back(info);
                            }
        
        [opts release];
                        }
                        
    _imageFetcherClient->onImageFetcherInit(&_groups);
}

void ImageFetcherHost::initAlbumList()
{
    @autoreleasepool {
        PHFetchOptions* opts = [PHFetchOptions new];
        opts.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey: @"creationDate" ascending: NO]];
        opts.predicate = [NSPredicate predicateWithFormat:@"mediaType=%d", PHAssetMediaTypeImage];
        
        // 유저앨범
        PHFetchResult* userAlbums = [PHAssetCollection fetchAssetCollectionsWithType: PHAssetCollectionTypeAlbum
                                                                             subtype: PHAssetCollectionSubtypeAlbumRegular
                                                                             options: nil];
            
            
        std::vector<ImageGroupInfo> userGroups;
        for (int i = 0; i < (int)userAlbums.count; i++) {
            PHAssetCollection* collection = userAlbums[i];
            PHFetchResult* fetchResult = [PHAsset fetchAssetsInAssetCollection: collection
                                                                       options: opts];
                
            if (fetchResult.count <= 0)
                continue;
                    
            [collection retain];
                    
            ImageGroupInfo info;
                        info.isDefault = true;
            info.name = std::string([collection.localizedTitle UTF8String]);
            info.numPhotos = (int)fetchResult.count;
            info.collection = collection;
            
            userGroups.push_back(info);
                    }
                    
        // 사진개수 순 정렬
        std::sort(userGroups.begin(), userGroups.end(), sortFunc);
                    
        // Camera Roll은 이미 있으므로 뒤에 붙임
        _groups.insert(_groups.end(), userGroups.begin(), userGroups.end());
                    
        // 스마트 앨범
        PHFetchResult* smartAlbums = [PHAssetCollection fetchAssetCollectionsWithType: PHAssetCollectionTypeSmartAlbum
                                                                              subtype: PHAssetCollectionSubtypeAlbumRegular
                                                                              options: nullptr];
        
        for (int i = 0; i < (int)smartAlbums.count; i++) {
            PHAssetCollection* collection = smartAlbums[i];
            PHFetchResult* fetchResult = [PHAsset fetchAssetsInAssetCollection: collection
                                                                       options: opts];
            
            if (fetchResult.count <= 0)
                continue;
            
            if (collection.assetCollectionSubtype == PHAssetCollectionSubtypeSmartAlbumUserLibrary) {
                // Camera Roll은 이미 있음
                continue;
            }
    
            [collection retain];
    
            ImageGroupInfo info;
            info.isDefault = true;
            info.name = std::string([collection.localizedTitle UTF8String]);
            info.numPhotos = (int)fetchResult.count;
            info.collection = collection;
        
            _groups.push_back(info);
        }
    
        [opts release];
    }
    
    int size = (int)_groups.size();
    for (int i = 0; i < size; i++) {
        _groups[i].seq = i;
    }
}

void ImageFetcherHost::requestAlbumList()
{
    if (!_initAlbum) {
        _initAlbum = true;
        initAlbumList();
    }
    
    _imageFetcherClient->onImageGroupResult(&_groups);
}

void* ImageFetcherHost::fetchPhotoAssetObject(int groupIndex, int index)
{
    
    auto& group = _groups.at(groupIndex);
    
    if (group.assets == nullptr) {
        auto collection = static_cast<PHAssetCollection*>(group.collection);
  
        PHFetchOptions* opts = [PHFetchOptions new];
        opts.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey: @"creationDate" ascending: NO]];
        opts.predicate = [NSPredicate predicateWithFormat:@"mediaType = %d", PHAssetMediaTypeImage];
        opts.wantsIncrementalChangeDetails = NO;
                                
        PHFetchResult* assets = [PHAsset fetchAssetsInAssetCollection: collection options: opts];
        [opts release];
                                    
        [assets retain];
                                    
        group.assets = assets;
                                    }
                
    auto assets = static_cast<PHFetchResult*>(group.assets);
    
    if (assets) {
        return assets[index];
            }
            
    return nullptr;
}

void* ImageFetcherHost::fetchPosterAssetObject(ImageGroupInfo& group)
{
    auto collection = static_cast<PHAssetCollection*>(group.collection);
    
    PHFetchOptions* opts = [PHFetchOptions new];
    opts.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey: @"creationDate" ascending: NO]];
    opts.predicate = [NSPredicate predicateWithFormat:@"mediaType = %d", PHAssetMediaTypeImage];
    opts.wantsIncrementalChangeDetails = NO;
    
    PHFetchResult* assets = [PHAsset fetchAssetsInAssetCollection: collection options: opts];
    [opts release];
                
    if (assets) {
        return [assets firstObject];
        }
    
    return nullptr;
}

void* ImageFetcherHost::getAlbumPosterAssetObject(int groupIndex)
{
    auto& group = _groups.at(groupIndex);
        
    if (group.numPhotos == 0) {
        // 사진 없음
        return nullptr;
                }
                
    if (group.posterAsset) {
        // 포스터 Asset이 이미 있음
        return group.posterAsset;
        }
        
    PHAsset* posterAsset = nullptr;
    
    if (group.assets) {
        // assets 있으면 첫번째 리턴
        PHFetchResult* assets = static_cast<PHFetchResult*>(group.assets);
        posterAsset = assets[0];
    } else {
        posterAsset = static_cast<PHAsset*>(fetchPosterAssetObject(group));
                }
    if (posterAsset) {
        [posterAsset retain];
        
        group.posterAsset = posterAsset;
        return group.posterAsset;
        }
        
    return nullptr;
}

ImageGroupInfo* ImageFetcherHost::getGroupInfo(int groupIndex)
{
    return &_groups.at(groupIndex);
}

int ImageFetcherHost::getGroupCount()
{
    return (int)_groups.size();
}

ImageItemInfo ImageFetcherHost::getImageItemInfo(int groupIndex, int index)
{
    
    ImageItemInfo info;
    
    auto asset = fetchPhotoAssetObject(groupIndex, index);
    PHAsset* pa = static_cast<PHAsset*>(asset);
    
    info.width = (int)pa.pixelWidth;
    info.height = (int)pa.pixelHeight;
    info.asset = asset;
    
    return info;
}
#endif
