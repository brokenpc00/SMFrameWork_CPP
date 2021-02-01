LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavcodec
LOCAL_SRC_FILES:= lib/libavcodec-57.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavformat
LOCAL_SRC_FILES:= lib/libavformat-57.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libswscale
LOCAL_SRC_FILES:= lib/libswscale-4.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavutil
LOCAL_SRC_FILES:= lib/libavutil-55.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libavfilter
LOCAL_SRC_FILES:= lib/libavfilter-6.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libswresample
LOCAL_SRC_FILES:= lib/libswresample-2.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=off
OPENCV_LIB_TYPE:=STATIC
#include $(LOCAL_PATH)/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

# c++_static build
include $(LOCAL_PATH)/opencv/sdk/native/jni/OpenCV.mk

$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos/audio/include)

LOCAL_MODULE := MyGame_shared

LOCAL_MODULE_FILENAME := libMyGame

LOCAL_SRC_FILES := $(LOCAL_PATH)/main/main.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Const/SMViewConstValue.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Const/SMFontColor.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SceneTransition.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/Intent.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/ShaderNode.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMApplication.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMButton.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMImageView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMPageView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMTableView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMWebView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/ViewAction.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMZoomView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMSlider.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/SMCircularListView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/ImageAutoPolygon.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/MeshSprite.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/EdgeSwipeLayer.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/Sticker.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/ColorSprite.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Base/MaskSprite.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Cutout/CutoutProcessor.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Cutout/DrawView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Cutout/DrawCropView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/BaseTableView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/Dynamics.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/ScrollController.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/SMScroller.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/VelocityTracker.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Interface/ZoomController.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/ImageProcess/ImageProcessor.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/ImageProcess/ImageProcessFunction.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/StringUtil.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/md5.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/FileManager.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/ImageDownloader.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/DownloadTask.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/JpegDecodeUtil.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/ImageFetcher.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/OSUtil.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/WorkThread.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/SMDownloader.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/nanovgGLDraw/nanovg.c \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/nanovgGLDraw/NanoWrapper.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/nanovgGLDraw/nanovg_core.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/nanovgGLDraw/NanoDrawNode.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/SMPath.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Util/encrypt/AES.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/OverlapContainer.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/DashedRoundRect.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CircularProgress.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/ProgressView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/KenBurnsView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CircleCropSprite.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/SideMenu.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/PulsActionView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/LoadingSprite.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/SlideButton.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CaptureView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CameraControlLayer.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/FocusingView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/ImagePickerView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/GroupSelectView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/ThumbImageView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/AlertView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/StencilView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/StickerCanvasView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/StickerControlView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/SwipeView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/SearchEditBox.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/MultiTouchController.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/WasteBasketActionNode.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CalendarDayCell.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/Calendar.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/CalendarView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/TransitionView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/UI/DropDownView.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Popup/Popup.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Popup/PopupManager.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Popup/LoadingPopup.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Popup/ProgressPopup.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Popup/ExtractPopup.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Picker/SMWheelPicker.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Network/SMComm.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Network/JsonWrapper.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Database/ContentValue.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Database/DBConstant.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Database/sqlite3.c \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Database/SQLiteWrapper.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Database/DBHelper.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/Bean/Table1.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/DeviceAsset/DeviceCamera.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/DeviceAsset/AndroidCamera.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/FilterShaderString.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilter1977.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterAmaro.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterBrannan.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterEarlyBird.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterHefe.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterHudson.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterInkwell.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterLomofi.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterLordKelvin.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterNashville.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterRise.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterSierra.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterSutro.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterToaster.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterValencia.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterWalden.cpp \
                   $(LOCAL_PATH)/../../../Classes/SMFrameWork/SpriteFilter/SpriteFilterXproii.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/MainScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorColorScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorCropGuideView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorCropScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorFilterScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorItemThumbView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorRotateScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStickerBoardView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStickerItemView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStickerListView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStickerParser.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStickerScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStraightenGuideView.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorStraightenScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImageEditor/SMImageEditorDrawScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/ImagePicker/SMImagePickerScene.cpp \
                   $(LOCAL_PATH)/../../../Classes/Scene/SMCocosAppDelegate.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../Classes

OPENCV_INCLUDE_DIR:=$(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/OpenCV-android-sdk/sdk/native/jni/include
LOCAL_SHARED_LIBRARIES:=libavcodec libavfilter libavformat libavutil libswscale libswresample


# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cc_static
LOCAL_STATIC_LIBRARIES += ext_curl
LOCAL_STATIC_LIBRARIES += ext_jpeg
#LOCAL_STATIC_LIBRARIES += ext_ffmpeg
LOCAL_STATIC_LIBRARIES += libopencv_contrib libopencv_legacy libopencv_ml libopencv_stitching libopencv_objdetect libopencv_videostab libopencv_calib3d libopencv_photo libopencv_video libopencv_features2d libopencv_highgui libopencv_androidcamera libopencv_flann libopencv_imgproc libopencv_ts libopencv_core


# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module, cocos)
$(call import-module,curl/prebuilt/android)
$(call import-module,jpeg/prebuilt/android)
#$(call import-module,ffmpeg/prebuilt/android)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
