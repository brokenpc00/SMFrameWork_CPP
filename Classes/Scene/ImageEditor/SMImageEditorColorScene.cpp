//
//  SMImageEditorColorScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorColorScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"
#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f


SMImageEditorColorScene::SMImageEditorColorScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _sliderView(nullptr)
, _mainImageView(nullptr)
, _currentImageSprite(nullptr)
, _mainccImage(nullptr)
, _curColorType(kColorTypeBrightness)
{
    
}

SMImageEditorColorScene::~SMImageEditorColorScene()
{
    
}

bool SMImageEditorColorScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
    
    auto programBundle = getSceneParam();
    if (programBundle) {
        _mainccImage = (cocos2d::Image*)programBundle->getRef("EDIT_IMAGE");
        if (_mainccImage) {
            auto tmpccImage = new cocos2d::Image;
            tmpccImage->initWithRawData(_mainccImage->getData(), _mainccImage->getDataLen(), _mainccImage->getWidth(), _mainccImage->getHeight(), _mainccImage->getBitPerPixel());
            auto texture = new cocos2d::Texture2D;
            texture->initWithImage(tmpccImage);
            tmpccImage->autorelease();
            cocos2d::Size imageSize = texture->getContentSizeInPixels();
            _currentImageSprite = ColorSprite::createWithSprite(cocos2d::Sprite::createWithTexture(texture));
            texture->autorelease();
        }
        
        kColorType type = (kColorType)programBundle->getInt("COLOR_TYPE", -1);
        if (type>=0) {
            _curColorType = type;
        }
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    _contentView->setBackgroundColor4F(SMColorConst::COLOR_F_DBDCDF);
    addChild(_contentView);
    
    _topMenuView = SMView::create(0, 0, s.height-TOP_MENU_HEIGHT, s.width, TOP_MENU_HEIGHT);
    _topMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    auto topMenuBottomLine = SMView::create(0, 0, 0, s.width, 1);
    topMenuBottomLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _topMenuView->addChild(topMenuBottomLine);
    _contentView->addChild(_topMenuView);
    topMenuBottomLine->setLocalZOrder(10);
    _topMenuView->setLocalZOrder(10);
    
    auto closeButton = SMButton::create(0, SMButton::Style::DEFAULT, 0, 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    closeButton->setIcon(SMButton::State::NORMAL, "images/popup_close.png");
    closeButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    closeButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    closeButton->setPushDownScale(0.9f);
    _topMenuView->addChild(closeButton);
    closeButton->setOnClickCallback([&](SMView*view){
        _currentImageSprite->finishEditMode(false);
        auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
        _director->popSceneWithTransition(pScene);
    });
    
    auto applyButton = SMButton::create(0, SMButton::Style::DEFAULT, s.width-(TOP_MENU_HEIGHT-20), 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    applyButton->setIcon(SMButton::State::NORMAL, "images/ic_titlebar_check.png");
    applyButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    applyButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    applyButton->setPushDownScale(0.9f);
    _topMenuView->addChild(applyButton);
    applyButton->setOnClickCallback([&](SMView * view){
        _director->getScheduler()->performFunctionInCocosThread([&]{
            applyImageColor();
        });
    });
    
    cocos2d::Rect cropGuideViewFrame = cocos2d::Rect(0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
    cocos2d::Rect photoViewFrame = cocos2d::Rect(PHOTO_MARGIN, BOTTOM_MENU_HEIGHT+PHOTO_MARGIN, s.width-PHOTO_MARGIN*2, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT-PHOTO_MARGIN*2);
    
    
    _mainImageView = SMImageView::create();
    _mainImageView->setSprite(_currentImageSprite);
    _mainImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _mainImageView->setPosition(cocos2d::Vec2(photoViewFrame.origin.x+photoViewFrame.size.width/2, photoViewFrame.origin.y+photoViewFrame.size.height/2));
    _mainImageView->setContentSize(photoViewFrame.size);
    _mainImageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _contentView->addChild(_mainImageView);
    
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    _bottomMenuView->setLocalZOrder(10);
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    
    //static SMSlider * create(const Type type = ZERO_TO_ONE, const InnerColor& initColor=InnerColor::LIGHT);
    if (_curColorType==kColorTypeTemperature) {
        _sliderView = SMSlider::create(SMSlider::Type::MINUS_ONE_TO_ONE, SMSlider::InnerColor(MAKE_COLOR4F(0xe94253, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xeeeff1, 1)));
    } else {
        _sliderView = SMSlider::create(SMSlider::Type::ZERO_TO_ONE, SMSlider::InnerColor(MAKE_COLOR4F(0xe94253, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xeeeff1, 1)));
    }
    _sliderView->setContentSize(cocos2d::Size(s.width-160, 80));
    _sliderView->setPosition(cocos2d::Vec2(80, 40));
    _bottomMenuView->addChild(_sliderView);
    _sliderView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _sliderView->onSliderValueChange = [&] (SMSlider* slider, float value) {
        changeColorValue(value);
    };
    
    _currentImageSprite->startEditMode();
    
    return true;
}

void SMImageEditorColorScene::applyImageColor()
{
    _currentImageSprite->finishEditMode(true);
    float oldScale = _currentImageSprite->getScale();
    cocos2d::Vec2 oldPos = _currentImageSprite->getPosition();
    cocos2d::Vec2 oldAnchor = _currentImageSprite->getAnchorPoint();
    cocos2d::Size imageSize = _currentImageSprite->getTexture()->getContentSizeInPixels();
    auto rt = cocos2d::RenderTexture::create(imageSize.width, imageSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->begin();
    _currentImageSprite->setScale(1.0f);
    _currentImageSprite->setAnchorPoint(cocos2d::Vec2::ZERO);
    _currentImageSprite->setPosition(cocos2d::Vec2::ZERO);
    _currentImageSprite->visit();
    rt->end();
    _director->getRenderer()->render();

    _currentImageSprite->setScale(oldScale);
    _currentImageSprite->setAnchorPoint(oldAnchor);
    _currentImageSprite->setPosition(oldPos);
    cocos2d::Image * newImage = rt->newImage();
    
    auto intent = Intent::create();
    intent->putRef("EDIT_IMAGE", newImage);
    setSceneResult(intent);
    
    newImage->autorelease();
    
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);
}

void SMImageEditorColorScene::changeColorValue(float value)
{
    int colorValue = value * 100;
    switch (_curColorType) {
        case kColorTypeBrightness:
        {
            _currentImageSprite->setBrightness(colorValue);
        }
            break;
        case kColorTypeContrast:
        {
            _currentImageSprite->setContrast(colorValue);
        }
            break;
        case kColorTypeSaturation:
        {
            _currentImageSprite->setSaturate(colorValue);
        }
            break;
        case kColorTypeTemperature:
        {
            _currentImageSprite->setTemperature(colorValue);
        }
            break;
    }
}
