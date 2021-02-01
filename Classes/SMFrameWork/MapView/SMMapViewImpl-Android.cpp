//
//  SMMapViewImpl-Android.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 8. 29..
//

#include "SMMapViewImpl-Android.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include <unordered_map>
#include <stdlib.h>
#include <string>

#include <platform/android/jni/JniHelper.h>
#include <jni.h>

#include "SMMapView.h"
#include <platform/CCGLView.h>
#include <base/CCDirector.h>
#include <platform/CCFileUtils.h>
#include <ui/UIHelper.h>

static const std::string className = "org/cocos2dx/lib/Cocos2dxMapViewHelper";

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"",__VA_ARGS__)

extern "C" {

    // 여기다가 받은 이벤트...
    // android google map에서 쓸만한거 3개 있음. OnMapLoadedCallback, OnMapReadyCallback, SnapshotReadyCallback
    // OnMapLoadedCallback : void onMapLoaded();
    // OnMapReadyCallback : void onMapReady(GoogleMap var1);
    // SnapshotReadyCallback : void onSnapshotReady(Bitmap var1);
    // 3개에 매핑되는 메소드를 만들어서 받자...
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxMapViewHelper_onMapLoaded(JNIEnv *env, jclass, jint index) {
        SMMapViewImpl::onMapLoaded(index);
    }
    
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxMapViewHelper_onMapReady(JNIEnv *env, jclass, jint index) {
        SMMapViewImpl::onMapReady(index);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxMapViewHelper_onSnapshotReady(JNIEnv *env, jclass, jint index, jbyteArray imageBytes) {
        
        auto srcImage = new cocos2d::Image();
        
        int len  = env->GetArrayLength(imageBytes );
        unsigned char * bytes = (unsigned char*)malloc(sizeof(unsigned char)*len);
        
        jbyte* imgData = env->GetByteArrayElements(imageBytes, NULL);
        if (imgData == NULL) {
            CCLOG("[[[[[ getting image is null ");
            //
            return;
        }
        
        memcpy(bytes, imgData, len);
        env->ReleaseByteArrayElements(imageBytes, imgData, JNI_ABORT);
        
        
        srcImage->initWithImageData(bytes, len);
        free(bytes);
        srcImage->autorelease();
        
        SMMapViewImpl::onCapturedMap(index, srcImage);
    }
    
    
}


/*
// 보내는 method
// createMapView
// removeMapView
// setLocation
// setDistanceLevel
// setMapViewRect
// setVisible
// captureMapView
 */

int createMapViewJNI()
{
    cocos2d::JniMethodInfo t;
    if (cocos2d::JniHelper::getStaticMethodInfo(t, className.c_str(), "createMapView", "()I")) {
        // LOGD("error: %s,%d",__func__,__LINE__);
        jint viewTag = t.env->CallStaticIntMethod(t.classID, t.methodID);
        t.env->DeleteLocalRef(t.classID);
        return viewTag;
    }
    return -1;
}

static std::unordered_map<int, SMMapViewImpl*> s_MapViewImpls;

SMMapViewImpl::SMMapViewImpl(SMMapView *mapView) : _viewTag(-1), _mapView(mapView), _captureCallback(nullptr) {
    _viewTag = createMapViewJNI();
    s_MapViewImpls[_viewTag] = this;
}

SMMapViewImpl::~SMMapViewImpl()
{
    cocos2d::JniHelper::callStaticVoidMethod(className, "removeMapView", _viewTag);
    s_MapViewImpls.erase(_viewTag);
}


void SMMapViewImpl::setLocation(float latitude, float longitude)
{
    cocos2d::JniHelper::callStaticVoidMethod(className, "setLocation", _viewTag, latitude, longitude);
}

void SMMapViewImpl::setDistanceLevel(float level)
{
    cocos2d::JniHelper::callStaticVoidMethod(className, "setDistanceLevel", _viewTag, level);
}

void SMMapViewImpl::draw(cocos2d::Renderer *renderer, cocos2d::Mat4 const &transform, uint32_t flags)
{
    if (flags & cocos2d::Node::FLAGS_TRANSFORM_DIRTY) {
        auto uiRect = cocos2d::ui::Helper::convertBoundingBoxToScreen(_mapView);
        cocos2d::JniHelper::callStaticVoidMethod(className, "setMapViewRect", _viewTag,
                                        (int)uiRect.origin.x, (int)uiRect.origin.y,
                                        (int)uiRect.size.width, (int)uiRect.size.height);
    }
}

void SMMapViewImpl::setVisible(bool visible)
{
    cocos2d::JniHelper::callStaticVoidMethod(className, "setVisible", _viewTag, visible);
}

void SMMapViewImpl::captureMapView(ccCapturedCallback callback)
{
    _captureCallback = callback;

    cocos2d::JniHelper::callStaticVoidMethod(className, "captureMapView", _viewTag);
}

// static method
void SMMapViewImpl::onMapReady(const int viewTag)
{
    auto it = s_MapViewImpls.find(viewTag);
    if (it != s_MapViewImpls.end()) {
        auto mapView = it->second->_mapView;
        if (mapView->_mapViewDidFinishLoadingMap) {
            mapView->_mapViewDidFinishLoadingMap(mapView);
        }
    }
}

void SMMapViewImpl::onMapLoaded(const int viewTag)
{
    auto it = s_MapViewImpls.find(viewTag);
    if (it != s_MapViewImpls.end()) {
        auto mapView = it->second->_mapView;
        if (mapView->_mapViewDidFinishRenderingMap) {
            mapView->_mapViewDidFinishRenderingMap(mapView);
        }
    }
}

void SMMapViewImpl::onCapturedMap(const int viewTag, cocos2d::Image* img)
{
    auto it = s_MapViewImpls.find(viewTag);
    if (it != s_MapViewImpls.end()) {
        auto second = it->second;
        if (second) {
            auto mapViewImpl = dynamic_cast<SMMapViewImpl*>(second);
            if (mapViewImpl && mapViewImpl->_captureCallback) {
                mapViewImpl->_captureCallback(img);
            }
        }
    }
}


#endif

