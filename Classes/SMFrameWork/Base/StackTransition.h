//
//  StackTransition.h
//  test
//
//  Created by SteveMac on 2017. 10. 27..
//

#ifndef StackTransition_h
#define StackTransition_h

#include <2d/CCTransition.h>
#include <2d/CCActionInterval.h>

class ShapeSolidRect;
class MainMenuBar;
class BaseScene;

namespace StackTransition {
    
#define CREATE_STACK_TRANSITION(__TYPE__) \
static __TYPE__* create(float duration, int inStackIndex, BaseScene* rootScene) \
{ \
    __TYPE__* scene = new (std::nothrow) __TYPE__(); \
    if (scene && scene->initWithDuration(duration, inStackIndex, rootScene)) { \
        scene->autorelease(); \
        return scene; \
    } else { \
        CC_SAFE_DELETE(scene);  \
        scene = nullptr; \
        return nullptr; \
    } \
}

class Time {
public:
    static const float NORMAL;
    static const float FAST;
    static const float SLOW;
};
    
class BaseStackTransition : public cocos2d::TransitionScene {
public:
    Scene * getOutScene() const;
    
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    
protected:
    BaseStackTransition();
    virtual ~BaseStackTransition();
    
    virtual bool initWithDuration(float duration, int inStackIndex, BaseScene* rootScene);
    virtual void onEnter() override;
    virtual void onExit() override;
    virtual void onTransitionEnter();
    virtual void onTransitionExit();
    virtual cocos2d::FiniteTimeAction* getInAction() {return nullptr;}
    virtual cocos2d::FiniteTimeAction* getOutAction() {return nullptr;}
    virtual bool isDimLayerEnable() {return true;}
    virtual void onTransitionUpdate(cocos2d::Node* target, int tag, float t);
    virtual void onTransitionComplete(cocos2d::Node* target, int tag);
    
protected:
    virtual void sceneOrder() override { _isInSceneOnTop = true;}
    
protected:
    int _inStackIndex;
    int _outStackIndex;
    
    enum MenuDrawType {
        ON_ON,
        ON_OFF,
        OFF_ON,
        OFF_OFF,
    };
    
    MenuDrawType _menuDrawType;
    float _lastProgress;
    ShapeSolidRect* _dimLayer;
    cocos2d::Node* _menuDrawStub;
    bool _canceled;
};
  
    
class StackSwipeBack : public BaseStackTransition {
public:
    static StackSwipeBack* create(const int inStackIndex);
    void cancel();
    virtual void onTransitionEnter() override;
    virtual void onTransitionExit() override;
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    
protected:
    StackSwipeBack();
    virtual ~StackSwipeBack();
    virtual bool initWithDuration(float duration, int inStackIndex, BaseScene* rootScene) override;
    virtual void onTransitionUpdate(cocos2d::Node * target, int tag, float t) override;
protected:
    virtual void sceneOrder() override { _isInSceneOnTop = false;}
    void cancelNewScene(float dt);
    
private:
    int _inIndex;
    int _outIndex;
};
    
class StackToLeft : public BaseStackTransition
{
public:
    CREATE_STACK_TRANSITION(StackToLeft);
    
    virtual cocos2d::FiniteTimeAction* getInAction() override;
    virtual cocos2d::FiniteTimeAction* getOutAction() override;
    
protected:
    virtual void sceneOrder() override {_isInSceneOnTop = true;}
};
  
class StackToRight : public BaseStackTransition
{
public:
    CREATE_STACK_TRANSITION(StackToRight);
    
    virtual cocos2d::FiniteTimeAction* getInAction() override;
    virtual cocos2d::FiniteTimeAction* getOutAction() override;
    
protected:
    virtual void sceneOrder() override {_isInSceneOnTop = true;}
};
    
class SlideToLeft : public BaseStackTransition
{
public:
    CREATE_STACK_TRANSITION(SlideToLeft);
    
    virtual cocos2d::FiniteTimeAction* getInAction() override;
    virtual cocos2d::FiniteTimeAction* getOutAction() override;
    
protected:
    virtual void sceneOrder() override { _isInSceneOnTop = true;}
};
    
class SlideToRight : public BaseStackTransition
{
public:
    CREATE_STACK_TRANSITION(SlideToRight);
    
    virtual cocos2d::FiniteTimeAction* getInAction() override;
    virtual cocos2d::FiniteTimeAction* getOutAction() override;
    
protected:
    virtual void sceneOrder() override {_isInSceneOnTop = false;}
};
}


#endif /* StackTransition_h */
