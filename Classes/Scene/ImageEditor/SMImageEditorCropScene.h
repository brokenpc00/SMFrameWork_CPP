//
//  SMImageEditorCropScene.h
//  iPet
//
//  Created by KimSteve on 2017. 4. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorCropScene_h
#define SMImageEditorCropScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "SMImageEditorCropGuideView.h"
#include "../../SMFrameWork/Cutout/DrawCropView.h"
//#include "SMImageEditorListener.h"

class SMView;
class SMImageView;
class SMZoomView;
class SMButton;
class SMTableView;
class SMPageView;
class SMCircularListView;
class SMSlider;
class MeshSprite;

class SMImageEditorCropGuideView;

class SMImageEditorCropScene : public SMScene, public OnClickListener
{
public:
    SMImageEditorCropScene();
    virtual ~SMImageEditorCropScene();
    CREATE_SCENE(SMImageEditorCropScene);
    
    class MenuCell : public SMView
    {
    public:
        MenuCell(){};
        ~MenuCell(){};
        CREATE_VIEW(MenuCell);
        
        SMButton * menuButton;
    };
    
//    void setOnImageResultListener(SMImageEditorListener * l){_listener = l;};
    void setOnEditImageResultCallback(std::function<void(cocos2d::Sprite * editSprite)> callback) {_onEditImageResultCallback = callback;};
protected:
    virtual void onClick(SMView * view) override;
    
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    
    SMImageView * _mainImageView;
//    MeshSprite * _meshSprite;
    SMTableView * _cropMenuTableView;
    
    SMImageEditorCropGuideView * _cropGuideView;
    
    cocos2d::Rect _imageRect;
    cocos2d::Sprite * _originImageSprite;
    cocos2d::Image * _mainccImage;
    
    SMView* _innerContentView;
    DrawCropView * _freeCropView;
    
//    SMImageEditorListener * _listener;
    std::function<void(cocos2d::Sprite * editSprite)> _onEditImageResultCallback;
    
    void showFreeCropView(bool bShow);
    
    SMButton * _drawCropButton;
    
private:
    virtual bool init() override;
    
//    SMImageView * _tmpImageView;
};

#endif /* SMImageEditorCropScene_h */
