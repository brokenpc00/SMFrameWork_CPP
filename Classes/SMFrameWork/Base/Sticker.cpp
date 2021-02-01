//
//  Sticker.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#include "Sticker.h"
#include "MeshSprite.h"
#include "../Const/SMViewConstValue.h"

#define FAT_STEP_VALUE 0.2

Sticker::Sticker() : _controlType(ControlType::NONE), _fatValue(0), _polyInfo(nullptr)
{
    
}

Sticker::~Sticker()
{
    
}

bool Sticker::init()
{
    setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    return SMImageView::init();
}

bool Sticker::isRemoved()
{
    return getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE) != nullptr;
}

void Sticker::onImageLoadComplete(cocos2d::Sprite *sprite, int tag, bool direct)
{
    if (sprite!=nullptr) {
        auto meshSprite = MeshSprite::create(sprite);
        if (meshSprite) {
            setSprite(meshSprite, true);
            
            if (_onSpriteLoadedCallback) {
                _onSpriteLoadedCallback(this, meshSprite);
            }
        } else {
            setSprite(sprite, true);
            
            if (_onSpriteLoadedCallback) {
                _onSpriteLoadedCallback(this, sprite);
            }
        }
    }
}

void Sticker::onPolygonInfoComplete(cocos2d::PolygonInfo *polyinfo, int tag)
{
    if (polyinfo) {
        if (getPhysicsBody()==nullptr) {
            auto body = ImageDownloader::polygonInfoToPhysicsBody(polyinfo, _contentSize);
            if (body) {
                // must be false
                body->setDynamic(false);
                
                body->setCollisionBitmask(0);
                body->setContactTestBitmask(0);
                setPhysicsBody(body);
            }
        }
        getSprite()->setPolygonInfo(*polyinfo);
    }
}

void Sticker::setFatValue(const float value)
{
    // fish eye effect
    _fatValue = false;
    
    if (_fatValue != 0) {
        auto sprite = dynamic_cast<MeshSprite*>(getSprite());
        if (sprite) {
            if (!sprite->isMesh()) {
                // mesh sprite가 아니면 mesh sprite로 만든다.
                sprite->convertToMesh();
            }
            // mesh sprite로 셋팅되어 있으면
            auto size = sprite->getTextureRect().size;
            // sprite의 중앙을 기준으로 value만큼 step value로 키운다.
            // width가 영향 범위이다.
            sprite->grow(size.width/2, size.height/2, value, FAT_STEP_VALUE, size.width);
        }
    }
}

Sticker * Sticker::clone()
{
    // 현재 sprite를 기준으로 sticker를 하나 복사한다.
    // 속성값은 현재 Sticker의 복사본이다.
    if (getSprite()) {
        
        auto sprite = cocos2d::Sprite::createWithTexture(getSprite()->getTexture());
        
        auto sticker = Sticker::create();
        sticker->setSprite(sprite, true);
        sticker->setPosition(getPosition());
        sticker->setScale(getScale());
        sticker->setAnchorPoint(getAnchorPoint());
        sticker->setRotation(getRotation());
        
        return sticker;
    }
    
    return nullptr;
}
