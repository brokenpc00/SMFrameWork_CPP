//
//  ProgressPopup.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#ifndef ProgressPopup_h
#define ProgressPopup_h

#include "Popup.h"

class ProgressView;

typedef std::function<void()> OnCloseCallback;

class ProgressPopup : public Popup
{
public:
    static ProgressPopup* showProgress(float maxProgress, float delay=0);
    
    void setOnDismissCallback(const OnCloseCallback& callback) {_dismissCallback = callback;}
    
    void close(bool success);
    
    void setProgress(const float progress);
    
protected:
    virtual void show() override;
    virtual void dismiss(bool imediate) override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
protected:
    ProgressPopup();
    virtual ~ProgressPopup();
    
    bool initWithDelay(float delay);
    
private:
    void onComplete(ProgressView* progress);
    
private:
    class ShowAction;
    float _showValue;
    
    ProgressView* _progress;
    OnCloseCallback _dismissCallback;
};


#endif /* ProgressPopup_h */
