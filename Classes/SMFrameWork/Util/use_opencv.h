#pragma once


#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include <opencv2/imgproc.hpp>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <opencv2/imgproc.hpp>
//#include "opencv2/imgproc.hpp"
//#include "opencv2/imgproc.hpp"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <opencv2/imgproc.hpp>
#ifdef _DEBUG
#pragma comment(lib,"opencv_imgproc343d.lib")
#else
#pragma comment(lib,"opencv_imgproc343.lib")
#endif

#endif



//#ifdef _DEBUG
//        #pragma comment(lib,"opencv_core2413d.lib")
//        #pragma comment(lib,"opencv_highgui2413d.lib")
//        #pragma comment(lib,"opencv_imgproc2413d.lib")
////      #pragma comment(lib,"opencv_video2413d.lib")
////      #pragma comment(lib,"opencv_nonfree2413d.lib")
////      #pragma comment(lib,"opencv_videostab2413d.lib")
////      #pragma comment(lib,"opencv_features2d2413d.lib")
////      #pragma comment(lib,"opencv_objdetect2413d.lib")
////      #pragma comment(lib,"opencv_flann2413d.lib")
////      #pragma comment(lib,"opencv_photo2413d.lib")
////      #pragma comment(lib,"opencv_calib3d2413d.lib")
////      #pragma comment(lib,"opencv_ml2413d.lib")
////      #pragma comment(lib,"opencv_contrib2413d.lib")
////      #pragma comment(lib,"opencv_ts2413d.lib")
////      #pragma comment(lib,"opencv_stitching2413d.lib")
////      #pragma comment(lib,"opencv_legacy2413d.lib")
//#else
//        #pragma comment(lib,"opencv_core2413.lib")
//        #pragma comment(lib,"opencv_highgui2413.lib")
//        #pragma comment(lib,"opencv_imgproc2413.lib")
////      #pragma comment(lib,"opencv_video2413.lib")
////      #pragma comment(lib,"opencv_nonfree2413.lib")
////      #pragma comment(lib,"opencv_videostab2413.lib")
////      #pragma comment(lib,"opencv_features2d2413.lib")
////      #pragma comment(lib,"opencv_objdetect2413.lib")
////      #pragma comment(lib,"opencv_flann2413.lib")
////      #pragma comment(lib,"opencv_photo2413.lib")
////      #pragma comment(lib,"opencv_calib3d2413.lib")
////      #pragma comment(lib,"opencv_ml2413.lib")
////      #pragma comment(lib,"opencv_contrib2413.lib")
////      #pragma comment(lib,"opencv_ts2413.lib")
////      #pragma comment(lib,"opencv_stitching2413.lib")
////      #pragma comment(lib,"opencv_legacy2413.lib")
//#endif
