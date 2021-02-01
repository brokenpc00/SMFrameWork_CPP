//
//  BaseScene.h
//  test
//
//  Created by SteveMac on 2017. 10. 27..
//

#ifndef BaseScene_h
#define BaseScene_h

#include "../Base/SMScene.h"

class BaseScene : public SMScene
{
public:
    bool isMainMenuEnabled() { return _mainMenuEnabled;}
    
    void setEnabledMainMenu(bool enabled) {_mainMenuEnabled = enabled;}
    
    virtual void onResetScene(){}
    
protected:
    BaseScene() : _mainMenuEnabled(true) {}
    
    
private:
    bool _mainMenuEnabled;
    
};

#endif /* BaseScene_h */
