//
//  ZoomController.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef ZoomController_h
#define ZoomController_h

#include <base/ccTypes.h>

class Dynamics;

class ZoomController
{
public:
    ZoomController();
    virtual ~ZoomController();
    
    void reset();
    
    float getPanX() const;
    
    float getPanY() const;
    
    float getZoom() const;
    
    float getZoomX() const;
    
    float getZoomY() const;
    
    void setPanX(const float panX);
    
    void setPanY(const float panY);
    
    void setZoom(const float zoom);
    
    void setViewSize(const cocos2d::Size& viewSize);
    
    void updateAspect(const cocos2d::Size& viewSize, const cocos2d::Size& contentSize, const bool fillMode);
    
    void zoom(const float zoom, const float panX, const float panY);
    
    void zoomImmediate(const float zoom, const float panX, const float panY);
    
    void pan(float dx, float dy);
    
    bool update();
    
    void startFling(float vx, float vy);
    
    void stopFling();
    
    void updateLimits();
    
    cocos2d::Vec2 computePanPosition(const float zoom, const cocos2d::Vec2& pivot);
    
    bool isPanning() const;
    
    
private:
    float getMaxPanDelta(float zoom);
    
    void limitZoom();
    
    void updatePanLimits();
    
private:
    static const float MIN_ZOOM;
    static const float MAX_ZOOM;
    static const float REST_VELOCITY_TOLERANCE;
    static const float REST_POSITION_TOLERANCE;
    static const float REST_ZOOM_TOLERANCE;
    static const float PAN_OUTSIDE_SNAP_FACTOR;
    static const float ZOOM_OUTSIDE_SNAP_FACTOR;
    
    Dynamics* _panDynamicsX;
    Dynamics* _panDynamicsY;
    Dynamics* _zoomDynamics;
    
    float _panMinX;
    float _panMaxX;
    float _panMinY;
    float _panMaxY;
    bool _needUpdate;
    
    float _aspect;
    
    float _zoom;
    float _panX;
    float _panY;
    
    bool _fillMode;
    
    cocos2d::Size _viewSize;
};

#endif /* ZoomController_h */
