//
//  SMCocosAppDelegate.c
//  MyGame
//
//  Created by KimSteve on 2017. 6. 8..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMCocosAppDelegate.h"
#include "../SMFrameWork/Base/ShaderNode.h"
#include "MainScene.h"
#include <cocos2d.h>

#include "ImageEditor/SMImageEditorScene.h"

#define FOR_TEST

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "ImagePicker/SMImagePickerScene.h"
#endif

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//#include "libavformat/avformat.h"
//#endif
}

//USING_NS_CC;

SMCocosAppDelegate::SMCocosAppDelegate() : _intent(nullptr)
{

}

SMCocosAppDelegate::~SMCocosAppDelegate()
{
    
}

void SMCocosAppDelegate::initGLContextAttrs()
{
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};
    
    cocos2d::GLView::setGLContextAttrs(glContextAttrs);
    
}

// 원래는 app did finish launch로 들어오지만
// 지금처럼 뷰 컨트롤러를 따로 호출 했을 경우는 did finish launch가 아니라
// 호출 시점이라고 보면 된다.
bool SMCocosAppDelegate::applicationDidFinishLaunching()
{
    // initialize director

    // 각종 configuration setting등
    // 뷰 크기, 터치 등록
    int st = kSceneTypeUnknown;
    SMApplication::applicationDidFinishLaunching();

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    // ffmpeg init
//    av_register_all();
    avformat_network_init();
//#endif

    if (_intent!=nullptr) {
        st = _intent->getInt("SCENE_TYPE");
        if (st>kSceneTypeUnknown) {
            st = kSceneTypeUnknown;
        }
    }
    
    kSceneType sceneType = (kSceneType)st;

    
    // 이 부분을 호출 할떄,,, 여러경로에서 들어올 수 있기 때문에
    // 특정 값을 통해서 분기 처리하도록 하자.
    // 프로젝트 마다 다르게 하기위해서 고민할 것.
    // 포토 에디터, 풀 메뉴 등등...
    // OpenGLES를 통해서 얻을 수 있는 가치 있는 화면 정의
    
    SMScene * scene = nullptr;
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    sceneType = kSceneTypeImagePicker;
//    sceneType = kSceneTypeDynamicView;
    switch (sceneType) {
        case kSceneTypeImageEditor:
        {
            scene = SMImageEditorScene::create(_intent, SMScene::SwipeType::NONE);
        }
            break;
        case kSceneTypeImagePicker:
        {
            scene = SMImagePickerScene::createForEdit(SMImagePickerScene::Mode::IMAGE_SELECT, SMImagePickerScene::StartWith::ALBUM, SMScene::SwipeType::NONE);
        }
            break;
        case kSceneTypeCamera:
        {
            scene = SMImagePickerScene::createForQRBarCode();
        }
            break;
        case kSceneTypeDynamicView:
        default:
        {
            scene = MainScene::create(_intent, SMScene::SwipeType::MENU);
        }
            break;
    }
    
#else
    sceneType = kSceneTypeDynamicView;
    scene = MainScene::create(_intent, SMScene::SwipeType::MENU);
#endif
    


    // 최초 실행 scene을 등록
    cocos2d::Director::getInstance()->runWithScene(scene);
//    const int initMenuIndex = 0;
//    Director::getInstance()->runWithSceneAt(initMenuIndex, scene);

    return true;
}

void SMCocosAppDelegate::applicationDidEnterBackground() {
    cocos2d::Director::getInstance()->stopAnimation();
    
    // if you use SimpleAudioEngine, it must be paused
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void SMCocosAppDelegate::applicationWillEnterForeground() {
    cocos2d::Director::getInstance()->startAnimation();
    
    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

void SMCocosAppDelegate::putIntent(Intent * intent)
{
    CCLOG("[[[[[ put intent");
    
    _intent = intent;
}

void SMCocosAppDelegate::setSchemeUrl(std::string param)
{
    
}
