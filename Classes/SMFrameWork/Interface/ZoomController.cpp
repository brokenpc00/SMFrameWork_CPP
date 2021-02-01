#include "ZoomController.h"
#include "Dynamics.h"
#include <cocos2d.h>

const float ZoomController::MIN_ZOOM = 1.0f;
const float ZoomController::MAX_ZOOM = 6.0f;
const float ZoomController::REST_VELOCITY_TOLERANCE = 0.1f;
const float ZoomController::REST_POSITION_TOLERANCE = 0.1f;
const float ZoomController::REST_ZOOM_TOLERANCE = 0.005f;
const float ZoomController::PAN_OUTSIDE_SNAP_FACTOR = 0.4f;
const float ZoomController::ZOOM_OUTSIDE_SNAP_FACTOR = 0.023f;



ZoomController::ZoomController() :
_panMinX(0),
_panMaxX(0),
_panMinY(0),
_panMaxY(0),
_needUpdate(false),
_fillMode(false),

_aspect(1.0),
_zoom(1.0),
_panX(0),
_panY(0),

_viewSize(cocos2d::Size::ZERO)
{
//    _panDynamicsX = new SpringDynamics();
//    _panDynamicsY = new SpringDynamics();
//    _zoomDynamics = new SpringDynamics();
    _panDynamicsX = new Dynamics();
    _panDynamicsY = new Dynamics();
    _zoomDynamics = new Dynamics();
    
    reset();
}

ZoomController::~ZoomController()
{
    delete _panDynamicsX;
    delete _panDynamicsY;
    delete _zoomDynamics;
}

void ZoomController::reset() {
    _panDynamicsX->reset();
    _panDynamicsY->reset();
    _panDynamicsX->setFriction(2.0f);
    _panDynamicsY->setFriction(2.0f);
    _panDynamicsX->setSpring(500.0f, 0.9f);
    _panDynamicsY->setSpring(500.0f, 0.9f);
    //    _panDynamicsX->setSpring(150.0f, 1.0f);
    //    _panDynamicsY->setSpring(150.0f, 1.0f);
    
    _zoomDynamics->reset();
    _zoomDynamics->setFriction(5.0f);
    _zoomDynamics->setSpring(800, 1.3f);
    
    _zoomDynamics->setMinPosition(MIN_ZOOM);
    _zoomDynamics->setMaxPosition(MAX_ZOOM);
    _zoomDynamics->setState(1.0, 0, 0);
    
    
    _zoom = 1.0f;
    _panX = 0.5f;
    _panY = 0.5f;
    updateLimits();
    
    _needUpdate = false;
}

float ZoomController::getPanX() const {
    return _panX;
}

float ZoomController::getPanY() const {
    return _panY;
}

float ZoomController::getZoom() const {
    return _zoom;
}

float ZoomController::getZoomX() const {
    if (_fillMode) {
        return std::max(_zoom, _zoom * _aspect);
    } else {
        return std::min(_zoom, _zoom * _aspect);
    }
    
}

float ZoomController::getZoomY() const {
    if (_fillMode) {
        return std::max(_zoom, _zoom / _aspect);
    } else {
        return std::min(_zoom, _zoom / _aspect);
    }
}

void ZoomController::setPanX(const float panX) {
    _panX = panX;
}

void ZoomController::setPanY(const float panY) {
    _panY = panY;
}

void ZoomController::setZoom(const float zoom) {
    _zoom = zoom;
}

void ZoomController::setViewSize(const cocos2d::Size& viewSize) {
    _viewSize = viewSize;
}

void ZoomController::updateAspect(const cocos2d::Size& viewSize, const cocos2d::Size& contentSize, const bool fillMode) {
    _aspect = (contentSize.width / contentSize.height) / (viewSize.width / viewSize.height);
    setViewSize(viewSize);
    
    _fillMode = fillMode;
}

void ZoomController::zoom(const float zoom, const float panX, const float panY) {
    float prevZoomX = getZoomX();
    float prevZoomY = getZoomY();
    
    float deltaZoom = zoom - getZoom();
    
    if ((zoom > MAX_ZOOM && deltaZoom > 0) || (zoom < MIN_ZOOM && deltaZoom < 0)) {
        deltaZoom *= ZOOM_OUTSIDE_SNAP_FACTOR;
    }
    
    setZoom(zoom);
    limitZoom();
    
    _zoomDynamics->setState(getZoom(), 0, cocos2d::Director::getInstance()->getGlobalTime());
    
    float newZoomX = getZoomX();
    float newZoomY = getZoomY();
    
    setPanX(getPanX() + (panX - 0.5f) * (1.0f / prevZoomX - 1.0f / newZoomX));
    setPanY(getPanY() + (panY - 0.5f) * (1.0f / prevZoomY - 1.0f / newZoomY));
    
    updatePanLimits();
}

void ZoomController::zoomImmediate(const float zoom, const float panX, const float panY) {
    setZoom(zoom);
    setPanX(panX);
    setPanY(panY);
    
    _zoomDynamics->setState(zoom, 0, cocos2d::Director::getInstance()->getGlobalTime());
    _panDynamicsY->setState(panY, 0, cocos2d::Director::getInstance()->getGlobalTime());
    _panDynamicsX->setState(panX, 0, cocos2d::Director::getInstance()->getGlobalTime());
    
    updatePanLimits();
}

void ZoomController::pan(float dx, float dy) {
    
    dx /= getZoomX();
    dy /= getZoomY();
    
    if ((getPanX() > _panMaxX && dx > 0) || (getPanX() < _panMinX && dx < 0)) {
        dx *= PAN_OUTSIDE_SNAP_FACTOR;
    }
    if ((getPanY() > _panMaxY && dy > 0) || (getPanY() < _panMinY && dy < 0)) {
        dy *= PAN_OUTSIDE_SNAP_FACTOR;
    }
    
    float newPanX = getPanX() + dx;
    float newPanY = getPanY() + dy;
    
    setPanX(newPanX);
    setPanY(newPanY);
}

bool ZoomController::update() {
    if (_needUpdate) {
        float nowTime = cocos2d::Director::getInstance()->getGlobalTime();
        _panDynamicsX->update(nowTime);
        _panDynamicsY->update(nowTime);
        _zoomDynamics->update(nowTime);
        
        bool isAtRest =
        _panDynamicsX->isAtRest(REST_VELOCITY_TOLERANCE, REST_POSITION_TOLERANCE, _viewSize.width) &&
        _panDynamicsY->isAtRest(REST_VELOCITY_TOLERANCE, REST_POSITION_TOLERANCE, _viewSize.height) &&
        _zoomDynamics->isAtRest(REST_VELOCITY_TOLERANCE, REST_ZOOM_TOLERANCE, 1);
        
        setPanX(_panDynamicsX->getPosition());
        setPanY(_panDynamicsY->getPosition());
        setZoom(_zoomDynamics->getPosition());
        
        if (isAtRest) {
            if (std::abs(MIN_ZOOM - getZoom()) < REST_ZOOM_TOLERANCE) {
                setZoom(MIN_ZOOM);
                _zoomDynamics->setState(MIN_ZOOM, 0, 0);
            }
            stopFling();
        }
        updatePanLimits();
    }
    
    return _needUpdate;
}

void ZoomController::startFling(float vx, float vy) {
    float now = cocos2d::Director::getInstance()->getGlobalTime();
    
    _panDynamicsX->setState(getPanX(), vx / getZoomX(), now);
    _panDynamicsY->setState(getPanY(), vy / getZoomY(), now);
    
    _panDynamicsX->setMinPosition(_panMinX);
    _panDynamicsX->setMaxPosition(_panMaxX);
    _panDynamicsY->setMinPosition(_panMinY);
    _panDynamicsY->setMaxPosition(_panMaxY);
    
    _needUpdate = true;
}

void ZoomController::stopFling() {
    _needUpdate = false;
}

float ZoomController::getMaxPanDelta(float zoom) {
    return std::max(0.0f, .5f * ((zoom - 1) / zoom));
}

void ZoomController::limitZoom() {
    
    if (getZoom() < MIN_ZOOM-0.3) {
        setZoom(MIN_ZOOM-0.3);
    } else if (getZoom() > MAX_ZOOM) {
        setZoom(MAX_ZOOM);
    }
}

void ZoomController::updatePanLimits() {
    float zoomX = getZoomX();
    float zoomY = getZoomY();
    
    _panMinX = .5f - getMaxPanDelta(zoomX);
    _panMaxX = .5f + getMaxPanDelta(zoomX);
    _panMinY = .5f - getMaxPanDelta(zoomY);
    _panMaxY = .5f + getMaxPanDelta(zoomY);
}

cocos2d::Vec2 ZoomController::computePanPosition(const float zoom, const cocos2d::Vec2& pivot) {
    
    float zoomX = std::min(zoom, getZoomX());
    float zoomY = std::min(zoom, getZoomY());
    
    float panMinX = .5f - getMaxPanDelta(zoomX);
    float panMaxX = .5f + getMaxPanDelta(zoomX);
    float panMinY = .5f - getMaxPanDelta(zoomY);
    float panMaxY = .5f + getMaxPanDelta(zoomY);
    
    float x = std::max(.0f, std::min(1.f, pivot.x));
    float y = std::max(.0f, std::min(1.f, pivot.y));
    
    cocos2d::Vec2 pan;
    
    pan.x = _panX + (x - 0.5f) * (1.0f / getZoomX() - 1.0f / zoomX);
    pan.y = _panY + (y - 0.5f) * (1.0f / getZoomY() - 1.0f / zoomY);
    
    pan.x = std::min(panMaxX, std::max(panMinX, pan.x));
    pan.y = std::min(panMaxY, std::max(panMinY, pan.y));
    
    return pan;
}

void ZoomController::updateLimits() {
    limitZoom();
    updatePanLimits();
}

bool ZoomController::isPanning() const {
    return _needUpdate;
}

