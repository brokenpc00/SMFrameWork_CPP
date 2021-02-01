//
//  LoadingPopup.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#ifndef LoadingPopup_h
#define LoadingPopup_h

#include "Popup.h"
#define POPUP_ID_LOADING    (0xFFFFF1)

class LoadingPopup : public Popup
{
public:
    static LoadingPopup* showLoading(float delay = 0, int tag=POPUP_ID_LOADING);
    
    static void close(bool imediate = false, int tag=POPUP_ID_LOADING);
    
    static bool isShow(int tag=POPUP_ID_LOADING);
    
protected:
    virtual void show() override;
    
    virtual void dismiss(bool imediate) override;
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;

protected:
    LoadingPopup();
    
    virtual ~LoadingPopup();
    
    bool initWithDelay(float delay);
    
private:
    class ShowAction;
    float _showValue;
};

#endif /* LoadingPopup_h */
