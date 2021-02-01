//
//  SMImageEditorStickerParser.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStickerListView.h"
#include "ItemInfo.h"
#include <iostream>
#include <sstream>
#include <string>

#define CANVAS_WIDTH    (912)
#define CANVAS_HEIGHT   (1216)

static uint32_t hexToColor(const std::string& str) {
    if (str.length() == 7 && str.front() == '#') {
        int64_t value = strtoll(str.substr(1).c_str(), nullptr, 16);
        if (value >= 0 && value <= 0xffffff) {
            return (uint32_t) value;
        }
    }
    CC_ASSERT(0);
    
    return 0xFFFFFF;
}

// string toolkit
static inline void split(const std::string& src, const std::string& token, std::vector<std::string>& vect)
{
    size_t nend = 0;
    size_t nbegin = 0;
    size_t tokenSize = token.size();
    while(nend != std::string::npos)
    {
        nend = src.find(token, nbegin);
        if(nend == std::string::npos)
            vect.push_back(src.substr(nbegin, src.length()-nbegin));
        else
            vect.push_back(src.substr(nbegin, nend-nbegin));
        nbegin = nend + tokenSize;
    }
}

// first, judge whether the form of the string like this: {x,y}
// if the form is right,the string will be split into the parameter strs;
// or the parameter strs will be empty.
// if the form is right return true,else return false.
static bool splitWithForm(const std::string& content, std::vector<std::string>& strs)
{
    bool bRet = false;
    
    do
    {
        CC_BREAK_IF(content.empty());
        
        size_t nPosLeft  = content.find('{');
        size_t nPosRight = content.find('}');
        
        // don't have '{' and '}'
        CC_BREAK_IF(nPosLeft == std::string::npos || nPosRight == std::string::npos);
        // '}' is before '{'
        CC_BREAK_IF(nPosLeft > nPosRight);
        
        const std::string pointStr = content.substr(nPosLeft + 1, nPosRight - nPosLeft - 1);
        // nothing between '{' and '}'
        CC_BREAK_IF(pointStr.length() == 0);
        
        size_t nPos1 = pointStr.find('{');
        size_t nPos2 = pointStr.find('}');
        // contain '{' or '}'
        CC_BREAK_IF(nPos1 != std::string::npos || nPos2 != std::string::npos);
        
        split(pointStr, ",", strs);
        //        if (strs.size() != 2 || strs[0].length() == 0 || strs[1].length() == 0)
        //        {
        //            strs.clear();
        //            break;
        //        }
        
        bRet = true;
    } while (0);
    
    return bRet;
}

//static cocos2d::Rect rectFromString(const std::string& str)
//{
//    cocos2d::Rect result = cocos2d::Rect::ZERO;
//    
//    do
//    {
//        std::vector<std::string> strs;
//        CC_BREAK_IF(!splitWithForm(str, strs));
//        
//        float width  = std::stof(strs[2]);
//        float height = std::stof(strs[3]);
//        
//        float x = std::stof(strs[0]);
//        float y = CANVAS_HEIGHT - (height + std::stof(strs[1]));
//        
//        result = cocos2d::Rect(x, y, width, height);
//    } while (0);
//    
//    return result;
//}
//
//static cocos2d::Vec2 pointFromString(const std::string& str)
//{
//    cocos2d::Vec2 ret;
//    
//    do
//    {
//        std::vector<std::string> strs;
//        CC_BREAK_IF(!splitWithForm(str, strs));
//        
//        float x = std::stof(strs[0]);
//        float y = CANVAS_HEIGHT - std::stof(strs[1]);
//        
//        ret.set(x, y);
//    } while (0);
//    
//    return ret;
//}

bool SMImageEditorStickerItemListView::parseStickerItem(StickerItem& item, int index) {
    auto m = _dict[ITEMS].asValueVector().at(index).asValueMap();
    
    item.index = index;
    item.decoded = true;
    
    // rootPath
    item._rootPath = _resourceRootPath;
    
    // name
    if (m.find(NAME) != m.end()) {
        item._name = m[NAME].asString();
    } else {
        item._name = "";
    }
    
    // sticker image array
    if (m.find(IMAGE) != m.end()) {
        auto str = m[IMAGE].asString();
        splitWithForm(str, item._imageArray);
    }
    
    // color
    if (m.find(LAYOUT) != m.end()) {
        item._layout = m[LAYOUT].asInt();
    } else {
        item._layout = -1;
    }
    
    return true;
}


