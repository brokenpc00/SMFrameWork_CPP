//
//  SMImageEditorDrawScene.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#ifndef SMImageEditorDrawScene_h
#define SMImageEditorDrawScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../ImagePicker/SMImagePickerScene.h"
class SMView;
class SMImageView;
class SMButton;
class SMTableView;
class DrawView;

enum DrawColorType {
    DrawColorTypeRed = 0,
    DrawColorTypeBlue,
    DrawColorTypeGreen,
    DrawColorTypeYellow,
    DrawColorTypeBlack,
    DrawColorTypeWhite,
};


class SMImageEditorDrawScene : public SMScene, public OnClickListener
{
public:
    CREATE_SCENE(SMImageEditorDrawScene);
    void setOnImageSelectedListener(OnImageSelectedListener * l) {_listener = l;}

protected:
    SMImageEditorDrawScene();
    virtual ~SMImageEditorDrawScene();

    class MenuCell : public SMView
    {
    public:
        MenuCell(){};
        ~MenuCell(){};
        CREATE_VIEW(MenuCell);
        
        SMButton * menuButton;
    };
    
    virtual bool init() override;
    
    virtual void onClick(SMView * view) override;

    void applyDrawImage();
    

    void changeColorButton(DrawColorType type);
    
    cocos2d::Color4F getColor(DrawColorType type);

private:
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    SMTableView * _drawMenuTableView;
    SMImageView * _mainImageView;
    cocos2d::Sprite * _currentImageSprite;
    cocos2d::Image * _mainccImage;
    
    MenuCell * _clearButton;
    SMView * _drawContainerView;
    DrawView * _drawView;
    bool _fromPicker;
    OnImageSelectedListener * _listener;
    int _callIndex;
    
    std::vector<SMButton*> _colorButtons;

};


#endif /* SMImageEditorDrawScene_h */
