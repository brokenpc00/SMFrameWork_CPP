//
//  StencilView.h
//  iPet
//
//  Created by KimSteve on 2017. 4. 24..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef StencilView_h
#define StencilView_h

#include "../Base/SMView.h"
#include <cocos2d.h>

class StencilView : public SMView
{
public:
    StencilView();
    virtual ~StencilView();
    static StencilView * create(int tag, float x, float y, float width, float height, cocos2d::Node * stencilNode = nullptr, float anchorX = 0.0f, float anchorY = 0.0f);
    
    CREATE_VIEW(StencilView);
    
    bool isInverted() const {
        return _isInverted;
    }
    
    void setInverted(const bool isInverted);
    
    
    cocos2d::Node * getStencilNode() const {
        return _stencilNode;
    }

    void setStencilNode(cocos2d::Node * node);
    
    
    SMView * getCoverView() const {
        return _coverView;
    }
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override;
    
protected:
    virtual bool init() override;
    
    cocos2d::ClippingNode * _contentNode;
    cocos2d::Node * _stencilNode;
    bool _isInverted;
    SMView * _coverView;
};


#endif /* StencilView_h */
