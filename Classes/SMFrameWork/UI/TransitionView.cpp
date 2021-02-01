//
//  TransitionView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#include "TransitionView.h"
#include "../Base/SMImageView.h"
#include "../Base/ViewAction.h"
#include "../Const/SMViewConstValue.h"

#define ACTION_TAG_POSITION         (SMViewConstValue::Tag::USER+1)
#define ACTION_TAG_FADE_THUMB       (SMViewConstValue::Tag::USER+2)
#define ACTION_TAG_FADE_IMAGE       (SMViewConstValue::Tag::USER+3)

#define TRANS_TIME (0.15)

TransitionView* TransitionView::create(const cocos2d::Size& imageSize, SMView* thumbView, const cocos2d::Rect& editRect, bool initThumbnail)
{
    auto view = new (std::nothrow)TransitionView();
    
    if (view!=nullptr) {
        if (view->init(imageSize, thumbView, editRect, initThumbnail)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

bool TransitionView::init(const cocos2d::Size& imageSize, SMView * thumbView, const cocos2d::Rect& editRect, bool initThumbnail)
{
    _fromPos = thumbView->getPosition();
    _toPos = cocos2d::Vec2(editRect.getMidX(), editRect.getMidY());
    
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    setContentSize(imageSize);
    
    _thumbView = thumbView;
    _thumbView->setPosition(imageSize/2);
    
    auto thumbSize = thumbView->getContentSize();
    float scale0 = imageSize.width / imageSize.height;
    float scale1 = thumbSize.width / thumbSize.height;
    
    if (initThumbnail) {
        if (scale0 > scale1) {
            // 세로 맞춤
            _fromScale = thumbSize.height / imageSize.height;
        } else {
            // 가로 맞춤
            _fromScale = thumbSize.width / imageSize.width;
        }
        _thumbView->setScale(1.0/_fromScale);
    } else {
        _fromScale = _director->getWinSize().width / imageSize.width;
        setScale(_fromScale);
    }
    
    addChild(thumbView);
    
    _imageView = SMImageView::create();
    _imageView->setContentSize(imageSize);
    _imageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _imageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _imageView->setPosition(imageSize/2);
    _imageView->setOpacity(0);
    addChild(_imageView);
    

    // transitino
    scale1 = editRect.size.width / editRect.size.height;
    if (scale0 > scale1) {
        // 가로 맞춤
        _toScale = editRect.size.width / imageSize.width;
    } else {
        // 세로 맞춤
        _toScale = editRect.size.height / imageSize.height;
    }
    
    float w = imageSize.width * _toScale;
    float h = imageSize.height * _toScale;
    
    _screenBounds.setRect(_toPos.x-w/2, _toPos.y-h/2, w, h);
    
    startTransitionIn();

    return true;
}


void TransitionView::setImageSprite(cocos2d::Sprite *sprite)
{
    _imageView->setSprite(sprite);
}

void TransitionView::startTransitionIn()
{
    if (getActionByTag(ACTION_TAG_POSITION)) {
        stopActionByTag(ACTION_TAG_POSITION);
    }
    
    setPosition(_fromPos);
    setScale(_fromScale);
    
    auto action = ViewAction::TransformAction::create();
    action->setTag(ACTION_TAG_POSITION);
    action->toScale(_toScale).setTweenFunc(cocos2d::tweenfunc::Cubic_EaseOut);
    action->setTimeValue(0.3, 0.2);
    runAction(action);
}

void TransitionView::startTrnasitionOut()
{
    if (getActionByTag(ACTION_TAG_POSITION)) {
        stopActionByTag(ACTION_TAG_POSITION);
    }
    if (_thumbView->getActionByTag(ACTION_TAG_FADE_THUMB)) {
        _thumbView->stopActionByTag(ACTION_TAG_FADE_THUMB);
    }
    if (_imageView->getActionByTag(ACTION_TAG_FADE_IMAGE)) {
        _imageView->stopActionByTag(ACTION_TAG_FADE_IMAGE);
    }
    
    auto a1 = ViewAction::TransformAction::create();
    a1->setTag(ACTION_TAG_POSITION);
    a1->toScale(_fromScale).toPosition(_fromPos).setTweenFunc(cocos2d::tweenfunc::Sine_EaseInOut);
    a1->setTimeValue(0.2, 0.1);
    runAction(a1);
    
    auto a2 = ViewAction::TransformAction::create();
    a2->setTag(ACTION_TAG_FADE_THUMB);
    a2->toAlpha(1).setTimeValue(0.15, 0.1);
    _thumbView->runAction(a2);
    
    auto a3 = ViewAction::TransformAction::create();
    a3->setTag(ACTION_TAG_FADE_IMAGE);
    a3->toAlpha(0).setTimeValue(0.20, 0.1);
    _imageView->runAction(a3);
}

void TransitionView::fadeToImage()
{
    if (_thumbView->getActionByTag(ACTION_TAG_FADE_THUMB)) {
        _thumbView->stopActionByTag(ACTION_TAG_FADE_THUMB);
    }
    if (_imageView->getActionByTag(ACTION_TAG_FADE_IMAGE)) {
        _imageView->stopActionByTag(ACTION_TAG_FADE_IMAGE);
    }
    
    // Thumb Fade-Out
    auto a2 = ViewAction::TransformAction::create();
    a2->setTag(ACTION_TAG_FADE_THUMB);
    a2->toAlpha(0).setTimeValue(0.3, 0.1);
    _thumbView->runAction(a2);
    
    // Image Fade-In
    auto a3 = ViewAction::TransformAction::create();
    a3->setTag(ACTION_TAG_FADE_IMAGE);
    a3->toAlpha(1).setTimeValue(0.3, 0);
    _imageView->setOpacity(0);
    _imageView->runAction(a3);
}

cocos2d::Sprite* TransitionView::getSprite()
{
    return _imageView->getSprite();
}

