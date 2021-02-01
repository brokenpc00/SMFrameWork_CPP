//
//  PopupManager.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#ifndef PopupManager_h
#define PopupManager_h

class Popup;

class PopupManager
{
public:
    static PopupManager& getInstance();
    
    Popup* findPopupByTag(const int tag);
    
    
    void showPopup(Popup* popup);
    
    bool dismissPopup(Popup* popup, bool imediate=false);
    
    void dismissAllPopup();
    
    void bringOnTop(Popup* popup);
    
protected:
    PopupManager();
    virtual ~PopupManager();
};


#endif /* PopupManager_h */
