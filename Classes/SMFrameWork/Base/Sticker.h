//
//  Sticker.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
// removable sticker view

#ifndef Sticker_h
#define Sticker_h

#include "SMImageView.h"
#include "../Util/DownloadProtocol.h"
#include <2d/CCAutoPolygon.h>

class RemovableSticker
{
public:
    virtual bool isRemovable() {return true;}
};


class Sticker : public SMImageView, public RemovableSticker
{
public:
    Sticker();
    virtual ~Sticker();
    
    enum ControlType {
        NONE,
        FAN,
        DELETE,
        UNPACK,
        PACK,
    };
    
    CREATE_VIEW(Sticker);
    
    bool isRemoved();
    
    void setControlType(ControlType controlType) {_controlType=controlType;}
    ControlType getControlType() {return _controlType;}

    virtual void onImageLoadComplete(cocos2d::Sprite * sprite, int tag, bool direct) override;
    
    virtual void onPolygonInfoComplete(cocos2d::PolygonInfo * polyinfo, int tag) override;
    
    // fish Eye effect
    virtual void setFatValue(const float value);
    
    float getFatValue() const {return _fatValue;}
    
    std::function<void(Sticker* sender, cocos2d::Sprite* sprite)> _onSpriteLoadedCallback;
    
    Sticker* clone();
    
protected:
    virtual bool init() override;
    
private:
    ControlType _controlType;
    cocos2d::PolygonInfo * _polyInfo;
    float _fatValue;
};

#endif /* Sticker_h */
