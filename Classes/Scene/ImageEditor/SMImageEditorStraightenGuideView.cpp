//
//  SMImageEditorStraightenGuideView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStraightenGuideView.h"
#include "../../SMFrameWork/Util/ViewUtil.h"

#define kStraightenGuideBorderLineWidth       4.0f
#define kStraightenGuideBorderLineColor       cocos2d::Color4F::WHITE
#define kStraightenGuideInnerLineWidth        2.0f


SMImageEditorStraightenGuideView::SMImageEditorStraightenGuideView()
{
    
}

SMImageEditorStraightenGuideView::~SMImageEditorStraightenGuideView()
{
    
}

SMImageEditorStraightenGuideView * SMImageEditorStraightenGuideView::create(int tag, float x, float y, float width, float height, cocos2d::Rect rect)
{
    SMImageEditorStraightenGuideView * view = new SMImageEditorStraightenGuideView();
    if (view) {
        view->setContentSize(cocos2d::Size(width, height));
        view->setAnchorPoint(cocos2d::Vec2::ZERO);
        view->setPosition(cocos2d::Vec2(x, y));
        view->imageRect = rect;
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_RELEASE(view);
        }
    }
    
    return view;
}


bool SMImageEditorStraightenGuideView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    // init crop rect
//    straightenRect.size.width = imageRect.size.width-kStraightenGuideBorderLineWidth*2;
//    straightenRect.size.height = imageRect.size.height-kStraightenGuideBorderLineWidth*2;
    straightenRect.size.width = imageRect.size.width;
    straightenRect.size.height = imageRect.size.height;
    straightenRect.origin.x = imageRect.origin.x+imageRect.size.width/2-straightenRect.size.width/2;
    straightenRect.origin.y = imageRect.origin.y+imageRect.size.height/2-straightenRect.size.height/2;
    
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
   
    return true;
}

void SMImageEditorStraightenGuideView::visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags)
{
    SMView::visit(renderer, parentTransform, parentFlags);
    
    _outLine->clear();
    
    _guideLine1->clear();
    _guideLine2->clear();
    _guideLine3->clear();
    _guideLine4->clear();
    
    _outLine->setLineWidth(kStraightenGuideBorderLineWidth);
    _outLine->drawRect(cocos2d::Vec2(straightenRect.origin.x, straightenRect.origin.y), cocos2d::Vec2(straightenRect.origin.x+straightenRect.size.width, straightenRect.origin.y+straightenRect.size.height), kStraightenGuideBorderLineColor);
    
    _guideLine1->setLineWidth(kStraightenGuideInnerLineWidth);
    _guideLine2->setLineWidth(kStraightenGuideInnerLineWidth);
    _guideLine3->setLineWidth(kStraightenGuideInnerLineWidth);
    _guideLine4->setLineWidth(kStraightenGuideInnerLineWidth);
    
    _guideLine1->drawLine(cocos2d::Vec2(straightenRect.origin.x, straightenRect.origin.y + straightenRect.size.height/3), cocos2d::Vec2(straightenRect.origin.x+straightenRect.size.width, straightenRect.origin.y + straightenRect.size.height/3), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine2->drawLine(cocos2d::Vec2(straightenRect.origin.x, straightenRect.origin.y + straightenRect.size.height/3*2), cocos2d::Vec2(straightenRect.origin.x+straightenRect.size.width, straightenRect.origin.y + straightenRect.size.height/3*2), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine3->drawLine(cocos2d::Vec2(straightenRect.origin.x + straightenRect.size.width/3, straightenRect.origin.y), cocos2d::Vec2(straightenRect.origin.x+straightenRect.size.width/3, straightenRect.origin.y+straightenRect.size.height), MAKE_COLOR4F(0xffffff, lineAlpha));
    _guideLine4->drawLine(cocos2d::Vec2(straightenRect.origin.x + straightenRect.size.width/3*2, straightenRect.origin.y), cocos2d::Vec2(straightenRect.origin.x+straightenRect.size.width/3*2, straightenRect.origin.y+straightenRect.size.height), MAKE_COLOR4F(0xffffff, lineAlpha));
}
