//
//  SMImageEditorScene.h
//  iPet
//
//  Created by KimSteve on 2017. 4. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorScene_h
#define SMImageEditorScene_h

#include "../../SMFrameWork/Base/SMScene.h"

class SMVIew;
class SMImageView;
class SMZoomView;
class SMButton;
class SMTableView;
class SMPageView;
class SMCircularListView;
class SMSlider;
class MeshSprite;

typedef enum {
    kMenuTypeEdit = 0,
    kMenuTypeAdjust,
    kMenuTypeMagic
} kMenuType;

typedef enum {
    kEditMenuCrop = 10,
    kEditMenuRotate,
    kEditMenuHorizon,
}kEditMenu;

typedef enum {
    kAdjustMenuBrightness = 20,
    kAdjustMenuContrast,
    kAdjustMenuSaturation,
    kAdjustMenuTemperature,
}kAdjustMenu;

typedef enum {
    kMagicMenuSticker = 30,
    kMagicMenuText,
    kMagicMenuDraw,
    kMagicMenuFilter,
}kMagicMenu;


class SMImageEditorScene : public SMScene, public OnClickListener
{
public:
    SMImageEditorScene();
    virtual ~SMImageEditorScene();
    
    CREATE_SCENE(SMImageEditorScene);
    
protected:
    
    virtual void onClick(SMView * view) override;
    
    class MenuCell : public SMView
    {
    public:
        MenuCell(){};
        ~MenuCell(){};
        CREATE_VIEW(MenuCell)
        
        SMButton * menuButton;
    };
    
    void setMenu(float dt);
    
    kMenuType currentMenuType;
    
    void showHideEditMenu(bool bShow);
    void showHideAdjustMenu(bool bShow);
    void showHideMagicMenu(bool bShow);
    
    bool _bEditMenuVisible;
    bool _bAdjustMenuVisible;
    bool _bMagicMenuVisible;
    
    void setMainImage(cocos2d::Sprite * sprite);
    void finishEditImage();
    
    virtual void onSceneResult(SMScene* fromScene, Intent* result) override;
    
private:
    virtual bool init() override;
    
    SMZoomView * _zoomView;
    SMImageView * _mainImageView;
    SMView * _topMenuView;
    SMView * _contentView;
    cocos2d::Sprite * _mainImageSprite;
    MeshSprite * _mainMeshSprite;
    SMTableView * _bottomMenuTableView;
    
    SMView * _editMenuView;
    SMView * _adjustMenuView;
    SMView * _magicMenuView;
    
    SMTableView * _editMenuTableView;
    SMTableView * _adjustMenuTableView;
    SMTableView * _magicMenuTableView;
    
    cocos2d::Image * _resultccImage;
    cocos2d::Image * _originccImage;
    cocos2d::Image * _mainccImage;
    
    cocos2d::Sprite * _resultSprite;
    
    bool _bAlreadyCrop;
    SMImageView * _tmpImageview;
};

#endif /* SMImageEditorScene_hpp */
