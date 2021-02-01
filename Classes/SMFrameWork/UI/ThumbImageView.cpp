//
//  ThumbImageView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "ThumbImageView.h"
#include "../Base/ShaderUtil.h"
#include "../Const/SMViewConstValue.h"

class ThumbImageView::ShowAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(ShowAction);
    
    virtual void onStart() override
    {
        auto target = (ThumbImageView*)_target;
        if (target->getSprite()) {
            target->getSprite()->setOpacity(0);
        }
    }
    
    virtual void onUpdate(float t) override
    {
        auto target = (ThumbImageView*)_target;
        if (target->getSprite()) {
            target->getSprite()->setOpacity((GLubyte)0xff*t);
        }
    }
    
    virtual void onEnd() override
    {
        
    }
};

ThumbImageView * ThumbImageView::create()
{
    ThumbImageView * view = new (std::nothrow)ThumbImageView();
    if (view!=nullptr) {
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    return view;
}


ThumbImageView::ThumbImageView() :
_dimLayer(nullptr)
, _dimAction(nullptr)
, _showAction(nullptr)
{
    
}

ThumbImageView::~ThumbImageView()
{
    CC_SAFE_RELEASE(_dimAction);
    CC_SAFE_RELEASE(_dimLayer);
}

bool ThumbImageView::init()
{
    if (!SMImageView::init()) {
        return false;
    }
    
    setScaleType(SMImageView::ScaleType::FIT_CENTER);
    
    return true;
}

void ThumbImageView::onEnter()
{
    SMImageView::onEnter();
    
    if (getSprite()) {
        getSprite()->setOpacity(0xff);
    } else {
        // deprecated
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
//        ImageDownloader::getInstance().loadImageFromLocalIdentifier(this, _item.url);
        ImageDownloader::getInstance().loadImagePHAssetThumbnail(this, _item.asset);
#else
        ImageDownloader::getInstance().loadImageFromFile(this, _item.url);
#endif
    }
}

void ThumbImageView::onExit()
{
    SMImageView::onExit();
    
    resetDownload();
    setSprite(nullptr);
    
    if (_dimLayer) {
        auto action = _dimLayer->getActionByTag(SMViewConstValue::Tag::USER+1);
        if (action) {
            _dimLayer->stopAction(action);
        }
    }
    
    auto action = getActionByTag(SMViewConstValue::Tag::USER+2);
    if (action) {
        stopAction(action);
    }
}

void ThumbImageView::onStateChangeNormalToPress()
{
    if (_dimLayer==nullptr) {
        _dimLayer = ShapeSolidRect::create();
        _dimLayer->setContentSize(_contentSize);
        _dimLayer->setColor(cocos2d::Color3B::BLACK);
        _dimLayer->setOpacity(0);
        addChild(_dimLayer);
    }
    
    if (_dimAction==nullptr) {
        _dimAction = ViewAction::AlphaTo::create(0.15f, 0.0f, _dimLayer);
        _dimAction->setTag(SMViewConstValue::Tag::USER+1);
        // destructor에서  release 해줘야 한다.
        _dimAction->retain();
    } else {
        auto action = _dimLayer->getActionByTag(SMViewConstValue::Tag::USER+1);
        if (action) {
            _dimLayer->stopAction(action);
        }
    }
    
    _dimAction->setValue(0.15f);
    _dimLayer->runAction(_dimAction);
}

void ThumbImageView::onStateChangePressToNormal()
{
    if (!_dimAction || !_dimLayer) {
        return;
    }
    
    auto action = _dimLayer->getActionByTag(SMViewConstValue::Tag::USER+1);
    if (action) {
        _dimLayer->stopAction(action);
    }
    _dimAction->setValue(0);
    _dimLayer->runAction(_dimAction);
}

void ThumbImageView::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    if (sprite==nullptr) {
        return;
    }
    
    setSprite(sprite);
    ShaderUtil::setBgr2RgbShader(sprite);
    
    cocos2d::Vec2 p1 = convertToWorldSpace(cocos2d::Vec2::ZERO);
    cocos2d::Vec2 p2 = convertToWorldSpace(cocos2d::Vec2(_contentSize.width, _contentSize.height));
    cocos2d::Size s = _director->getWinSize();
    
    if (p1.y >= s.height || p2.y <= 0 || p1.x >= s.width || p2.x <= 0) {
        // 현재 node가 화면 밖에 있다. -> 바로 세팅한다.
        sprite->setOpacity(0xff);
        return;
    }
    
    // 현재 node가 화면에 보이고 있으므로
    // Animation 처리한다.
    
    // 일단 안보이게
    sprite->setOpacity(0);
    
    // 본체
    auto action = getActionByTag(SMViewConstValue::Tag::USER+2);
    if (action) {
        stopAction(action);
    }
    
    if (_showAction==nullptr) {
        _showAction = ShowAction::create();
        _showAction->setTag(SMViewConstValue::Tag::USER+2);
    }
    
    _showAction->setTimeValue(0.1f, 0);
    runAction(_showAction);
}

void ThumbImageView::setImageItemInfo(const ImageItemInfo& info)
{
    _item = info;
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    // deprecated
//    ImageDownloader::getInstance().loadImageFromNSURL(this, _item.url);
//    ImageDownloader::getInstance().loadImageFromLocalIdentifier(this, _item.url);
    ImageDownloader::getInstance().loadImagePHAssetThumbnail(this, _item.asset);
    #else
    ImageDownloader::getInstance().loadImageFromFile(this, _item.url);
    #endif
}
