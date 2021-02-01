//
//  SMMapViewImpl-ios.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 8. 29..
//

#ifndef SMMapViewImpl_ios_h
#define SMMapViewImpl_ios_h

#include <iosfwd>
#include <stdint.h>
#include <platform/CCImage.h>

@class MKMapViewWrapper;


class SMMapView;

class SMMapViewImpl
{
public:
    SMMapViewImpl(SMMapView * mapView);
    virtual ~SMMapViewImpl();
    
    void setLocation(float latitude, float longitude);
    void setDistanceLevel(float level);
    void setDrawvisible(bool visible);

    virtual void draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags);
    virtual void setVisible(bool visible);
    
    
    cocos2d::Image * captureMapView();
    
private:
    MKMapViewWrapper * _mkMapViewWrapper;
    SMMapView * _mapView;
    cocos2d::Image * _captureImage;
    
};


#endif /* SMMapViewImpl_ios_h */
