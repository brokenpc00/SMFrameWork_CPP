//
//  SMImageEditorStickerScene.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStickerScene_h
#define SMImageEditorStickerScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/Base/SMSlider.h"
#include "../../SMFrameWork/Base/SMZoomView.h"
#include "SMImageEditorStickerItemView.h"
#include "SMImageEditorStickerBoardView.h"
#include "../../SMFrameWork/UI/StickerCanvasView.h"
#include "../../SMFrameWork/UI/StickerControlView.h"
#include "SMImageEditorStickerListView.h"
#include "SMImageEditorItemThumbView.h"

class SMView;
class SMImageView;
class SMButton;

class SMImageEditorStickerScene : public SMScene, public StickerLayoutListener, public StickerCanvasListener, public StickerControlListener, public DownloadProtocol, public ImageProcessProtocol, public OnItemClickListener
{
public:
    SMImageEditorStickerScene();
    virtual ~SMImageEditorStickerScene();
    CREATE_SCENE(SMImageEditorStickerScene);
    
    
    virtual void onStickerTouch(cocos2d::Node* node, int action) override;
    
    virtual void onStickerSelected(cocos2d::Node* node, const bool selected) override;
    
    virtual void onStickerDoubleClicked(cocos2d::Node* node, const cocos2d::Vec2& worldPoint) override;
    
    virtual void onStickerRemoveBegin(cocos2d::Node * node) override;

    virtual void onStickerRemoveEnd(cocos2d::Node * node) override;
    
    virtual void onItemClick(ItemListView* sender, SMImageEditorItemThumbView * view) override;
    virtual void onStickerLayout(SMImageEditorStickerItemView* sticker, cocos2d::Sprite* sprite, const StickerItem& item, const int colorIndex) override;
    virtual void onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite* sprite, Intent* result) override;
    virtual void onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct) override;
    virtual void onStickerMenuClick(cocos2d::Node* sticker, int menuId) override;
    
    SMImageEditorStickerItemView * runSelectSticker(int index, bool fromTemplate=false, int colorIndex=0, int code=-1);
    
protected:
    virtual bool init() override;
    
    SMView * _contentView;
    SMView * _topMenuView;
    SMView * _bottomMenuView;
    //SMZoomView * _editZoomView; // StickerItemView로 대체
    SMImageEditorStickerBoardView * _stickerBoardView;
    
//    SMImageView * _mainImageView;
    cocos2d::Sprite * _currentImageSprite;
    cocos2d::Image * _mainccImage;
    cocos2d::Sprite* _capturedSprite;
    
    SMImageEditorStickerItemListView * _stickerListView;
    
    void applyStickerImage();
};


#endif /* SMImageEditorStickerScene_h */
