//
//  Popup.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#ifndef Popup_h
#define Popup_h

#include "../Base/SMView.h"

class Popup;

class OnDismissListener {
public:
    virtual void onDismiss(Popup * popup) = 0;
};

class Popup : public SMView
{
public:
    typedef std::function<void(Popup*)> OnDismissCallback;
    
    virtual void show();
    
    virtual void dismiss(bool imediate=false);
    
    virtual void cancel() override;
    
    virtual void setCancelable(bool cancelable);
    
    void setOnDismissListener(OnDismissListener* l);
    
    void setOnDismissCallback(OnDismissCallback callback);
    
public:
    static void dismissAllPopup();
    
    static Popup* findPopupByTag(const int tag);
    
protected:
    Popup();
    
    virtual ~Popup();
    
    virtual bool init() override;
    
    void attachToSharedLayer();
    
    void callbackOnDismiss();
    
    void setBackgroundFadeOpacity(float bgFadeOpacity);
    
    float getBackgroundFadeOpacity() {return _bgFadeOpacity;}
    
private:
    OnDismissListener * _dismissListener;
    
    OnDismissCallback _dismissCallback;
    
    float _bgFadeOpacity;
    
    friend class PopupManager;
};


#endif /* Popup_h */
