//
//  CameraConstant.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 5..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef CameraConstant_h
#define CameraConstant_h

// 실시간 영상 전송을 위한 구조체
struct VideoFrame {
    int width;
    int height;
    size_t length;
    uint8_t* buffer;
    
    VideoFrame() : width(0), height(0), length(0), buffer(nullptr) {}
};

// camera 설정을 위한 value class
class CameraDeviceConst {
public:
    // camera - back, front
    enum Facing {
        UNSPECIFIED = -1,
        BACK        = 0,
        FRONT       = 1
    };
    
    // flash - auto, on, off
    enum Flash {
        AUTO    = 0,
        ON      = 1,
        OFF = 2
    };
    
    // camera image frame crop
    enum Crop {
        NONE = 0,
        RATIO_4_3 = 1,
        RATIO_1_1 = 2
    };
    
    // camera state notify
    enum Notify {
        START = 0,  // camera 기동 시
        STOP = 1, // camera 종료 시
        SNAP = 2, // still image capture
        FIRST = 3, // 첫번째 image frame 전달 시
        DENY = 4, // 카메라 접근권한 없음
        RECORD = 5, // video recode 시작
        REWIND =  6, // 뒤로 감기
        FORWARD = 7, // 앞으로 감기
    };
};


#endif /* CameraConstant_h */
