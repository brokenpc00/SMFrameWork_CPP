//
//  SMImageEditorFilterScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorFilterScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMZoomView.h"
#include "../../SMFrameWork/Base/SMTableView.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Base/ViewAction.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f


SMImageEditorFilterScene::SMImageEditorFilterScene() :
_currentImageSprite(nullptr)
, _originImageSprite(nullptr)
{
    
}

SMImageEditorFilterScene::~SMImageEditorFilterScene()
{
    CC_SAFE_RELEASE_NULL(_originImageSprite);
}

bool SMImageEditorFilterScene::init()
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
            _currentImageSprite = cocos2d::Sprite::createWithTexture(texture);
            _originImageSprite = cocos2d::Sprite::createWithTexture(texture);
            _originImageSprite->retain();
            texture->autorelease();
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
            applyFilterImage();
        });
    });
    
     _editZoomView = SMZoomView::create(0, 0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
     _editZoomView->setBackgroundColor4F(SMColorConst::COLOR_F_DBDCDF);
     _mainImageView = SMImageView::create(_currentImageSprite);
     _mainImageView->setContentSize(_currentImageSprite->getContentSize());
     _editZoomView->setPadding(PHOTO_MARGIN);
     _editZoomView->setContentNode(_mainImageView);
     _contentView->addChild(_editZoomView);
     _editZoomView->setScissorEnable(true);
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    _bottomMenuView->setLocalZOrder(10);
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);

    _filterListView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->addChild(_filterListView);
    
    _filterListView->numberOfRowsInSection = [&](int section) {
        return SpriteFilterMaxCount;
    };
    
    _filterListView->cellForRowAtIndexPath = [&](const IndexPath& indexPath)->cocos2d::Node*{
        kSpriteFilter filterType = (kSpriteFilter)indexPath.getIndex();
        cocos2d::Node * convertView = _filterListView->dequeueReusableCellWithIdentifier("FILTERCELL");
        FilterCell * cell;
        if (convertView) {
            cell = (FilterCell*)convertView;
        } else {
            cell = FilterCell::create(0, 0, 0, BOTTOM_MENU_HEIGHT, BOTTOM_MENU_HEIGHT);
            cell->contentView = SMView::create(0, 5, 5, BOTTOM_MENU_HEIGHT-10, BOTTOM_MENU_HEIGHT-10);
            cell->addChild(cell->contentView);
            cell->filterImage = SMImageView::create();
            cell->filterImage->setContentSize(cell->contentView->getContentSize());
            cell->contentView->addChild(cell->filterImage);
        }
        
        auto filterSprite = getFIlterSprite(filterType);
        
        cell->setTag(filterType);
        cell->filterImage->setSprite(filterSprite);
        cell->setOnClickListener(this);
        
        
        return cell;
    };
    
    return true;
}

cocos2d::Sprite * SMImageEditorFilterScene::getFIlterSprite(kSpriteFilter type)
{
    cocos2d::Sprite * filterSprite = nullptr;
    
    switch (type) {
        case SpriteFilter_LordKelvin:
        {
            filterSprite = SpriteFilterLordKelvin::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_1977:
        {
            filterSprite = SpriteFilter1977::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_AMARO:
        {
            filterSprite = SpriteFilterAmaro::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Brannan:
        {
            filterSprite = SpriteFilterBrannan::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_EarlyBird:
        {
            filterSprite = SpriteFilterEarlyBird::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Hefe:
        {
            filterSprite = SpriteFilterHefe::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Hudson:
        {
            filterSprite = SpriteFilterHudson::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Inkwell:
        {
            filterSprite = SpriteFilterInkwell::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Lomofi:
        {
            filterSprite = SpriteFilterLomofi::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Nashville:
        {
            filterSprite = SpriteFilterNashville::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Rise:
        {
            filterSprite = SpriteFilterRise::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Sierra:
        {
            filterSprite = SpriteFilterSierra::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Sutro:
        {
            filterSprite = SpriteFilterSutro::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Toaster:
        {
            filterSprite = SpriteFilterToaster::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Valencia:
        {
            filterSprite = SpriteFilterValencia::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFIlter_Walden:
        {
            filterSprite = SpriteFilterWalden::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        case SpriteFilter_Xproii:
        {
            filterSprite = SpriteFilterXproii::createWithTexture(_originImageSprite->getTexture());
        }
            break;
        default:
        {
            filterSprite = cocos2d::Sprite::createWithTexture(_originImageSprite->getTexture());
        }
            break;
    }

    return filterSprite;
}

void SMImageEditorFilterScene::onClick(SMView *view)
{
    auto sprite = getFIlterSprite((kSpriteFilter)view->getTag());
    _mainImageView->setSprite(sprite);
}

void SMImageEditorFilterScene::applyFilterImage()
{
    CCLOG("[[[[[] applied current cpature image");

    cocos2d::Sprite * captureSprite = _mainImageView->getSprite();
    
    auto oldPos = captureSprite->getPosition();
    auto oldAnc = captureSprite->getAnchorPoint();
    auto oldScale = captureSprite->getScale();
    auto size = captureSprite->getContentSize();
    
    auto rt = cocos2d::RenderTexture::create(size.width, size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->begin();
    captureSprite->setAnchorPoint(cocos2d::Vec2::ZERO);
    captureSprite->setPosition(cocos2d::Vec2::ZERO);
    captureSprite->setScale(1.0f);
    captureSprite->visit(_director->getRenderer(), cocos2d::Mat4::IDENTITY, 0);
    rt->end();
    
    _director->getRenderer()->render();
    
    captureSprite->setAnchorPoint(oldAnc);
    captureSprite->setPosition(oldPos);
    captureSprite->setScale(oldScale);
    
    auto newImage = rt->newImage();
    
    auto intent = Intent::create();
    intent->putRef("EDIT_IMAGE", newImage);
    setSceneResult(intent);
    
    newImage->autorelease();
    
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);

}

