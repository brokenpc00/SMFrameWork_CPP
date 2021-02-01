//
//  SMImageEditorStickerBoardView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStickerBoardView_h
#define SMImageEditorStickerBoardView_h

#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/UI/StickerCanvasView.h"
#include "../../SMFrameWork/UI/StickerControlView.h"
#include "../../SMFrameWork/Base/MeshSprite.h"

class SMZoomView;
class SMImageView;

class SMImageEditorStickerBoardView : public SMView, public StickerCanvasListener, public StickerControlListener
{
public:
    static SMImageEditorStickerBoardView * create();
    static SMImageEditorStickerBoardView * createWithSprite(cocos2d::Sprite* sprite);
    void setStickerListener(StickerCanvasListener * canvasListener, StickerControlListener * controlListener);
    
    // main method
    void startGeineRemove(cocos2d::Node* node);

    void addSticker(cocos2d::Node* sticker);
    void addStickerAboveAt(cocos2d::Node* sticker, cocos2d::Node* aboveAt);
    
    void reorderStickerAboveAt(cocos2d::Node* sticker, cocos2d::Node* aboveAt);
    
    void removeSticker(cocos2d::Node* sticker);
    void removeStickerWithFadeOut(cocos2d::Node* sticker, const float duration, const float delay);
    void removeAllStickerWithFly();
    void removeAllSticker();
    
    // sticker listenr
    virtual void onStickerMenuClick(cocos2d::Node* sticker, int menuId) override;
    virtual void onStickerTouch(cocos2d::Node * node, int action) override;
    virtual void onStickerSelected(cocos2d::Node * node, const bool selected) override;
    virtual void onStickerDoubleClicked(cocos2d::Node * node, const cocos2d::Vec2& worldPoint) override;
    virtual void onStickerRemoveBegin(cocos2d::Node * node) override;
    virtual void onStickerRemoveEnd(cocos2d::Node * node) override;
    
    SMImageView * getBgImageView() { return _bgImageView;}
    StickerCanvasView * getCanvas() { return _canvasView;}
    StickerControlView * getControl() { return _controlView;}
    SMZoomView * getZoomView() { return _zoomView;}
    SMView * getContentView() { return _contentView;}
    
    void setZoomStatus(const float panX, const float panY, const float zoomScale, const float duration);
    void cancelTouch() {cancel();}
    
    virtual bool containsPoint(const cocos2d::Vec2& point) override;
    virtual int dispatchTouchEvent(MotionEvent * event, SMView * view, bool checkBounds) override;
    
protected:
    SMImageEditorStickerBoardView();
    virtual ~SMImageEditorStickerBoardView();
    virtual bool init() override;
    
    virtual bool initWithSprite(cocos2d::Sprite * sprite);
        
private:
    SMView * _contentView;
    SMZoomView * _zoomView;
    
    StickerCanvasView * _canvasView;
    StickerControlView * _controlView;
    SMImageView * _bgImageView;

    StickerCanvasListener * _canvasListener;
    StickerControlListener * _controlListener;
    
    MeshSprite * _meshSprite;
};


#endif /* SMImageEditorStickerBoardView_h */
