//
//  StickerCanvasView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef StickerCanvasView_h
#define StickerCanvasView_h

#include "../Base/SMImageView.h"
#include "MultiTouchController.h"
#include <cocos/physics/CCPhysicsWorld.h>
#include <cocos2d.h>

class VelocityTracker;

class StickerCanvasListener
{
public:
    virtual void onStickerSelected(cocos2d::Node * node, const bool selected) {}
    virtual void onStickerRemoveBegin(cocos2d::Node * node) {}
    virtual void onStickerRemoveEnd(cocos2d::Node * node) {}
    virtual void onStickerDoubleClicked(cocos2d::Node * node, const cocos2d::Vec2& worldPoint) {}
    virtual void onStickerTouch(cocos2d::Node * node, int action) {}
};

class StickerCanvasView : public SMView, public MultiTouch::ObjectCanvas
{
public:
    static StickerCanvasView * create();
    static StickerCanvasView * createWithPhysicsWorld(cocos2d::PhysicsWorld * physicsWorld);

    virtual bool setSelectedSticker(cocos2d::Node * node);
    cocos2d::Node * getSelectedSticker();
    
    void setRemoveAfterFlying(const bool bEnable);
    
    void setChildPosition(cocos2d::Node * node, const int position);
    
    void bringChildToTop(cocos2d::Node * node);
    void sendChildToBack(cocos2d::Node * node);
    
    void aboveNode(cocos2d::Node * node, cocos2d::Node * aboveNode); // aboveNode 바로 아래로 이동
    void belowNode(cocos2d::Node * node, cocos2d::Node * belowNode);   // belowNode 바로 위로 이동
    
    virtual bool containsPoint(const cocos2d::Vec2& point) override;
    
    virtual int dispatchTouchEvent(MotionEvent * event, SMView * view, bool checkBounds) override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch * touch, const cocos2d::Vec2 * point, MotionEvent * event) override;
    
    virtual void addChild(cocos2d::Node* child, int localZOrder = 0, const std::string &name = "") override;
    virtual void removeChild(cocos2d::Node* child, bool cleanup = true) override;
    
    void removeChildWithGenieAction(cocos2d::Node* child, cocos2d::Sprite* sprite, const cocos2d::Vec2& removeAnchor, float duration = 0.7, float delay = 0.15);    // 쓰레기 통에 넣기
    void removeChildWithFadeOut(Node* child, float duration, float delay);  // 그냥 사라지기
    void removeChildWithFly(Node* child, const float degrees, const float speed); // 날아가기
    
    
    void setStickerListener(StickerCanvasListener * l) {_listener = l;};
    
protected:
    virtual cocos2d::Node * getDraggableObjectAtPoint(MultiTouch::PointInfo & touchPoint) override;
    virtual bool pointInObjectGrabArea(MultiTouch::PointInfo & touchPoint, cocos2d::Node * node) override;
    virtual void getPositionAndScale(cocos2d::Node * node, MultiTouch::PositionAndScale& objPosAndScaleOut) override;
    virtual bool setPositionAndScale(cocos2d::Node * node, MultiTouch::PositionAndScale& newObjPosAndScale, MultiTouch::PointInfo& touchPoint) override;
    virtual void selectObject(cocos2d::Node* node, MultiTouch::PointInfo& touchPoint) override;
    virtual void doubleClickObject(cocos2d::Node * node, MultiTouch::PointInfo& touchPoint) override;
    virtual void touchModeChanged(const int touchMode, MultiTouch::PointInfo& touchPoint) override;
    virtual cocos2d::Vec2 toWorldPoint(const cocos2d::Vec2& canvasPoint) override;
    virtual cocos2d::Vec2 toCanvasPoint(const cocos2d::Vec2& worldPoint) override;
    virtual void performSelected(cocos2d::Node* node, bool selected);
    virtual void performFly(cocos2d::Node* node, const float degrees, const float speed);
    virtual void cancel() override;
    
protected:
    StickerCanvasView();
    virtual ~StickerCanvasView();
    
    virtual bool init() override;
    
    
private:
    MultiTouch::Controller* _controller;
    cocos2d::Node* _selectedNode;
    cocos2d::PhysicsWorld* _physicsWorld;
    VelocityTracker * _velocityTracker;
    
    bool _trackFlyEvent;
    bool _flyRemovable;
    
    MultiTouch::PointInfo _lastTouchPoint;
    StickerCanvasListener * _listener;
    
};


#endif /* StickerCanvasView_h */
