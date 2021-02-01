//
//  SMCocosAppDelegate.h
//  ePubCheck
//
//  Created by KimSteve on 2017. 4. 13..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMCocosAppDelegate_h
#define SMCocosAppDelegate_h

#include "cocos2d.h"
#include "../SMFrameWork/Base/Intent.h"
#include "../SMFrameWork/Base/SMApplication.h"
#include "../SMFrameWork/Base/SMScene.h"

typedef enum {
    kSceneTypeImageEditor = 0,
    kSceneTypeImagePicker,
    kSceneTypeCamera,
    kSceneTypeDynamicView,  // using xml, lua... auto layout view
    kSceneTypeUnknown
}kSceneType;


//typedef enum {
//    SP_ANIMATION_START, SP_ANIMATION_END, SP_ANIMATION_COMPLETE, SP_ANIMATION_EVENT
//} spEventType;

class SMCocosAppDelegate : private SMApplication
{
public:
    SMCocosAppDelegate();
    virtual ~SMCocosAppDelegate();

    virtual void initGLContextAttrs() override;

    virtual bool applicationDidFinishLaunching() override;
    
    void putIntent(Intent * intent);

    
    /**
     @brief  Called when the application moves to the background
     @param  the pointer of the application
     */
    virtual void applicationDidEnterBackground() override;
    
    /**
     @brief  Called when the application reenters the foreground
     @param  the pointer of the application
     */
    virtual void applicationWillEnterForeground() override;
    
    virtual void setSchemeUrl(std::string param) override;
    virtual std::string getSchemeUrl() override {return _url;}

private:
    Intent * _intent;
    bool _isRealServer;
    std::string _url;
};

#endif /* SMCocosAppDelegate_h */
