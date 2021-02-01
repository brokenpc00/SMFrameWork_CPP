//
//  SMMenuTransitionScene.h
//  FreeTrip
//
//  Created by N15051 on 2020/04/07.
//

#ifndef SMMenuTransitionScene_h
#define SMMenuTransitionScene_h

#include "SMScene.h"
#include "../UI/ActionBar.h"
#include <cocos2d.h>

class SMMenuTransitionScene : public SMScene, public ActionBarListener
{
public:
    static SMMenuTransitionScene * create(ActionBar * menuBar, Intent* sceneParam = nullptr, SwipeType type = SwipeType::BACK) {
        SMMenuTransitionScene * scene = new (std::nothrow)SMMenuTransitionScene();
        if (scene && scene->initWithMenuBar(menuBar, sceneParam, type)) {
            scene->autorelease();
            return scene;
        } else {
            delete scene;
            scene = nullptr;
            return nullptr;
        }
    }
    
protected:
    SMMenuTransitionScene();
    virtual ~SMMenuTransitionScene();
    virtual bool initWithMenuBar(ActionBar * menuBar, Intent* sceneParam = nullptr, SwipeType type = SwipeType::BACK);
    
    
    virtual bool onActionBarClick(SMView * view) override;
    virtual void goHome() override;
    
    // scene transition override
    virtual void onTransitionProgress(const Transition type, const int tag, const float progress) override;
    
    virtual void onTransitionStart(const Transition type, const int tag) override;
    
    virtual void onTransitionComplete(const Transition type, const int tag) override;

    ActionBar * _menuBar;
    
    
    void setActionBarTitle(const std::string& titleText);

    void setActionBarMenu(ActionBar::MenuType menuButtonType);
    
    void setActionBarColorSet(const ActionBar::ColorSet& colorSet);

    void setActionBarButton(const ActionBar::MenuType butttonId1, const ActionBar::MenuType butttonId12 = ActionBar::MenuType::NONE);

private:
    std::string _menuBarTitle;
    std::string _prevMenuTitle;
    ActionBar::MenuType _menuBarButton[2];
    ActionBar::MenuType _menuBarMenu;
    ActionBar::ColorSet _menuBarColorSet;
    std::vector<ActionBar::MenuType> _prevManuBarButton;
    bool _swipeStarted;
    
    ActionBar::MenuType _fromMenuType;
    ActionBar::MenuType _toMenuType;
};



#endif /* SMMenuTransitionScene_h */
