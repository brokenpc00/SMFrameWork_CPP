#include <memory>

#include <android/log.h>
#include <jni.h>

#include "Scene/SMCocosAppDelegate.h"

#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

namespace {
std::unique_ptr<SMCocosAppDelegate> appDelegate;
}

void cocos_android_app_init(JNIEnv* env) {
    LOGD("cocos_android_app_init");
    appDelegate.reset(new SMCocosAppDelegate());
}
//
//void cocos_android_app_onFrameReceive(uint8_t* bytes, int length, int preWidth, int preHeight)
//{
//    //LOGD("[[[[ on frame received..... JNI~~~~~ : %ul", array_length);
//    //jbyte* jBytes = env->GetByteArrayElements(array,NULL);
//    //int8_t* bytes = (int8_t*)jBytes;;
//    appDelegate->onFrameReceiveFromJNI(bytes, length, preWidth, preHeight);
//    //LOGD("[[[[ on frame received..... JNI~~~~~ : %d", length);
//}
//
//void cocos_android_app_nativeOnPictureTaken(uint8_t * bytes, int length, int width, int height)
//{
//    appDelegate->onPictureTakenFromJNI(bytes, length, width, height);
//}

//void cocos_android_app_onCodeDetected(const char * result)
//{
//    std::string scanResult(result);
//    LOGD("[[[[[ main.cpp onCodeDetected from JNI : %s", result);
//}

//void cocos_android_app_onAskPermissionComplete(int, bool)
//{
//
//}