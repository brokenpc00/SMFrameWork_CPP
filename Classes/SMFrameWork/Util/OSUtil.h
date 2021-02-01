//
//  OSUtil.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 20..
//

#ifndef OSUtil_h
#define OSUtil_h

#include <cocos2d.h>
#include <string>


class OSUtil {
public:
    static std::string getAppDisplayName();
    static std::string getAppVersionName();
    static std::string getAppBuildVersion();

    // -1 : app newest, 0 : latest version, 1: need update
    static int compareVersion(const std::string& targetVersion);

    static std::string getCountryCode();
    static std::string getLanguageCode();

    static const std::string& getPlatform();
    
    static bool openAppMarket();
    
    static bool openURL(const std::string& url);
    static bool openDeviceSettings();
    
    static void setTintColor(const cocos2d::Color4F& tintColor);
    
    static std::vector<std::string> getSystemFontList();
    
    static bool isScreenShortRatio();
    
    bool isPushNotificationEnabled();
    
    static void showShareView(const std::string& imageUrl, const std::string& text);

    

    // permision for asset
    static bool canAcceessibleFile();
    static bool canAcceessibleCamera();
    static bool canAcceessibleGPS();
    static bool canAcceessiblePhoneCall();
    static bool canAcceessibleContact();
    
    static void askAccessPermissionFile(const std::function<void(bool)>& completeBlock);
    static void askAccessPermissionCamera(const std::function<void(bool)>& completeBlock);
    static void askAccessPermissionGPS(const std::function<void(bool)>& completeBlock);
    static void askAccessPermissionPhoneCall(const std::function<void(bool)>& completeBlock);
    static void askAccessPermissionContact(const std::function<void(bool)>& completeBlock);
    static void getLocationInfo(const std::function<void(float, float, std::string)>& completeBlock, bool bWithAddress=true);

    
    // for android
    #if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    static void getLL();
    #endif

public:
    OSUtil() : _locationOnceFlag(false), _address(""), _latitude(0), _longitude(0) {
        
    };
    virtual ~OSUtil(){};
    
    static OSUtil * getInstance() {
        static OSUtil * instance = nullptr;
        if (instance==nullptr) {
            instance = new OSUtil();
        }
        return instance;
    }

    bool getLocationOnceFlag() {return _locationOnceFlag;}
    void setLocationOnceFlag(bool bFlag) {_locationOnceFlag=bFlag;}

    std::string getTargetName();
    std::string getBundleID();
    
    std::string getAddress() {return _address;}
    float getLatitude() {return _latitude;}
    float getLongitude() {return _longitude;}
    
private:
    bool _locationOnceFlag;
    std::mutex _mutex;
    std::condition_variable _cond;
    bool _pushEnable;
    std::string _address;
    float _latitude;
    float _longitude;
};


#endif /* OSUtil_h */
