//
//  CameraControlLayer.cpp
//  iPet
//
//  Created by KimSteve on 2017. 7. 4..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "CameraControlLayer.h"
#include "../DeviceAsset/DeviceCamera.h"
#include "CaptureView.h"
#include "../Base/SMButton.h"
#include "../Base/ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"

#define BUTTON_ID_SWITCH    (100)
#define BUTTON_ID_FLASH     (101)

#define ACTION_TAG_ENABLE (SMViewConstValue::Tag::USER+1)
#define ACTION_TAG_RING (SMViewConstValue::Tag::USER+2)

#define RING_BASE_SIZE  (202)

#define RING_WIDTH_NORMAL   (8)
#define RING_WIDTH_PRESS    (14)

#define OPACITY_ENABLE      (0xFF)
#define OPACITY_DISABLE     (100)


class RingAction : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(RingAction);
    
    virtual void onStart() override {
        _ring = (ShapeCircle*)_target;
        _from = _ring->getLineWidth();
    }
    
    virtual void onUpdate(float t) override {
        float width = ViewUtil::interpolation(_from, _to, t);
        float size = RING_BASE_SIZE + width/2;
        _ring->setContentSize(cocos2d::Size(size, size));
        _ring->setLineWidth(width);
    };
    
    void setLineWidth(float lineWidth) {
        setTimeValue(0.1, 0);
        _to = lineWidth;
    }
    
private:
    float _from, _to;
    
    ShapeCircle* _ring;
};

class EnableAction : public ViewAction::DelayBaseAction {
public:
    CREATE_DELAY_ACTION(EnableAction);
    
    virtual void onStart() override {
        _from = _target->getOpacity();
        if (_enabled) {
            _to = OPACITY_ENABLE;
        } else {
            _to = OPACITY_DISABLE;
        }
    }
    
    virtual void onUpdate(float t) override {
        float opacity = ViewUtil::interpolation(_from, _to, t);
        _target->setOpacity((GLubyte)opacity);
    };
    
    void setEnabled(bool enabled) {
        setTimeValue(0.15, 0);
        _enabled = enabled;
    }
    
private:
    bool _enabled;
    
    float _from, _to;
};

class EnabledButton : public SMButton {
    
public:
    void setEnabled(const bool enabled, bool immediate) {
        SMButton::setEnabled(enabled);
        
        auto action = getActionByTag(ACTION_TAG_ENABLE);
        if (action) {
            stopAction(action);
        }
        
        if (immediate) {
            if (enabled) {
                setOpacity(OPACITY_ENABLE);
            } else {
                setOpacity(OPACITY_DISABLE);
            }
        } else {
            if (_action == nullptr) {
                _action = EnableAction::create(false);
                _action->setTag(ACTION_TAG_ENABLE);
            }
            _action->setEnabled(enabled);
            runAction(_action);
        }
    }
    
protected:
    EnabledButton() : _action(nullptr) {
    }
    
    virtual ~EnabledButton() {
        CC_SAFE_RELEASE(_action);
    }
    
private:
    EnableAction* _action;
};

class CameraControlLayer::ShutterButton : public EnabledButton {
public:
    static ShutterButton* create(CameraInterface* i) {
        auto view = new (std::nothrow)ShutterButton();
        if (view && view->initWithStyle(SMButton::Style::SOLID_CIRCLE)) {
            view->initLayout(i);
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
            view = nullptr;
        }
        
        return view;
    }
    
    virtual void onStateChangeNormalToPress() override {
        if (isEnabled()) {
            SMButton::onStateChangeNormalToPress();
            if (_interface) {
                _interface->onCameraInterface(this, CameraInterface::STATUS::SHUTTER_PRESSED);
            }
            
            _canceled = false;
            setRingLineWidth(RING_WIDTH_PRESS, false);
        } else {
            _canceled = true;
        }
    }
    
    virtual void onStateChangePressToNormal() override {
        SMButton::onStateChangePressToNormal();
        if (_interface) {
            _interface->onCameraInterface(this, CameraInterface::STATUS::SHUTTER_RELEASED);
        }
        
        setRingLineWidth(RING_WIDTH_NORMAL, false);
        
        if (!_canceled) {
            auto worldPt = getLastTouchLocation();
            auto pt = convertToNodeSpace(getLastTouchLocation());
            if (containsPoint(pt)) {
                // get Snap shot
                if (_interface) {
                    _interface->onCameraInterface(this, CameraInterface::STATUS::SHUTTER_CLICKED);
                }
                
            }
        }
    }
    
protected:
    ShutterButton() : _ringAction(nullptr) {}
    
    virtual ~ShutterButton() {
        CC_SAFE_RELEASE(_ringAction);
    }
    
    bool initLayout(CameraInterface* i) {
        _interface = i;
        
        setContentSize(cocos2d::Size(222, 222));
        setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        _ring = ShapeCircle::create();
        _ring->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        setIcon(SMView::State::NORMAL, _ring);
        setRingLineWidth(RING_WIDTH_NORMAL, true);
        
        setButtonColor(SMView::State::NORMAL, MAKE_COLOR4F(0xADAFB3, 1));
        setButtonColor(SMView::State::PRESSED, MAKE_COLOR4F(0x96989C, 1));
        
        return true;
    }
    
    void setRingLineWidth(float lineWidth, bool immediate) {
        auto action = _ring->getActionByTag(ACTION_TAG_RING);
        if (action) {
            _ring->stopAction(action);
        }
        if (immediate) {
            float size = RING_BASE_SIZE + lineWidth/2;
            _ring->setContentSize(cocos2d::Size(size, size));
            _ring->setLineWidth(lineWidth);
        } else {
            if (_ringAction == nullptr) {
                _ringAction = RingAction::create(false);
                _ringAction->setTag(ACTION_TAG_RING);
            }
            _ringAction->setLineWidth(lineWidth);
            _ring->runAction(_ringAction);
        }
    }
    
private:
    
    RingAction* _ringAction;
    
    ShapeCircle* _ring;
    
    bool _canceled;
    
    CameraInterface* _interface;
    
};

class CameraControlLayer::SwitchButton : public EnabledButton {
public:
    static SwitchButton* create() {
        auto view = new (std::nothrow)SwitchButton();
        if (view && view->initWithStyle(SMButton::Style::SOLID_CIRCLE)) {
            view->initLayout();
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
            view = nullptr;
        }
        
        return view;
    }
    
protected:
    bool initLayout() {
        setContentSize(cocos2d::Size(132, 132));
        setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        setIcon(SMView::State::NORMAL, "images/ic_titlebar_camera_selfie2.png");
        
        setButtonColor(SMView::State::NORMAL, MAKE_COLOR4F(0xDBDCDF, 1));
        setButtonColor(SMView::State::PRESSED, MAKE_COLOR4F(0xC6C7CB, 1));
        
        return true;
    }
};


class CameraControlLayer::FlashButton : public EnabledButton {
public:
    static FlashButton* create() {
        auto view = new (std::nothrow)FlashButton();
        if (view && view->initWithStyle(SMButton::Style::SOLID_CIRCLE)) {
            view->initLayout();
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
            view = nullptr;
        }
        
        return view;
    }
    
    void setFlashState(int flashState) {
        if (_flashState != flashState) {
            _flashState = flashState;
            
            cocos2d::Sprite* icon;
            
            switch (flashState) {
                case CameraDeviceConst::Flash::OFF:
                    icon = cocos2d::Sprite::create("images/camera_flash_off.png");
                    break;
                case CameraDeviceConst::Flash::ON:
                    icon = cocos2d::Sprite::create("images/camera_flash.png");
                    break;
                default:
                case CameraDeviceConst::Flash::AUTO:
                    icon = cocos2d::Sprite::create("images/camera_flash_auto.png");
                    break;
            }
            setIcon(SMView::State::NORMAL, icon);
        }
    }
    
protected:
    bool initLayout() {
        setContentSize(cocos2d::Size(132, 132));
        setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        
        _flashState = CameraDeviceConst::Flash::OFF;
        setIcon(SMView::State::NORMAL, "images/camera_flash_off.png");
        
        setButtonColor(SMView::State::NORMAL, MAKE_COLOR4F(0xDBDCDF, 1));
        setButtonColor(SMView::State::PRESSED, MAKE_COLOR4F(0xC6C7CB, 1));
        
        return true;
    }
    
private:
    int _flashState;
};


CameraControlLayer::CameraControlLayer()
{
    
}

CameraControlLayer::~CameraControlLayer()
{
    
}

CameraControlLayer * CameraControlLayer::create(const cocos2d::Size &size, CameraInterface *i)
{
    CameraControlLayer * layer = new (std::nothrow)CameraControlLayer();
    if (layer != nullptr) {
        if (layer->initWithSize(size, i)) {
            layer->autorelease();
        } else {
            CC_SAFE_DELETE(layer);
        }
    }
    
    return layer;
}

bool CameraControlLayer::initWithSize(const cocos2d::Size &size, CameraInterface *i)
{
    if (!SMView::init()) {
        return false;
    }
    
    _interface = i;
    
    setContentSize(size);
    setBackgroundColor4F(cocos2d::Color4F::WHITE);

    _shutterButton = ShutterButton::create(_interface);
    _shutterButton->setPosition(size/2);
    addChild(_shutterButton);
    
    _switchButton = SwitchButton::create();
    _switchButton->setTag(BUTTON_ID_SWITCH);
    _switchButton->setPosition(size.width-214, size.height/2);
    _switchButton->setOnClickListener(this);
    addChild(_switchButton);
    
    _flashButton = FlashButton::create();
    _flashButton->setTag(BUTTON_ID_FLASH);
    _flashButton->setPosition(214, size.height/2);
    _flashButton->setOnClickListener(this);
    addChild(_flashButton);
    
    enableControl(false, true);
    
    return true;
}

void CameraControlLayer::onClick(SMView *view)
{
    switch (view->getTag()) {
        case BUTTON_ID_SWITCH:
        {
            if (_interface) {
                _interface->onCameraInterface(view, CameraInterface::STATUS::FRONTBACK_CLICKED);
            }
        }
            break;
        case BUTTON_ID_FLASH:
        {
            if (_interface) {
                _interface->onCameraInterface(view, CameraInterface::STATUS::FLASH_CLICKED);
            }
        }
            break;
    }
}

void CameraControlLayer::setDeviceInfo(int cameraId, bool hasFlash, bool hasFrontFacing)
{
    _cameraId = cameraId;
    _cameraHasFlash = hasFlash;
    _cameraHasFrontFacing = hasFrontFacing;
}

void CameraControlLayer::setFlashState(int flashState)
{
    if (_cameraHasFlash) {
        _flashButton->setFlashState(flashState);
    } else {
        _flashButton->setFlashState(CameraDeviceConst::Flash::OFF);
    }
}

void CameraControlLayer::enableControl(bool enabled, bool immediate)
{
    _shutterButton->setEnabled(enabled, immediate);
    if (enabled) {
        if (_cameraHasFlash) {
            _flashButton->setEnabled(enabled, immediate);
        }
        if (_cameraHasFrontFacing) {
            _switchButton->setEnabled(enabled, immediate);
        }
    } else {
        _switchButton->setEnabled(enabled, immediate);
        _flashButton->setEnabled(enabled, immediate);
    }
}

int CameraControlLayer::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    
    return TOUCH_TRUE;
}


















































