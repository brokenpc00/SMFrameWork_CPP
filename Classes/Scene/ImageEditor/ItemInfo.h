//
//  ItemInfo.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ItemInfo_h
#define ItemInfo_h

struct StickerInfo {
    std::string name;
    int code;
    int color;
    cocos2d::Vec2 position;
    float scale;
    float rotate;
    float opacity;
};


struct StickerItem {
    int index;
    bool decoded;
    std::string _name;
    std::vector<std::string> _imageArray;
    int _layout;
    int _defaultIndex;
    int _selectIndex;
    std::string _rootPath;
};

#endif /* ItemInfo_h */
