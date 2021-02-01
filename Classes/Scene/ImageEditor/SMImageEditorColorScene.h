//
//  SMImageEditorColorScene.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorColorScene_h
#define SMImageEditorColorScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/Base/SMSlider.h"
#include "../../SMFrameWork/Base/ColorSprite.h"

class SMView;
class SMImageView;
class SMButton;

typedef enum {
    kColorTypeBrightness,
    kColorTypeContrast,
    kColorTypeSaturation,
    kColorTypeTemperature,
}kColorType;

class SMImageEditorColorScene : public SMScene
{
public:
    SMImageEditorColorScene();
    virtual ~SMImageEditorColorScene();
    CREATE_SCENE(SMImageEditorColorScene);
    
protected:
    virtual bool init() override;
    
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    SMSlider * _sliderView;

    SMImageView * _mainImageView;
    ColorSprite * _currentImageSprite;
    cocos2d::Image * _mainccImage;
    
    void applyImageColor();
    void changeColorValue(float value);
    
    kColorType _curColorType;
};

#endif /* SMImageEditorColorScene_h */
