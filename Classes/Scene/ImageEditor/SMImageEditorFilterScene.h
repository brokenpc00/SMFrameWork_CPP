//
//  SMImageEditorFilterScene.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorFilterScene_h
#define SMImageEditorFilterScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/SpriteFilter/SpriteFilter.h"

class SMZoomView;
class SMTableView;
class SMImageView;

struct FilterInfo {
    FilterInfo() : index(-1), filterImagePath1(""), filterImagePath2(""), filterImagePath3(""), filterImagePath4(""), filterImagePath5(""), filterImagePath6("") {}
    
    int index;
    std::string filterName;
    std::string filterImagePath1;
    std::string filterImagePath2;
    std::string filterImagePath3;
    std::string filterImagePath4;
    std::string filterImagePath5;
    std::string filterImagePath6;
};

class SMImageEditorFilterScene : public SMScene, public OnClickListener
{
public:
    CREATE_SCENE(SMImageEditorFilterScene);
protected:
    SMImageEditorFilterScene();
    virtual ~SMImageEditorFilterScene();
    
    virtual void onClick(SMView * view) override;
    
protected:
    virtual bool init() override;
    
    class FilterCell : public SMView
    {
    public:
        FilterCell(){}
        ~FilterCell(){};
        CREATE_VIEW(FilterCell);
        
        SMView * contentView;
        SMImageView * filterImage;
    };
    
    cocos2d::Sprite * getFIlterSprite(kSpriteFilter type);
    
private:
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    
    SMImageView * _mainImageView;
    SMZoomView * _editZoomView;
    SMTableView * _filterListView;
    
    cocos2d::Sprite * _currentImageSprite;
    cocos2d::Sprite * _originImageSprite;
    cocos2d::Image * _mainccImage;
    cocos2d::Sprite * _capturedSprite;
    
    void applyFilterImage();
};

#endif /* SMImageEditorFilterScene_h */
