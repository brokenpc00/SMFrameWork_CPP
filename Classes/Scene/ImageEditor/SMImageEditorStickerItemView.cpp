//
//  SMImageEditorStickerItemView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStickerItemView.h"
#include "../../SMFrameWork/Util/ImageDownloader.h"
#include "../../SMFrameWork/ImageProcess/ImgPrcSimpleCapture.h"

#define STICKER_PATH "sticker/"
#define STICKER_EXTEND  ".wbep"
//#define STICKER_EXTEND  ".png"


SMImageEditorStickerItemView::SMImageEditorStickerItemView() :
_colorSelectable(true)
, _layout(nullptr)
, _listener(nullptr)
, _isBlack(false)
, _alpha(1.0f)
{
    setControlType(Sticker::ControlType::DELETE);
}

SMImageEditorStickerItemView::~SMImageEditorStickerItemView()
{
    
}

SMImageEditorStickerItemView * SMImageEditorStickerItemView::createWithItem(const StickerItem &item, StickerLayoutListener *l)
{
    auto sticker = new (std::nothrow)SMImageEditorStickerItemView();
    if (sticker && sticker->initWithStickerItem(item, l)) {
        sticker->autorelease();
    } else {
        CC_SAFE_DELETE(sticker);
    }
    
    return sticker;
}

bool SMImageEditorStickerItemView::initWithStickerItem(const StickerItem &item, StickerLayoutListener *l)
{
    if (!SMView::init()) {
        return false;
    }
    _item = item;
    _listener = l;
    
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    
    _isBlack = false;
    // sticker path
    // 일단 webp로 하자
    // 나중에 png 구하면 그걸로 하자
    //std::string path = _item._rootPath + "image/" + _item._imageArray.at(0) + STICKER_EXTEND;
    //std::string path = cocos2d::StringUtils::format("%simage/%s.webp", _item._rootPath.c_str(), _item._imageArray.at(0).c_str());
    std::string path = cocos2d::StringUtils::format("%simage/%s.png", _item._rootPath.c_str(), _item._imageArray.at(0).c_str());
    // resource로 부터 읽는다.
//    ImageDownloader::getInstanceForFitting().loadImageFromResource(this, path, 0, (DownloadConfig*)&ImageDownloader::DC_NO_DISK);
    auto sprite = cocos2d::Sprite::create(path);
    if (sprite) {
        Sticker::onImageLoadComplete(sprite, 0, true);
    }
  
    
    return true;
}

// init에서 읽어온 이미지가 로드되면
void SMImageEditorStickerItemView::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    Sticker::onImageLoadComplete(sprite, tag, direct);
    if (getSprite()) {
        // 잘 읽어왔냐.
        if (tag==0 && isBlack()) {
            // 깜장이면 깜장칠을 해준다.
            getSprite()->setColor(cocos2d::Color3B::BLACK);
        }
        
        if (_item._layout>0 && _listener) {
            _listener->onStickerLayout(this, getSprite(), _item, isBlack()?1:0);
        }
    }
}

void SMImageEditorStickerItemView::setBlack()
{
    if (_isBlack) { // 이미 깜장이면 패스
        return;
    }
    
    _isBlack = true;
    
    if (_item._imageArray.size()>=2) {
        // 파일이 2개 이상이면 깜장, 하양 파일이 있으니 파일을 읽고
        resetDownload();
        std::string path = _item._rootPath + STICKER_PATH + _item._imageArray.at(1) + STICKER_EXTEND;
        ImageDownloader::getInstanceForFitting().loadImageFromResource(this, path, 1, (DownloadConfig*)&ImageDownloader::DC_NO_DISK);
    } else {
        // 파일이 1개라면 직접 깜장으로 칠한다.
        setColor(cocos2d::Color3B::BLACK);
        if (getSprite()) {
            getSprite()->setColor(cocos2d::Color3B::BLACK);
        }
        
        if (_item._layout>0 && getSprite() && _listener) {
            _listener->onStickerLayout(this, getSprite(), _item, 1);
        }
    }
}

void SMImageEditorStickerItemView::setWhite()
{
    if (!_isBlack) {    // 이미 하양이면 pass
        return;
    }
    
    if (_item._imageArray.size()>=2) {
        // 파일이 2개 이상이면 깜장, 하양 파일이 있으니 파일을 읽고
        resetDownload();
        std::string path = _item._rootPath + STICKER_PATH + _item._imageArray.at(0) + STICKER_EXTEND;
        ImageDownloader::getInstanceForFitting().loadImageFromResource(this, path, 0, (DownloadConfig*)&ImageDownloader::DC_NO_DISK);
    } else {
        // 파일이 1개라면 직접 하양으로 칠한다.
        setColor(cocos2d::Color3B::WHITE);
        if (getSprite()) {
            getSprite()->setColor(cocos2d::Color3B::WHITE);
        }
        
        if (_item._layout>0 && getSprite() && _listener) {
            _listener->onStickerLayout(this, getSprite(), _item, 0);
        }
    }
}

void SMImageEditorStickerItemView::setAlphaValue(float alpha)
{
    alpha = MAX(0.0f, MIN(1.0f, alpha));
    
    setOpacity((GLubyte)(0xFF*(0.1 + 0.9*alpha)));
    _alpha = alpha;
}

float SMImageEditorStickerItemView::getAlphaValue()
{
    return _alpha;
}

void SMImageEditorStickerItemView::clearLayout()
{
    if (_layout) {
        removeChild(_layout);
        _layout = nullptr;
    }
}

void SMImageEditorStickerItemView::setLayout(cocos2d::Node *node)
{
    clearLayout();
    node->setCascadeOpacityEnabled(true);
    addChild(node);
    _layout = node;
}

void SMImageEditorStickerItemView::prepareRemove()
{
    if (_layout!=nullptr) {
        ImageProcessor::getInstance().executeImageProcess(this, this, new ImgPrcSimpleCapture(), 0);
    }
}

void SMImageEditorStickerItemView::onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite *sprite, Intent *result)
{
    clearLayout();
    setSprite(sprite, true);
    setColor4F(cocos2d::Color4F::WHITE);
}

