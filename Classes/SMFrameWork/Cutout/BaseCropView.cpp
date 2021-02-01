//
//  BaseCropView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#include "BaseCropView.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"

#define ACTION_TAG_DIM (0x1000)
#define ACTION_TAG_SHOW (0x1001)

BaseCropView::BaseCropView() : _maskSprite(nullptr)
, _dim(nullptr)
{
    
}

BaseCropView::~BaseCropView()
{
    
}

bool BaseCropView::init(cocos2d::Sprite *sprite, const cocos2d::Rect &boundRect)
{
    auto s = _director->getWinSize();
    setContentSize(s);
    
    _boundRect = boundRect;
    
    _dim = ShapeSolidRect::create();
    _dim->setContentSize(s);
    _dim->setColor4F(MAKE_COLOR4F(0, 0));
    addChild(_dim);
    
    // mask sprite
    auto texture = sprite->getTexture();
    _textureSize = texture->getContentSizeInPixels();
    
    _maskSprite = MaskSprite::createWithTexture(texture);
    _maskSprite->setScale(_boundRect.size.width/_textureSize.width);
    _maskSprite->setPosition(_boundRect.getMidX(), _boundRect.getMidY());
    addChild(_maskSprite);
    
    _uiLayer = SMView::create();
    _uiLayer->setContentSize(s);
    addChild(_uiLayer);
    
    return true;
}


void BaseCropView::showUILayer(bool show)
{
    if (_uiLayer->getActionByTag(ACTION_TAG_SHOW)) {
        _uiLayer->stopActionByTag(ACTION_TAG_SHOW);
    }
    
    auto action = ViewAction::TransformAction::create();
    if (show) {
        action->toAlpha(1);
        action->setTimeValue(0.3, 0.2);
    } else {
        action->toAlpha(0);
        action->setTimeValue(0.1, 0);
    }
    
    action->setTag(ACTION_TAG_SHOW);
    _uiLayer->runAction(action);
}

void BaseCropView::setDimOpacity(float opacity, float duration, float delay)
{
    auto a = _dim->getActionByTag(ACTION_TAG_DIM);
    if (a) {
        _dim->stopAction(a);
    }
    
    auto action = ViewAction::TransformAction::create();
    action->setTag(ACTION_TAG_DIM);
    action->toAlpha(opacity).setTimeValue(duration, delay);
    _dim->runAction(action);
}

float BaseCropView::getSpriteScale()
{
    return _maskSprite->getScale();
}

cocos2d::Rect BaseCropView::getBoundRect()
{
    return _boundRect;
}

cocos2d::Sprite * BaseCropView::cropSprite(cocos2d::Rect &outRect)
{
    return cropSpriteInternal(outRect);
}

bool BaseCropView::cropData(uint8_t **outPixelData, uint8_t **outMaskData, cocos2d::Rect &outRect)
{
    return cropDataInternal(outPixelData, outMaskData, outRect);
}

