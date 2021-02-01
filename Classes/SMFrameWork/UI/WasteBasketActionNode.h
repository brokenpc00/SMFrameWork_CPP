//
//  WasteBasketActionNode.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef WasteBasketActionNode_h
#define WasteBasketActionNode_h

#include "../Base/SMView.h"
#include "../Base/ShaderNode.h"


class WasteBasketActionNode : public cocos2d::Node
{
public:
    static WasteBasketActionNode * show(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    static WasteBasketActionNode * showForList(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    static WasteBasketActionNode * showForUtil(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    
    virtual void onExit() override;
protected:
    WasteBasketActionNode();
    virtual ~WasteBasketActionNode();
    
    virtual bool initWithParam(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    virtual bool initWithParam2(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    virtual bool initWithParam3(cocos2d::Node * parent, const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    
private:
    bool _removeSelfOnExit;
    cocos2d::Vec2 _from;
    cocos2d::Vec2 _to;
};


#endif /* WasteBasketActionNode_h */
