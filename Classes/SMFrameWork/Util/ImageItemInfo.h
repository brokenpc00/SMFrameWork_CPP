//
//  ImageItemInfo.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 21..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ImageItemInfo_h
#define ImageItemInfo_h

#include <string>

/*
    Image를 나타내는 정보 structure
 */

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
struct ImageItemInfo {
    ImageItemInfo() {
        url = "";
        photoIndex = 0;
        orient = 0;
        width = 0;
        height = 0;
    }
    
    // asset url
    std::string url;
    
    int photoIndex;
    
    int orient;
    
    int width;
    
    int height;
};

struct ImageGroupInfo {
    ImageGroupInfo(){
        seq = 0;
        key = "";
        name = "";
        groupIndex = 0;
        numPhotos = 0;
        isDefault = false;
    }
    int seq;
    
    // group id -> asset url
    std::string key;
    
    // group name
    std::string name;
    
    // group을 대표할 thumb image item info
    ImageItemInfo thumb;
    
    int groupIndex;
    
    int numPhotos;
    
    bool isDefault;
};


#else
struct ImageItemInfo {
    ImageItemInfo() : asset(nullptr) {
        url = "";
        photoIndex = 0;
        orient = 0;
        width = 0;
        height = 0;
    }
    
    // asset url
    std::string url;
    
    int photoIndex;
    
    int orient;
    
    int width;
    
    int height;
    
    void* asset;
};

struct ImageGroupInfo {
    ImageGroupInfo() : numPhotos(0), collection(nullptr), assets(nullptr), posterAsset(nullptr){
        seq = 0;
        name = "";
        isDefault = false;
        key = "";
    }
    int seq;
    
    // group name
    std::string name;
    
    // group을 대표할 thumb image item info
    ImageItemInfo thumb;
    
    int numPhotos;
    
    bool isDefault;
    
    // for andnroid cover url
    std::string key;
    
    /** PHAssetCollection Object */
    void* collection;
    
    /** PHFetchResult Object */
    void* assets;
    
    /** PHAsset Object */
    void* posterAsset;
};
#endif

#endif /* ImageItemInfo_h */
