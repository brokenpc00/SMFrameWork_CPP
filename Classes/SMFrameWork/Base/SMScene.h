//
//  SMScene.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef SMScene_h
#define SMScene_h

#include <2d/CCScene.h>
#include "SMView.h"
#include "Intent.h"
#include "../Base/SceneTransition.h"


#define CREATE_SCENE(__TYPE__) \
static __TYPE__* create(Intent* sceneParam = nullptr, SwipeType type = SwipeType::NONE) \
{ \
__TYPE__ *scene = new(std::nothrow) __TYPE__(); \
if (scene && scene->initWithSceneParam(sceneParam, type)) { \
scene->autorelease(); \
return scene; \
} else { \
delete scene; \
scene = nullptr; \
return nullptr; \
} \
}

class SMScene : public cocos2d::Scene {
    
public:
    enum SwipeType {
        NONE,
        MENU,
        BACK,
        DISMISS
    };
    
    enum Transition {
        Transition_IN,
        Transition_OUT,
        Transition_PAUSE,
        Transition_RESUME,
        Transition_SWIPE_IN,
        Transition_SWIPE_OUT
    };
    
    CREATE_SCENE(SMScene);
    
public:
    SMView * getRootView() { return _rootView; }
    
    SwipeType getSwipeType() { return _swipeType; }
    
    virtual void addChild(cocos2d::Node* child) override;
    virtual void addChild(cocos2d::Node* child, int localZOrder) override;
    virtual void addChild(cocos2d::Node* child, int localZOrder, int tag) override;
    virtual void addChild(cocos2d::Node* child, int localZOrder, const std::string& name) override;
    
    virtual cocos2d::Node* getChildByTag(int tag) const override;
    virtual cocos2d::Node* getChildByName(const std::string& name) const override;
    
    virtual void removeChild(cocos2d::Node* child, bool cleanup = true) override;
    virtual void removeChildByTag(int tag, bool cleanup = true) override;
    virtual void removeChildByName(const std::string& name, bool cleanup = true) override;
    
    virtual bool onBackPressed() { return false; }
    
    Intent* getSceneParam() { return _sceneParam; }
    
    Intent* getSceneResult() { return _sceneResult; }
    
    void setSceneResult(Intent* result);
    
    virtual void onSceneResult(SMScene* fromScene, Intent* result);
    
    virtual void onTransitionProgress(const Transition t, const int tag, const float progress) {}
    
    virtual void onTransitionStart(const Transition t, const int tag) {}
    
    virtual void onTransitionComplete(const Transition t, const int tag) {}
    
    virtual void onTransitionReplaceSceneDidFinish(){}
    
    virtual bool canSwipe(const cocos2d::Vec2& worldPoint, const SwipeType type);
    
    virtual void onExitBackground() {}
    
    virtual void onEnterForground() {}
    
    virtual void startScene(SMScene* sccene);
    
    virtual void finishScene(Intent* result = nullptr);
    
    cocos2d::Director * getDirector() {return _director;}
    

protected:
    SMScene();
    virtual ~SMScene();

    virtual bool initWithSceneParam(Intent* sceneParam, SwipeType type);
    
    void setRootView(SMView* newRootView);
    
    void setSwipeType(SwipeType type) { _swipeType = type; }
    
    void setTransitionAnim(SMView* view, float offsetX, float offsetY, float duration, float delay, bool isIn);
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(SMScene);
    
private:
    virtual void onKeyReleased(const cocos2d::EventKeyboard::KeyCode keyCode, const cocos2d::Event * event);
    
    Intent* _sceneParam;
    
    Intent* _sceneResult;
    
    SMView* _rootView;
    
    SwipeType _swipeType;

    
public:
    bool isMainMenuEnabled() { return _mainMenuEnabled;}
    
    void setEnabledMainMenu(bool enabled) {_mainMenuEnabled = enabled;}
    
    virtual void onResetScene(){}
    
protected:
//    BaseScene() : _mainMenuEnabled(true) {}
    
    
private:
    bool _mainMenuEnabled;
};





#endif /* SMScene_h */
