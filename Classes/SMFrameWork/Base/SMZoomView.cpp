//
//  SMZoomView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 9..
//
//

#include "SMZoomView.h"
#include "../Util/ViewUtil.h"
#include <2d/CCActionInterval.h>
#include <2d/CCActionInstant.h>
#include <2d/CCActionEase.h>
#include "../Interface/ZoomController.h"
#include "../Interface/VelocityTracker.h"
#include <cmath>
#include <math.h>
#include <cocos2d.h>

#define FLAG_ZOOM_UPDATE   (1<<0)


class SMZoomView::ZoomTo : public cocos2d::ActionInterval
{
public:
    static ZoomTo* create(float zoom, const cocos2d::Vec2& pan, float duration) {
        auto zoomTo = new (std::nothrow)ZoomTo();
        if (zoomTo!=nullptr && zoomTo->initWithDuration(duration)) {
            zoomTo->autorelease();
            zoomTo->_toPan = pan;
            zoomTo->_toZoom = zoom;
        }
        return zoomTo;
    }
    
    virtual void startWithTarget(cocos2d::Node* target) override {
        cocos2d::ActionInterval::startWithTarget(target);
        
        auto zoomView = (SMZoomView*)_target;
        auto controller = zoomView->getController();
        
        _fromPan = cocos2d::Vec2(controller->getPanX(), controller->getPanY());
        _fromZoom = controller->getZoom();
    }
    
    virtual void update(float t)  override {
        auto zoomView = (SMZoomView*)_target;
        auto controller = zoomView->getController();
        
        float zoom = ViewUtil::interpolation(_fromZoom, _toZoom, t);
        float panX = ViewUtil::interpolation(_fromPan.x, _toPan.x, t);
        float panY = ViewUtil::interpolation(_fromPan.y, _toPan.y, t);
        
        controller->zoomImmediate(zoom, panX, panY);
        zoomView->registerUpdate(FLAG_ZOOM_UPDATE);
    }
    
private:
    ZoomTo(){}
    virtual ~ZoomTo(){}
    cocos2d::Vec2 _fromPan, _toPan;
    float _fromZoom, _toZoom;
};

SMZoomView::SMZoomView() :
_mode(Mode::UNDEFINED),
_contentNode(nullptr),
_controller(nullptr),
_velocityTracker(nullptr),
_baseZoom(1.0f),
_prevZoom(1.0f),
_zoom(1.0f),
_prevDistance(0),
_prevTouchX(0),
_prevTouchY(0),
_initTouchX(0),
_initTouchY(0),
_interpolate(true),
_panEnable(true),
_zoomEnable(true),
_fillType(FillType::INSIDE),
_innerSize(cocos2d::Size::ZERO),
_zoomListener(nullptr),
_zoomCallback(nullptr)
{
    
}

SMZoomView::~SMZoomView()
{
    CC_SAFE_DELETE(_velocityTracker);
    CC_SAFE_DELETE(_controller);
}

bool SMZoomView::init()
{
    if (_UIContainerView::init()) {
        _uiContainer->setAnchorPoint(cocos2d::Vec2::ZERO);
        _uiContainer->setPosition(cocos2d::Vec2::ZERO);
        
        _controller = new ZoomController();
        setTouchMask(TOUCH_MASK_DOUBLECLICK);
        
        return true;
    }
    return false;
}

void SMZoomView::setFillType(const FillType type)
{
    _fillType = type;
}

void SMZoomView::setPanEnable(bool enable)
{
    _panEnable = enable;
}

void SMZoomView::setZoomEnable(bool enable)
{
    _zoomEnable = enable;
}

bool SMZoomView::isIdle()
{
    return _mode==Mode::UNDEFINED;
}

bool SMZoomView::isPanning()
{
    return _controller->isPanning();
}

cocos2d::Node* SMZoomView::getContentNode()
{
    return _contentNode;
}

void SMZoomView::setPadding(const float padding)
{
    CCASSERT(_contentNode==nullptr, "setPadding() must call before setContentNode()");
        
    _UIContainerView::setPadding(padding);
}

void SMZoomView::setPadding(const float left, const float top, const float right, const float bottom)
{
    CCASSERT(_contentNode==nullptr, "setPadding() must call before setContentNode()");
    
    _UIContainerView::setPadding(left, top, right, bottom);
}

void SMZoomView::setContentSize(const cocos2d::Size &size)
{
    _innerSize.setSize(size.width-_paddingLeft-_paddingRight, size.height-_paddingTop-_paddingBottom);
    SMView::setContentSize(size);
    
    registerUpdate(FLAG_ZOOM_UPDATE);
}

void SMZoomView::refreshContentNode(const bool reset)
{
    if (_contentNode!=nullptr) {
        _uiContainer->setContentSize(_contentNode->getContentSize());
        
        if (reset) {
        _controller->reset();
            
            _uiContainer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            _uiContainer->setPosition(_paddingLeft+_innerSize.width/2, _paddingBottom+_innerSize.height/2);
            _uiContainer->setScale(_initScale);
        }
        
        _controller->updateAspect(_innerSize, _contentNode->getContentSize(), _fillType==FillType::FILL);
        _controller->updateLimits();
        _baseZoom = computeBaseZoom(_innerSize, _contentNode->getContentSize());
    }
}

void SMZoomView::setContentNode(cocos2d::Node *node)
{
    if (_contentNode==node) {
        // already setting same node...
        return;
    }
    
    if (_contentNode!=nullptr) {
        _uiContainer->removeChild(_contentNode);
        _controller->reset();
    }
    
    if (node!=nullptr) {
        cocos2d::Size size = node->getContentSize();
        
        _uiContainer->addChild(node);
        _uiContainer->setContentSize(size);
        
        _controller->reset();
        _controller->updateAspect(_innerSize, size, _fillType==FillType::FILL);
        _controller->updateLimits();
        _baseZoom = computeBaseZoom(_innerSize, size);
        
        _uiContainer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        _uiContainer->setPosition(size/2);
        _initScale = _controller->getZoom()*_baseZoom;
        _uiContainer->setScale(_initScale);
        
        auto view = dynamic_cast<SMView*>(node);
        if (view) {
            // child 까지 touch를 전달
            view->setIgnoreTouchBounds(true);
        }
        
        _interpolate = true;
    }
    
    _contentNode = node;
}


float SMZoomView::computeBaseZoom(const cocos2d::Size &viewSize, const cocos2d::Size &contentSize)
{
    float zoomX = viewSize.width/contentSize.width;
    float zoomY = viewSize.height/contentSize.height;
    
    if (_fillType==FillType::INSIDE) {
        // 안에 맞게 하려면 작은 값을 기준으로
        return std::min(zoomX, zoomY);
    } else {
        // 채우려면 큰 값을 기준으로
        return std::max(zoomX, zoomY);
    }
}

// getZoom() same...
float SMZoomView::getContentZoomScale()
{
    return _zoom;
}

float SMZoomView::getContentBaseScale()
{
    return _baseZoom;
}

const cocos2d::Vec2& SMZoomView::getContentPosition() const
{
    return _uiContainer->getPosition();
}

// getContentZoomScale() same...
float SMZoomView::getZoom()
{
    return _zoom;
}

float SMZoomView::getPanX()
{
    return _panX;
}

float SMZoomView::getPanY()
{
    return _panY;
}

bool SMZoomView::containsPoint(const cocos2d::Vec2 &point)
{
    if (_ignoreTouchBounds) {
        return true;
    }
    
    return SMView::containsPoint(point);
}

void SMZoomView::onUpdateOnVisit()
{
    if (!_controller->update()) {
        unregisterUpdate(FLAG_ZOOM_UPDATE);
    }
    
    _panX = _controller->getPanX();
    _panY = _controller->getPanY();
    _zoom = _controller->getZoom();
    
    float scale = _baseZoom*_zoom;
    
    float x = scale * (-(_panX-0.5)) * _uiContainer->getContentSize().width;
    float y = scale * (-(_panY-0.5)) * _uiContainer->getContentSize().height;
    
    _uiContainer->setPosition(_paddingLeft + _innerSize.width/2 + x, _paddingBottom + _innerSize.height/2 + y, _interpolate);
    if (_uiContainer->getScale()!=scale) {
    _uiContainer->setScale(scale, _interpolate);
        if (_zoomListener) {
            _zoomListener->onZoomScaleChange(this, _initScale, scale);
        }
        if (_zoomCallback) {
            _zoomCallback(this, _initScale, scale);
        }
    }
    
    
    _interpolate = false;
}

cocos2d::Rect SMZoomView::getReverseFocusRect(const cocos2d::Rect &focusRect)
{
    // To. Do.... realization
    return cocos2d::Rect();
}

int SMZoomView::dispatchTouchEvent(MotionEvent *event, SMView *view, bool checkBounds)
{
    return _UIContainerView::dispatchTouchEvent(event, view, false);
}

int SMZoomView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *givenPoint, MotionEvent *event)
{
    if (_mode==Mode::UNDEFINED) {
        int ret = SMView::dispatchTouchEvent(action, touch, givenPoint, event);
        if (ret==TOUCH_INTERCEPT) {
            return TOUCH_INTERCEPT;
        }
    }
    
    cocos2d::Vec2 point = *givenPoint - cocos2d::Vec2(_paddingLeft, _paddingBottom);
    
    float x = point.x;
    float y = point.y;
    
    if (_velocityTracker==nullptr) {
        _velocityTracker = new VelocityTracker();
    }
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
            {
                if (_panEnable) {
                // scrolling을 멈추고
                    _controller->stopFling();
                }
            
                _initTouchX = x;
                _initTouchY = y;
                _prevTouchX = x;
                _prevTouchY = y;
                
                _accuX = _accuY = 0;
                
                _velocityTracker->addMovement(action, _accuX, _accuY, event->getEventTime());
            }
            break;
        case MotionEvent::ACTION_POINTER_DOWN:
            {
                if (_zoomEnable) {
                    cocos2d::Vec2 point2 = convertToNodeSpace(event->getTouch(1)->getLocation());
                    point2 -= cocos2d::Vec2(_paddingLeft, _paddingBottom);
                    
                    float distance = point.distance(point2);
                
                if (distance>0) {
                        _prevDistance = distance;
                    if (_prevDistance>10.0f) {
                            _mode = Mode::ZOOM;
                            
                        cocos2d::Vec2 midPoint = (point + point2)/2.0f;
                            _initTouchX = _prevTouchX = midPoint.x;
                            _initTouchY = _prevTouchY = midPoint.y;
                        _prevZoom =_controller->getZoom();
                        }
                    }
                }
                
                _velocityTracker->addMovement(action, _accuX, _accuY, event->getEventTime());
            }
            break;
        case MotionEvent::ACTION_MOVE:
            {
            if (_mode==Mode::ZOOM) {
                    cocos2d::Vec2 point2 = convertToNodeSpace(event->getTouch(1)->getLocation());
                    point2 -= cocos2d::Vec2(_paddingLeft, _paddingBottom);
                
                cocos2d::Vec2 midPoint = (point + point2)/2.0f;
                    
                    x = midPoint.x;
                    y = midPoint.y;
                    
                float dx = (x-_prevTouchX)/_innerSize.width;
                float dy = (y-_prevTouchY)/_innerSize.height;
                
                    _controller->pan(-dx, -dy);
                    
                    float distance = point.distance(point2);
                float scale = (distance/_prevDistance)*_prevZoom;
                    
                _controller->zoom(scale, x/_innerSize.width, y/_innerSize.height);
                    registerUpdate(FLAG_ZOOM_UPDATE);
                
                    _accuX += x - _prevTouchX;
                    _accuY += y - _prevTouchY;
                
                    _velocityTracker->addMovement(action, _accuX, _accuY, event->getEventTime());
                
                    _prevTouchX = x;
                    _prevTouchY = y;
            } else if (_mode==Mode::PAN) {
                float dx = (x-_prevTouchX)/_innerSize.width;
                float dy = (y-_prevTouchY)/_innerSize.height;
                    
                    _controller->pan(-dx, -dy);
                    registerUpdate(FLAG_ZOOM_UPDATE);
                    
                    _accuX += x - _prevTouchX;
                    _accuY += y - _prevTouchY;
                    _velocityTracker->addMovement(action, _accuX, _accuY, event->getEventTime());
                    
                    _prevTouchX = x;
                    _prevTouchY = y;
                } else {
                    if (_panEnable) {
                        float scrollX = _initTouchX - x;
                        float scrollY = _initTouchY - y;
                    float distance = std::sqrt(scrollX*scrollX+scrollY*scrollY);
                        
                    if (distance>SMViewConstValue::Config::SCALED_TOUCH_SLOPE) {
                            _mode = Mode::PAN;
                            registerUpdate(FLAG_ZOOM_UPDATE);
                        }
                    }
                    
                    _accuX += x - _prevTouchX;
                    _accuY += y - _prevTouchY;
                    _velocityTracker->addMovement(action, _accuX, _accuY, event->getEventTime());
                
                    _prevTouchX = x;
                    _prevTouchY = y;
                }
            }
            break;
        case MotionEvent::ACTION_POINTER_UP:
            {
            // multi touch는 zoom일 경우만 반응
            if (_mode==Mode::ZOOM) {
                    int index = event->getActionIndex();
                if (index==0 || index==1) {
                        _mode = Mode::PAN;
                        
                    cocos2d::Vec2 pt;
                    if (index==1) {
                        // 두번째가 떨어짐
                        pt = convertToNodeSpace(event->getTouch(0)->getLocation()) - cocos2d::Vec2(_paddingLeft, _paddingBottom);
                    } else {
                        // 첫번째가 떨어짐
                        pt = convertToNodeSpace(event->getTouch(1)->getLocation()) - cocos2d::Vec2(_paddingLeft, _paddingBottom);
                    }
                            _initTouchX = _prevTouchX = pt.x;
                            _initTouchY = _prevTouchY = pt.y;
                        }
                    }
                }
            break;
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_CANCEL:
            {
            if (_mode==Mode::PAN) {
                    float vx, vy;
                    _velocityTracker->getVelocity(0, &vx, &vy);
                _controller->startFling(-vx/_innerSize.width,
                                                    -vy/_innerSize.height);
            } else {
                    if (_panEnable) {
                        _controller->updateLimits();
                        _controller->startFling(0, 0);
                    }
                }
                
            if (_velocityTracker!=nullptr) {
                    _velocityTracker->clear();
                }
                
                _mode = Mode::UNDEFINED;
                registerUpdate(FLAG_ZOOM_UPDATE);
            }
            break;
    }
    
    return TOUCH_TRUE;
}

void SMZoomView::updateZoom()
{
    registerUpdate(FLAG_ZOOM_UPDATE);
}

void SMZoomView::performDoubleClick(const cocos2d::Vec2 &worldPoint)
{
    if (!_zoomEnable || _contentNode==nullptr) {
        return;
    }
    
    auto action = getActionByTag(SMViewConstValue::Tag::ACTION_ZOOM);
    if (action) {
        stopAction(action);
    }
    
    float zoom = _controller->getZoom();
    // 뷰 안에 찍은 포인트
    cocos2d::Vec2 point = convertToNodeSpace(worldPoint) - cocos2d::Vec2(_paddingLeft, _paddingBottom);
    // 원래 크기 비율의 포인트
    cocos2d::Vec2 pivot = cocos2d::Vec2(point.x/_innerSize.width, point.y/_innerSize.height);
    
    float newZoom = 1.0f;
    if (std::abs(zoom-1)<=0.5f) {
        // 기본 근처일 경우 2배로 키운다.
        newZoom = 2.0f;
    } else if (std::abs(zoom-2)<=0.5f) {
        // 2배 근처일 경우 4배로 키운다.
        newZoom = 4.0f;
    } else {
        // 기본으로 초기화 한다.
        newZoom = 1.0f;
    }
    
    cocos2d::Vec2 newPan = _controller->computePanPosition(newZoom, pivot);
    
    auto zoomTo = cocos2d::EaseSineInOut::create(ZoomTo::create(newZoom, newPan, SMViewConstValue::Config::ZOOM_NORMAL_TIME));
    zoomTo->setTag(SMViewConstValue::Tag::ACTION_ZOOM);
    runAction(zoomTo);
}

void SMZoomView::setFocusRect(const cocos2d::Rect &focusRect, const float duration)
{
    if (focusRect.size.width<=0 || focusRect.size.height<=0) {
        return;
    }
    
    auto action = getActionByTag(SMViewConstValue::Tag::ACTION_ZOOM);
    if (action) {
        stopAction(action);
    }
    
    // 기본 zoom을 1로 놓고 계산
    cocos2d::Size size = _contentNode->getContentSize();
    float aspectView = _contentSize.width/_contentSize.height;
    float aspectCont = size.width/size.height;
    
    float width, height;
    
    if (aspectCont>aspectView) {
        // 좌우가  걸림
        width = size.width;
        height = size.width / aspectView;
    } else {
        // 위아래가 걸림
        height = size.height;
        width = size.height * aspectView;
    }
    
    float newZoom = std::min(width/(focusRect.size.width), height/(focusRect.size.height));
    float newPanX = focusRect.getMidX() / size.width;
    float newPanY = focusRect.getMidY() / size.height;
    
    auto zoomTo = cocos2d::EaseSineInOut::create(ZoomTo::create(newZoom, cocos2d::Vec2(newPanX, newPanY), duration));
    zoomTo->setTag(SMViewConstValue::Tag::ACTION_ZOOM);
    
    runAction(zoomTo);
}

void SMZoomView::setZoomWithAnimation(const float panX, const float panY, const float zoom, const float duration)
{
    auto action = getActionByTag(SMViewConstValue::Tag::ACTION_ZOOM);
    if (action) {
        stopAction(action);
    }
    
    auto zoomTo = cocos2d::EaseSineInOut::create(ZoomTo::create(zoom, cocos2d::Vec2(panX, panY), duration));
    zoomTo->setTag(SMViewConstValue::Tag::ACTION_ZOOM);
    
    runAction(zoomTo);
}

