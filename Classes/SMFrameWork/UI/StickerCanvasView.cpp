//
//  StickerCanvasView.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "StickerCanvasView.h"
#include "../Base/Sticker.h"
#include "MultiTouchController.h"
#include "../Interface/VelocityTracker.h"
#include "../Util/ViewUtil.h"
#include "../Base/ViewAction.h"
#include "../Base/ShaderUtil.h"
#include <math.h>



#define MIN_FLY_TOLERANCE (6000)
#define FLY_DURATION (0.3f)


StickerCanvasView * StickerCanvasView::create()
{
    auto view = new StickerCanvasView;
    if (view && view->init()) {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

StickerCanvasView * StickerCanvasView::createWithPhysicsWorld(cocos2d::PhysicsWorld *physicsWorld)
{
    CCASSERT(physicsWorld!=nullptr, "Physics World must be not nullptr");
    auto view = create();
    
    if (view) {
        view->_physicsWorld = physicsWorld;
    }
    
    return view;
}


StickerCanvasView::StickerCanvasView() :
_controller(nullptr)
, _selectedNode(nullptr)
, _physicsWorld(nullptr)
, _velocityTracker(nullptr)
, _trackFlyEvent(false)
, _flyRemovable(true)
, _listener(nullptr)
{
    
}

StickerCanvasView::~StickerCanvasView()
{
    CC_SAFE_DELETE(_controller);
    CC_SAFE_DELETE(_velocityTracker);
}

bool StickerCanvasView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    _controller = new MultiTouch::Controller(this);
    _velocityTracker = new VelocityTracker();
    
    return true;
}

void StickerCanvasView::setRemoveAfterFlying(const bool bEnable)
{
    _flyRemovable = bEnable;
}

void StickerCanvasView::setChildPosition(cocos2d::Node *node, const int position)
{
    CCASSERT(node!=nullptr, "node must be not nullptr");
    
    auto children = getChildren();
    
    CCASSERT(children.contains(node), "addChild() first!!!");
    
    int zorder = 0;
    for (auto iter=children.begin(); iter!=children.end(); ++iter) {
        if (*iter==_bgNode) {   // _bgNode는 pass~
            continue;
        }
        
        if (*iter==node) {
            node->setLocalZOrder(position);
            continue;
        }
        
        if (zorder==-position) {
            zorder--;
        }
        
        (*iter)->setLocalZOrder(zorder--);  // zorder를 세팅하고 -1 해줌
    }
    sortAllChildren();
}

void StickerCanvasView::bringChildToTop(cocos2d::Node *node)
{
    CCASSERT(node!=nullptr, "node must be not nullptr");
    
    auto children = getChildren();
    
    CCASSERT(children.contains(node), "addChild() first!!!");
    
    int zorder = -1;
    for (auto iter=children.begin(); iter!=children.end(); ++iter) {
        if (*iter==_bgNode) {   // _bgNode는 pass~
            continue;
        }
        
        if (*iter==node) {
            continue;
        }
        
        // node가 아닌넘들은 0을 비워두고 -1부터 하나씩 내림
        (*iter)->setLocalZOrder(zorder--);
    }
    
    // 0을 빼고 -1부터 하나씩 내렸고 0이 비었으니 0으로 세팅하면 top이다.
    node->setLocalZOrder(0);
    sortAllChildren();
}

void StickerCanvasView::sendChildToBack(cocos2d::Node *node)
{
    CCASSERT(node!=nullptr, "node must be not nullptr");
    
    auto children = getChildren();
    
    CCASSERT(children.contains(node), "addChild() first!!!");

    int zorder = 0;
    for (auto iter=children.begin(); iter!=children.end(); ++iter) {
        if (*iter==_bgNode) {   // _bgNode pass~
            continue;
        }
        
        if (*iter==node) {
            continue;
        }
        
        // node가 아닌넘들은 0부터 하나씩 내림
        (*iter)->setLocalZOrder(zorder--);
    }
    
    // 0부터 하나씩 내렸고 zorder가 마지막이다.
    node->setLocalZOrder(zorder);
    sortAllChildren();
}

// aboveNode 바로 아래로 이동
void StickerCanvasView::aboveNode(cocos2d::Node *node, cocos2d::Node *aboveNode)
{
    CCASSERT(node!=nullptr, "node must be not nullptr");
    if (aboveNode==nullptr) {   // 위에 아무것도 없으면 top이다.
        bringChildToTop(node);
        return;
    }
    
    auto children = getChildren();
    
    CCASSERT(children.contains(node), "addChild() first!!!");
    
    if (!children.contains(aboveNode) || node==aboveNode) {
        // aboveNode가 그룹에 없거나 그넘이 그넘이면 상관 없으니 종료
        return;
    }
    
    int zorder = 0;
    int target = 0;
    
    for (auto iter=children.begin(); iter!=children.end(); ++iter) {
        if (*iter==_bgNode) {   // _bgNode는 pass~
            continue;
        }
        
        if (*iter==node) {
            continue;
        }
        
        if (*iter==aboveNode) {
            target = zorder;
            zorder--;
        }
        
        (*iter)->setLocalZOrder(zorder--);
    }
    
    node->setLocalZOrder(target);
    sortAllChildren();
}

// belowNode 바로 위로 이동
void StickerCanvasView::belowNode(cocos2d::Node *node, cocos2d::Node *belowNode)
{
    CCASSERT(node!=nullptr, "node must be not nullptr");
    if (belowNode==nullptr) {   // 아래에 아무것도 없으면 bottom이다.
        sendChildToBack(node);
        return;
    }
    
    auto children = getChildren();
    
    CCASSERT(children.contains(node), "addChild() first!!!");
    
    if (!children.contains(belowNode) || node==belowNode) {
        // belowNode가 그룹에 없거나 그넘이 그넘이면 상관 없으니 종료
        return;
    }

    int zorder = 0;
    int target = 0;
    
    for (auto iter=children.begin(); iter!=children.end(); ++iter) {
        if (*iter==_bgNode) {   // _bgNode는 pass~
            continue;
        }
        
        if (*iter==node) {
            continue;
        }
        
        (*iter)->setLocalZOrder(zorder--);
        
        if (*iter==belowNode) {
            target = zorder;
            zorder--;
        }
    }
    
    node->setLocalZOrder(target);
    sortAllChildren();
}

void StickerCanvasView::addChild(cocos2d::Node *child, int localZOrder, const std::string &name)
{
    SMView::addChild(child, localZOrder, name);
    setSelectedSticker(nullptr);
}

void StickerCanvasView::removeChild(cocos2d::Node *child, bool cleanup)
{
    if (child!=nullptr && child == _selectedNode) {
        performSelected(_selectedNode, false);
        _selectedNode = nullptr;
    }
    
    SMView::removeChild(child, cleanup);
}

bool StickerCanvasView::setSelectedSticker(cocos2d::Node *node)
{
    if (node==nullptr) {
        if (_selectedNode) {
            performSelected(_selectedNode, false);
            _selectedNode = nullptr;
        }
        return true;
    } else if (node != _selectedNode) {
        auto children = getChildren();
        if (children.contains(node)) {
            if (_selectedNode) {
                performSelected(_selectedNode, false);
            }
            _selectedNode = node;
            performSelected(node, true);
            return true;
        }
    }
    
    return false;
}

cocos2d::Node * StickerCanvasView::getSelectedSticker()
{
    return _selectedNode;
}


int StickerCanvasView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    int ret = SMView::dispatchTouchEvent(action, touch, point, event);
    
    int mode = _controller->getMode();
    
    if (action == MotionEvent::ACTION_UP) {
        // touch up일때 알림
        if (_listener) {
            _listener->onStickerTouch(_selectedNode, MotionEvent::ACTION_UP);
        }
    }
    
    if (_controller->onTouchEvent(this, event)) {
        if (mode==MultiTouch::Controller::MODE_NOTHING && action==MotionEvent::ACTION_DOWN) {
            _listener->onStickerTouch(_selectedNode, MotionEvent::ACTION_DOWN);
        }
        _velocityTracker->addMovement(event);
        
        if (mode==MultiTouch::Controller::MODE_DRAG && action==MotionEvent::ACTION_MOVE) {
            if (!_trackFlyEvent) {
                auto removable = dynamic_cast<RemovableSticker*>(_selectedNode);
                if (removable==nullptr || removable->isRemovable()) {
                    auto point = _controller->getCurrentPoint();
                    auto dist = point.getDistance(_lastTouchPoint);
                    if (dist>SMViewConstValue::Config::SCROLL_TOLERANCE) {  // fly remove가 될 수 있는 만큼 시간대비 이동했냐? (플리킹 했는지 체크)
                        _trackFlyEvent = true;
                    }
                }
            }
        }
        
        return TOUCH_INTERCEPT;
    } else if (mode!=MultiTouch::Controller::MODE_NOTHING && action==MotionEvent::ACTION_UP) {
        if (mode==MultiTouch::Controller::MODE_DRAG) {
            if (_trackFlyEvent) {   // fly remove로 날려야 되는 상황이냐..
                _trackFlyEvent = false;
                
                float vx, vy;
                _velocityTracker->getVelocity(0, &vx, &vy); // 진행 방향의 속도에 맞는 좌표 구함.
                
                // fly
                double radians = atan2(vy, vx);
                float degrees = (float)ViewUtil::toDegrees(radians);
                float speed = (float)sqrt(vx*vx + vy*vy);
                
                degrees = fmod(degrees+360.0f, 360.0f);
                
                if (speed>MIN_FLY_TOLERANCE) {  // fly remove를 하기 위한 최소 속도가 넘어야 지우게 한다.
                    auto wspeed = convertToNodeSpace(cocos2d::Vec2(speed, 0));
                    performFly(_selectedNode, degrees, wspeed.x);
                }
            }
            _velocityTracker->clear();
        }
        return TOUCH_TRUE;
    }
    
    return ret;
}

cocos2d::Node * StickerCanvasView::getDraggableObjectAtPoint(MultiTouch::PointInfo& touchPoint)
{
    auto worldPoint = convertToWorldSpace(cocos2d::Vec2(touchPoint.getX(), touchPoint.getY()));
    
    if (_physicsWorld) {
        auto shapes = _physicsWorld->getShapes(worldPoint);
        
        auto children = getChildren();
        
        for (auto iter=children.begin(); iter!=children.end(); ++iter) {
            if (*iter==_bgNode) {   // _bgNode는 pass
                continue;
            }
            
            if ((*iter)->getPhysicsBody()==nullptr) { // PhysicsBody가 없는 자체 영역으로 검사
                auto nodePoint = (*iter)->convertToNodeSpace(worldPoint);
                auto size = (*iter)->getContentSize();
                if (!(nodePoint.x<0 || nodePoint.y<0 || nodePoint.x>size.width-1 || nodePoint.y>size.height-1)) {   // 벗어난게 아니라면
                    return *iter;   // 이넘이  그넘이다.
                }
            } else {
                // PhysicsBody가 있으면 PhysicsShape에서 검사
                for (auto& shape : shapes) {
                    if (*iter == shape->getBody()->getNode()) {
                        if (!(*iter)->getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE)) {
                            return *iter;
                        }
                    }
                }
            }
        }
    } else {
        auto children = getChildren();
        for (auto iter=children.begin(); iter!=children.end(); ++iter) {
            if (*iter==_bgNode) {   // _bgNode는 패스
                continue;
            }
            
            auto nodePoint = (*iter)->convertToNodeSpace(worldPoint);
            auto size = (*iter)->getContentSize();
            if (!(nodePoint.x<0 || nodePoint.y<0 || nodePoint.x>size.width-1 || nodePoint.y>size.height-1)) {   // 벗어난게 아니라면
                if (!(*iter)->getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE)) {
                    return *iter;
                }
            }
        }
    }
    
    // 여기에 내려오면 아무것도 선택 안된거다.
    if (_selectedNode) {
        performSelected(_selectedNode, false);
        _selectedNode = nullptr;
    }
    
    return nullptr;
}

bool StickerCanvasView::pointInObjectGrabArea(MultiTouch::PointInfo &touchPoint, cocos2d::Node *node)
{
    // 사용 안함.
    // 나중에 지울거임. override 괜히 했음.
    return false;
}

void StickerCanvasView::getPositionAndScale(cocos2d::Node * node, MultiTouch::PositionAndScale& objPosAndScaleOut)
{
    auto pt = node->getPosition();
    objPosAndScaleOut.set(pt.x, pt.y, true, node->getScale(), false, 1.0f, 1.0f, true, (float)ViewUtil::toRadians(-node->getRotation()));
}

bool StickerCanvasView::setPositionAndScale(cocos2d::Node *node, MultiTouch::PositionAndScale &newObjPosAndScale, MultiTouch::PointInfo &touchPoint)
{
    auto view = dynamic_cast<SMView*>(node);
    
    if (view) {
        // SMView냐?? smview꺼 적용... 이상하게 띄엄띄엄 가면 안되니까.
        view->setPosition(newObjPosAndScale.getXOff(), newObjPosAndScale.getYOff(), false);
        view->setScale(newObjPosAndScale.getScale(), false);
        view->setRotation((float)(-ViewUtil::toDegrees(newObjPosAndScale.getAngle())), false);
    } else {
        // 일반 node이면 node꺼 적용
        node->setPosition(newObjPosAndScale.getXOff(), newObjPosAndScale.getYOff());
        node->setScale(newObjPosAndScale.getScale());
        node->setRotation((float)(-ViewUtil::toDegrees(newObjPosAndScale.getAngle())));
    }
    
    return true;
}


void StickerCanvasView::selectObject(cocos2d::Node *node, MultiTouch::PointInfo &touchPoint)
{
    if (node) {
        bringChildToTop(node);  // 선택했으면 일단 맨 위로
        
        if (_selectedNode!=node) {  // 아까 선택한 넘이랑 같으면 pass
            if (_selectedNode) {    // 이전 선택한 넘은 deselect
                performSelected(_selectedNode, false);
            }
            
            _selectedNode = node;
            performSelected(node, true);    // 이넘이 새로 선택된 넘이다.
            
            if (node) {
                _velocityTracker->clear();
                _lastTouchPoint.set(touchPoint);
            }
        }
    }
}

void StickerCanvasView::doubleClickObject(cocos2d::Node * node, MultiTouch::PointInfo &touchPoint)
{
    if (_listener) {    // 더블 클릭은 여기서는 알바 아니고 상위에서 처리할꺼임...  zoom view 확대 또는 축소 해야함.
        auto point = cocos2d::Vec2(touchPoint.getX(), touchPoint.getY());
        _listener->onStickerDoubleClicked(node, convertToWorldSpace(point));    // node가 더블클릭 되었음
    }
}

void StickerCanvasView::touchModeChanged(const int touchMode, MultiTouch::PointInfo &touchPoint)
{
    if (touchMode==MultiTouch::Controller::MODE_DRAG) { // 그냥 drag라서 fly 같은 액션 안한다.
        _velocityTracker->clear();
        _lastTouchPoint.set(touchPoint);
        _trackFlyEvent = false;
    }
}

cocos2d::Vec2 StickerCanvasView::toWorldPoint(const cocos2d::Vec2 &canvasPoint)
{
    return convertToWorldSpace(canvasPoint);
}

cocos2d::Vec2 StickerCanvasView::toCanvasPoint(const cocos2d::Vec2 &worldPoint)
{
    return convertToNodeSpace(worldPoint);
}

void StickerCanvasView::performSelected(cocos2d::Node *node, bool selected)
{
    if (_listener) {
        _listener->onStickerSelected(node, selected);
    }
}

void StickerCanvasView::performFly(cocos2d::Node *node, const float degrees, const float speed)
{
    if (!_flyRemovable) { // 날아갈 상태가 아니면 pass
        return;
    }
    
    if (_selectedNode==node) {  // 선택된 넘이면 선택해제
        performSelected(node, false);
        _selectedNode = nullptr;
    }
    
    auto pt = node->getPosition();
    auto dist = speed / 10.0f;
    
    float deltaX = dist * cos(ViewUtil::toRadians(degrees));
    float deltaY = dist * sin(ViewUtil::toRadians(degrees));
    float rotate = speed / 100.0f;
    
    float dir = (degrees > 90 && degrees < 270) ? -1 : 1;   // 반바퀴가 넘지 않은거면 반대로 돌자
    auto moveTo = cocos2d::EaseOut::create(cocos2d::MoveBy::create(FLY_DURATION, cocos2d::Vec2(deltaX, deltaY)), 3.0f); // 움직이면서
    auto rotateTo = cocos2d::RotateBy::create(FLY_DURATION, dir*rotate); // 돌면서
    auto fadeTo = cocos2d::Sequence::create(cocos2d::DelayTime::create(FLY_DURATION/2), cocos2d::FadeOut::create(FLY_DURATION/2), NULL); // 중간에 사라지면서
    auto remove = cocos2d::Spawn::create(moveTo, rotateTo, fadeTo, NULL);
    auto seq = cocos2d::Sequence::create(remove, cocos2d::CallFuncN::create([this](cocos2d::Node * target){
        if (_listener) {
            _listener->onStickerRemoveEnd(target); // 날아가기 끝남
        }
        target->removeFromParent();
    }), NULL);
    
    seq->setTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE);
    node->runAction(seq);
    
    if (_listener) {
        _listener->onStickerRemoveBegin(node); // 날아가기 시작
    }
    
}

void StickerCanvasView::removeChildWithGenieAction(cocos2d::Node *child, cocos2d::Sprite *sprite, const cocos2d::Vec2 &removeAnchor, float duration, float delay)
{
    if (sprite==nullptr) {
        // sprite가 없을 수 있나???
        removeChild(child);
        return;
    }
    
    if (child->getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE)) {
        // 현재 가지고 있는 action이 sticker라면 이미 삭제 중이니까 넘어간다.
        // 중복 호출 방지
        return;
    }
    
    if (_selectedNode==child) { // 이넘이 그넘이냐
        performSelected(child, false);
        _selectedNode = nullptr;
    }
    
    auto genie = cocos2d::EaseBackIn::create(ViewAction::GenieAction::create(duration, sprite, removeAnchor));
    auto seq = cocos2d::Sequence::create(cocos2d::DelayTime::create(delay), genie, cocos2d::CallFuncN::create([this](cocos2d::Node* target){
        if (_listener) {
            _listener->onStickerRemoveEnd(target);
        }
        target->removeFromParent();
    }), nullptr);
    
    seq->setTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE);
    child->runAction(seq);
    
    if (_listener) {
        _listener->onStickerRemoveBegin(child);
    }
}

void StickerCanvasView::removeChildWithFadeOut(cocos2d::Node *child, float duration, float delay)
{
    if (child->getOpacity()<=0) {
        // 이미 사라지고 없다.
        removeChild(child);
        return;
    }
    
    if (child->getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE)) {
        // remove action 중이라면 중복방지
        return;
    }
    
    if (_selectedNode==child) { // 이넘이 그넘이면
        performSelected(child, false);  // 선택해제
        _selectedNode = nullptr;
    }
    
    auto seq = cocos2d::Sequence::create(cocos2d::DelayTime::create(delay), cocos2d::FadeTo::create(duration, 0), cocos2d::CallFuncN::create([this](cocos2d::Node* target){
        if (_listener) {
            _listener->onStickerRemoveEnd(target);  // 삭제 action 끝났음을 통지
        }
        target->removeFromParent();
    }), NULL);
    
    seq->setTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE);
    child->runAction(seq);
    
    if (_listener) {
        _listener->onStickerRemoveBegin(child); // 삭제 action 시작함을 통지
    }
}

void StickerCanvasView::removeChildWithFly(cocos2d::Node *child, const float degrees, const float speed)
{
    if (child->getOpacity()<=0) {
        // 사라지고 없으면 pass~
        removeChild(child);
        return;
    }
    
    if (child->getActionByTag(SMViewConstValue::Tag::ACTION_STICKER_REMOVE)) {
        // remove action 중이라면 중복방지
        return;
    }
    
    if (_selectedNode==child) { // 이넘이 그넘이냐
        performSelected(child, false);  // 선택해제
        _selectedNode = nullptr;
    }
    
    performFly(child, degrees, speed);  // 날아가는 action 호출
}

bool StickerCanvasView::containsPoint(const cocos2d::Vec2 &point)
{
    // 안씀.
    return true;
}

int StickerCanvasView::dispatchTouchEvent(MotionEvent *event, SMView *view, bool checkBounds)
{
    return SMView::dispatchTouchEvent(event, view, false);
}

void StickerCanvasView::cancel()
{
    if (_selectedNode) {
        setSelectedSticker(nullptr);
    }
}
