//
//  SMImageEditorStraightenScene.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStraightenScene_h
#define SMImageEditorStraightenScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/Base/SMSlider.h"
#include "SMImageEditorStraightenGuideView.h"
class SMView;
class SMImageView;
class SMButton;


class SMImageEditorStraightenScene : public SMScene
{
public:
    SMImageEditorStraightenScene();
    virtual ~SMImageEditorStraightenScene();
    CREATE_SCENE(SMImageEditorStraightenScene);
    
    
protected:
    virtual bool init() override;
    
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    SMSlider * _sliderView;
    
    SMImageEditorStraightenGuideView * _straightenGuideView;
    
    cocos2d::Rect _imageRect;
    SMImageView * _mainImageView;
    cocos2d::Sprite * _currentImageSprite;
    cocos2d::Image * _mainccImage;
    
    float _currentDgrees;
    float _currentScale;
    float _initScale;

    void applyStraightenImage();
    void changeStraighten(float value);
    double calculateShortestDistance(double x1, double y1, double x2, double y2, double x, double y);
};

#endif /* SMImageEditorStraightenScene_h */
