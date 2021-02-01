//
//  SideMenu.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#ifndef SideMenu_h
#define SideMenu_h

#include "../Base/SMView.h"
#include "../Base/SMImageView.h"
#include "../Base/SMButton.h"
#include "../Base/SMScene.h"
#include "../Base/SMTableView.h"
#include "../Network/SMComm.h"

typedef enum {
    kSideMenuStateClose = 0,
    kSideMenuStateOpen,
    kSideMenuStateMoving,
}kSideMenuState;

typedef std::function<void()> SideMenuOpenClose;

class SideMenuListener
{
public:
    virtual bool onSideMenuSelect(int tag) {return false;};
    virtual void onSideMenuVisible(bool visible){}
};

class EdgeSwipeForMenu;


class SideMenu : public SMView, public OnClickListener, public ApiListener
{
public:
    SideMenu();
    virtual ~SideMenu();
    CREATE_VIEW(SideMenu);
    static SideMenu* getInstance();
    static void openMenu(SMScene * mainScene, const SideMenuOpenClose & callback=NULL);
    static void closeMenu(const SideMenuOpenClose & callback=NULL);
    
    virtual void onClick(SMView* view) override;
    
    void setSwipeLayer(EdgeSwipeForMenu* swipeLayer) { _swipeLayer = swipeLayer; }
    
    void setSideMenuListener(SideMenuListener * l) {_listener=l;}

    void setOpenPosition(const float position);
    float getOpenPosition() const { return _lastPosition; }
    kSideMenuState getState() const { return _state; }
    
    SideMenuOpenClose _callback;
    std::function<void(int, float)> _sideMenuUpdateCallback;
    
    void setUserInfo();
    
    cocos2d::Node * cellForRowAtIndexPath(const IndexPath& indexPath);
    
    
    class SideMenuCell : public SMView, public OnClickListener
    {
    public:
        SideMenuCell(){}
        ~SideMenuCell(){}
        CREATE_VIEW(SideMenuCell);
        
        virtual void onClick(SMView*view) override;
        SideMenu * parent;
        SMView * contentsView;
        cocos2d::Label * cellTitle;
        SMImageView * cellRightIcon;
        cocos2d::Label * cellRightLabel;
        SMButton * cartCountButton;
        
    };
    
public:
//    void onNoticeMenu();
//    void onContractMenu();
//    void onDelayMenu();
//    void onExchangeMenu();
//    void onUploadMenu();
//    void onLogOutMenu();
//    void onLoginMenu();
    
    
    
// network test
public:
    
private:
    SMView* _contentsView;
    SMButton * _profileButton;
    SMButton * _signInButton;
    std::vector<std::string> _menuNames;
    SMTableView * _sideMenuTableView;
    
private:
    virtual bool init() override;
    EdgeSwipeForMenu* _swipeLayer;
    kSideMenuState _state;
    SideMenuListener* _listener;
    float _lastPosition;
    DISALLOW_COPY_AND_ASSIGN(SideMenu);
};





#endif /* SideMenu_h */
