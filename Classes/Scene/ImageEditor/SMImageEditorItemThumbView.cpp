//
//  SMImageEditorItemThumbView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorItemThumbView.h"
#include "ItemInfo.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Base/ViewAction.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/UI/LoadingSprite.h"

#define SHAKE_TIME      (1.2)
#define SHAKE_ANGLE     (8.0)


class SMImageEditorItemThumbView::ShakeAction : public cocos2d::ActionInterval
{
public:
    static ShakeAction* create() {
        auto action = new (std::nothrow)ShakeAction();
        if (action && action->initWithDuration(0)) {
            
        }
        return action;
    }
    
    float getShakeAngle(float t) const {
        float f = 1.0f - (float)std::sin(t*M_PI_2);
        return (float)(_shakeDir * SHAKE_ANGLE * f * std::sin(_shakeCount * std::sin(t * M_PI_2) * M_PI_2));
    }
    
    virtual void update(float t) override {
        auto target = (SMImageEditorItemThumbView*)_target;
        auto imageView = target->_imageView;
        float time = t*(getDuration()+_delay);
        if (time<_delay) {
            return;
        }
        
        time -= _delay;
        t = time / _duration;
        
        const float deg = getShakeAngle(t);
        const float hangFactor = 2.2f;
        auto size = imageView->getContentSize();
        
        float cx = size.width * 0.5f;
        float cy = size.height * 0.5f;
        
        float dx = (float)(cx * std::cos(ViewUtil::toRadians(deg) - M_PI_2));
        float dy = (float)(cy * std::sin(ViewUtil::toRadians(deg) - M_PI_2) * hangFactor);
        
        float x = size.width * 0.5f;
        float y = size.height * 0.5f;
        
        imageView->setPosition(x-dx, y+dy+(cy*hangFactor));
        imageView->setRotation(deg/M_PI);
    }
    
    void setValue(float delay) {
        float duration = SHAKE_TIME + cocos2d::random(0.0f, 0.8f);
        setDuration(duration+delay);
        
        _delay = delay;
        
        _shakeCount = (int)cocos2d::random(6.0f, 10.0f);
        _shakeDir = cocos2d::random(0.0f, 1.0f) > 0.5f ? 1 : -1;
    }
    
private:
    float _delay;
    float _shakeCount;
    int _shakeDir;
};


class SMImageEditorItemThumbView::SelectAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(SelectAction);
    
    virtual void onStart() override {
        auto target = (SMImageEditorItemThumbView*)_target;
        
        _from = target->_selectBox->getLineWidth();
        if (_select) {
            _to = 12.0f;
        } else {
            _to = 0.0f;
        }
        
        target->_selectBox->setVisible(true);
    }
    
    virtual void onUpdate(float t) override {
        float lineWidth = ViewUtil::interpolation(_from, _to, t);
        
        auto target = (SMImageEditorItemThumbView*)_target;
        target->_selectBox->setLineWidth(lineWidth);
    }
    
    virtual void onEnd() override {
        if (!_select) {
            auto target = (SMImageEditorItemThumbView*)_target;
            target->_selectBox->setVisible(false);
        }
    }
    
    void select(bool select) {
        _select = select;
        if (select) {
            setTimeValue(0.15f, 0.0f);
        } else {
            setTimeValue(0.1f, 0.0f);
        }
    }
    
    bool _select;
    float _from, _to;
};

class SMImageEditorItemThumbView::FocusAction : public ViewAction::DelayBaseAction
{
public:
    CREATE_DELAY_ACTION(FocusAction);
    
    virtual void onStart() override {
        auto target = (SMImageEditorItemThumbView*)_target;
        
        _from = target->_selectBox->getLineWidth();
        target->_selectBox->setVisible(true);
    }
    
    virtual void onUpdate(float t) override {
        float time = t*getDuration();
        float lineWidth;
        if (time<0.15f) {
            t = time / 0.15f;
            lineWidth = ViewUtil::interpolation(_from, 12.0f, t);
        } else if (time<0.15f-_focusTime) {
            time -= 0.15f;
            t = time / _focusTime;
            lineWidth = 12.0f;
        } else {
            time -= 0.15 + _focusTime;
            t = time / 0.10;
            lineWidth = ViewUtil::interpolation(12, 0, t);
        }
        
        auto target = (SMImageEditorItemThumbView*)_target;
        target->_selectBox->setLineWidth(lineWidth);
    }
    
    virtual void onEnd() override {
        auto target = (SMImageEditorItemThumbView*)_target;
        target->_selectBox->setVisible(false);
    }
    
    void setFocusTime(float focusTime) {
        _focusTime = focusTime;
        setTimeValue(0.15f+focusTime+0.1f, 0.0f);
    }
    
    float _focusTime;
    float _from;
};


SMImageEditorItemThumbView::SMImageEditorItemThumbView() :
_shakeAction(nullptr)
, _selectAction(nullptr)
, _spinner(nullptr)
, _selectBox(nullptr)
, _imageView(nullptr)
, _selected(false)
, _imageDlConfig(nullptr)
, _thumbDlConfig(nullptr)
{
    
}

SMImageEditorItemThumbView::~SMImageEditorItemThumbView()
{
    CC_SAFE_DELETE(_shakeAction);
    CC_SAFE_DELETE(_selectAction);
}

SMImageEditorItemThumbView * SMImageEditorItemThumbView::create(const DownloadConfig *thumbDlConfig, const DownloadConfig *imageDlConfig)
{
    auto view = new (std::nothrow)SMImageEditorItemThumbView();
    if (view && view->init()) {
        view->_thumbDlConfig = (DownloadConfig*)thumbDlConfig;
        view->_imageDlConfig = (DownloadConfig*)imageDlConfig;
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    return view;
}

bool SMImageEditorItemThumbView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    _imageView = SMImageView::create();
    _imageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_imageView);
    
    _spinner = LoadingSprite::createWithFileName();
    _spinner->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    addChild(_spinner);
    
    _imageView->setPadding(10);
    _imageView->setScaleType(SMImageView::ScaleType::CENTER_INSIDE);
    
    return true;
}

void SMImageEditorItemThumbView::setContentSize(const cocos2d::Size &size)
{
    SMView::setContentSize(size);
    _imageView->setContentSize(size);
    _imageView->setPosition(size/2);
    _spinner->setPosition(size/2);
}

void SMImageEditorItemThumbView::onStateChangePressToNormal()
{
    setAnimOffset(cocos2d::Vec2::ZERO);
}

void SMImageEditorItemThumbView::onStateChangeNormalToPress()
{
    setAnimOffset(cocos2d::Vec2(0, -15));
}

void SMImageEditorItemThumbView::onEnter()
{
    SMView::onEnter();
    
    if (_imageView->getSprite()) {
        _imageView->getSprite()->setColor(cocos2d::Color3B::WHITE);
        _imageView->getSprite()->setOpacity(0xff);
        _spinner->setVisible(false);
    } else {
        _spinner->setVisible(true);
        ImageDownloader::getInstance().loadImageFromResource(this, _imagePath, 0, _thumbDlConfig);
    }
}

void SMImageEditorItemThumbView::cleanup()
{
    SMView::cleanup();
    
    resetDownload();
    
    _imageView->setSprite(nullptr);
    
    _spinner->setOpacity(0xff);
    _spinner->stopAllActions();
}

void SMImageEditorItemThumbView::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    if (sprite) {
        _imageView->setSprite(sprite);
        _spinner->setOpacity(0);
        _spinner->runAction(cocos2d::FadeIn::create(0.1f));
        
        startShowAction();
        
    }
    _spinner->setVisible(false);
}

void SMImageEditorItemThumbView::performClick(const cocos2d::Vec2 &worldPoint)
{
    SMView::performClick(worldPoint);
}

void SMImageEditorItemThumbView::setImagePath(const std::string &imagePath)
{
    if (_imagePath != imagePath) {
        resetDownload();
    }
    
    _imagePath = imagePath;
}

void SMImageEditorItemThumbView::startShowAction()
{
    if (_imageView->getSprite()!=nullptr) {
        if (!_shakeAction) {
            _shakeAction = ShakeAction::create();
            _shakeAction->setTag(SMViewConstValue::Tag::USER+1);
        }
        
        if (getActionByTag(SMViewConstValue::Tag::USER+1)) {
            stopAction(_shakeAction);
        }
        
        _shakeAction->setValue(0);
        runAction(_shakeAction);
    }
}

void SMImageEditorItemThumbView::setSelect(bool select, bool immediate)
{
    if (_selected==select) {
        return;
    }
    
    auto aciton = getActionByTag(SMViewConstValue::Tag::USER+2);
    if (aciton) {
        stopAction(aciton);
    }
    
    if (select && _selectBox==nullptr) {
        _selectBox = ShapeRect::create();
        _selectBox->setColor4F(SMColorConst::COLOR_F_222222);
        _selectBox->setContentSize(cocos2d::Size(204, 256));
        _selectBox->setPosition(15, 30);
        _imageView->addChild(_selectBox);
    }
    
    if (immediate) {
        if (_selectBox) {
            _selectBox->setVisible(select);
            if (select) {
                _selectBox->setLineWidth(12.0f);
            }
        }
    } else {
        if (_selectAction==nullptr) {
            _selectAction = SelectAction::create();
            _selectAction->setTag(SMViewConstValue::Tag::USER+2);
        }
        _selectAction->select(select);
        runAction(_selectAction);
    }
}

void SMImageEditorItemThumbView::setFocus()
{
    auto action = getActionByTag(SMViewConstValue::Tag::USER+3);
    if (action) {
        stopAction(action);
    }
    
    if (_selectBox==nullptr) {
        _selectBox = ShapeRect::create();
        _selectBox->setColor4F(SMColorConst::COLOR_F_222222);
        _selectBox->setContentSize(cocos2d::Size(204, 256));
        _selectBox->setPosition(15, 30);;
        _selectBox->setLineWidth(1.0f);
        _imageView->addChild(_selectBox);
    }
    
    auto focusAction = FocusAction::create();
    focusAction->setTag(SMViewConstValue::Tag::USER+3);
    focusAction->setFocusTime(0.1f);
    runAction(focusAction);
}
