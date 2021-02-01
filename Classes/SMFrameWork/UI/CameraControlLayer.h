//
//  CameraControlLayer.h
//  iPet
//
//  Created by KimSteve on 2017. 7. 4..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef CameraControlLayer_h
#define CameraControlLayer_h

#include "../Base/SMView.h"

class CameraInterface;

// shutter, flash on/off, front/back, record/stop, rewind/forward... 등 camera control에 대한 button등의 액션처리
class CameraControlLayer : public SMView, public OnClickListener
{
public:
    static CameraControlLayer * create(const cocos2d::Size& size, CameraInterface* i);
    
    void enableControl(bool enabled, bool immediate);
    
    virtual void onClick(SMView * view) override;
    
    void setDeviceInfo(int cameraId, bool hasFlash, bool hasFrontFacing);
    
    void setFlashState(int flashState);
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch * touch, const cocos2d::Vec2 * point, MotionEvent * event) override;
    
protected:
    CameraControlLayer();
    virtual ~CameraControlLayer();
    
    bool initWithSize(const cocos2d::Size& size, CameraInterface * i);
    
private:
    class ShutterButton;
    class FlashButton;
    class SwitchButton;
    
    ShutterButton* _shutterButton;
    FlashButton* _flashButton;
    SwitchButton * _switchButton;
    CameraInterface * _interface;
    
    int _cameraId;
    bool _cameraHasFlash;
    bool _cameraHasFrontFacing;
};



#endif /* CameraControlLayer_h */
