//
//  StickerControlView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef StickerControlView_h
#define StickerControlView_h

#include "../Base/SMView.h"
#include "../Base/SMButton.h"


class DashedRoundRect;

class StickerControlListener {
public:
    virtual void onStickerMenuClick(cocos2d::Node* sticker, int menuId) {}
};



class StickerControlView : public SMView, public OnTouchListener, public OnClickListener
{
public:
    static StickerControlView * create();
    
    void linkStickerNode(cocos2d::Node* node);
    
    void startGeineRemove(cocos2d::Node * node);
    
    bool canSwipe(const cocos2d::Vec2& worldPoint) const;
    
    void setStickerListener(StickerControlListener* l);
    
    void highlightSizeButton() {};
    
protected:
    StickerControlView();
    virtual ~StickerControlView();
    
    virtual void onClick(SMView* view) override;
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;

    virtual bool init() override;
    virtual void onUpdateOnVisit() override;
    virtual int dispatchTouchEvent(MotionEvent* event, SMView * view, bool checkBounds) override;
    
private:
    SMView * _uiView;
    DashedRoundRect * _borderSprite;
    SMButton * _sizeButton;
    SMButton * _utilButton; // for Trash
    
    cocos2d::Node * _targetNode;
    cocos2d::Size _targetSize;
    cocos2d::Vec2 _grabPt;
    cocos2d::Vec2 _deltaPt;
    bool _reset;
    
    StickerControlListener* _listener;
    int _utilButtonMode;
    cocos2d::Node * _sizeButtonIndicator;
    bool _highlightSizeButton;
};

#endif /* StickerControlView_h */
