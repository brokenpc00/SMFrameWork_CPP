//
//  SMMapView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 8. 29..
//

#ifndef SMMapView_h
#define SMMapView_h

#include <platform/CCPlatformConfig.h>
#include <ui/UIWidget.h>
#include <ui/GUIExport.h>
#include <base/CCData.h>

#include <platform/CCImage.h>


class SMMapViewImpl;

class CC_GUI_DLL SMMapView : public cocos2d::ui::Widget
{
public:
    static SMMapView * create();
    
    void setLocation(float latitude, float longitude);
    void setDistanceLevel(float level);
    void setDrawvisible(bool visible);
    
    
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void setVisible(bool visible) override;
    virtual void draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags) override;
    
    typedef std::function<void(cocos2d::Image*)> ccCapturedCallback;
    
    void captureMapView(const ccCapturedCallback& callback=nullptr);
    
    ccCapturedCallback _captureCallback;
    
    void setCaptureWebPageCallback(const ccCapturedCallback& callback) {_captureCallback = callback;}
    
    typedef std::function<void(SMMapView *sender)> ccMapViewCallback;
    
    void setMapViewWillStartRenderingMap(const ccMapViewCallback& callback);
    void setMapViewWillStartLoadingMap(const ccMapViewCallback& callback);
    void setMapViewDidFinishLoadingMap(const ccMapViewCallback& callback);
    void setMapViewDidFinishRenderingMap(const ccMapViewCallback& callback);
    void setMapViewDidFailLoadingMap(const ccMapViewCallback& callback);
    
protected:
    ccMapViewCallback _mapViewWillStartRenderingMap;
    ccMapViewCallback _mapViewWillStartLoadingMap;
    ccMapViewCallback _mapViewDidFinishLoadingMap;
    ccMapViewCallback _mapViewDidFinishRenderingMap;
    ccMapViewCallback _mapViewDidFailLoadingMap;
    
protected:
    virtual cocos2d::ui::Widget* createCloneInstance() override;
//    virtual void copySpecialProperties(Widget* model) override;

protected:
    
CC_CONSTRUCTOR_ACCESS:
    SMMapView();
    virtual ~SMMapView();
    
private:
    SMMapViewImpl * _impl;
    friend class SMMapViewImpl;
};


#endif /* SMMapView_h */
