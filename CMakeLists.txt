#/****************************************************************************
# Copyright (c) 2013-2014 cocos2d-x.org
# Copyright (c) 2015-2017 Chukong Technologies Inc.
#
# http://www.cocos2d-x.org
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ****************************************************************************/

cmake_minimum_required(VERSION 3.6)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1 -DMYLIB_STATIC)

set(LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proj.android/app)




# ffmpeg
set(FFMPEG_LIB_DIR ${LOCAL_PATH}/jni/ffmpeg/libs/${CMAKE_ANDROID_ARCH_ABI})
set(FFMPEG_INCLUDE ${LOCAL_PATH}/jni/ffmpeg/include)

add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavcodec.so)


add_library(libavfilter SHARED IMPORTED)
set_target_properties(libavfilter
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavfilter.so)

add_library(libswresample SHARED IMPORTED)
set_target_properties(libswresample
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswresample.so)

add_library(libavutil STATIC IMPORTED)
set_target_properties(libavutil
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavutil.so)


add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavformat.so)


add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale
        PROPERTIES
        IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswscale.so)

set(FFMPEG_LIBS
        libavcodec
        libavfilter
        libswresample
        libavutil
        libavformat
        libswscale
        )

include_directories(${FFMPEG_INCLUDE})

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=softfp -mfpu=neon -g -O0")

#opencv 2.4.13.6 import
set(OpenCV_SDK ${LOCAL_PATH}/jni/opencv/sdk)
set(OpenCV_INCLUDE ${OpenCV_SDK}/native/jni/include)
set(OpenCV_LIB ${OpenCV_SDK}/native/libs/${CMAKE_ANDROID_ARCH_ABI}/)


#library directory
set(OpenCV_SHARED OFF)
set(OpenCV_DIR ${OpenCV_SDK}/native/jni)
find_package (OpenCV REQUIRED)
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
add_library(lib_opencv STATIC IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${OpenCV_LIB}/libopencv_java.so)
include_directories(${OpenCV_INCLUDE})

set(APP_NAME SMFrameWork)

project(${APP_NAME})

set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)
set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)

include(CocosBuildSet)
add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)

# record sources, headers, resources...
set(GAME_SOURCE)
set(GAME_HEADER)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    )
if(APPLE OR WINDOWS)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# add cross-platforms source files and header files 
list(APPEND GAME_SOURCE
        Classes/SMFrameWork/Base/ColorSprite.cpp
        Classes/SMFrameWork/Base/EdgeSwipeLayer.cpp
        Classes/SMFrameWork/Base/ImageAutoPolygon.cpp
        Classes/SMFrameWork/Base/Intent.cpp
        Classes/SMFrameWork/Base/MaskSprite.cpp
        Classes/SMFrameWork/Base/MeshSprite.cpp
        Classes/SMFrameWork/Base/SceneTransition.cpp
        Classes/SMFrameWork/Base/ShaderNode.cpp
        Classes/SMFrameWork/Base/SMApplication.cpp
        Classes/SMFrameWork/Base/SMButton.cpp
        Classes/SMFrameWork/Base/SMCircularListView.cpp
        Classes/SMFrameWork/Base/SMImageView.cpp
        Classes/SMFrameWork/Base/SMMeshView.cpp
        Classes/SMFrameWork/Base/SMPageView.cpp
        Classes/SMFrameWork/Base/SMScene.cpp
        Classes/SMFrameWork/Base/SMSlider.cpp
        Classes/SMFrameWork/Base/SMTableView.cpp
        Classes/SMFrameWork/Base/SMView.cpp
        Classes/SMFrameWork/Base/SMWebView.cpp
        Classes/SMFrameWork/Base/SMZoomView.cpp
        Classes/SMFrameWork/Base/StackTransition.cpp
        Classes/SMFrameWork/Base/Sticker.cpp
        Classes/SMFrameWork/Base/ViewAction.cpp
        Classes/SMFrameWork/Bean/Table1.cpp
        Classes/SMFrameWork/Const/SMFontColor.cpp
        Classes/SMFrameWork/Const/SMViewConstValue.cpp
        Classes/SMFrameWork/Cutout/BaseCropView.cpp
        Classes/SMFrameWork/Cutout/CropRectView.cpp
        Classes/SMFrameWork/Cutout/CropFreeView.cpp
        Classes/SMFrameWork/Cutout/CutoutProcessor.cpp
        Classes/SMFrameWork/Cutout/DrawCropView.cpp
        Classes/SMFrameWork/Cutout/DrawView.cpp
        Classes/SMFrameWork/Database/ContentValue.cpp
        Classes/SMFrameWork/Database/DBConstant.cpp
        Classes/SMFrameWork/Database/DBHelper.cpp
        Classes/SMFrameWork/Database/sqlite3.c
        Classes/SMFrameWork/Database/SQLiteWrapper.cpp
        Classes/SMFrameWork/DeviceAsset/AndroidCamera.cpp
        Classes/SMFrameWork/DeviceAsset/DeviceCamera.cpp
        Classes/SMFrameWork/ImageProcess/ImageProcessFunction.cpp
        Classes/SMFrameWork/ImageProcess/ImageProcessor.cpp
        Classes/SMFrameWork/ImageProcess/ImgPrcUploadData.cpp
        Classes/SMFrameWork/Interface/BaseTableView.cpp
        Classes/SMFrameWork/Interface/Dynamics.cpp
        Classes/SMFrameWork/Interface/ScrollController.cpp
        Classes/SMFrameWork/Interface/SMScroller.cpp
        Classes/SMFrameWork/Interface/VelocityTracker.cpp
        Classes/SMFrameWork/Interface/ZoomController.cpp
        Classes/SMFrameWork/MapView/SMMapView.cpp
        Classes/SMFrameWork/MapView/SMMapViewImpl-Android.cpp
        Classes/SMFrameWork/Network/JsonWrapper.cpp
        Classes/SMFrameWork/Network/JsonWriter.cpp
        Classes/SMFrameWork/Network/SMComm.cpp
        Classes/SMFrameWork/Picker/SMWheelPicker.cpp
        Classes/SMFrameWork/Popup/ExtractPopup.cpp
        Classes/SMFrameWork/Popup/LoadingPopup.cpp
        Classes/SMFrameWork/Popup/Popup.cpp
        Classes/SMFrameWork/Popup/PopupManager.cpp
        Classes/SMFrameWork/Popup/ProgressPopup.cpp
        Classes/SMFrameWork/SpriteFilter/FilterShaderString.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilter1977.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterAmaro.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterBrannan.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterEarlyBird.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterHefe.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterHudson.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterInkwell.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterLomofi.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterLordKelvin.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterNashville.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterRise.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterSierra.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterSutro.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterToaster.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterValencia.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterWalden.cpp
        Classes/SMFrameWork/SpriteFilter/SpriteFilterXproii.cpp
        Classes/SMFrameWork/UI/ActionBar.cpp
        Classes/SMFrameWork/UI/AlertView.cpp
        Classes/SMFrameWork/UI/Calendar.cpp
        Classes/SMFrameWork/UI/CalendarDayCell.cpp
        Classes/SMFrameWork/UI/CalendarView.cpp
        Classes/SMFrameWork/UI/CameraControlLayer.cpp
        Classes/SMFrameWork/UI/CaptureView.cpp
        Classes/SMFrameWork/UI/CircleCropSprite.cpp
        Classes/SMFrameWork/UI/CircularProgress.cpp
        Classes/SMFrameWork/UI/DashedRoundRect.cpp
        Classes/SMFrameWork/UI/DropDownView.cpp
        Classes/SMFrameWork/UI/FocusingView.cpp
        Classes/SMFrameWork/UI/GroupSelectView.cpp
        Classes/SMFrameWork/UI/ImagePickerView.cpp
        Classes/SMFrameWork/UI/KenBurnsView.cpp
        Classes/SMFrameWork/UI/LoadingSprite.cpp
        Classes/SMFrameWork/UI/MultiTouchController.cpp
        Classes/SMFrameWork/UI/OverlapContainer.cpp
        Classes/SMFrameWork/UI/ProgressView.cpp
        Classes/SMFrameWork/UI/PulsActionView.cpp
        Classes/SMFrameWork/UI/SearchEditBox.cpp
        Classes/SMFrameWork/UI/SideMenu.cpp
        Classes/SMFrameWork/UI/SlideButton.cpp
        Classes/SMFrameWork/UI/StencilView.cpp
        Classes/SMFrameWork/UI/StickerCanvasView.cpp
        Classes/SMFrameWork/UI/StickerControlView.cpp
        Classes/SMFrameWork/UI/SwipeView.cpp
        Classes/SMFrameWork/UI/ThumbImageView.cpp
        Classes/SMFrameWork/UI/TransitionView.cpp
        Classes/SMFrameWork/UI/WasteBasketActionNode.cpp
        Classes/SMFrameWork/Util/AudioFrame.cpp
        Classes/SMFrameWork/Util/CCImageEx.cpp
        Classes/SMFrameWork/Util/DownloadTask.cpp
        Classes/SMFrameWork/Util/encrypt/AES.cpp
        Classes/SMFrameWork/Util/FileManager.cpp
        Classes/SMFrameWork/Util/ImageDownloader.cpp
        Classes/SMFrameWork/Util/ImageFetcher.cpp
        Classes/SMFrameWork/Util/JpegDecodeUtil.cpp
#        Classes/SMFrameWork/Util/libmosquitto/logging_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/memory_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/messages_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/mosquitto.c
#        Classes/SMFrameWork/Util/libmosquitto/net_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/read_handle.c
#        Classes/SMFrameWork/Util/libmosquitto/read_handle_client.c
#        Classes/SMFrameWork/Util/libmosquitto/read_handle_shared.c
#        Classes/SMFrameWork/Util/libmosquitto/send_client_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/send_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/util_mosq.c
#        Classes/SMFrameWork/Util/libmosquitto/will_mosq.c
        Classes/SMFrameWork/Util/md5.cpp
#        Classes/SMFrameWork/Util/MosquittoClient.cpp
        Classes/SMFrameWork/Util/nanovgGLDraw/NanoDrawNode.cpp
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg.c
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg_core.cpp
        Classes/SMFrameWork/Util/nanovgGLDraw/NanoWrapper.cpp
        Classes/SMFrameWork/Util/OSUtil.cpp
        Classes/SMFrameWork/Util/SMDownloader.cpp
        Classes/SMFrameWork/Util/SMPath.cpp
        Classes/SMFrameWork/Util/SMVideoPlayer.cpp
        Classes/SMFrameWork/Util/StringUtil.cpp
        Classes/SMFrameWork/Util/VideoDecoder.cpp
        Classes/SMFrameWork/Util/VideoFrame.cpp
        Classes/SMFrameWork/Util/VideoSprite.cpp
        Classes/SMFrameWork/Util/WorkThread.cpp


        Classes/Scene/ImageEditor/SMImageEditorColorScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorCropGuideView.cpp
        Classes/Scene/ImageEditor/SMImageEditorCropScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorDrawScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorFilterScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorItemThumbView.cpp
        Classes/Scene/ImageEditor/SMImageEditorRotateScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorStickerBoardView.cpp
        Classes/Scene/ImageEditor/SMImageEditorStickerItemView.cpp
        Classes/Scene/ImageEditor/SMImageEditorStickerListView.cpp
        Classes/Scene/ImageEditor/SMImageEditorStickerParser.cpp
        Classes/Scene/ImageEditor/SMImageEditorStickerScene.cpp
        Classes/Scene/ImageEditor/SMImageEditorStraightenGuideView.cpp
        Classes/Scene/ImageEditor/SMImageEditorStraightenScene.cpp
        Classes/Scene/ImagePicker/SMImagePickerScene.cpp


        Classes/Scene/Misc/JsonData.cpp
        Classes/Scene/Misc/SMDateTimePickerScene.cpp


        Classes/Scene/SMCocosAppDelegate.cpp
        Classes/Scene/MainScene.cpp

     )
list(APPEND GAME_HEADER
        Classes/SMFrameWork/Base/ColorSprite.h
        Classes/SMFrameWork/Base/EdgeSwipeLayer.h
        Classes/SMFrameWork/Base/ImageAutoPolygon.h
        Classes/SMFrameWork/Base/Intent.h
        Classes/SMFrameWork/Base/MaskSprite.h
        Classes/SMFrameWork/Base/MeshSprite.h
        Classes/SMFrameWork/Base/SceneTransition.h
        Classes/SMFrameWork/Base/ShaderNode.h
        Classes/SMFrameWork/Base/ShaderUtil.h
        Classes/SMFrameWork/Base/SMApplication.h
        Classes/SMFrameWork/Base/SMButton.h
        Classes/SMFrameWork/Base/SMCircularListView.h
        Classes/SMFrameWork/Base/SMImageView.h
        Classes/SMFrameWork/Base/SMMeshView.h
        Classes/SMFrameWork/Base/SMPageView.h
        Classes/SMFrameWork/Base/SMScene.h
        Classes/SMFrameWork/Base/SMSlider.h
        Classes/SMFrameWork/Base/SMTableView.h
        Classes/SMFrameWork/Base/SMView.h
        Classes/SMFrameWork/Base/SMWebView.h
        Classes/SMFrameWork/Base/SMZoomView.h
        Classes/SMFrameWork/Base/StackTransition.h
        Classes/SMFrameWork/Base/Sticker.h
        Classes/SMFrameWork/Base/ViewAction.h
        Classes/SMFrameWork/Bean/Table1.h
        Classes/SMFrameWork/Const/SMFontColor.h
        Classes/SMFrameWork/Const/SMViewConstValue.h
        Classes/SMFrameWork/Cutout/BaseCropView.h
        Classes/SMFrameWork/Cutout/CropFreeView.h
        Classes/SMFrameWork/Cutout/CropRectView.h
        Classes/SMFrameWork/Cutout/CutoutProcessor.h
        Classes/SMFrameWork/Cutout/DrawCropView.h
        Classes/SMFrameWork/Cutout/DrawView.h
        Classes/SMFrameWork/Cutout/FuncCutout.h
        Classes/SMFrameWork/Database/ContentValue.h
        Classes/SMFrameWork/Database/DBConstant.h
        Classes/SMFrameWork/Database/DBHelper.h
        Classes/SMFrameWork/Database/sqlite3.h
        Classes/SMFrameWork/Database/SQLiteWrapper.h
        Classes/SMFrameWork/DeviceAsset/AndroidCamera.h
        Classes/SMFrameWork/DeviceAsset/DeviceCamera.h
        Classes/SMFrameWork/ImageProcess/ImageProcessFunction.h
        Classes/SMFrameWork/ImageProcess/ImageProcessor.h
        Classes/SMFrameWork/ImageProcess/ImgPrcSimpleCapture.h
        Classes/SMFrameWork/ImageProcess/ImgPrcSnapShotToLocal.h
        Classes/SMFrameWork/ImageProcess/ImgPrcUploadData.h
        Classes/SMFrameWork/Interface/BaseTableView.h
        Classes/SMFrameWork/Interface/Dynamics.h
        Classes/SMFrameWork/Interface/MotionEvent.h
        Classes/SMFrameWork/Interface/ScrollController.h
        Classes/SMFrameWork/Interface/SMScroller.h
        Classes/SMFrameWork/Interface/VelocityTracker.h
        Classes/SMFrameWork/Interface/ZoomController.h
        Classes/SMFrameWork/MapView/SMMapView.h
        Classes/SMFrameWork/MapView/SMMapView-ini.h
        Classes/SMFrameWork/MapView/SMMapViewImpl-Android.h
        Classes/SMFrameWork/Network/JsonWrapper.h
        Classes/SMFrameWork/Network/JsonWriter.h
        Classes/SMFrameWork/Network/SMComm.h
        Classes/SMFrameWork/Picker/SMWheelPicker.h
        Classes/SMFrameWork/Popup/ExtractPopup.h
        Classes/SMFrameWork/Popup/LoadingPopup.h
        Classes/SMFrameWork/Popup/Popup.h
        Classes/SMFrameWork/Popup/PopupManager.h
        Classes/SMFrameWork/Popup/ProgressPopup.h
        Classes/SMFrameWork/SpriteFilter/FilterShaderString.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilter.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilter1977.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterAmaro.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterBrannan.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterEarlyBird.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterHefe.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterHudson.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterInkwell.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterLomofi.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterLordKelvin.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterNashville.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterRise.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterSierra.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterSutro.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterToaster.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterValencia.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterWalden.h
        Classes/SMFrameWork/SpriteFilter/SpriteFilterXproii.h
        Classes/SMFrameWork/UI/ActionBar.h
        Classes/SMFrameWork/UI/AlertView.h
        Classes/SMFrameWork/UI/BaseScene.h
        Classes/SMFrameWork/UI/Calendar.h
        Classes/SMFrameWork/UI/CalendarDayCell.h
        Classes/SMFrameWork/UI/CalendarView.h
        Classes/SMFrameWork/UI/CameraControlLayer.h
        Classes/SMFrameWork/UI/CaptureView.h
        Classes/SMFrameWork/UI/CircleCropSprite.h
        Classes/SMFrameWork/UI/CircularProgress.h
        Classes/SMFrameWork/UI/DashedRoundRect.h
        Classes/SMFrameWork/UI/DropDownView.h
        Classes/SMFrameWork/UI/FocusingView.h
        Classes/SMFrameWork/UI/GroupSelectView.h
        Classes/SMFrameWork/UI/ImagePickerView.h
        Classes/SMFrameWork/UI/KenBurnsView.h
        Classes/SMFrameWork/UI/LoadingSprite.h
        Classes/SMFrameWork/UI/MultiTouchController.h
        Classes/SMFrameWork/UI/OverlapContainer.h
        Classes/SMFrameWork/UI/ProgressView.h
        Classes/SMFrameWork/UI/PulsActionView.h
        Classes/SMFrameWork/UI/SearchEditBox.h
        Classes/SMFrameWork/UI/SideMenu.h
        Classes/SMFrameWork/UI/SlideButton.h
        Classes/SMFrameWork/UI/StencilView.h
        Classes/SMFrameWork/UI/StickerCanvasView.h
        Classes/SMFrameWork/UI/StickerControlView.h
        Classes/SMFrameWork/UI/SwipeView.h
        Classes/SMFrameWork/UI/ThumbImageView.h
        Classes/SMFrameWork/UI/TransitionView.h
        Classes/SMFrameWork/UI/WasteBasketActionNode.h
        Classes/SMFrameWork/Util/AudioFrame.h
        Classes/SMFrameWork/Util/CCImageEx.h
        Classes/SMFrameWork/Util/cvImageUtil.h
        Classes/SMFrameWork/Util/DownloadProtocol.h
        Classes/SMFrameWork/Util/DownloadTask.h
        Classes/SMFrameWork/Util/encrypt/AES.h
        Classes/SMFrameWork/Util/FileManager.h
        Classes/SMFrameWork/Util/ImageCache.h
        Classes/SMFrameWork/Util/ImageDownloader.h
        Classes/SMFrameWork/Util/ImageDownloaderThreadPool.h
        Classes/SMFrameWork/Util/ImageFetcher.h
        Classes/SMFrameWork/Util/ImageItemInfo.h
        Classes/SMFrameWork/Util/JpegDecodeUtil.h
#        Classes/SMFrameWork/Util/libmosquitto/logging_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/memory_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/messages_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/mosquitto_config.h
#        Classes/SMFrameWork/Util/libmosquitto/mosquitto_internal.h
#        Classes/SMFrameWork/Util/libmosquitto/mosquitto.h
#        Classes/SMFrameWork/Util/libmosquitto/mqtt3_protocol.h
#        Classes/SMFrameWork/Util/libmosquitto/net_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/read_handle.h
#        Classes/SMFrameWork/Util/libmosquitto/send_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/util_mosq.h
#        Classes/SMFrameWork/Util/libmosquitto/will_mosq.h
        Classes/SMFrameWork/Util/LRUCache.h
        Classes/SMFrameWork/Util/md5.h
        Classes/SMFrameWork/Util/MemoryCache.h
#        Classes/SMFrameWork/Util/MosquittoClient.h
        Classes/SMFrameWork/Util/nanovgGLDraw/fontstash.h
        Classes/SMFrameWork/Util/nanovgGLDraw/NanoDrawNode.h
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg_core.h
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg_gl_utils.h
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg_gl.h
        Classes/SMFrameWork/Util/nanovgGLDraw/nanovg.h
        Classes/SMFrameWork/Util/nanovgGLDraw/NanoWrapper.h
        Classes/SMFrameWork/Util/nanovgGLDraw/stb_image.h
        Classes/SMFrameWork/Util/nanovgGLDraw/stb_truetype.h
        Classes/SMFrameWork/Util/OSUtil.h
        Classes/SMFrameWork/Util/SMDownloader.h
        Classes/SMFrameWork/Util/SMPath.h
        Classes/SMFrameWork/Util/StringUtil.h
        Classes/SMFrameWork/Util/TimerUtil.h
        Classes/SMFrameWork/Util/use_opencv.h
        Classes/SMFrameWork/Util/SMVideoPlayer.h
        Classes/SMFrameWork/Util/ViewUtil.h
        Classes/SMFrameWork/Util/VideoSprite.h
        Classes/SMFrameWork/Util/VideoDecoder.h
        Classes/SMFrameWork/Util/VideoFrame.h
        Classes/SMFrameWork/Util/WorkThread.h


        Classes/Scene/ImageEditor/ItemInfo.h
        Classes/Scene/ImageEditor/SMImageEditorColorScene.h
        Classes/Scene/ImageEditor/SMImageEditorCropGuideView.h
        Classes/Scene/ImageEditor/SMImageEditorCropScene.h
        Classes/Scene/ImageEditor/SMImageEditorDrawScene.h
        Classes/Scene/ImageEditor/SMImageEditorFilterScene.h
        Classes/Scene/ImageEditor/SMImageEditorItemThumbView.h
        Classes/Scene/ImageEditor/SMImageEditorListener.h
        Classes/Scene/ImageEditor/SMImageEditorRotateScene.h
        Classes/Scene/ImageEditor/SMImageEditorScene.h
        Classes/Scene/ImageEditor/SMImageEditorStickerBoardView.h
        Classes/Scene/ImageEditor/SMImageEditorStickerItemView.h
        Classes/Scene/ImageEditor/SMImageEditorStickerListView.h
        Classes/Scene/ImageEditor/SMImageEditorStickerScene.h
        Classes/Scene/ImageEditor/SMImageEditorStraightenScene.h
        Classes/Scene/ImageEditor/SMImageEditorStraightenGuideView.h
        Classes/Scene/ImagePicker/SMImagePickerScene.h
        Classes/Scene/Misc/JsonData.h
        Classes/Scene/Misc/SMDateTimePickerScene.h


        Classes/Scene/SMCocosAppDelegate.h
        Classes/Scene/MainScene.h
     )

if(ANDROID)
    # change APP_NAME to the share library name for Android, it's value depend on AndroidManifest.xml
    set(APP_NAME MyGame)
    list(APPEND GAME_SOURCE
            ${LOCAL_PATH}/jni/main/main.cpp
         )
elseif(LINUX)
    list(APPEND GAME_SOURCE
         proj.linux/main.cpp
         )
elseif(WINDOWS)
    list(APPEND GAME_HEADER
         proj.win32/main.h
         proj.win32/resource.h
         )
    list(APPEND GAME_SOURCE
         proj.win32/main.cpp
         proj.win32/game.rc
         ${common_res_files}
         )
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
             proj.ios_mac/ios/AppController.h
             proj.ios_mac/ios/RootViewController.h
             )
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/ios/main.m
             proj.ios_mac/ios/AppController.mm
             proj.ios_mac/ios/RootViewController.mm
             proj.ios_mac/ios/Prefix.pch
             ${APP_UI_RES}
             )
    elseif(MACOSX)
        set(APP_UI_RES
            proj.ios_mac/mac/Icon.icns
            proj.ios_mac/mac/Info.plist
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/mac/main.cpp
             proj.ios_mac/mac/Prefix.pch
             ${APP_UI_RES}
             )
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

# mark app complie info and libs info
set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
    )
if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

target_link_libraries(${APP_NAME}
        cocos2d

        ext_curl

        #opencv
        ${OpenCV_LIBS}

        #ffmpeg
        ${FFMPEG_LIBS}
        )

target_include_directories(${APP_NAME}
        PRIVATE Classes
        PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
        PRIVATE ${LOCAL_PATH}/jni/ffmpeg/include/
        PRIVATE ${LOCAL_PATH}/jni/OpenCV-android-sdk/sdk/native/jni/include/
)

# mark app resources
setup_cocos_app_config(${APP_NAME})
if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_target_properties(${APP_NAME} PROPERTIES
                              MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist"
                              )
    elseif(IOS)
        cocos_pak_xcode(${APP_NAME} INFO_PLIST "iOSBundleInfo.plist.in")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
        set_xcode_property(${APP_NAME} DEVELOPMENT_TEAM "")
        set_xcode_property(${APP_NAME} CODE_SIGN_IDENTITY "iPhone Developer")
    endif()
elseif(WINDOWS)
    cocos_copy_target_dll(${APP_NAME})
endif()

if(LINUX OR WINDOWS)
    set(APP_RES_DIR "$<TARGET_FILE_DIR:${APP_NAME}>/Resources")
    cocos_copy_target_res(${APP_NAME} COPY_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()
