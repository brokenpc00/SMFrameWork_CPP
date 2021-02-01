//
//  SMMapViewImpl-Android.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 8. 29..
//

#ifndef SMMapViewImpl_Android_h
#define SMMapViewImpl_Android_h
#include <platform/CCPlatformConfig.h>

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include <iosfwd>
#include <stdint.h>
#include <platform/CCImage.h>
#include <cocos/renderer/CCRenderer.h>

class SMMapView;

class SMMapViewImpl
{
public:
    SMMapViewImpl(SMMapView * mapView);
    virtual ~SMMapViewImpl();
    
    void setLocation(float latitude, float longitude);
    void setDistanceLevel(float level);

    virtual void draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags);
    virtual void setVisible(bool visible);
    
//    cocos2d::Image * captureMapView();
    typedef std::function<void(cocos2d::Image*)> ccCapturedCallback;
    ccCapturedCallback _captureCallback;
    void captureMapView(ccCapturedCallback);
    
    
    // JNI에서 받을 3가지 method
    static void onMapLoaded(const int viewTag);
    static void onMapReady(const int viewTag);
    static void onCapturedMap(const int viewTag, cocos2d::Image* img);

private:
    int _viewTag;
    SMMapView * _mapView;
};


#endif


#endif /* SMMapViewImpl_Android_h */
