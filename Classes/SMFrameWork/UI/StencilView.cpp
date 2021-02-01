//
//  StencilView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 4. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "StencilView.h"

StencilView * StencilView::create(int tag, float x, float y, float width, float height, cocos2d::Node *stencilNode, float anchorX, float anchorY)
{
    StencilView * view = new StencilView();
    
    if (view) {
        view->setPosition(cocos2d::Vec2(x, y));
        view->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
        view->setContentSize(cocos2d::Size(width, height));
        view->_stencilNode = stencilNode;
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

StencilView::StencilView() : _contentNode(nullptr), _stencilNode(nullptr), _isInverted(true), _coverView(nullptr)
{
    
}

StencilView::~StencilView()
{
    
}

bool StencilView::init()
{
    if (!SMView::init()) {
        return false;
    }

    cocos2d::Size s = getContentSize();
    
    _contentNode = cocos2d::ClippingNode::create();
    _contentNode->setAnchorPoint(cocos2d::Vec2::ZERO);
    _contentNode->setPosition(cocos2d::Vec2::ZERO);
    _contentNode->setContentSize(s);
    addChild(_contentNode);
    
    CCASSERT(_stencilNode!=nullptr, "stencilNode is must be not NULL");
    _contentNode->setStencil(_stencilNode);

    /*
    // for test
    _stencilNode->setVisible(false);
    _contentNode->addChild(_stencilNode);
    
    auto drawNode = cocos2d::DrawNode::create();
    drawNode->drawSolidCircle(cocos2d::Vec2(s.width / 2, s.height / 2), s.width/4, 0, 200, cocos2d::Color4F::WHITE);
    _contentNode->setStencil(drawNode);
*/
    
    _contentNode->setInverted(_isInverted);
    
    _coverView = SMView::create(0, 0, 0, s.width, s.height);
    _contentNode->addChild(_coverView);
    
    return true;
}

void StencilView::setInverted(const bool isInverted)
{
    _isInverted = isInverted;
}

void StencilView::setStencilNode(cocos2d::Node *node)
{
    _stencilNode = node;
    _contentNode->setStencil(_stencilNode);
}

void StencilView::setContentSize(const cocos2d::Size& contentSize)
{
    SMView::setContentSize(contentSize);
    if (_contentNode) {
        _contentNode->setContentSize(contentSize);
    }
    if (_coverView) {
        _coverView->setContentSize(contentSize);
    }
}
