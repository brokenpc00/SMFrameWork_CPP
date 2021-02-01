//
//  SMImageEditorRotateScene.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 8..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorRotateScene_h
#define SMImageEditorRotateScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../ImagePicker/SMImagePickerScene.h"
class SMView;
class SMImageView;
class SMButton;
class SMTableView;

typedef enum {
    kRotateMenuLeft = 0,
    kRotateMenuRight,
    kRotateMenuFlipY,
    kRotateMenuFlipX,
    kRotateMenuCount,
} kRotateMenu;

class SMImageEditorRotateScene : public SMScene, public OnClickListener
{
public:
    SMImageEditorRotateScene();
    virtual ~SMImageEditorRotateScene();
    CREATE_SCENE(SMImageEditorRotateScene);
    void setOnImageSelectedListener(OnImageSelectedListener * l) {_listener = l;}
    
    class MenuCell : public SMView
    {
    public:
        MenuCell(){};
        ~MenuCell(){};
        CREATE_VIEW(MenuCell);
        
        SMButton * menuButton;
    };
    
    
protected:
    virtual bool init() override;

    virtual void onClick(SMView * view) override;
    
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    SMTableView * _rotateMenuTableView;
    
    SMImageView * _mainImageView;
    cocos2d::Sprite * _currentImageSprite;
    cocos2d::Image * _mainccImage;
    
    void doRotate(float dt);
    kRotateMenu _currentRotateType;
    bool _isRotateAnimation;
    
    void applyRotateImage();
    
    bool _fromPicker;
    OnImageSelectedListener * _listener;
    int _callIndex;

};

#endif /* SMImageEditorRotateScene_h */
