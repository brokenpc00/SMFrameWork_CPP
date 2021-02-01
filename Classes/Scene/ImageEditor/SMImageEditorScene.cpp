//
//  SMImageEditorScene.cpp
//  ePubCheck
//
//  Created by KimSteve on 2017. 4. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMZoomView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMTableView.h"
#include "../../SMFrameWork/Base/SMPageView.h"
#include "../../SMFrameWork/Base/SMCircularListView.h"
#include "../../SMFrameWork/Base/SMSlider.h"
#include "../../SMFrameWork/Base/MeshSprite.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Base/ViewAction.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "SMImageEditorListener.h"

#include "SMImageEditorCropScene.h"
#include "SMImageEditorRotateScene.h"
#include "SMImageEditorStraightenScene.h"
#include "SMImageEditorColorScene.h"
#include "SMImageEditorStickerScene.h"
#include "SMImageEditorFilterScene.h"
#include "SMImageEditorDrawScene.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f
#define MAX_PHOTO_SIZE 1080.0f


SMImageEditorScene::SMImageEditorScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuTableView(nullptr)
, _mainMeshSprite(nullptr)
, _mainImageSprite(nullptr)
, _mainImageView(nullptr)
, _zoomView(nullptr)
, _editMenuView(nullptr)
, _adjustMenuView(nullptr)
, _magicMenuView(nullptr)
, _bEditMenuVisible(false)
, _bAdjustMenuVisible(false)
, _bMagicMenuVisible(false)
, _resultSprite(nullptr)
, _bAlreadyCrop(false)
, _tmpImageview(nullptr)
, _mainccImage(nullptr)
, _originccImage(nullptr)
, _resultccImage(nullptr)
{
    
}

SMImageEditorScene::~SMImageEditorScene()
{
    CC_SAFE_RELEASE_NULL(_mainccImage);
    CC_SAFE_RELEASE_NULL(_originccImage);

/*
    CC_SAFE_RELEASE_NULL(_resultSprite);
    CC_SAFE_RELEASE_NULL(_resultccImage);
    CC_SAFE_RELEASE_NULL(_editMenuTableView);
    CC_SAFE_RELEASE_NULL(_adjustMenuTableView);
    CC_SAFE_RELEASE_NULL(_magicMenuTableView);
    CC_SAFE_RELEASE_NULL(_mainImageView);
    CC_SAFE_RELEASE_NULL(_zoomView);
    CC_SAFE_RELEASE_NULL(_mainImageSprite);
    CC_SAFE_RELEASE_NULL(_mainMeshSprite);
*/
}

bool SMImageEditorScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    _contentView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    addChild(_contentView);
    
    _topMenuView = SMView::create(0, 0, s.height-TOP_MENU_HEIGHT, s.width, TOP_MENU_HEIGHT);
    _topMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_topMenuView);
    
    SMView * topMenuBottomLine = SMView::create(0, 0, 0, s.width, 1);
    topMenuBottomLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _topMenuView->addChild(topMenuBottomLine);
    _topMenuView->setLocalZOrder(10);
    
    auto programBundle = this->getSceneParam();
    if (programBundle && programBundle->getBool("FROM_PICKER", false)) {
        SMButton * backButton = SMButton::create(0, SMButton::Style::DEFAULT, 0, 0, TOP_MENU_HEIGHT, TOP_MENU_HEIGHT);
        backButton->setIcon(SMButton::State::NORMAL, "images/ic_titlebar_back.png");
        backButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
        backButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
        backButton->setPushDownScale(0.9f);
        _topMenuView->addChild(backButton);
        backButton->setOnClickCallback([&](SMView * view){
            auto pScene = SceneTransition::SlideOutToRight::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
        });
    } else {
        SMButton * closeButton = SMButton::create(0, SMButton::Style::DEFAULT, 0, 0, TOP_MENU_HEIGHT, TOP_MENU_HEIGHT);
    closeButton->setIcon(SMButton::State::NORMAL, "images/popup_close.png");
    closeButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    closeButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    closeButton->setPushDownScale(0.9f);
    _topMenuView->addChild(closeButton);
    closeButton->setOnClickCallback([&](SMView * view){
        cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{

        });
    });
    }
    
    
    auto applyButton = SMButton::create(0, SMButton::Style::DEFAULT, s.width-110, 0, 110, 110);
    applyButton->setIcon(SMButton::State::NORMAL, "images/ic_titlebar_check.png");
    applyButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    applyButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    applyButton->setPushDownScale(0.9f);
    _topMenuView->addChild(applyButton);
    applyButton->setOnClickCallback([&](SMView * view){
        _director->getScheduler()->performFunctionInCocosThread([&]{
            finishEditImage();
        });
    });
    
    
    
    // intent로 sprite를 받으려 했으나
    // RenderTexture에서 capture를 하면 화질 손실이 있다.
    // 처음 부터 받을때 Data로 받고 OpenCV에서 Resize & Rotate & Crop 등을 진행한다.
    
    
    if (programBundle) {
        cocos2d::Image * receiveImage = (cocos2d::Image*)programBundle->getRef("INTENT_IMAGE");

        if ((float)receiveImage->getWidth()>MAX_PHOTO_SIZE || (float)receiveImage->getHeight()>MAX_PHOTO_SIZE) {
            // resize image
            float widhtRatio = MAX_PHOTO_SIZE/(float)receiveImage->getWidth();
            float heightRatio = MAX_PHOTO_SIZE/(float)receiveImage->getHeight();
            float ratio = MIN(widhtRatio, heightRatio);
            cv::Mat src = cvImageUtil::ccImage2cvMat(receiveImage);
            cv::Size scaleSize(receiveImage->getWidth()*ratio, receiveImage->getHeight()*ratio);
            cv::Mat dst;
            cv::resize(src, dst, scaleSize);
            _originccImage = cvImageUtil::cvMat2ccImage(dst);
            src.release();
            dst.release();
        } else {
            _originccImage = new cocos2d::Image();
            _originccImage->initWithRawData(receiveImage->getData(), receiveImage->getDataLen(), receiveImage->getWidth(), receiveImage->getHeight(), receiveImage->getBitPerPixel());
            _originccImage->autorelease();
        }
        _originccImage->retain();
        
    } else {
        _originccImage = new cocos2d::Image();
        _originccImage->initWithImageFile("images/defaults.jpg");
        _originccImage->retain();
    }
    
    _mainccImage = new cocos2d::Image;
    _mainccImage->initWithRawData(_originccImage->getData(), _originccImage->getDataLen(), _originccImage->getWidth(), _originccImage->getHeight(), _originccImage->getBitPerPixel());
    _mainccImage->retain();
    
    auto texture = new cocos2d::Texture2D;
    texture->initWithImage(_mainccImage);
    _mainccImage->release();
    
    auto tmp = cocos2d::Sprite::createWithTexture(texture);
    texture->release();
    //        auto tmp = (cocos2d::Sprite *)programBundle->getRef("INTENT_IMAGE");
    if (tmp) {
        _mainImageSprite = cocos2d::Sprite::createWithTexture(tmp->getTexture());
    }
    
    if (_mainImageSprite==nullptr) {
        cocos2d::Label * noti = cocos2d::Label::createWithSystemFont("이미지가 없습니다.", SMFontConst::SystemFontRegular, 36.0f);
        noti->setTextColor(cocos2d::Color4B::BLACK);
        noti->setPosition(s/2);
        _contentView->addChild(noti);
        return true;
    }

    SMView * bottomMenuContainer = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    bottomMenuContainer->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(bottomMenuContainer);
    
    SMView * bottomMenuTopLine = SMView::create(0, 0, 159, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    bottomMenuContainer->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    

    _zoomView = SMZoomView::create(0, 0, BOTTOM_MENU_HEIGHT, s.width, s.height-290);
    _zoomView->setBackgroundColor4F(SMColorConst::COLOR_F_DBDCDF);
//    _mainMeshSprite = MeshSprite::create(_mainImageSprite);
    _mainImageView = SMImageView::create(_mainImageSprite);
    _mainImageView->setContentSize(_mainImageSprite->getContentSize());


    // setPadding must call before setContentNode()
    _zoomView->setPadding(PHOTO_MARGIN);
    _zoomView->setContentNode(_mainImageView);
    _contentView->addChild(_zoomView);
    _zoomView->setScissorEnable(true);
    
    _bottomMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuTableView->numberOfRowsInSection = [&](int section)->int {
        return 3;
    };
    _bottomMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath &indexPath)->cocos2d::Node * {
        cocos2d::Node * converView = _bottomMenuTableView->dequeueReusableCellWithIdentifier("MENU_CELL");
        MenuCell * cell = nullptr;
        int index = indexPath.getIndex();
        if (converView) {
            cell = (MenuCell*)converView;
        } else {
            cell = MenuCell::create(0, 0, 0, 160, 160);
            cell->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, 140, 140);
            cell->addChild(cell->menuButton);
            cell->menuButton->setShapeCornerRadius(10);
            cell->menuButton->setOutlineWidth(2.0f);
            cell->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            cell->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            cell->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            cell->menuButton->setPushDownScale(0.9f);
        }
        cell->setTag(index);
        cell->menuButton->setTag(index);
        cell->menuButton->setOnClickListener(this);
        switch (index) {
            case 0:
            {
                cell->menuButton->setTextSystemFont("편집", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 1:
            {
                cell->menuButton->setTextSystemFont("보정", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 2:
            {
                cell->menuButton->setTextSystemFont("꾸미기", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        
        return cell;
    };
    
    bottomMenuContainer->addChild(_bottomMenuTableView);
    bottomMenuContainer->setLocalZOrder(10);

    _editMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _editMenuView->setBackgroundColor4F(cocos2d::Color4F(1, 1, 1, 0.8f));
    auto border = ShapeRect::create();
    border->setContentSize(_editMenuView->getContentSize());
    border->setColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    border->setLineWidth(2.0f);
    _editMenuView->addChild(border);

//    auto tmpEditLabel = cocos2d::Label::createWithSystemFont("편집 메뉴", SMFontConst::SystemFontRegular, 22.0f);
//    tmpEditLabel->setTextColor(cocos2d::Color4B::BLACK);
//    tmpEditLabel->setPosition(_editMenuView->getContentSize()/2);
//    _editMenuView->addChild(tmpEditLabel);

    _editMenuView->setVisible(false);
    _contentView->addChild(_editMenuView);
    
    _adjustMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _adjustMenuView->setBackgroundColor4F(cocos2d::Color4F(1, 1, 1, 0.8f));
    auto border2 = ShapeRect::create();
    border2->setContentSize(_adjustMenuView->getContentSize());
    border2->setColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    border2->setLineWidth(2.0f);
    _adjustMenuView->addChild(border2);
    
//    auto tmpAdjustLabel = cocos2d::Label::createWithSystemFont("보정 메뉴", SMFontConst::SystemFontRegular, 22.0f);
//    tmpAdjustLabel->setTextColor(cocos2d::Color4B::BLACK);
//    tmpAdjustLabel->setPosition(_adjustMenuView->getContentSize()/2);
//    _adjustMenuView->addChild(tmpAdjustLabel);
    
    _adjustMenuView->setVisible(false);
    _contentView->addChild(_adjustMenuView);
    
    _magicMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _magicMenuView->setBackgroundColor4F(cocos2d::Color4F(1, 1, 1, 0.8f));
    auto border3 = ShapeRect::create();
    border3->setContentSize(_magicMenuView->getContentSize());
    border3->setColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    border3->setLineWidth(2.0f);
    _magicMenuView->addChild(border3);
    
//    auto tmpMagicLabel = cocos2d::Label::createWithSystemFont("꾸미기 메뉴", SMFontConst::SystemFontRegular, 22.0f);
//    tmpMagicLabel->setTextColor(cocos2d::Color4B::BLACK);
//    tmpMagicLabel->setPosition(_magicMenuView->getContentSize()/2);
//    _magicMenuView->addChild(tmpMagicLabel);
    
    _magicMenuView->setVisible(false);
    _contentView->addChild(_magicMenuView);
    
    // edit menu button list
    _editMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _editMenuTableView->setLocalZOrder(10);
    _editMenuView->addChild(_editMenuTableView);
    _adjustMenuTableView =  SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _adjustMenuTableView->setLocalZOrder(10);
    _adjustMenuView->addChild(_adjustMenuTableView);
    _magicMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _magicMenuTableView->setLocalZOrder(10);
    _magicMenuView->addChild(_magicMenuTableView);
    
    // 자르기, 회전, 수평
    _editMenuTableView->numberOfRowsInSection = [&] (int section) {
        return 3;
    };
    // brightness, contrast, saturation, temperature
    _adjustMenuTableView->numberOfRowsInSection = [&] (int section) {
        return 4;
    };
    // sticker, text, draw, filter
    _magicMenuTableView->numberOfRowsInSection = [&] (int section) {
        return 4;
    };
    
    _editMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * converView = _editMenuTableView->dequeueReusableCellWithIdentifier("EDIT_CELL");
        MenuCell * cell = nullptr;
        int index = indexPath.getIndex();
        if (converView) {
            cell = (MenuCell*)converView;
        } else {
            cell = MenuCell::create(0, 0, 0, 160, 160);
            cell->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, 140, 140);
            cell->addChild(cell->menuButton);
            cell->menuButton->setShapeCornerRadius(10);
            cell->menuButton->setOutlineWidth(2.0f);
            cell->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            cell->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            cell->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            cell->menuButton->setPushDownScale(0.9f);
            cell->menuButton->setOnClickListener(this);
        }
        switch (index) {
            case 0:
            {
                cell->menuButton->setTextSystemFont("자르기", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 1:
            {
                cell->menuButton->setTextSystemFont("회전", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 2:
            {
                cell->menuButton->setTextSystemFont("수평", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        cell->setTag(index+10);
        cell->menuButton->setTag(index+10);
        
        return cell;
    };
    
    _adjustMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * converView = _adjustMenuTableView->dequeueReusableCellWithIdentifier("ADJUST_CELL");
        MenuCell * cell = nullptr;
        int index = indexPath.getIndex();
        if (converView) {
            cell = (MenuCell*)converView;
        } else {
            cell = MenuCell::create(0, 0, 0, 160, 160);
            cell->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, 140, 140);
            cell->addChild(cell->menuButton);
            cell->menuButton->setShapeCornerRadius(10);
            cell->menuButton->setOutlineWidth(2.0f);
            cell->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            cell->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            cell->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            cell->menuButton->setPushDownScale(0.9f);
            cell->menuButton->setOnClickListener(this);
        }
        switch (index) {
            case 0:
            {
                cell->menuButton->setTextSystemFont("밝기", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 1:
            {
                cell->menuButton->setTextSystemFont("명도", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 2:
            {
                cell->menuButton->setTextSystemFont("채도", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 3:
            {
                cell->menuButton->setTextSystemFont("색조", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        cell->setTag(index+20);
        cell->menuButton->setTag(index+20);
        
        return cell;
    };
    
    _magicMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * converView = _magicMenuTableView->dequeueReusableCellWithIdentifier("MAGIC_CELL");
        MenuCell * cell = nullptr;
        int index = indexPath.getIndex();
        if (converView) {
            cell = (MenuCell*)converView;
        } else {
            cell = MenuCell::create(0, 0, 0, 160, 160);
            cell->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, 140, 140);
            cell->addChild(cell->menuButton);
            cell->menuButton->setShapeCornerRadius(10);
            cell->menuButton->setOutlineWidth(2.0f);
            cell->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            cell->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            cell->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            cell->menuButton->setPushDownScale(0.9f);
            cell->menuButton->setOnClickListener(this);
        }
        switch (index) {
            case 0:
            {
                cell->menuButton->setTextSystemFont("스티커", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 1:
            {
                cell->menuButton->setTextSystemFont("텍스트", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 2:
            {
                cell->menuButton->setTextSystemFont("그리기", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case 3:
            {
                cell->menuButton->setTextSystemFont("필터", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        cell->setTag(index+30);
        cell->menuButton->setTag(index+30);
        
        return cell;
    };
    
    
    return true;
}

void SMImageEditorScene::setMainImage(cocos2d::Sprite *sprite)
{
    _mainImageView->setSprite(nullptr);
    _zoomView->setContentNode(nullptr);
    
//    CC_SAFE_RELEASE_NULL(_mainImageSprite);
    _mainImageSprite = cocos2d::Sprite::createWithTexture(sprite->getTexture());
//    _mainImageSprite =  cocos2d::Sprite::createWithSpriteFrame(sprite->getSpriteFrame());

    _mainImageView = SMImageView::create(_mainImageSprite);
    _mainImageView->setContentSize(_mainImageSprite->getContentSize());
    
    _zoomView->setContentNode(_mainImageView);
    _zoomView->refreshContentNode();
}

void SMImageEditorScene::setMenu(float dt)
{
    unschedule(schedule_selector(SMImageEditorScene::setMenu));
    
    // 현재 메뉴 체크
    switch (currentMenuType) {
        case kMenuTypeEdit:
        {
            showHideEditMenu(!_bEditMenuVisible);
            showHideAdjustMenu(false);
            showHideMagicMenu(false);
            CCLOG("[[[[[ 편집 눌렀음.");
        }
            break;
        case kMenuTypeAdjust:
        {
            showHideAdjustMenu(!_bAdjustMenuVisible);
            showHideEditMenu(false);
            showHideMagicMenu(false);
            CCLOG("[[[[[ 보정 눌렀음.");
        }
            break;
        case kMenuTypeMagic:
        {
            showHideMagicMenu(!_bMagicMenuVisible);
            showHideEditMenu(false);
            showHideAdjustMenu(false);
            CCLOG("[[[[[ 꾸미기 눌렀음.");
        }
            break;
            
        default:
            break;
    }
    
}

void SMImageEditorScene::showHideEditMenu(bool bShow)
{
    if (_bEditMenuVisible==bShow) {
        return;
    }
    _bEditMenuVisible = bShow;
 
    cocos2d::Vec2 pos = cocos2d::Point::ZERO;
    if (_bEditMenuVisible) {
        pos = cocos2d::Vec2(0, BOTTOM_MENU_HEIGHT);
        _editMenuView->setVisible(_bEditMenuVisible);
    } else {
        pos = cocos2d::Vec2::ZERO;
    }
    auto moveTo = cocos2d::MoveTo::create(0.15f, pos);
    auto action = cocos2d::Sequence::create(moveTo, cocos2d::CallFunc::create([&]{
        if (_bEditMenuVisible) {
            CCLOG("[[[[[ 편집 메뉴 보임");
        } else {
            CCLOG("[[[[[ 편집 메뉴 사라짐");
            _editMenuView->setVisible(_bEditMenuVisible);
        }
    }), NULL);
    _editMenuView->runAction(action);

    float op = 0;
    if (_bEditMenuVisible) {
        op = 255.0f;
    }
    
    auto fadeTo = cocos2d::FadeTo::create(0.15f, op);
    _editMenuView->runAction(fadeTo);
}

void SMImageEditorScene::showHideAdjustMenu(bool bShow)
{
    if (_bAdjustMenuVisible==bShow) {
        return;
    }
    _bAdjustMenuVisible = bShow;
    
    cocos2d::Vec2 pos = cocos2d::Point::ZERO;
    if (_bAdjustMenuVisible) {
        pos = cocos2d::Vec2(0, BOTTOM_MENU_HEIGHT);
        _adjustMenuView->setVisible(_bAdjustMenuVisible);
    } else {
        pos = cocos2d::Vec2::ZERO;
    }
    auto moveTo = cocos2d::MoveTo::create(0.15f, pos);
    auto action = cocos2d::Sequence::create(moveTo, cocos2d::CallFunc::create([&]{
        if (_bAdjustMenuVisible) {
            CCLOG("[[[[[ 보정 메뉴 보임");
        } else {
            CCLOG("[[[[[ 보정 메뉴 사라짐");
            _adjustMenuView->setVisible(_bAdjustMenuVisible);
        }
    }), NULL);
    _adjustMenuView->runAction(action);
    
    float op = 0;
    if (_bAdjustMenuVisible) {
        op = 255.0f;
    }
    
    auto fadeTo = cocos2d::FadeTo::create(0.15f, op);
    _adjustMenuView->runAction(fadeTo);
}

void SMImageEditorScene::showHideMagicMenu(bool bShow)
{
    if (_bMagicMenuVisible==bShow) {
        return;
    }
    _bMagicMenuVisible = bShow;
    
    cocos2d::Vec2 pos = cocos2d::Point::ZERO;
    if (_bMagicMenuVisible) {
        pos = cocos2d::Vec2(0, BOTTOM_MENU_HEIGHT);
        _magicMenuView->setVisible(_bMagicMenuVisible);
    } else {
        pos = cocos2d::Vec2::ZERO;
    }
    auto moveTo = cocos2d::MoveTo::create(0.15f, pos);
    auto action = cocos2d::Sequence::create(moveTo, cocos2d::CallFunc::create([&]{
        if (_bMagicMenuVisible) {
            CCLOG("[[[[[ 꾸미기 메뉴 보임");
        } else {
            CCLOG("[[[[[ 꾸미기 메뉴 사라짐");
            _magicMenuView->setVisible(_bMagicMenuVisible);
        }
    }), NULL);
    _magicMenuView->runAction(action);
    
    float op = 0;
    if (_bMagicMenuVisible) {
        op = 255.0f;
    }
    
    auto fadeTo = cocos2d::FadeTo::create(0.15f, op);
    _magicMenuView->runAction(fadeTo);
}

void SMImageEditorScene::onClick(SMView *view)
{
    int tag = view->getTag();
    if (tag<10) {
        if (isScheduled(schedule_selector(SMImageEditorScene::setMenu))) {
            unschedule(schedule_selector(SMImageEditorScene::setMenu));
        }
        
        currentMenuType = (kMenuType)tag;
        schedule(schedule_selector(SMImageEditorScene::setMenu), 0.1f);
    } else if (tag<20) {
        // edit menu
        switch (tag) {
            case kEditMenuCrop:
            {
//                cv::Mat src = cvImageUtil::ccImage2cvMat(_mainccImage);
//
//                cv::Mat dst;
//
//                cv::cvtColor(src, dst, cv::COLOR_RGBA2YUV_I420);
//
//                cv::cvtColor(dst, src, cv::COLOR_YUV2RGBA_IYUV);
//
//                auto ccImage = cvImageUtil::cvMat2ccImage(src);
//
//                auto texture = new cocos2d::Texture2D;
//                texture->initWithImage(ccImage);
//                texture->autorelease();
//
//                auto sprite = cocos2d::Sprite::createWithTexture(texture);
//
//                auto layer = (SMView*)_director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
//
//                auto imgView = SMImageView::create(sprite);
//                imgView->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 0.6f));
//                auto s = _director->getWinSize();
//                imgView->setContentSize(s);
//                layer->addChild(imgView);
//
//                imgView->setOnClickCallback([this](SMView * view){
//                    _director->getScheduler()->performFunctionInCocosThread([this]{
//                        auto layer = (SMView*)_director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
//                        layer->removeAllChildren();
//                    });
//                });
//
//                return;
                CCLOG("[[[[[ 자르기");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorCropScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            case kEditMenuRotate:
            {
                CCLOG("[[[[[ 회전");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorRotateScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            case kEditMenuHorizon:
            {
                CCLOG("[[[[[ 수평");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorStraightenScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            default:
            {
                CCLOG("[[[[[ WTF???");
            }
                break;
        }
    } else if (tag<30) {
        // adjust menu
        kColorType type;
        switch (tag) {
            case kAdjustMenuBrightness:
            {
                CCLOG("[[[[[ 밝기");
                type = kColorTypeBrightness;
            }
                break;
            case kAdjustMenuContrast:
            {
                CCLOG("[[[[[ 명도");
                type = kColorTypeContrast;
            }
                break;
            case kAdjustMenuSaturation:
            {
                CCLOG("[[[[[ 채도");
                type = kColorTypeSaturation;
            }
                break;
            case kAdjustMenuTemperature:
            {
                CCLOG("[[[[[ 색조");
                type = kColorTypeTemperature;
            }
                break;
            default:
            {
                CCLOG("[[[[[ WTF???");
            }
                break;
        }
        Intent * intent = Intent::create();
        intent->putRef("EDIT_IMAGE", _mainccImage);
        intent->putInt("COLOR_TYPE", type);
        auto scene = SMImageEditorColorScene::create(intent, SMScene::SwipeType::NONE);
        auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
        _director->pushScene(pScene);
        
    } else if (tag<40) {
        // magic menu
        switch (tag) {
            case kMagicMenuSticker:
            {
                CCLOG("[[[[[ 스티커");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorStickerScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            case kMagicMenuText:
            {
                CCLOG("[[[[[ 텍스트");
            }
                break;
            case kMagicMenuDraw:
            {
                CCLOG("[[[[[ 그리기");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorDrawScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            case kMagicMenuFilter:
            {
                CCLOG("[[[[[ 필터");
                Intent * intent = Intent::create();
                intent->putRef("EDIT_IMAGE", _mainccImage);
                auto scene = SMImageEditorFilterScene::create(intent, SMScene::SwipeType::NONE);
                auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
                _director->pushScene(pScene);
            }
                break;
            default:
            {
                CCLOG("[[[[[ WTF???");
            }
                break;
        }
    } else {
        CCLOG("[[[[[ WTF???");
    }
}

void SMImageEditorScene::onSceneResult(SMScene *fromScene, Intent *result)
{
    CCLOG("[[[[[ editor scene on scene result");
    if (result!=nullptr) {

        auto ccImage = static_cast<cocos2d::Image*>(result->getRef("EDIT_IMAGE"));
        if (ccImage) {
            _mainccImage->release();
            _mainccImage = new cocos2d::Image;
            _mainccImage->initWithRawData(ccImage->getData(), ccImage->getDataLen(), ccImage->getWidth(), ccImage->getHeight(), ccImage->getBitPerPixel());
            
            _mainccImage->retain();
            
            auto texture = new cocos2d::Texture2D;
            texture->initWithImage(_mainccImage);
            _mainccImage->release();
            
            auto tmp = cocos2d::Sprite::createWithTexture(texture);
            texture->release();
            
            setMainImage(tmp);
            
        } else {
            // else는 나중에 지우자
            auto sprite = static_cast<cocos2d::Sprite*>(result->getRef("CROP_SPRITE"));
            if (sprite) {
                setMainImage(sprite);
            }
        }
    }
}

void SMImageEditorScene::finishEditImage()
{

}
