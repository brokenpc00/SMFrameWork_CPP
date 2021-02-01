//
//  OSUtil.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 20..
//


#include "OSUtil.h"
#include "StringUtil.h"

typedef std::function<void(bool)> boolOneCompleteBlock;
typedef std::function<void(float, float, std::string)> floatTwoStringOCompleteBlock;


class AndrodUtil {
public:
    
    static AndrodUtil * getInstance() {
        static AndrodUtil * instance = nullptr;
        if (instance==nullptr) {
            instance = new (std::nothrow)AndrodUtil();
        }
        return instance;
    }
    
    AndrodUtil() : _fileAskCompleteBlock(nullptr)
    , _cameraAskCompleteBlock(nullptr)
    , _GPSAskCompleteBlock(nullptr)
    , _callAskCompleteBlock(nullptr)
    , _contactAskCompleteBlock(nullptr)
    , _locationGetCompleteBlock(nullptr)
    {
        
    }
    virtual ~AndrodUtil()
    {
        
    }
    
    boolOneCompleteBlock _fileAskCompleteBlock;
    boolOneCompleteBlock _cameraAskCompleteBlock;
    boolOneCompleteBlock _GPSAskCompleteBlock;
    boolOneCompleteBlock _callAskCompleteBlock;
    boolOneCompleteBlock _contactAskCompleteBlock;
    floatTwoStringOCompleteBlock _locationGetCompleteBlock;
};



int OSUtil::compareVersion(const std::string& targetVersion) {
    
    auto s1 = StringUtil::split(OSUtil::getAppVersionName() , ".");
    //    auto s1 = StringUtil::split("1.0.0" , ".");
    auto s2 = StringUtil::split(targetVersion , ".");
    
    size_t length = std::max(s1.size(), s2.size());
    for (size_t i = 0; i < length; i++) {
        int n1 = 0, n2 = 0;
        if (i < s1.size()) n1 = StringUtil::toInt(s1.at(i));
        if (i < s2.size()) n2 = StringUtil::toInt(s2.at(i));
        
        if (n2 > n1) {
            // targetVersion이 최신 버전임.
            return 1;
        }
        
        if (n2 < n1) {
            // targetVersion이 더 오래된 버전임.
            return -1;
        }
    }
    
    // 같은 버전
    return 0;
}


bool OSUtil::isScreenShortRatio() {
    auto size = cocos2d::Director::getInstance()->getWinSize();
    return (size.height / size.width < 1.6);
}

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static const std::string PLATFORM_NAME("android");

// android url
static const std::string MARKET_URL("");


std::string OSUtil::getTargetName()
{
    return "";
}

std::string OSUtil::getBundleID()
{
    return "";
}

bool OSUtil::openURL(const std::string& url) {
    return true;
//    auto urlString = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
//    auto targetURL = [NSURL URLWithString:urlString];
//
//    return [[UIApplication sharedApplication] openURL: targetURL];
}

bool OSUtil::openDeviceSettings() {
    return true;
//    NSURL* settingsUrl = [NSURL URLWithString:UIApplicationOpenSettingsURLString];
//
//    return [[UIApplication sharedApplication] openURL: settingsUrl];
}

std::string OSUtil::getAppDisplayName() {
    return "";
//    NSString* appDisplayName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"];
//
//    return [appDisplayName UTF8String];
}

std::string OSUtil::getAppVersionName() {
    return "";
//    NSString* versionName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
//    if (versionName) {
//        return [versionName UTF8String];
//    }
//    return std::string("0");
}

std::string OSUtil::getAppBuildVersion() {
    return "";
//    NSString* buildVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
//
//    return [buildVersion UTF8String];
}

std::string OSUtil::getCountryCode() {
    return "";
//    NSLocale* locale = [NSLocale currentLocale];
//    NSString* countryCode = [locale objectForKey:NSLocaleCountryCode];
//
//    return [countryCode UTF8String];
}

std::string OSUtil::getLanguageCode() {
    return "";
//    NSLocale* locale = [NSLocale currentLocale];
//    NSString* languageCode = [locale objectForKey:NSLocaleLanguageCode];
//
//    return [languageCode UTF8String];
}

const std::string& OSUtil::getPlatform() {
    return PLATFORM_NAME;
}

bool OSUtil::openAppMarket() {
    return openURL(MARKET_URL);
}

void OSUtil::setTintColor(const cocos2d::Color4F& tintColor) {
//    [[UITextField appearance] setTintColor:[UIColor colorWithRed:tintColor.r green:tintColor.g blue:tintColor.b alpha:tintColor.a]];
//    [[UITextView appearance] setTintColor:[UIColor colorWithRed:tintColor.r green:tintColor.g blue:tintColor.b alpha:tintColor.a]];
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
    return true;
//    UIUserNotificationType type = [[[UIApplication sharedApplication] currentUserNotificationSettings] types];
//
//    if (type == UIUserNotificationTypeNone) {
//        return false;
//    } else {
//        return true;
//    }
}

// 해당 접근 권한이 있는지 체크
bool OSUtil::canAcceessibleFile()
{
    return true;
//    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
//    return (status == PHAuthorizationStatusAuthorized);// || status == PHAuthorizationStatusNotDetermined);
}

bool OSUtil::canAcceessibleCamera()
{
    return true;
//    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
//
//    return (status == AVAuthorizationStatusAuthorized);// || status == AVAuthorizationStatusNotDetermined);
}

bool OSUtil::canAcceessibleGPS()
{
    cocos2d::JniMethodInfo isAccesibleGPS;
    if (!cocos2d::JniHelper::getStaticMethodInfo(isAccesibleGPS, "org/cocos2dx/cpp/AppActivity", "isAccesibleGPS", "()Z")) {
        return false;
    }
    
    jboolean jAccessible = (jboolean)isAccesibleGPS.env->CallStaticBooleanMethod(isAccesibleGPS.classID, isAccesibleGPS.methodID);

    isAccesibleGPS.env->DeleteLocalRef(isAccesibleGPS.classID);
    
    return (bool)(jAccessible==JNI_TRUE);
}

bool OSUtil::canAcceessiblePhoneCall()
{
    return true;
}

bool OSUtil::canAcceessibleContact()
{
    return true;
}

// 해당 접근 권한을 부여할 것인지 물어보자
// 이미 거부한 경우는 설정으로 보내자
void OSUtil::askAccessPermissionFile(const std::function<void(bool)>& completeBlock)
{
    completeBlock(true);
//    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
//    if (status == PHAuthorizationStatusDenied || status == PHAuthorizationStatusRestricted) {
//        // 거부되었으니 종료하든가 설정으로 보내든가
//        completeBlock(false);
//    } else if (status == PHAuthorizationStatusNotDetermined) {
//        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
//         switch (status) {
//         case PHAuthorizationStatusAuthorized:
//         completeBlock(true);
//         break;
//                                     default:
//         case PHAuthorizationStatusRestricted:
//         case PHAuthorizationStatusDenied:
//         completeBlock(false);
//         break;
//         }
//         }];
//    }
}

void OSUtil::askAccessPermissionCamera(const std::function<void(bool)>& completeBlock)
{
    completeBlock(true);
//    AVAuthorizationStatus status = [ AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo ];
//
//    if (status == AVAuthorizationStatusDenied ||
//        status == AVAuthorizationStatusRestricted) {
//        // notify listener
//        completeBlock(false);
//    } else if(status == AVAuthorizationStatusNotDetermined) {
//        [ AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler: ^(BOOL granted) {
//         if (granted) {
//         completeBlock(true);
//         } else {
//         completeBlock(false);
//         }
//         } ];
//    }
}

void OSUtil::askAccessPermissionGPS(const std::function<void(bool)>& completeBlock)
{

    AndrodUtil::getInstance()->_GPSAskCompleteBlock = completeBlock;
    
    cocos2d::JniMethodInfo askPermissionGPS;
    if (!cocos2d::JniHelper::getStaticMethodInfo(askPermissionGPS, "org/cocos2dx/cpp/AppActivity", "askPermissionGPS", "()V")) {
        completeBlock(false);
        return;
    }
    
    askPermissionGPS.env->CallStaticVoidMethod(askPermissionGPS.classID, askPermissionGPS.methodID);
    askPermissionGPS.env->DeleteLocalRef(askPermissionGPS.classID);
}

void OSUtil::askAccessPermissionPhoneCall(const std::function<void(bool)>& completeBlock)
{
    
}

void OSUtil::askAccessPermissionContact(const std::function<void(bool)>& completeBlock)
{
    
}

void OSUtil::getLocationInfo(const std::function<void(float, float, std::string)>& completeBlock, bool bWithAddress)
{

    if (OSUtil::getInstance()->getLocationOnceFlag()==true) {
        return;
    }
    OSUtil::getInstance()->setLocationOnceFlag(true);

    std::string className = "org/cocos2dx/cpp/AppActivity";
    float latitude = cocos2d::JniHelper::callStaticFloatMethod(className, "getLatitude");
    float longitude = cocos2d::JniHelper::callStaticFloatMethod(className, "getLongitude");
    std::string address = cocos2d::JniHelper::callStaticStringMethod(className, "getLocationName");
    
    completeBlock(latitude, longitude, address);
    
    OSUtil::getInstance()->setLocationOnceFlag(false);

}

void OSUtil::getLL()
{
//    getLatitude
//    getLongitude
    
    cocos2d::JniMethodInfo getLatitude;
    if (!cocos2d::JniHelper::getStaticMethodInfo(getLatitude, "org/cocos2dx/cpp/AppActivity", "getLatitude", "()F")) {
        return;
    }
    
    jfloat jLatitude = (jfloat)getLatitude.env->CallStaticFloatMethod(getLatitude.classID, getLatitude.methodID);
    getLatitude.env->DeleteLocalRef(getLatitude.classID);
    
    cocos2d::JniMethodInfo getLongitude;
    if (!cocos2d::JniHelper::getStaticMethodInfo(getLongitude, "org/cocos2dx/cpp/AppActivity", "getLongitude", "()F")) {
        return;
    }
    
    jfloat jLongitude = (jfloat)getLongitude.env->CallStaticFloatMethod(getLongitude.classID, getLongitude.methodID);
    getLongitude.env->DeleteLocalRef(getLongitude.classID);

    float latitude = (float)jLatitude;
    float longitude = (float)jLongitude;
    if (AndrodUtil::getInstance()->_locationGetCompleteBlock) {
        AndrodUtil::getInstance()->_locationGetCompleteBlock(latitude, longitude, "");
    }
    AndrodUtil::getInstance()->_locationGetCompleteBlock = nullptr;
}

void cocos_android_app_setExternalScheme(std::string scheme)
{
//    CCLOG("[[[[[ scheme : %s", scheme.c_str());
    cocos2d::Application::getInstance()->setSchemeUrl(scheme);
}

void cocos_android_app_onAskPermissionComplete(int type, bool bAccess)
{
    switch (type) {
        case 1001:
        {
            CCLOG("[[[[[ cocos_android_app_onAskPermissionComplete : %d, %d", type, bAccess);
            // GPS
            if (AndrodUtil::getInstance()->_GPSAskCompleteBlock) {
                AndrodUtil::getInstance()->_GPSAskCompleteBlock(bAccess);
            }
            AndrodUtil::getInstance()->_GPSAskCompleteBlock = nullptr;
        }
            break;
        case 1002:
        {
            // MEDIA
            CCLOG("[[[[[ cocos_android_app_onAskPermissionComplete MEDIA : %d, %d", type, bAccess);
        }
            break;
        case 1003:
        {
            // NFC...??
            CCLOG("[[[[[ cocos_android_app_onAskPermissionComplete NFC : %d, %d", type, bAccess);
        }
            break;

        default:
        {
            CCLOG("[[[[[ cocos_android_app_onAskPermissionComplete WTF????? : %d, %d", type, bAccess);
        }
            break;
    }
}

void cocos_android_app_onGetLocationInfoComplete()
{
    CCLOG("[[[[[ cocos_android_app_onGetLocationInfoComplete!!!!!!!");
    return;
    cocos2d::JniMethodInfo stopGPS;
    if (!cocos2d::JniHelper::getStaticMethodInfo(stopGPS, "org/cocos2dx/cpp/AppActivity", "stopGPS", "()V")) {
        return;
    }
    
    stopGPS.env->CallStaticVoidMethod(stopGPS.classID, stopGPS.methodID);
    stopGPS.env->DeleteLocalRef(stopGPS.classID);

    OSUtil::getLL();
}

void OSUtil::showShareView(const std::string &imageUrl, const std::string &text)
{
    
}

#endif
