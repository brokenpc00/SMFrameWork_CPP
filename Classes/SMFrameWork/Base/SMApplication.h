//
//  SMApplication.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef SMApplication_h
#define SMApplication_h

#include <cocos2d.h>
#include "SMView.h"
#include "EdgeSwipeLayer.h"
#include "../UI/SideMenu.h"

class SMApplication : public cocos2d::Application
{
public:
    SMApplication();
    ~SMApplication();
    
    virtual bool applicationDidFinishLaunching()override;
    
private:
    cocos2d::Director * _director;    
    SMView * _dimLayer;
    SMView * _swipeLayer;   // first interaction for other view
    EdgeSwipeForBack* _backSwipe;
    EdgeSwipeForDismiss* _dismissSwipe;
    EdgeSwipeForMenu* _menuSwipe;
    SideMenu* _sideMenu;
    static const int VALID_TOUCH_COUNT;
    
    MotionEvent _motionEvent;
    
    SMView* _touchMotionTarget;
    
    float _lastTouchDownTime;
    
    cocos2d::EventListenerTouchOneByOne* _touchHandler;
    
private:
    void onSideMenuUpdateCallback(int state, float position);
    void onEdgeBackUpdateCallback(int state, float position);
    void onEdgeDismissUpdateCallback(int state, float position);
    
private:
    // method
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event);
    
    void handleTouchEvent(const int action, const std::vector<cocos2d::Touch*>& touches, const int actionIndex);
    void setupScaleFactor(const float designWidth);
    std::vector<cocos2d::Touch*> _touchHolder;
    std::vector<cocos2d::Touch*> _touchSender;
    
    int getTouchCount();
    
    int getTouchIndex(cocos2d::Touch* touch);
    
    void setTouchBit(cocos2d::Touch* touch);
    
    void clearTouchBit(cocos2d::Touch* touch = nullptr);
    
    bool isTouchBitSetted(cocos2d::Touch* touch);
    
    std::vector<cocos2d::Touch*>& buildTouchs();
    std::string _schemeUrl;

public:
    virtual void setSchemeUrl(std::string param) override {_schemeUrl = param;};
    virtual std::string getSchemeUrl() override {return _schemeUrl;}
};

#endif /* SMApplication_hpp */
