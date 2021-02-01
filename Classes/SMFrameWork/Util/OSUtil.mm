//
//  OSUtil.m
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 20..
//

#include "OSUtil.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <Foundation/Foundation.h>
#include <Photos/Photos.h>
#include <CoreLocation/CoreLocation.h>
#include "../UI/AlertView.h"
#import <UserNotifications/UserNotifications.h>


static std::function<void(bool)> askCompleteBlock = nullptr;
static std::function<void(float, float)> getLocationCompleteBlock = nullptr;


@interface iOSUtil : NSObject <CLLocationManagerDelegate>
{
    
}

@property (nonatomic, strong) CLLocationManager * manager;
+ (instancetype) sharedInstance;
- (void)startLocation;
- (void)stopLocation;
- (void)requestGPSAuthorization;
- (void)showShareView:(const std::string&)linkUrl withText:(const std::string&)text;
@end

@implementation iOSUtil
+ (instancetype) sharedInstance
{
    static iOSUtil * instance = nil;
    if (instance==nil) {
        instance = [iOSUtil new];
    }
    return instance;
}
- (id)init
{
    if (self=[super init]) {
    }
    return self;
}

- (void)requestGPSAuthorization
{
    if (_manager) {
        [self stopLocation];
    }
    if (_manager==nil) {
        _manager = [CLLocationManager new];
    }
    _manager.delegate = self;
    [_manager requestWhenInUseAuthorization];

    [self startLocation];
}

- (void)startLocation
{
    if (_manager==nil) {
        _manager = [CLLocationManager new];
        _manager.delegate = self;
    }

    [_manager startUpdatingLocation];
}

- (void)stopLocation
{
    if (_manager) {
        [_manager stopUpdatingLocation];
    }
}

- (void)locationManager:(CLLocationManager *)manager
    didUpdateToLocation:(CLLocation *)newLocation
           fromLocation:(CLLocation *)oldLocation
{
    if ([newLocation horizontalAccuracy] < 0)
        return;
    
//    if (-[[newLocation timestamp] timeIntervalSinceNow] < 5.0)
//        NSLog([NSString stringWithFormat:
//               @"Altitude: %f Latitude: %f Longitude: %f"
//               @" Course: %f Speed: %f",
//               [newLocation altitude],
//               [newLocation coordinate].latitude,
//               [newLocation coordinate].longitude,
//               [newLocation course],
//               [newLocation speed]]);
    
    if (getLocationCompleteBlock) {
        getLocationCompleteBlock((float) [newLocation coordinate].latitude, (float) [newLocation coordinate].longitude);
    }
    
    [_manager stopUpdatingLocation];
    
}

-(void)locationManager:(CLLocationManager *)manager didUpdateLocations:(nonnull NSArray<CLLocation *> *)locations
{
    CLLocation *clLocation = [locations lastObject];
    
//    NSLog([NSString stringWithFormat:
//           @"Altitude: %f Latitude: %f Longitude: %f"
//           @" Course: %f Speed: %f",
//           clLocation.altitude,
//           clLocation.coordinate.latitude,
//           clLocation.coordinate.longitude,
//           clLocation.speed]);
    
    if (getLocationCompleteBlock) {
        getLocationCompleteBlock((float) clLocation.coordinate.latitude, (float) clLocation.coordinate.longitude);
    }
    
    [_manager stopUpdatingLocation];
}


-(void)locationManager:(CLLocationManager *)locationManager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    switch (status) {
        case kCLAuthorizationStatusAuthorizedAlways:
        case kCLAuthorizationStatusAuthorizedWhenInUse:
        {
            if (askCompleteBlock) {
                askCompleteBlock(true);
                askCompleteBlock = nullptr;
                [[iOSUtil sharedInstance] stopLocation];

            }
        }
            break;
        case kCLAuthorizationStatusNotDetermined:
        {
            // not yet;
        }
            break;
        default:
        {
            if (askCompleteBlock)
            {
                askCompleteBlock(false);
                askCompleteBlock = nullptr;
                [[iOSUtil sharedInstance] stopLocation];

            }
        }
            break;
    }
}

- (void)showShareView:(const std::string&)linkUrl withText:(const std::string&)text
{
    NSMutableArray* items = [NSMutableArray array];
    
    if (!linkUrl.empty()) {
        auto textObj = [NSString stringWithCString: linkUrl.c_str() encoding:NSUTF8StringEncoding];
        [items addObject: textObj];
    }
    
    if (!text.empty()) {
        auto textObj = [NSString stringWithCString: text.c_str() encoding:NSUTF8StringEncoding];
        [items addObject: textObj];
    }
    
    if ([items count] > 0) {
        
        UIActivityViewController* sharing = [[UIActivityViewController alloc] initWithActivityItems: items
                                                                              applicationActivities: nullptr];
        sharing.completionWithItemsHandler = ^(NSString* activityType, BOOL completed, NSArray* returnedItems, NSError* activityError) {
            
        };
        
        UIViewController* viewController = [UIApplication sharedApplication].keyWindow.rootViewController;
        
        [viewController.self presentViewController: sharing
                                          animated: true
                                        completion: nullptr];
        // 호출 성공
    } else {
        // 초기화 실패함.
        CCLOG("[[[[[ 공유 창 호출 실패");
    }
}

@end


static const std::string PLATFORM_NAME("ios");

// 이건 나중에
static const std::string MARKET_URL("itms://itunes.apple.com/us/app/apple-store/id1190169671?mt=8");

std::string OSUtil::getTargetName()
{
    NSString * plistName = [NSBundle mainBundle].infoDictionary[@"CFBundleName"];
    return [plistName UTF8String];
}

std::string OSUtil::getBundleID()
{
    NSString * plistName = [NSBundle mainBundle].infoDictionary[@"CFBundleIdentifier"];
    return [plistName UTF8String];
}



bool OSUtil::openURL(const std::string& url) {
    auto urlString = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    auto targetURL = [NSURL URLWithString:urlString];
//    openURL:options:completionHandler: instead
    UIApplication * app = [UIApplication sharedApplication];
    if ([app respondsToSelector:@selector(openURL:options:completionHandler:)]) {
        [app openURL:targetURL options:@{} completionHandler:^(BOOL success) { }];
        return true;
    } else {
        return [app openURL: targetURL];
    }
    
}

bool OSUtil::openDeviceSettings() {
    NSURL* settingsUrl = [NSURL URLWithString:UIApplicationOpenSettingsURLString];
    
    UIApplication * app = [UIApplication sharedApplication];
    if ([app respondsToSelector:@selector(openURL:options:completionHandler:)]) {
        [app openURL:settingsUrl options:@{} completionHandler:^(BOOL success) { }];
        return true;
    } else {
        return [app openURL: settingsUrl];
    }

    return [[UIApplication sharedApplication] openURL: settingsUrl];
}

std::string OSUtil::getAppDisplayName() {
    NSString* appDisplayName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"];
    
    return [appDisplayName UTF8String];
}

std::string OSUtil::getAppVersionName() {
    NSString* versionName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    if (versionName) {
        return [versionName UTF8String];
    }
    return std::string("0");
}

std::string OSUtil::getAppBuildVersion() {
    NSString* buildVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
    
    return [buildVersion UTF8String];
}

std::string OSUtil::getCountryCode() {
    NSLocale* locale = [NSLocale currentLocale];
    NSString* countryCode = [locale objectForKey:NSLocaleCountryCode];
    
    return [countryCode UTF8String];
}

std::string OSUtil::getLanguageCode() {
    NSLocale* locale = [NSLocale currentLocale];
    NSString* languageCode = [locale objectForKey:NSLocaleLanguageCode];
    
    return [languageCode UTF8String];
}

const std::string& OSUtil::getPlatform() {
    return PLATFORM_NAME;
}

bool OSUtil::openAppMarket() {
    return openURL(MARKET_URL);
}

void OSUtil::setTintColor(const cocos2d::Color4F& tintColor) {
    [[UITextField appearance] setTintColor:[UIColor colorWithRed:tintColor.r green:tintColor.g blue:tintColor.b alpha:tintColor.a]];
    [[UITextView appearance] setTintColor:[UIColor colorWithRed:tintColor.r green:tintColor.g blue:tintColor.b alpha:tintColor.a]];
}


std::vector<std::string> OSUtil::getSystemFontList() {
    
    std::vector<std::string> items;
    /*
     for (NSString* familyName in [UIFont familyNames]) {
     //        NSLog(@"Family name: %@", familyName);
     for (NSString* fontName in [UIFont fontNamesForFamilyName:familyName]) {
     items.push_back([fontName UTF8String]);
     }
     }
     
     for (auto& item : items) {
     CCLOG("%s", item.c_str());
     }
     */
    
    
    return items;
}

bool OSUtil::isPushNotificationEnabled() {
    
    if ([UNUserNotificationCenter class] != nil) {
        std::unique_lock<std::mutex> lock(_mutex);
        _pushEnable = false;
        UNUserNotificationCenter * center = [UNUserNotificationCenter currentNotificationCenter];
        [center requestAuthorizationWithOptions:UNAuthorizationOptionAlert completionHandler:^(BOOL granted, NSError * _Nullable error) {
            // push alert 이 허용 되어있냐??
            _pushEnable = granted;
            _cond.notify_one();
        }];
        
        _cond.wait(lock);
        return _pushEnable;
    } else {
    UIUserNotificationType type = [[[UIApplication sharedApplication] currentUserNotificationSettings] types];
    
        if (type == UIUserNotificationTypeNone) {
            return false;
        } else {
            return true;
        }
    }
    
}

// 해당 접근 권한이 있는지 체크, apple의 경우 파일은 앨범이다.
bool OSUtil::canAcceessibleFile()
{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    return (status == PHAuthorizationStatusAuthorized);// || status == PHAuthorizationStatusNotDetermined);
}

bool OSUtil::canAcceessibleCamera()
{
    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
    
    return (status == AVAuthorizationStatusAuthorized);// || status == AVAuthorizationStatusNotDetermined);
}

bool OSUtil::canAcceessibleGPS()
{
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    return (status == kCLAuthorizationStatusAuthorizedAlways || status == kCLAuthorizationStatusAuthorizedWhenInUse);// || status == kCLAuthorizationStatusNotDetermined);
}

bool OSUtil::canAcceessiblePhoneCall()
{
    return false;
}

bool OSUtil::canAcceessibleContact()
{
    return false;
}

// 해당 접근 권한을 부여할 것인지 물어보자
// 이미 거부한 경우는 설정으로 보내자
void OSUtil::askAccessPermissionFile(const std::function<void(bool)>& completeBlock)
{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    if (status == PHAuthorizationStatusDenied || status == PHAuthorizationStatusRestricted) {
        // 거부되었으니 종료하든가 설정으로 보내든가
        completeBlock(false);
    } else if (status == PHAuthorizationStatusNotDetermined) {
        askCompleteBlock = completeBlock;
        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
            switch (status) {
                case PHAuthorizationStatusAuthorized:
                {
                    if (askCompleteBlock) {
                        askCompleteBlock(true);
                        askCompleteBlock = nullptr;
                    }
                }
                    break;
                default:
                case PHAuthorizationStatusRestricted:
                case PHAuthorizationStatusDenied:
                {
                    if (askCompleteBlock) {
                        askCompleteBlock(false);
                        askCompleteBlock = nullptr;
                    }
                }
                    break;
            }
        }];
    }
}

void OSUtil::askAccessPermissionCamera(const std::function<void(bool)>& completeBlock)
{
    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
    
    if (status == AVAuthorizationStatusDenied ||
        status == AVAuthorizationStatusRestricted) {
        // notify listener
        completeBlock(false);
    } else if(status == AVAuthorizationStatusNotDetermined) {
        askCompleteBlock = completeBlock;
        [ AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler: ^(BOOL granted) {
            if (granted) {
                if (askCompleteBlock) {
                    askCompleteBlock(true);
                    askCompleteBlock = nullptr;
                }
            } else {
                if (askCompleteBlock) {
                    askCompleteBlock(false);
                    askCompleteBlock = nullptr;
                }
            }
        } ];
    }
}

void OSUtil::askAccessPermissionGPS(const std::function<void(bool)>& completeBlock)
{
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    if (status == kCLAuthorizationStatusDenied ||
        status == kCLAuthorizationStatusRestricted) {
        // 이미 거부됨
        completeBlock(false);
    } else if (status == kCLAuthorizationStatusNotDetermined) {
        // 물어봐도 되는 상태이면(아직 결정 안했으면) 물어보자
        askCompleteBlock = completeBlock;
//        
//        askCompleteBlock = [completeBlock](bool bSuccess) {
//            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                askCompleteBlock(bSuccess);
//                [[iOSUtil sharedInstance] stopLocation];
//                askCompleteBlock = nullptr;
//            });
//        };

        [[iOSUtil sharedInstance] requestGPSAuthorization];
    }
    
}

void OSUtil::askAccessPermissionPhoneCall(const std::function<void(bool)>& completeBlock)
{
    
}

void OSUtil::askAccessPermissionContact(const std::function<void(bool)>& completeBlock)
{
    
}

void OSUtil::getLocationInfo(const std::function<void(float, float, std::string)>& completeBlock, bool bWithAddress)
{
    if (OSUtil::getInstance()->getAddress()!="") {
        completeBlock(OSUtil::getInstance()->getLatitude(), OSUtil::getInstance()->getLongitude(), OSUtil::getInstance()->getAddress());
        return;
    }
    if (OSUtil::canAcceessibleGPS()) {
        
        getLocationCompleteBlock = [completeBlock, bWithAddress](float latitude, float longitude) {
//            CLLocationCoordinate2D coordinate;
//            coordinate.latitude = latitude;
//            coordinate.longitude = longitude;
            if (OSUtil::getInstance()->getLocationOnceFlag()==true) {
                return;
            }
            OSUtil::getInstance()->setLocationOnceFlag(true);
            [[iOSUtil sharedInstance] stopLocation];

            if (bWithAddress) {
                CLLocation * loc = [[CLLocation alloc] initWithLatitude:latitude longitude:longitude];
                CLGeocoder * ceo = [[CLGeocoder alloc] init];
                [ceo reverseGeocodeLocation:loc completionHandler:^(NSArray<CLPlacemark *> * _Nullable placemarks, NSError * _Nullable error) {
                    CLPlacemark *placemark = [placemarks objectAtIndex:0];
                    //                NSLog(@"[[[[[ placemark %@",placemark);
                    NSString *locatedAt = [[placemark.addressDictionary valueForKey:@"FormattedAddressLines"] componentsJoinedByString:@", "];
                    //                NSLog(@"[[[[[ addressDictionary %@", placemark.addressDictionary);
                    //
                    //                NSLog(@"[[[[[ placemark %@",placemark.region);
                    //                NSLog(@"[[[[[ placemark %@",placemark.country);  // Give Country Name
                    //                NSLog(@"[[[[[ placemark %@",placemark.locality); // Extract the city name
                    //                NSLog(@"[[[[[ location %@",placemark.name);
                    //                NSLog(@"[[[[[ location %@",placemark.ocean);
                    //                NSLog(@"[[[[[ location %@",placemark.postalCode);
                    //                NSLog(@"[[[[[ location %@",placemark.subLocality);
                    //
                    //                NSLog(@"[[[[[ location %@",placemark.location);
                    //                //Print the location to console
                    //                NSLog(@"[[[[[ I am currently at %@",locatedAt);
                    //
                    //                NSString * city = [placemark.addressDictionary objectForKey:@"City"];
                    //                NSLog(@"[[[[[ my city : %@", city);
                    
                    getLocationCompleteBlock = nullptr;
                    OSUtil::getInstance()->setLocationOnceFlag(false);
                    OSUtil::getInstance()->_latitude = latitude;
                    OSUtil::getInstance()->_longitude = longitude;
                    OSUtil::getInstance()->_address = std::string(locatedAt.UTF8String);
                    
                    completeBlock(latitude, longitude, OSUtil::getInstance()->_address);

                }];
                
            } else {
                getLocationCompleteBlock = nullptr;
                OSUtil::getInstance()->setLocationOnceFlag(false);
                OSUtil::getInstance()->_latitude = latitude;
                OSUtil::getInstance()->_longitude = longitude;
                OSUtil::getInstance()->_address = "";
                completeBlock(OSUtil::getInstance()->_latitude, OSUtil::getInstance()->_longitude, OSUtil::getInstance()->_address);
            }
        };
        
        [[iOSUtil sharedInstance] startLocation];
    }
}

void OSUtil::showShareView(const std::string &linkUrl, const std::string &text)
{
    [[iOSUtil sharedInstance]  showShareView:linkUrl withText:text];
}

#endif
