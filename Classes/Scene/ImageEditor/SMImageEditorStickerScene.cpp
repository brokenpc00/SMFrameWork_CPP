//
//  SMImageEditorStickerScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStickerScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/ImageProcess/ImgPrcSimpleCapture.h"
#include "../../SMFrameWork/Base/ShaderUtil.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f

#define CANVAS_WIDTH    (912)
#define CANVAS_HEIGHT   (1216)

#define STICKER_LIST_ACTION_TAG    (SMViewConstValue::Tag::USER-1)

SMImageEditorStickerScene::SMImageEditorStickerScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _stickerBoardView(nullptr)
//, _mainImageView(nullptr)
, _currentImageSprite(nullptr)
, _mainccImage(nullptr)
{
    
}

SMImageEditorStickerScene::~SMImageEditorStickerScene()
{
    
}

bool SMImageEditorStickerScene::init()
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
            applyStickerImage();
        });
    });
    
    /*
    _editZoomView = SMZoomView::create(0, 0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
    _editZoomView->setBackgroundColor4F(SMColorConst::COLOR_F_DBDCDF);
    _mainImageView = SMImageView::create(_currentImageSprite);
    _mainImageView->setContentSize(_currentImageSprite->getContentSize());
    _editZoomView->setPadding(PHOTO_MARGIN);
    _editZoomView->setContentNode(_mainImageView);
    _contentView->addChild(_editZoomView);
    _editZoomView->setScissorEnable(true);
    */
    
    auto boardSize = cocos2d::Size(s.width, s.height-TOP_MENU_HEIGHT-BOTTOM_MENU_HEIGHT);
    _stickerBoardView = SMImageEditorStickerBoardView::createWithSprite(_currentImageSprite);
    _stickerBoardView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _stickerBoardView->setPosition(cocos2d::Vec2(s.width/2, s.height-(TOP_MENU_HEIGHT+boardSize.height/2)));
    _stickerBoardView->setStickerListener(this, this);
    _contentView->addChild(_stickerBoardView);
    
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    _bottomMenuView->setLocalZOrder(10);
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    
    _stickerListView = SMImageEditorStickerItemListView::create();
    _stickerListView->setOnItemClickListener(this);
    _bottomMenuView->addChild(_stickerListView);
    
    _stickerListView->setAnchorPoint(cocos2d::Vec2::ANCHOR_TOP_LEFT);
    
    auto action = _stickerListView->getActionByTag(STICKER_LIST_ACTION_TAG);
    if (action) {
        action->stop();
    }
    _stickerListView->setVisible(true);
    _stickerListView->setPositionY(_stickerListView->getContentSize().height);
    
    
    //_stickerListView->show();
    
    return true;
}

void SMImageEditorStickerScene::onStickerMenuClick(cocos2d::Node *node, int menuId)
{
    CCLOG("[[[[[ sticker menu click");
    auto sticker = dynamic_cast<SMImageEditorStickerItemView*>(node);
    if (sticker) {
        sticker->prepareRemove();
    }
    _stickerBoardView->startGeineRemove(node);
}

void SMImageEditorStickerScene::onStickerTouch(cocos2d::Node *node, int action)
{
    CCLOG("[[[[[ sticker touch");
}

void SMImageEditorStickerScene::onStickerSelected(cocos2d::Node *node, const bool selected)
{
    CCLOG("[[[[[ sticker select");
}

void SMImageEditorStickerScene::onStickerDoubleClicked(cocos2d::Node *node, const cocos2d::Vec2 &worldPoint)
{
    CCLOG("[[[[[ sticker double click");
}

void SMImageEditorStickerScene::onStickerRemoveBegin(cocos2d::Node *node)
{
    CCLOG("[[[[[ sticker remove begin");
}

void SMImageEditorStickerScene::onStickerRemoveEnd(cocos2d::Node *node)
{
    CCLOG("[[[[[ sticker remove end");
}

void SMImageEditorStickerScene::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    CCLOG("[[[[[ Sticker Edit scene image load complete callback");
}

void SMImageEditorStickerScene::onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite *sprite, Intent *result)
{
    CCLOG("[[[[[ Sticker Edit scene image process complete callback");

    ///*
    
    //auto captureSprite = cocos2d::Sprite::createWithTexture(ViewUtil::makeBitmapCopy(sprite, sprite->getContentSize(), sprite->getPosition(), sprite->getAnchorPoint(), 1.0f, 1.0f));
    
    
    float oldScale = sprite->getScale();
    cocos2d::Vec2 oldPos = sprite->getPosition();
    cocos2d::Vec2 oldAnchor = sprite->getAnchorPoint();
    cocos2d::Size imageSize = sprite->getTexture()->getContentSizeInPixels();
    auto rt = cocos2d::RenderTexture::create(imageSize.width, imageSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->begin();
    //sprite->setScale(1.0f);
    sprite->setScale(1.0f, -1.0f);
    sprite->setAnchorPoint(cocos2d::Vec2::ZERO);
    sprite->setPosition(cocos2d::Vec2::ZERO);
    sprite->visit();
    rt->end();
    _director->getRenderer()->render();
    
    sprite->setScale(oldScale);
    sprite->setAnchorPoint(oldAnchor);
    sprite->setPosition(oldPos);
    cocos2d::Image * newImage = rt->newImage();
    //*/
    
    /*
    auto rt = cocos2d::RenderTexture::create(sprite->getTexture()->getPixelsWide(), sprite->getTexture()->getPixelsHigh(), cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->begin();
    sprite->visit();
    rt->end();

    auto newImage = rt->newImage();
    */
    auto intent = Intent::create();
    intent->putRef("EDIT_IMAGE", newImage);
    setSceneResult(intent);
    
    newImage->autorelease();
    
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);
}

void SMImageEditorStickerScene::applyStickerImage()
{
    
    // 여기에 모든게 다 올라간다. 이넘이 zoomview 안에 있는거라서... 이넘을 캡쳐한다.
    auto drawBoard = _stickerBoardView->getContentView();
    
    auto drawSize = drawBoard->getContentSize();
    auto oldPos = drawBoard->getPosition();
    auto oldAnc = drawBoard->getAnchorPoint();
    auto oldScale = drawBoard->getScale();
    
    auto rt = cocos2d::RenderTexture::create(drawSize.width, drawSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->begin();
    drawBoard->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    drawBoard->setPosition(drawSize/2);
    drawBoard->setScale(1.0f);
    drawBoard->visit(_director->getRenderer(), cocos2d::Mat4::IDENTITY, 0);
    rt->end();
    
    _director->getRenderer()->render();
    
    drawBoard->setAnchorPoint(oldAnc);
    drawBoard->setPosition(oldPos);
    drawBoard->setScale(oldScale);
    
    auto newImage = rt->newImage();
    
     auto intent = Intent::create();
    intent->putRef("EDIT_IMAGE", newImage);
    setSceneResult(intent);
    
    newImage->autorelease();
    
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);
    
}

void SMImageEditorStickerScene::onStickerLayout(SMImageEditorStickerItemView *sticker, cocos2d::Sprite *sprite, const StickerItem &item, const int colorIndex)
{
    CCLOG("[[[[[ sticker layout");
}

void SMImageEditorStickerScene::onItemClick(ItemListView *sender, SMImageEditorItemThumbView *view)
{
    runSelectSticker(view->getTag());
    CCLOG("[[[[[ item click");
}

SMImageEditorStickerItemView * SMImageEditorStickerScene::runSelectSticker(int index, bool fromTemplate, int colorIndex, int code)
{
    if (index==0 && code<0) {
        // remove all sticker
        _stickerBoardView->removeAllStickerWithFly();
    } else {
        // select sticker
        SMImageEditorStickerItemView * sticker = nullptr;
        auto item = _stickerListView->getItem(index);
        if (item) {
            sticker = SMImageEditorStickerItemView::createWithItem(*item, this);
            _stickerBoardView->addSticker(sticker);
        }
        if (sticker) {
            sticker->setPosition(_stickerBoardView->getContentSize()/2);
            _stickerBoardView->getCanvas()->setSelectedSticker(sticker);
        }
    }
    
    return nullptr;
}
