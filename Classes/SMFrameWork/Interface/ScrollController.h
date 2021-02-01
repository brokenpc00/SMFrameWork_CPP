//
//  ScrollController.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 7..
//
//

#ifndef ScrollController_h
#define ScrollController_h
#include <base/ccTypes.h>

class Dynamics;

// 스크롤 컨트롤러
class ScrollController {
public:
    ScrollController();
    virtual ~ScrollController();
    
    void reset();
    
    float getPanY() const;
    
    void setPanY(const float panY, const bool force = false);
    
    void setViewSize(const float viewSize);
    
    void setScrollSize(const float scrollSize);
    
    void pan(float dy);
    
    bool update();
    
    void startFling(float vy);
    
    void stopFling();
    
    void updateLimits();
    
    bool isPanning() const;
    
    void stopIfExceedLimit();
    
    void setHangSize(float hangSize);
private:
    void updatePanLimits();
    
private:
    static const float REST_VELOCITY_TOLERANCE;
    static const float REST_POSITION_TOLERANCE;
    static const float PAN_OUTSIDE_SNAP_FACTOR;
    
    Dynamics* _panDynamicsY;
    
    float _panMinY;
    float _panMaxY;
    bool _needUpdate;
    
    float _panY;
    
    float _viewSize;
    float _scrollSize;
    
    float _hangSize;
    
};

#endif /* ScrollController_h */
