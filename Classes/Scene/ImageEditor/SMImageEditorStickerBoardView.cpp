//
//  SMImageEditorStickerBoardView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStickerBoardView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "SMImageEditorStickerItemView.h"
#include "../../SMFrameWork/Base/SMZoomView.h"
#include "../../SMFrameWork/UI/StickerControlView.h"
#include "../../SMFrameWork/Base/MeshSprite.h"
#include "../../SMFrameWork/Base/Sticker.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"
#include <cmath>

//#define CANVAS_WIDTH    (912)
//#define CANVAS_HEIGHT   (1216)
#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f

SMImageEditorStickerBoardView * SMImageEditorStickerBoardView::createWithSprite(cocos2d::Sprite *sprite)
{
    auto view = new (std::nothrow)SMImageEditorStickerBoardView();
    if (view && view->initWithSprite(sprite)) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

SMImageEditorStickerBoardView * SMImageEditorStickerBoardView::create()
{
    auto view = new (std::nothrow)SMImageEditorStickerBoardView();
    
    if (view && view->init()) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}


SMImageEditorStickerBoardView::SMImageEditorStickerBoardView() :
_canvasView(nullptr)
, _controlView(nullptr)
{
    
}

SMImageEditorStickerBoardView::~SMImageEditorStickerBoardView()
{
    CC_SAFE_RELEASE_NULL(_meshSprite);
}

bool SMImageEditorStickerBoardView::initWithSprite(cocos2d::Sprite * sprite)
{
    if (sprite) {
        _meshSprite = MeshSprite::create(sprite);
        _meshSprite->retain();
    }
    
    return init();
}

bool SMImageEditorStickerBoardView::init()
{
    if (!SMView::init()) {
        return false;
    }
    auto s = _director->getWinSize();
    s = cocos2d::Size(s.width, s.height-TOP_MENU_HEIGHT-BOTTOM_MENU_HEIGHT);
    setContentSize(s);
    
    // 스티커를 담을 줌뷰
    _zoomView = SMZoomView::create();
    _zoomView->setContentSize(s);
    _zoomView->setPadding(PHOTO_MARGIN);
    addChild(_zoomView);
    
    // 스티커를 조절할 컨트롤뷰
    _controlView = StickerControlView::create();
    _controlView->setContentSize(s);
    _controlView->setStickerListener(this);
    addChild(_controlView);
    
    // 스티커 컨테이너
    // 컨테이너의 크기는 받은 sprite의 크기로 계산
    _contentView = SMView::create();
    _contentView->setBackgroundColor4F(cocos2d::Color4F(1, 1, 1, 0.6f));
    if (_meshSprite!=nullptr) {
        _contentView->setContentSize(_meshSprite->getContentSize());
    } else {
        _contentView->setContentSize(s);
    }
    _zoomView->setContentNode(_contentView);
    
    if (_meshSprite!=nullptr) {
        _bgImageView = SMImageView::create(_meshSprite);
        _bgImageView->setContentSize(_meshSprite->getContentSize());
        _bgImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _bgImageView->setPosition(_meshSprite->getContentSize()/2);
    } else {
        _bgImageView = SMImageView::create();
        _bgImageView->setContentSize(s);
        _bgImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _bgImageView->setPosition(s/2);
    }
    _contentView->addChild(_bgImageView);

    // bgimage guide
    auto rect = ShapeRect::create();
    if (_meshSprite!=nullptr) {
        rect->setContentSize(_meshSprite->getContentSize());
    } else {
        rect->setContentSize(s);
    }
    rect->setLineWidth(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
    rect->setColor4F(SMColorConst::COLOR_F_DBDCDF);
    _bgImageView->addChild(rect);
    
    // 스티커가 올라가는 canvas
    // canvas는 conentView보다 커도 됨. 스티커 올라가야 하니까...
    _canvasView = StickerCanvasView::create();
    _canvasView->setContentSize(s);
    _canvasView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _canvasView->setPosition(_contentView->getContentSize()/2);
    _canvasView->setStickerListener(this);
    _contentView->addChild(_canvasView);
    
    
    return true;
}

bool SMImageEditorStickerBoardView::containsPoint(const cocos2d::Vec2 &point)
{
    return true;
}

int SMImageEditorStickerBoardView::dispatchTouchEvent(MotionEvent *event, SMView *view, bool checkBounds)
{
    return SMView::dispatchTouchEvent(event, view, false);
}

void SMImageEditorStickerBoardView::setStickerListener(StickerCanvasListener *canvasListener, StickerControlListener *controlListener)
{
    _canvasListener = canvasListener;
    _controlListener = controlListener;
}

void SMImageEditorStickerBoardView::addSticker(cocos2d::Node *sticker)
{
    if (sticker!=nullptr) {
        _canvasView->addChild(sticker);
    }
}

void SMImageEditorStickerBoardView::addStickerAboveAt(cocos2d::Node *sticker, cocos2d::Node *aboveAt)
{
    if (sticker!=nullptr) {
        _canvasView->addChild(sticker);
        reorderStickerAboveAt(sticker, aboveAt);
    }
}

void SMImageEditorStickerBoardView::reorderStickerAboveAt(cocos2d::Node *sticker, cocos2d::Node *aboveAt)
{
    if (aboveAt) {
        _canvasView->aboveNode(sticker, aboveAt);
    } else {
        _canvasView->sendChildToBack(sticker);
    }
}

void SMImageEditorStickerBoardView::removeSticker(cocos2d::Node *sticker)
{
    if (sticker!=nullptr) {
        _canvasView->removeChild(sticker);
    }
}

void SMImageEditorStickerBoardView::removeStickerWithFadeOut(cocos2d::Node *sticker, const float duration, const float delay)
{
    if (sticker!=nullptr) {
        _canvasView->removeChildWithFadeOut(sticker, duration, delay);
    }
}

void SMImageEditorStickerBoardView::removeAllStickerWithFly()
{
    auto children = _canvasView->getChildren();
    
    cocos2d::Vec2 pt1(_contentSize.width/2, _contentSize.height/2);
    
    for (auto child : children) {
        auto sticker = dynamic_cast<SMImageEditorStickerItemView*>(child);
        if (!sticker) {
            continue;
        }
        
        cocos2d::Vec2 pt2 = sticker->getPosition();
        
        double radians = std::atan2(pt2.y-pt1.y, pt2.x-pt1.x);
        float degrees = (float)ViewUtil::toDegrees(radians);

        // 좀더 부산스럽게 흩어져라.
        if (degrees>90 && degrees<120) {
            degrees += cocos2d::random(0.0f, 30.0f);
        }
        if (degrees<90 && degrees>60) {
            degrees -= cocos2d::random(0.0f, 30.0f);
        }
        
        _canvasView->removeChildWithFly(sticker, degrees, cocos2d::random(7000.0f, 8000.0f));
    }
}

void SMImageEditorStickerBoardView::removeAllSticker()
{
    auto children = _contentView->getChildren();
    
    int size = (int)children.size();
    // 뒤에서 부터
    for (int i=size-1; i>=0; i--) {
        auto sticker = dynamic_cast<SMImageEditorStickerItemView*>(children.at(i));
        if (sticker) {
            if (!sticker) {
                continue;
            }
            
            removeSticker(sticker);
        }
    }
}

// trash animation
void SMImageEditorStickerBoardView::startGeineRemove(cocos2d::Node *node)
{
    auto sticker = dynamic_cast<SMImageEditorStickerItemView*>(node);
    
    if (!sticker) {
        return;
    }
    
    auto sprite = sticker->getSprite();
    if (!sprite) {
        return;
    }
    
    _controlView->startGeineRemove(node);
    _canvasView->removeChildWithGenieAction(sticker, sprite, cocos2d::Vec2(0.0f, 1.0f), 0.5f, 0.01f);
}

void SMImageEditorStickerBoardView::onStickerTouch(cocos2d::Node *node, int action)
{
    if (_canvasListener) {
        _canvasListener->onStickerTouch(node, action);
    }
}

void SMImageEditorStickerBoardView::onStickerMenuClick(cocos2d::Node *sticker, int menuId)
{
    if (_controlListener) {
        _controlListener->onStickerMenuClick(sticker, menuId);
    }
}

void SMImageEditorStickerBoardView::onStickerSelected(cocos2d::Node *node, const bool selected)
{
    if (selected) {
        _controlView->linkStickerNode(node);
    } else {
        _controlView->linkStickerNode(nullptr);
    }
    
    if (_canvasListener) {
        _canvasListener->onStickerSelected(node, selected);
    }
}

void SMImageEditorStickerBoardView::onStickerDoubleClicked(cocos2d::Node *node, const cocos2d::Vec2 &worldPoint)
{
    _zoomView->performDoubleClick(worldPoint);
    
    if (_canvasListener) {
        _canvasListener->onStickerDoubleClicked(node, worldPoint);
    }
}

void SMImageEditorStickerBoardView::onStickerRemoveBegin(cocos2d::Node *node)
{
    if (node && node==_canvasView->getSelectedSticker()) {
        _controlView->linkStickerNode(nullptr);
    }
    
    if (_canvasListener) {
        _canvasListener->onStickerRemoveBegin(node);
    }
}

void SMImageEditorStickerBoardView::onStickerRemoveEnd(cocos2d::Node *node)
{
    if (node && node==_canvasView->getSelectedSticker()) {
        _controlView->linkStickerNode(nullptr);
    }
    
    if (_canvasListener) {
        _canvasListener->onStickerRemoveEnd(node);
    }
}

void SMImageEditorStickerBoardView::setZoomStatus(const float panX, const float panY, const float zoomScale, const float duration)
{
    _zoomView->setZoomWithAnimation(panX, panY, zoomScale, duration);
}
