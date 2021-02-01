//
//  SMImageEditorCropGuideView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 4. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorCropGuideView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Util/ViewUtil.h"

#define kCropGuideBorderLineWidth       4.0f
#define kCropGuideBorderLineColor       cocos2d::Color4F::WHITE
#define kCropGuideInnerLineWidth        2.0f

#define kCropGuideAreaColor             cocos2d::Color4F(0, 0, 0, 0.9f)

#define kCropGuideMinSize               50.0f

#define kKnobButtonSize 88.0f

SMImageEditorCropGuideView::SMImageEditorCropGuideView() :
_outLine(nullptr)
, _guideLine1(nullptr)
, _guideLine2(nullptr)
, _guideLine3(nullptr)
, _guideLine4(nullptr)
, _stencilView(nullptr)
, _cropView(nullptr)
{
    
}

SMImageEditorCropGuideView::~SMImageEditorCropGuideView()
{
    
}

SMImageEditorCropGuideView * SMImageEditorCropGuideView::create(int tag, float x, float y, float width, float height, cocos2d::Rect rect)
{
    SMImageEditorCropGuideView * view = new SMImageEditorCropGuideView();
    if (view != nullptr) {
        view->setAnchorPoint(cocos2d::Vec2::ZERO);
        view->setPosition(cocos2d::Vec2(x, y));
        view->setContentSize(cocos2d::Size(width, height));
        view->imageRect = rect;
        view->cropType = kCropMenuRect;
        if (view->init()) {
            view->autorelease();
        }
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

bool SMImageEditorCropGuideView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    setScissorEnable(false);
    // init crop rect
    cropRect.size.width = imageRect.size.width*0.95f;
    cropRect.size.height = imageRect.size.height*0.95f;
    cropRect.origin.x = imageRect.origin.x+imageRect.size.width/2-cropRect.size.width/2;
    cropRect.origin.y = imageRect.origin.y+imageRect.size.height/2-cropRect.size.height/2;

    
    _cropView = SMView::create(0, cropRect.origin.x, cropRect.origin.y, cropRect.size.width, cropRect.size.height);
    _cropView->setBackgroundColor4F(cocos2d::Color4F::BLACK);

    _stencilView = StencilView::create(0, imageRect.origin.x, imageRect.origin.y, imageRect.size.width, imageRect.size.height, _cropView);
    _stencilView->getCoverView()->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 0.7f));
    addChild(_stencilView);
    
    sizeWithPanningBeginPoint = cocos2d::Vec2::ZERO;
    
    leftTopButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, kKnobButtonSize, kKnobButtonSize);
    rightTopButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, kKnobButtonSize, kKnobButtonSize);
    leftBottomButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, kKnobButtonSize, kKnobButtonSize);
    rightBottomButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, kKnobButtonSize, kKnobButtonSize);
    leftTopButton->setLocalZOrder(90);
    rightTopButton->setLocalZOrder(90);
    leftBottomButton->setLocalZOrder(90);
    rightBottomButton->setLocalZOrder(90);
    
    leftTopButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xe94253, 1.0f));
    rightTopButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xe94253, 1.0f));
    leftBottomButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xe94253, 1.0f));
    rightBottomButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xe94253, 1.0f));
    
    leftTopButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x37c267, 1.0f));
    rightTopButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x37c267, 1.0f));
    leftBottomButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x37c267, 1.0f));
    rightBottomButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x37c267, 1.0f));
    
    leftTopButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x333333, 1.0f));
    rightTopButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x333333, 1.0f));
    leftBottomButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x333333, 1.0f));
    rightBottomButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x333333, 1.0f));
    
    leftTopButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x999999, 1.0f));
    rightTopButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x999999, 1.0f));
    leftBottomButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x999999, 1.0f));
    rightBottomButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x999999, 1.0f));
    
    leftTopButton->setOutlineWidth(2.0f);
    rightTopButton->setOutlineWidth(2.0f);
    leftBottomButton->setOutlineWidth(2.0f);
    rightBottomButton->setOutlineWidth(2.0f);
    
    addChild(leftTopButton);
    addChild(rightTopButton);
    addChild(leftBottomButton);
    addChild(rightBottomButton);
    
    adjustmentButtonPosition();
    
    lineAlpha = 0.5f;
    
    _outLine = cocos2d::DrawNode::create();
    addChild(_outLine);
    
    _guideLine1 = cocos2d::DrawNode::create();
    _guideLine2 = cocos2d::DrawNode::create();
    _guideLine3 = cocos2d::DrawNode::create();
    _guideLine4 = cocos2d::DrawNode::create();
    
    addChild(_guideLine1);
    addChild(_guideLine2);
    addChild(_guideLine3);
    addChild(_guideLine4);
    
    setCropType(kCropMenuRect);
    return true;
}

void SMImageEditorCropGuideView::adjustmentButtonPosition()
{
    leftBottomButton->setPosition(cocos2d::Vec2(cropRect.origin.x-kKnobButtonSize/2, cropRect.origin.y-kKnobButtonSize/2));
    rightBottomButton->setPosition(cocos2d::Vec2(cropRect.origin.x+cropRect.size.width-kKnobButtonSize/2, cropRect.origin.y-kKnobButtonSize/2));

    leftTopButton->setPosition(cocos2d::Vec2(cropRect.origin.x-kKnobButtonSize/2, cropRect.origin.y+cropRect.size.height-kKnobButtonSize/2));
    rightTopButton->setPosition(cocos2d::Vec2(cropRect.origin.x+cropRect.size.width-kKnobButtonSize/2, cropRect.origin.y+cropRect.size.height-kKnobButtonSize/2));
}

void SMImageEditorCropGuideView::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags)
{
    SMView::visit(renderer, parentTransform, parentFlags);
    
    _outLine->clear();
    
    _guideLine1->clear();
    _guideLine2->clear();
    _guideLine3->clear();
    _guideLine4->clear();
    
    _outLine->setLineWidth(kCropGuideBorderLineWidth);
    _outLine->drawRect(cocos2d::Vec2(cropRect.origin.x, cropRect.origin.y), cocos2d::Vec2(cropRect.origin.x+cropRect.size.width, cropRect.origin.y+cropRect.size.height), kCropGuideBorderLineColor);
    
    _guideLine1->setLineWidth(kCropGuideInnerLineWidth);
    _guideLine2->setLineWidth(kCropGuideInnerLineWidth);
    _guideLine3->setLineWidth(kCropGuideInnerLineWidth);
    _guideLine4->setLineWidth(kCropGuideInnerLineWidth);
    
    _guideLine1->drawLine(cocos2d::Vec2(cropRect.origin.x, cropRect.origin.y + cropRect.size.height/3), cocos2d::Vec2(cropRect.origin.x+cropRect.size.width, cropRect.origin.y + cropRect.size.height/3), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine2->drawLine(cocos2d::Vec2(cropRect.origin.x, cropRect.origin.y + cropRect.size.height/3*2), cocos2d::Vec2(cropRect.origin.x+cropRect.size.width, cropRect.origin.y + cropRect.size.height/3*2), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine3->drawLine(cocos2d::Vec2(cropRect.origin.x + cropRect.size.width/3, cropRect.origin.y), cocos2d::Vec2(cropRect.origin.x+cropRect.size.width/3, cropRect.origin.y+cropRect.size.height), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine4->drawLine(cocos2d::Vec2(cropRect.origin.x + cropRect.size.width/3*2, cropRect.origin.y), cocos2d::Vec2(cropRect.origin.x+cropRect.size.width/3*2, cropRect.origin.y+cropRect.size.height), MAKE_COLOR4F(0xffffff, lineAlpha));
}

void SMImageEditorCropGuideView::setCropType(kCropMenu type)
{
    cropType = type;

    if (type==kCropMenuFree) {
        return;
    }
    float widthRatio;
    float heightRatio;
    switch (cropType) {
        case kCropMenuOriginal:
        {
            cropRect = imageRect;
        }
            break;
        case kCropMenuSquare:
        {
            if (imageRect.size.width>imageRect.size.height) {
                float height = imageRect.size.height;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f-height/2.0f, imageRect.origin.y, height, height);
            } else {
                float width = imageRect.size.width;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-width/2.0f, width, width);
            }
        }
            break;
        case kCropMenu16_9:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 16.0f;
            heightRatio = 9.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu3_2:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 3.0f;
            heightRatio = 2.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu4_3:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 4.0f;
            heightRatio = 3.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu4_6:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 4.0f;
            heightRatio = 6.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu5_7:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 5.0f;
            heightRatio = 7.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu8_10:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 8.0f;
            heightRatio = 10.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        case kCropMenu14_8:
        {
            float ratio = imageRect.size.width / imageRect.size.height;
            widthRatio = 14.0f;
            heightRatio = 8.0f;
            if (ratio > widthRatio/heightRatio) {
                float height = imageRect.size.height;
                float width = height / heightRatio * widthRatio;
                cropRect = cocos2d::Rect(getContentSize().width/2.0f - width/2.0f, imageRect.origin.y, width, height);
            } else {
                float width = imageRect.size.width;
                float height = width / widthRatio * heightRatio;
                cropRect = cocos2d::Rect(imageRect.origin.x, getContentSize().height/2.0f-height/2.0f, width, height);
            }
        }
            break;
        default:
            break;
    }
    
    cocos2d::Vec2 cropPoint = cropRect.origin - imageRect.origin;
    _cropView->setPosition(cropPoint);
    _cropView->setContentSize(cropRect.size);
    _cropView->scheduleUpdate();
    
    adjustmentButtonPosition();
    scheduleUpdate();
}

void SMImageEditorCropGuideView::startFadeIn(float dt)
{
    lineAlpha += 0.15f;
    scheduleUpdate();
    
    if (0.5f <= lineAlpha) {
        lineAlpha = 0.5f;
        unschedule(schedule_selector(SMImageEditorCropGuideView::startFadeIn));
    }
}

void SMImageEditorCropGuideView::startFadeOut(float dt)
{
    lineAlpha -= 0.03f;
    scheduleUpdate();
    
    if (0.0f >= lineAlpha) {
        lineAlpha = 0.0f;
        unschedule(schedule_selector(SMImageEditorCropGuideView::startFadeOut));
    }
}

void SMImageEditorCropGuideView::innerLineFadeIn()
{
    if (isScheduled(schedule_selector(SMImageEditorCropGuideView::startFadeIn))) {
        unschedule(schedule_selector(SMImageEditorCropGuideView::startFadeIn));
    }
    if (isScheduled(schedule_selector(SMImageEditorCropGuideView::startFadeOut))) {
        unschedule(schedule_selector(SMImageEditorCropGuideView::startFadeOut));
    }
    
    schedule(schedule_selector(SMImageEditorCropGuideView::startFadeIn), 0.1f);
}

void SMImageEditorCropGuideView::innerLineFadeOut()
{
    if (isScheduled(schedule_selector(SMImageEditorCropGuideView::startFadeOut))) {
        unschedule(schedule_selector(SMImageEditorCropGuideView::startFadeOut));
    }
    
    schedule(schedule_selector(SMImageEditorCropGuideView::startFadeOut), 0.1f);
}

int SMImageEditorCropGuideView::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event)
{
    SMView::dispatchTouchEvent(action, touch, point, event);
    
    float x = point->x;
    float y = point->y;
    
    if (getMotionTarget()==leftTopButton || getMotionTarget()==rightTopButton || getMotionTarget()==leftBottomButton || getMotionTarget()==rightBottomButton) {
        // resize
        switch (action) {
            case MotionEvent::ACTION_DOWN:
            {
                innerLineFadeIn();
            }
                break;
            case MotionEvent::ACTION_UP:
            case MotionEvent::ACTION_CANCEL:
            {
                innerLineFadeOut();
            }
                break;
            case MotionEvent::ACTION_MOVE:
            {
                float oldLeft = cropRect.origin.x;
                float oldBottom = cropRect.origin.y;
                float oldRight = cropRect.origin.x+cropRect.size.width;
                float oldTop = cropRect.origin.y+cropRect.size.height;
                
                float newLeft = 0.0f;
                float newTop = 0.0f;
                float newRight = 0.0f;
                float newBottom = 0.0f;
                
                if (getMotionTarget()==leftTopButton) {
                    // 왼쪽 상단 버튼이 움직일때는 우측 하단이 고정
                    newLeft = MIN(MAX(x, imageRect.origin.x), oldRight - kCropGuideMinSize);
                    newTop = MAX(MIN(y, imageRect.origin.y+imageRect.size.height), oldBottom + kCropGuideMinSize);
                    newRight = oldRight;
                    newBottom = oldBottom;
                } else if (getMotionTarget()==rightTopButton) {
                    // 우측 상단 버튼이 움직일때는 좌측 하단이 고정
                    newLeft = oldLeft;
                    newTop = MAX(MIN(y, imageRect.origin.y+imageRect.size.height), oldBottom + kCropGuideMinSize);
                    newRight = MAX(MIN(x, imageRect.origin.x+imageRect.size.width), oldLeft + kCropGuideMinSize);
                    newBottom = oldBottom;
                } else if (getMotionTarget()==leftBottomButton) {
                    // 좌측 하단 버튼이 움직일때는 우측 상단이 고정
                    newLeft = MIN(MAX(x, imageRect.origin.x), oldRight - kCropGuideMinSize);
                    newTop = oldTop;
                    newRight = oldRight;
                    newBottom = MIN(MAX(y, imageRect.origin.y), oldTop-kCropGuideMinSize);
                } else if (getMotionTarget()==rightBottomButton) {
                    // 우측 하단 버튼이 움직일때는 좌측 상단이 고정
                    newLeft = oldLeft;
                    newTop = oldTop;
                    newRight = MAX(MIN(x, imageRect.origin.x + imageRect.size.width), oldLeft + kCropGuideMinSize);
                    newBottom = MIN(MAX(y, imageRect.origin.y), oldTop-kCropGuideMinSize);
                } else {
                    CCLOG("[[[[[ WTF????");
                }

                if (cropType!=kCropMenuRect) {
                    float ratio = 1.0f;
                    
                    switch (cropType) {
                        case kCropMenuOriginal:
                        {
                            ratio = imageRect.size.height / imageRect.size.width;
                        }
                            break;
                        case kCropMenu16_9:
                        {
                            ratio = 9.0f/16.0f;
                        }
                            break;
                        case kCropMenu3_2:
                        {
                            ratio = 2.0f/3.0f;
                        }
                            break;
                        case kCropMenu4_3:
                        {
                            ratio = 3.0f/4.0f;
                        }
                            break;
                        case kCropMenu4_6:
                        {
                            ratio = 6.0f/4.0f;
                        }
                            break;
                        case kCropMenu5_7:
                        {
                            ratio = 7.0f/5.0f;
                        }
                            break;
                        case kCropMenu8_10:
                        {
                            ratio = 10.0f/8.0f;
                        }
                            break;
                        case kCropMenu14_8: // --> sns profile cover... Do not use.
                        {
                            ratio = 8.0f/14.0f;
                        }
                            break;
                            
                        default:
                            break;
                    }
                    
                    float width = newRight - newLeft;
                    float newWidth = width;
                    float newHeight = width*ratio;
                    if (getMotionTarget()==leftTopButton) {
                        if (newBottom+newHeight > imageRect.origin.y + imageRect.size.height) {
                            newHeight = imageRect.origin.y + imageRect.size.height - newBottom;
                            newWidth = newHeight / ratio;
                        }
                        newLeft = newRight-newWidth;
                        newTop = newBottom + newHeight;
                    } else if (getMotionTarget()==rightTopButton) {
                        if (newBottom+newHeight > imageRect.origin.y + imageRect.size.height) {
                            newHeight = imageRect.origin.y + imageRect.size.height - newBottom;
                            newWidth = newHeight / ratio;
                        }
                        newRight = newLeft+newWidth;
                        newTop = newBottom + newHeight;
                    } else if (getMotionTarget()==leftBottomButton) {
                        if (newTop-newHeight < imageRect.origin.y) {
                            newHeight = newTop - imageRect.origin.y;
                            newWidth = newHeight / ratio;
                        }
                        newLeft = newRight-newWidth;
                        newBottom = newTop - newHeight;
                    } else if (getMotionTarget()==rightBottomButton) {
                        if (newTop-newHeight < imageRect.origin.y) {
                            newHeight = newTop - imageRect.origin.y;
                            newWidth = newHeight / ratio;
                        }
                        newRight = newLeft+newWidth;
                        newBottom = newTop - newHeight;
                    } else {
                        CCLOG("[[[[[ WTF????");
                    }
                }
                
                cropRect = cocos2d::Rect(newLeft, newBottom, newRight-newLeft, newTop-newBottom);
                
                cocos2d::Vec2 cropPoint = cropRect.origin - imageRect.origin;
                _cropView->setPosition(cropPoint);
                _cropView->setContentSize(cropRect.size);
                _cropView->scheduleUpdate();
                
                adjustmentButtonPosition();
                scheduleUpdate();
            }
                break;
        }
    } else {
        // panning
//        CCLOG("[[[[[ Guide View Panning");
        switch (action) {
            case MotionEvent::ACTION_DOWN:
            {
                sizeWithPanningBeginPoint.width = x - cropRect.origin.x;
                sizeWithPanningBeginPoint.height = y - cropRect.origin.y;
                
                innerLineFadeIn();
            }
                break;
            case MotionEvent::ACTION_UP:
            case MotionEvent::ACTION_CANCEL:
            {
                sizeWithPanningBeginPoint = cocos2d::Size::ZERO;
                innerLineFadeOut();
            }
                break;
            case MotionEvent::ACTION_MOVE:
            {
                if (sizeWithPanningBeginPoint.width != 0 && sizeWithPanningBeginPoint.height != 0) {
                    float changeX = x - sizeWithPanningBeginPoint.width;
                    float changeY = y - sizeWithPanningBeginPoint.height;
                    
                    if (changeX <= imageRect.origin.x) {
                        changeX = imageRect.origin.x;
                    }
                    if (changeY <= imageRect.origin.y) {
                        changeY = imageRect.origin.y;
                    }
                    if (changeX + cropRect.size.width >= imageRect.origin.x + imageRect.size.width) {
                        changeX = imageRect.size.width - cropRect.size.width + imageRect.origin.x;
                    }
                    if (changeY + cropRect.size.height >= imageRect.origin.y + imageRect.size.height) {
                        changeY = imageRect.size.height - cropRect.size.height + imageRect.origin.y;
                    }
                    
                    cropRect.origin.x = changeX;
                    cropRect.origin.y = changeY;
                    
                    cocos2d::Vec2 cropPoint = cropRect.origin - imageRect.origin;
                    _cropView->setPosition(cropPoint);
                    _cropView->setContentSize(cropRect.size);
                    _cropView->scheduleUpdate();
                    
                    adjustmentButtonPosition();
                    scheduleUpdate();
                }
            }
                break;
        }
    }
    
    return TOUCH_TRUE;
}
