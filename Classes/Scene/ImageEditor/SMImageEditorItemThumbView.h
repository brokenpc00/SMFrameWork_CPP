//
//  SMImageEditorItemThumbView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorItemThumbView_h
#define SMImageEditorItemThumbView_h

#include "../../SMFrameWork/Base/SMImageView.h"
#include <functional>

class LoadingSprite;
class ShapeRect;

class SMImageEditorItemThumbView : public SMView, public DownloadProtocol
{
public:
    static SMImageEditorItemThumbView * create(const DownloadConfig* thumbDlConfig, const DownloadConfig* imageDlConfig);
    void setSelect(bool select, bool immediate);
    
    void setFocus();
    
    bool isSelected() {return _selected;}
    
    virtual void onEnter() override;
    virtual void cleanup() override;
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    virtual void startShowAction();
    virtual void onImageLoadComplete(cocos2d::Sprite * sprite, int tag, bool direct) override;
    void setImagePath(const std::string& imagePath);
    
    SMImageView * getImageView() {return _imageView;}
    
protected:
    SMImageEditorItemThumbView();
    virtual ~SMImageEditorItemThumbView();
    
    virtual bool init() override;
    virtual void onStateChangePressToNormal() override;
    virtual void onStateChangeNormalToPress() override;
    virtual void performClick(const cocos2d::Vec2& worldPoint) override;
    
    // action
protected:
    // 나타날때 흔들 흔들
    class ShakeAction;
    // 선택할때
    class SelectAction;
    // 포커싱
    class FocusAction;
    
    ShakeAction * _shakeAction;
    SelectAction * _selectAction;
    bool _selected;

    ShapeRect * _selectBox;
    SMImageView * _imageView;
    LoadingSprite * _spinner;
    
    DownloadConfig * _thumbDlConfig;
    DownloadConfig * _imageDlConfig;
    std::string _imagePath;
    
    friend class SMImageEditorStickerListView;
    
};




#endif /* SMImageEditorItemThumbView_h */
