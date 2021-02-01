//
//  Popup.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 24..
//

#include "Popup.h"
#include "PopupManager.h"


Popup::Popup() : _bgFadeOpacity(0.6), _dismissListener(nullptr), _dismissCallback(nullptr)
{
    
}

Popup::~Popup()
{
    
}

bool Popup::init()
{
    setCancelable(false);
    return true;
}

void Popup::show()
{
    PopupManager::getInstance().showPopup(this);
}

void Popup::dismiss(bool imediate)
{
    if (imediate) {
        callbackOnDismiss();
    }
    
    if (PopupManager::getInstance().dismissPopup(this, imediate)) {
        if (!imediate) {
            callbackOnDismiss();
        }
    }
}

void Popup::callbackOnDismiss()
{
    if (_dismissListener) {
        _dismissListener->onDismiss(this);
    }
    
    if (_dismissCallback) {
        _dismissCallback(this);
    }
}

void Popup::cancel()
{
    dismiss();
}

void Popup::setCancelable(bool cancelable)
{
    setCancelIfTouchOutside(cancelable);
}

void Popup::setOnDismissListener(OnDismissListener *l)
{
    _dismissListener = l;
}

void Popup::setOnDismissCallback(OnDismissCallback callback)
{
    _dismissCallback = callback;
}

Popup* Popup::findPopupByTag(const int tag)
{
    return PopupManager::getInstance().findPopupByTag(tag);
}

void Popup::dismissAllPopup()
{
    return PopupManager::getInstance().dismissAllPopup();
}
