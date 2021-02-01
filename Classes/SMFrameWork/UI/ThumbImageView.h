//
//  ThumbImageView.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef ThumbImageView_h
#define ThumbImageView_h
#include "../Base/SMImageView.h"
#include "../Util/ImageItemInfo.h"
#include "../Util/ImageDownloader.h"
#include "../Util/DownloadProtocol.h"
#include "../Base/ViewAction.h"
#include <string>

class ShapeSolidRect;

class ThumbImageView : public SMImageView
{
public:
    static ThumbImageView* create();
    
    void setImageItemInfo(const ImageItemInfo& info);
    
    ImageItemInfo getImageItemInfo() {return _item;};
  
    virtual void onEnter() override;
    
    virtual void onExit() override;
    
    virtual void onImageLoadComplete(cocos2d::Sprite * sprite, int tag, bool direct) override;
    
protected:
    virtual bool init() override;
    
    virtual void onStateChangeNormalToPress() override;
    
    virtual void onStateChangePressToNormal() override;
    
    ThumbImageView();
    virtual ~ThumbImageView();
    
private:
    ImageItemInfo _item;
    class ShowAction;
    ShapeSolidRect * _dimLayer;
    std::string _imgPath;
    ShowAction* _showAction;
    ViewAction::AlphaTo* _dimAction;
};


#endif /* ThumbImageView_h */
