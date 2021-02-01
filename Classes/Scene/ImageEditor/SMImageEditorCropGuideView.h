//
//  SMImageEditorCropGuideView.h
//  iPet
//
//  Created by KimSteve on 2017. 4. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorCropGuideView_h
#define SMImageEditorCropGuideView_h

#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/UI/StencilView.h"

typedef enum {
    kCropMenuRect = 0,
    kCropMenuOriginal,
    kCropMenuSquare,
    kCropMenu16_9,
    kCropMenu3_2,
    kCropMenu4_3,
    kCropMenu4_6,
    kCropMenu5_7,
    kCropMenu8_10,
    kCropMenu14_8,
    kCropMenuFree,
    kCropMenuCount
} kCropMenu;

class SMButton;

class SMImageEditorCropGuideView : public SMView
{
public:
    SMImageEditorCropGuideView();
    virtual ~SMImageEditorCropGuideView();
    
    static SMImageEditorCropGuideView * create(int tag, float x, float y, float width, float height, cocos2d::Rect rect);
    
    cocos2d::Rect imageRect;
    cocos2d::Rect cropRect;
    kCropMenu cropType;

    void startFadeOut(float dt);
    void startFadeIn(float dt);
    
    void setCropType(kCropMenu type);
    
protected:
    SMButton * leftTopButton;
    SMButton * rightTopButton;
    SMButton * leftBottomButton;
    SMButton * rightBottomButton;
    cocos2d::Size sizeWithPanningBeginPoint;
    
    float lineAlpha;
    
    cocos2d::DrawNode * _outLine;
    
    cocos2d::DrawNode * _guideLine1;
    cocos2d::DrawNode * _guideLine2;
    cocos2d::DrawNode * _guideLine3;
    cocos2d::DrawNode * _guideLine4;
    
    StencilView * _stencilView;
    SMView * _cropView;
        
    void adjustmentButtonPosition();
    
private:
    virtual bool init() override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
    void innerLineFadeIn();
    void innerLineFadeOut();
};

#endif /* SMImageEditorCropGuideView_h */
