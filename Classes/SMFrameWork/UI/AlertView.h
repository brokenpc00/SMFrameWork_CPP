//
//  AlertView.h
//  iPet
//
//  Created by KimSteve on 2017. 6. 23..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef AlertView_h
#define AlertView_h

#include "../Base/SMView.h"
#include <cocos2d.h>

#define ALERT_MOVE_DOWN 0.2f
#define ALERT_DURATION 2.0f
#define ALERT_FADE_OUT 0.2f     // for toast

typedef std::function<void()> OK_BLOCK;
typedef std::function<void()> CANCEL_BLOCK;


typedef enum {
    kAlertTypeAlert = 0,
    kAlertTypeConfirm,
    kAlertTypeInformation,
    kAlertTypeToast
}kAlertType;

class SMButton;
class SMImageView;
class SMScene;

class AlertView : public SMView, public OnClickListener
{
public:
    AlertView();
    virtual ~AlertView();
    CREATE_VIEW(AlertView);
    
public:
    static void showAlert(int alertCode, std::string msg="", float duration=ALERT_DURATION);
    static void showAlert(std::string alertMemo, float duration=ALERT_DURATION);
    
    static void showInformation(std::string infoMessage, float duration=ALERT_DURATION);
    
    static void showConfirm(std::string title, std::string confirmMessage, std::string cancelTitle, std::string okTitle, const OK_BLOCK& okBlock, const CANCEL_BLOCK& cancelBlock);
    
    static void showToast(std::string title, std::string toastMessage, std::string cancelTitle, std::string okTitle, const OK_BLOCK& okBlock, const CANCEL_BLOCK& cancelBlock);
    
    static void showInputBox(std::string message, std::string iconName, std::string placeHolder, std::string cancelName, std::string okName, const CANCEL_BLOCK& cancelBlock=nullptr, const OK_BLOCK& okBlock=nullptr);
    
    virtual void onClick(SMView * view) override;
    
    static std::string getErrorMessageFromCode(int alertCode);
    
    void setHideAlert(float dt);
    
    static bool isAlreadyShow();

    static void hideAlert();
    
private:
    bool _isAlreadyShow;
    float _duration;
    cocos2d::Sprite* _alertSprite;
    cocos2d::Sprite* _notiSprite;
    static AlertView* getInstance();
    
    SMImageView * _alertImageView;
    SMImageView * _notiImageView;
    cocos2d::Label * _alertMemoLabel;
    
    SMView * _messageBodyBgView;
    SMButton * _okButton;
    SMButton * _cancelButton;
    SMButton * _closeButton;
    cocos2d::Label * _titleLabel;
    SMView * _contentLabelBgView;
    cocos2d::Label * _contentLabel;
    kAlertType _alertType;
    OK_BLOCK _okBlock;
    CANCEL_BLOCK _cancelBlock;
    std::string _okTitleString;
    
    void closeAlertView();
};


#endif /* AlertView_h */
