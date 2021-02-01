//
//  SMZoomView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 9..
//
//

#ifndef SMZoomView_h
#define SMZoomView_h

#include "SMView.h"

class ZoomController;
class VelocityTracker;
class SMZoomView;

typedef std::function<void(SMZoomView * , const float, const float)> ZoomCallback;

class ZoomListener
{
public:
    virtual void onZoomScaleChange(SMZoomView * zoomView, const float initScale, const float currentScale) = 0;
};

class SMZoomView : public _UIContainerView
{
public:
    enum FillType {
        INSIDE, //default... 뷰 안쪽으로 맞춤
        FILL,   // 뷰에 꽉 채움
    };
    
    CREATE_VIEW(SMZoomView);
    
    void setPanEnable(bool enable);
    void setZoomEnable(bool enable);
    bool isIdle();
    bool isPanning();
  
    cocos2d::Node* getContentNode();
    void refreshContentNode(const bool reset=true);
    // 줌뷰의 메인 컨텐츠... 지도 또는 배경 뷰 등, 직접 확대/축소 되는 뷰를 여기에 세팅
    void setContentNode(cocos2d::Node* node);
    float getContentZoomScale();
    float getContentBaseScale();
    virtual void setContentSize(const cocos2d::Size& size) override;
    const cocos2d::Vec2& getContentPosition() const;
    float getZoom();
    float getPanX();
    float getPanY();
    void setState(float zoom, float panX, float panY);
    virtual void setPadding(const float padding) override;
    virtual void setPadding(const float left, const float top, const float right, const float bottom) override;
    virtual void performDoubleClick(const cocos2d::Vec2& worldPoint) override;
    void setFocusRect(const cocos2d::Rect& focusRect, const float duration);
    cocos2d::Rect getReverseFocusRect(const cocos2d::Rect& focusRect);
    void setZoomWithAnimation(const float panX, const float panY, const float zoom, const float duration=SMViewConstValue::Config::ZOOM_NORMAL_TIME);
    void setAdditionScale(float _s=1.0f);
    void setFillType(const FillType type);
    virtual bool containsPoint(const cocos2d::Vec2& point) override;
    
    void updateZoom();
    
    void setZoomListener(ZoomListener * l) {_zoomListener = l;}
    
    ZoomCallback _zoomCallback;
    
protected:
    SMZoomView();
    virtual ~SMZoomView();
    
    virtual bool init() override;
    virtual int dispatchTouchEvent(MotionEvent* event, SMView* view, bool checkBounds) override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    virtual void onUpdateOnVisit() override;
    
    ZoomController* getController() {return _controller;}
    
private:
    float computeBaseZoom(const cocos2d::Size& viewSize, const cocos2d::Size& contentSize);
    
private:
    enum class Mode {
        UNDEFINED,
        PAN,
        ZOOM
    };
    
    Mode _mode;
    cocos2d::Node* _contentNode;
    ZoomController* _controller;
    VelocityTracker* _velocityTracker;
    
    float _panX;
    float _panY;
    float _zoom;
    
    float _baseZoom;
    float _prevZoom;
    float _prevDistance;

    float _prevTouchX;
    float _prevTouchY;
    float _initTouchX;
    float _initTouchY;
    
    bool _panEnable;
    bool _zoomEnable;
    bool _interpolate;
    
    int _touchPointerIndex;
    
    float _accuX, _accuY;
    
    FillType _fillType;
    
    static cocos2d::Vec2 _midPoint;
    
    cocos2d::Size _innerSize;
    
    class ZoomTo;
    
    friend class ZoomTo;
    float _initScale;
    
    ZoomListener * _zoomListener;
    
};

#endif /* SMZoomView_h */
