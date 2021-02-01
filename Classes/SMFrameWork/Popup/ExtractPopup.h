//
//  ExtractPopup.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 13..
//

#ifndef ExtractPopup_h
#define ExtractPopup_h

#include "Popup.h"
#include <cocos2d.h>

class ShapeRoundLine;

class ExtractPopup : public Popup {
public:
    static ExtractPopup* extractShow(float delay = 0);
    
    static void close(bool imediate = false);
    
    static bool isShow();
    
    static void setProgress(int cur, int total, float progress, std::string desc);
    
protected:
    virtual void show() override;
    
    virtual void dismiss(bool imediate) override;
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
protected:
    ExtractPopup();
    
    virtual ~ExtractPopup();
    
    bool initWithDelay(float delay);
    
private:
    class ShowAction;
    float _showValue;
    
    int _cur;
    int _total;
    
    ShapeRoundLine * _progress;
    cocos2d::Label * _title;
    cocos2d::Label * _desc;
};

#endif /* ExtractPopup_h */
