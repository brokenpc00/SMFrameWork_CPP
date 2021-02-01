//
//  SMMapViewImpl-ios.mm
//  SMFrameWork
//
//  Created by SteveMac on 2018. 8. 29..
//

#include "SMMapViewImpl-ios.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <renderer/CCRenderer.h>
#include <base/CCDirector.h>
#include <platform/CCGLView.h>
#include <platform/ios/CCEAGLView-ios.h>
#include <platform/CCFileUtils.h>
#include "SMMapView.h"
#import <MapKit/MapKit.h>
#import <CoreLocation/CoreLocation.h>

static std::function<void()> g_mapViewWillStartRenderingMap = nullptr;
static std::function<void()> g_mapViewWillStartLoadingMap = nullptr;
static std::function<void()> g_mapViewDidFinishLoadingMap = nullptr;
static std::function<void()> g_mapViewDidFinishRenderingMap = nullptr;
static std::function<void()> g_mapViewDidFailLoadingMap = nullptr;

@interface MyPin : NSObject <MKAnnotation>
{
    
}
@property (nonatomic, strong) NSString * locationName;
@property (nonatomic, strong) NSString * address;
@property (nonatomic, assign) CLLocationCoordinate2D coordinate;
- (id)init:(NSString *)title locationName:(NSString*)locationName address:(NSString*)address coordinate:(CLLocationCoordinate2D)coordinate;
@end

@implementation MyPin
- (id)init:(NSString *)title locationName:(NSString*)locationName address:(NSString*)address coordinate:(CLLocationCoordinate2D)coordinate
{
    if (self = [super init]) {
        self.locationName = locationName;
        self.address = address;
        self.coordinate = coordinate;
    }
    
    return self;
}
@end

@interface MKMapViewWrapper : NSObject
+ (instancetype)mapViewWrapper;
@property (nonatomic, assign) std::function<void()> mapViewWillStartRenderingMap;
@property (nonatomic, assign) std::function<void()> mapViewWillStartLoadingMap;
@property (nonatomic, assign) std::function<void()> mapViewDidFinishLoadingMap;
@property (nonatomic, assign) std::function<void()> mapViewDidFinishRenderingMap;
@property (nonatomic, assign) std::function<void()> mapViewDidFailLoadingMap;

- (void)setVisible:(bool)visible;
- (void)setLocationLatitude:(CGFloat)latitude Longitude:(CGFloat)longitude;
- (void)setFrameWithX:(float)x y:(float)y width:(float)width height:(float)height;
- (UIImage*)captureMapView;
- (void)setDrawvisible:(BOOL)visible;
@end

@interface MKMapViewWrapper() <MKMapViewDelegate>
@property(nonatomic, retain) MKMapView *mkMapView;
@property(nonatomic, assign) CGFloat latitude;
@property(nonatomic, assign) CGFloat longitude;
@property(nonatomic, assign) CGFloat distance;
@end

@implementation MKMapViewWrapper

+ (instancetype)mapViewWrapper {
    return [[[self alloc] init] autorelease];
}

- (void)setupMapView
{
    // ios map view를 생성
    if (!self.mkMapView) {
        self.mkMapView = [[MKMapView alloc] init];
        self.mapViewDidFinishRenderingMap = nullptr;
        self.mapViewDidFinishLoadingMap = nullptr;
        self.mapViewWillStartLoadingMap = nullptr;
        self.mapViewWillStartRenderingMap = nullptr;
        self.mapViewDidFailLoadingMap = nullptr;
        self.mkMapView.delegate = self;
        self.distance = 300;
        [self setDrawvisible:NO];

        [self.mkMapView setShowsScale:NO];

        
//        auto scaleView = [MKScaleView scaleViewWithMapView:self.mkMapView];
//        [self.mkMapView addSubview:scaleView];
        
//        scaleView.scaleVisibility = MKFeatureVisibilityVisible;
    }
    
    // 어디 안 붙어 있으면 붙이자
    if (!self.mkMapView.superview) {
        auto view = cocos2d::Director::getInstance()->getOpenGLView();
        
        // ios viewcontroller에 addsubview 되어 있는 eaglview를 가져온다.
        auto eaglview = (CCEAGLView*)view->getEAGLView();
        
        // 붙인다.
        [eaglview addSubview:self.mkMapView];
    }
}

- (void)setDrawvisible:(BOOL)visible
{
    if (visible) {
        self.mkMapView.layer.opacity = 1;
    } else {
        self.mkMapView.layer.opacity = 0;
    }
}

- (void)setLocationLatitude:(CGFloat)latitude Longitude:(CGFloat)longitude
{
    if (!self.mkMapView) {[self setupMapView];}
    
    self.latitude = latitude;
    self.longitude = longitude;
    
    [self setMapAnnotation];
    
    MyPin * pin = [[MyPin alloc] init:@"SMFrameWork" locationName:@"SMFrameWork" address:@"SMFrameWork" coordinate:CLLocationCoordinate2DMake(self.latitude, self.longitude)];
    [self.mkMapView addAnnotation:pin];

    [self.mkMapView setUserInteractionEnabled:NO];
}

- (void)setFrameWithX:(float)x y:(float)y width:(float)width height:(float)height {
    if (!self.mkMapView) {[self setupMapView];}
    
    CGRect newFrame = CGRectMake(x, y, width, height);
    if (!CGRectEqualToRect(self.mkMapView.frame, newFrame)) {
        self.mkMapView.frame = CGRectMake(x, y, width, height);
    }
}

- (void)setMapAnnotation
{
    CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(self.latitude, self.longitude);
    MKCoordinateRegion reg = MKCoordinateRegionMakeWithDistance(coordinate, self.distance, self.distance);
    [self.mkMapView setRegion:reg];
}

- (void)setDistance:(CGFloat)distance
{
    _distance = distance;
    
    [self setMapAnnotation];
}

- (void)setVisible:(bool)visible
{
    self.mkMapView.hidden = !visible;
}

- (UIImage*)captureMapView
{
    CGFloat scale = [UIScreen mainScreen].scale;
    UIGraphicsBeginImageContextWithOptions(_mkMapView.bounds.size, YES, scale);
    [self.mkMapView.layer renderInContext:UIGraphicsGetCurrentContext()];
    
    UIImage * img = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    return img;
}


- (void)mapViewWillStartLoadingMap:(MKMapView *)mapView
{
    if (g_mapViewWillStartLoadingMap) {
        g_mapViewWillStartLoadingMap();
    }
//    if (self.mapViewWillStartLoadingMap) {
//        self.mapViewWillStartLoadingMap();
//    }
    CCLOG("[[[[[ mapViewWillStartLoadingMap");
}

- (void)mapViewDidFinishLoadingMap:(MKMapView *)mapView
{
    if (g_mapViewDidFinishLoadingMap) {
        g_mapViewDidFinishLoadingMap();
    }
//    if (self.mapViewDidFinishLoadingMap) {
//        self.mapViewDidFinishLoadingMap();
//    }
    CCLOG("[[[[[ mapViewDidFinishLoadingMap");
}

- (void)mapViewDidFailLoadingMap:(MKMapView *)mapView withError:(NSError *)error
{
    if (g_mapViewDidFailLoadingMap) {
        g_mapViewDidFailLoadingMap();
    }
//    if (self.mapViewDidFailLoadingMap) {
//        self.mapViewDidFailLoadingMap();
//    }
    CCLOG("[[[[[ mapViewDidFailLoadingMap");
}

- (void)mapViewWillStartRenderingMap:(MKMapView *)mapView
{
    if (g_mapViewWillStartRenderingMap) {
        g_mapViewWillStartRenderingMap();
    }
//    if (self.mapViewWillStartRenderingMap) {
//        self.mapViewWillStartRenderingMap();
//    }
    CCLOG("[[[[[ mapViewWillStartRenderingMap");
}

- (void)mapViewDidFinishRenderingMap:(MKMapView *)mapView fullyRendered:(BOOL)fullyRendered
{
    if (g_mapViewDidFinishRenderingMap) {
        g_mapViewDidFinishRenderingMap();
    }
//    if (self.mapViewDidFinishRenderingMap) {
//        self.mapViewDidFinishRenderingMap();
//    }
    CCLOG("[[[[[ mapViewDidFinishRenderingMap");
}


@end

SMMapViewImpl::SMMapViewImpl(SMMapView* mapView) : _mapView(mapView), _mkMapViewWrapper([MKMapViewWrapper mapViewWrapper])
{
    [_mkMapViewWrapper retain];

    g_mapViewWillStartRenderingMap = [this]{
        if (this->_mapView->_mapViewWillStartRenderingMap) {
            this->_mapView->_mapViewWillStartRenderingMap(this->_mapView);
        }
    };
    
    g_mapViewWillStartLoadingMap = [this]{
        if (this->_mapView->_mapViewWillStartLoadingMap) {
            this->_mapView->_mapViewWillStartLoadingMap(this->_mapView);
        }
    };
    
    g_mapViewDidFinishLoadingMap = [this]{
        if (this->_mapView->_mapViewDidFinishLoadingMap) {
            this->_mapView->_mapViewDidFinishLoadingMap(this->_mapView);
        }
    };
    
    g_mapViewDidFinishRenderingMap = [this]{
        if (this->_mapView->_mapViewDidFinishRenderingMap) {
            this->_mapView->_mapViewDidFinishRenderingMap(this->_mapView);
        }
    };
    
    g_mapViewDidFailLoadingMap = [this] {
        if (this->_mapView->_mapViewDidFailLoadingMap) {
            this->_mapView->_mapViewDidFailLoadingMap(this->_mapView);
        }
    };
    
//    _mkMapViewWrapper.mapViewWillStartRenderingMap = [this]{
//        if (this->_mapView->_mapViewWillStartRenderingMap) {
//            this->_mapView->_mapViewWillStartRenderingMap(this->_mapView);
//        }
//    };
//
//    _mkMapViewWrapper.mapViewWillStartLoadingMap = [this]{
//        if (this->_mapView->_mapViewWillStartLoadingMap) {
//            this->_mapView->_mapViewWillStartLoadingMap(this->_mapView);
//        }
//    };
//
//    _mkMapViewWrapper.mapViewDidFinishLoadingMap = [this]{
//        if (this->_mapView->_mapViewDidFinishLoadingMap) {
//            this->_mapView->_mapViewDidFinishLoadingMap(this->_mapView);
//        }
//    };
//
//    _mkMapViewWrapper.mapViewDidFinishRenderingMap = [this]{
//        if (this->_mapView->_mapViewDidFinishRenderingMap) {
//            this->_mapView->_mapViewDidFinishRenderingMap(this->_mapView);
//        }
//    };
//
//    _mkMapViewWrapper.mapViewDidFailLoadingMap = [this] {
//        if (this->_mapView->_mapViewDidFailLoadingMap) {
//            this->_mapView->_mapViewDidFailLoadingMap(this->_mapView);
//        }
//    };
    
}

SMMapViewImpl::~SMMapViewImpl()
{
    g_mapViewWillStartRenderingMap = nullptr;
    
    g_mapViewWillStartLoadingMap = nullptr;
    
    g_mapViewDidFinishLoadingMap = nullptr;
    
    g_mapViewDidFinishRenderingMap = nullptr;
    
    g_mapViewDidFailLoadingMap = nullptr;
    
    [_mkMapViewWrapper release];
    _mkMapViewWrapper = nullptr;
}

void SMMapViewImpl::setLocation(float latitude, float longitude)
{
    [_mkMapViewWrapper setLocationLatitude:latitude Longitude:longitude];
}

void SMMapViewImpl::setVisible(bool visible)
{
    [_mkMapViewWrapper setVisible:visible];
}

void SMMapViewImpl::setDistanceLevel(float level)
{
    [_mkMapViewWrapper setDistance:level];
}

void SMMapViewImpl::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    if (flags & cocos2d::Node::FLAGS_TRANSFORM_DIRTY) {
        
        auto director = cocos2d::Director::getInstance();
        auto glView = director->getOpenGLView();
        auto frameSize = glView->getFrameSize();
        
        auto scaleFactor = [static_cast<CCEAGLView *>(glView->getEAGLView()) contentScaleFactor];
        
        auto winSize = director->getWinSize();
        
        auto leftBottom = this->_mapView->convertToWorldSpace(cocos2d::Vec2::ZERO);
        auto rightTop = this->_mapView->convertToWorldSpace(cocos2d::Vec2(this->_mapView->getContentSize().width, this->_mapView->getContentSize().height));
        
        auto x = (frameSize.width / 2 + (leftBottom.x - winSize.width / 2) * glView->getScaleX()) / scaleFactor;
        auto y = (frameSize.height / 2 - (rightTop.y - winSize.height / 2) * glView->getScaleY()) / scaleFactor;
        auto width = (rightTop.x - leftBottom.x) * glView->getScaleX() / scaleFactor;
        auto height = (rightTop.y - leftBottom.y) * glView->getScaleY() / scaleFactor;
        
        [_mkMapViewWrapper setFrameWithX:x
                                       y:y
                                   width:width
                                  height:height];
    }
}

cocos2d::Image * SMMapViewImpl::captureMapView()
{
    UIImage * captureImage = [_mkMapViewWrapper captureMapView];
    NSData *imgData = UIImageJPEGRepresentation(captureImage, 1.0f);
    NSUInteger len = [imgData length];
    Byte *byteData = (Byte*)malloc(len);
    memcpy(byteData, [imgData bytes], len);
    auto newImage = new cocos2d::Image();
    newImage->initWithImageData(byteData,imgData.length);
    newImage->autorelease();
    
    return newImage;

}

void SMMapViewImpl::setDrawvisible(bool visible)
{
    [_mkMapViewWrapper setDrawvisible:visible];
}

#endif

