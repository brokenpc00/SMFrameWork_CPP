//
//  SMMapView-ini.h
//  ECheckForm
//
//  Created by SteveMac on 2018. 8. 29..
//

#ifndef SMMapView_ini_h
#define SMMapView_ini_h

#include "SMMapView.h"
#include <platform/CCGLView.h>
#include <base/CCDirector.h>
#include <platform/CCFileUtils.h>
#include <platform/CCPlatformConfig.h>

#include <cocos2d.h>

//ccMapViewCallback mapViewWillStartRenderingMap;
//ccMapViewCallback mapViewWillStartLoadingMap;
//ccMapViewCallback mapViewDidFinishLoadingMap;
//ccMapViewCallback mapViewDidFinishRenderingMap;

SMMapView::SMMapView() : _impl(new SMMapViewImpl(this))
, _mapViewWillStartRenderingMap(nullptr)
, _mapViewWillStartLoadingMap(nullptr)
, _mapViewDidFinishLoadingMap(nullptr)
, _mapViewDidFinishRenderingMap(nullptr)
, _mapViewDidFailLoadingMap(nullptr)
{
    
}

SMMapView::~SMMapView()
{
    CC_SAFE_DELETE(_impl);
}

SMMapView * SMMapView::create()
{
    auto mapView = new (std::nothrow)SMMapView();
    if (mapView && mapView->init()) {
        mapView->autorelease();
        return mapView;
    }
    
    CC_SAFE_DELETE(mapView);
    return nullptr;
}

void SMMapView::setLocation(float latitude, float longitude)
{
    _impl->setLocation(latitude, longitude);
}

void SMMapView::setDistanceLevel(float level)
{
    _impl->setDistanceLevel(level);
}

void SMMapView::draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags)
{
    cocos2d::ui::Widget::draw(renderer, transform, flags);
    _impl->draw(renderer, transform, flags);
}

void SMMapView::setVisible(bool visible)
{
    Node::setVisible(visible);
    if (!visible || isRunning())
    {
        _impl->setVisible(visible);
    }
}

void SMMapView::onEnter()
{
    Widget::onEnter();
    if(isVisible())
    {
        _impl->setVisible(true);
    }
}

void SMMapView::onExit()
{
    Widget::onExit();
    _impl->setVisible(false);
}

cocos2d::ui::Widget* SMMapView::createCloneInstance()
{
    return SMMapView::create();
}


void SMMapView::captureMapView(const ccCapturedCallback& callback)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    _impl->captureMapView([&](cocos2d::Image* img){
        callback(img);
    });
#else
    auto capturedImage = _impl->captureMapView();
    callback(capturedImage);
#endif
}


void SMMapView::setMapViewWillStartRenderingMap(const ccMapViewCallback& callback)
{
    _mapViewWillStartRenderingMap = callback;
}

void SMMapView::setMapViewWillStartLoadingMap(const ccMapViewCallback& callback)
{
    _mapViewWillStartLoadingMap = callback;
}

void SMMapView::setMapViewDidFinishLoadingMap(const ccMapViewCallback& callback)
{
    _mapViewDidFinishLoadingMap = callback;
}

void SMMapView::setMapViewDidFinishRenderingMap(const ccMapViewCallback& callback)
{
    _mapViewDidFinishRenderingMap = callback;
}

void SMMapView::setMapViewDidFailLoadingMap(const ccMapViewCallback& callback)
{
    _mapViewDidFailLoadingMap = callback;
}

void SMMapView::setDrawvisible(bool visible)
{
//    _impl->setDrawvisible(visible);
}

#endif /* SMMapView_ini_h */
