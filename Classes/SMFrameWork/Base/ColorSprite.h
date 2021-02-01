//
//  ColorSprite.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ColorSprite_h
#define ColorSprite_h

#include "MeshSprite.h"

class ColorSprite : public MeshSprite
{
public:
    static ColorSprite * createWithSprite(cocos2d::Sprite * sprite);
    
    void startEditMode();
    void finishEditMode(const bool apply);
    
protected:
    ColorSprite();
    virtual ~ColorSprite();
    
public:
    struct Color {
        int brig;   // brightness
        int cont;   // contrast
        int satu;   // saturation
        int temp;   // temperature
    };

    void setBrightness(const int brightness);
    void setContrast(const int contrast);
    void setSaturate(const int saturate);
    void setTemperature(const int temperature);
    
protected:
    void restoreColorValue(const Color& color);
    const Color& getColorValue() { return _editColor; }
    

    void setColorShader(cocos2d::Sprite* sprite);
    
    void setColorValue(cocos2d::Sprite* sprite, const ColorSprite::Color& color);

    bool isColorModified(Color& value);
    
    bool isColorModified();

private:
    bool _editMode;
    Color _editColor;
    Color _initColor;
    
    cocos2d::Sprite* _rawSprite;
    cocos2d::Size _rawSize;
};


#endif /* ColorSprite_h */
