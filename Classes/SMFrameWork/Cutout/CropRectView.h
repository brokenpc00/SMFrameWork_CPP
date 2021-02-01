//
//  CropRectView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#ifndef CropRectView_h
#define CropRectView_h

#include "BaseCropView.h"

class SMButton;

class CropRectView : public BaseCropView, public OnTouchListener
{
public:
    static CropRectView* create(cocos2d::Sprite* sprite, const cocos2d::Rect& boundRect);
    
protected:
    virtual bool init(cocos2d::Sprite* sprite, const cocos2d::Rect& boundRect) override;
    
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    virtual cocos2d::Sprite* cropSpriteInternal(cocos2d::Rect& outRect) override;
    
    virtual bool cropDataInternal(uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect) override;
    
private:
    void updateCropBound(const cocos2d::Rect& newRect, bool force=false, int edgeIndex=-1);
    
    void showGrid(bool show, float duration, float delay);
    
private:
    class FocusView;
    cocos2d::Rect _cropRect;
    cocos2d::Vec2 _grabPt;
    FocusView* _focusView;
    SMButton* _cornerMover[4];
    SMView* _edgeMover[4];
};




#endif /* CropRectView_h */
