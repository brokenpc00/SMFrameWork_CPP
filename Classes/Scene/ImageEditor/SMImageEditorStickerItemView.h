//
//  SMImageEditorStickerItemView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 22..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStickerItemView_h
#define SMImageEditorStickerItemView_h

#include "../../SMFrameWork/Base/Sticker.h"
#include "../../SMFrameWork/ImageProcess/ImageProcessor.h"
#include "../../SMFrameWork/Base/Intent.h"
#include "ItemInfo.h"
#include <cocos2d.h>



class SMImageEditorStickerItemView;

class StickerLayoutListener {
public:
    virtual void onStickerLayout(SMImageEditorStickerItemView * sticker, cocos2d::Sprite* sprite, const StickerItem& item, const int colorIndex) {}
};

class SMImageEditorStickerItemView : public Sticker, public ImageProcessProtocol
{
public:
    static SMImageEditorStickerItemView* createWithItem(const StickerItem& item, StickerLayoutListener* l);
    
    bool isBlack() { return _isBlack;}
    
    void setBlack();
    
    void setWhite();
    
    void setAlphaValue(float alpha);
    float getAlphaValue();
    
    virtual void onImageLoadComplete(cocos2d::Sprite * sprite, int tag, bool direct) override;
    virtual void onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite * sprite, Intent* result) override;
    
    void clearLayout();
    void setLayout(cocos2d::Node * node);
    bool isColorSelectable() { return _colorSelectable;}
    void setColorSelectable(bool selectable) { _colorSelectable = selectable;}
    void prepareRemove();
    
protected:
    SMImageEditorStickerItemView();
    virtual ~SMImageEditorStickerItemView();
    
    bool initWithStickerItem(const StickerItem& item, StickerLayoutListener* l);
    
private:
    bool _isBlack;
    bool _colorSelectable;
    float _alpha;
    StickerItem _item;
    StickerLayoutListener* _listener;
    cocos2d::Node * _layout;
};


#endif /* SMImageEditorStickerItemView_h */
