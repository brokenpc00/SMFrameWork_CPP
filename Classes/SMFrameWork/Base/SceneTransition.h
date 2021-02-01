//
//  SceneTransition.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef SceneTransition_h
#define SceneTransition_h

#include <2d/CCTransition.h>
#include <2d/CCActionInterval.h>


class ShapeSolidRect;

namespace SceneTransition {
    
    class Time {
    public:
        static const float NORMAL;
        static const float FAST;
        static const float SLOW;
    };
    
    
#define CREATE_SCENE_TRANSITION(__TYPE__) \
static __TYPE__* create(float t, Scene* inScene) \
{ \
__TYPE__ *scene = new(std::nothrow) __TYPE__(); \
if (scene && scene->initWithDuration(t, inScene)) { \
scene->autorelease(); \
return scene; \
} else { \
delete scene; \
scene = nullptr; \
return nullptr; \
} \
}
    
    //---------------------------------------------------------------------------
    // Base Scene Transition
    //---------------------------------------------------------------------------
    class BaseSceneTransition : public cocos2d::TransitionScene {
        
    public:
        Scene* getOutScene() const;
        
        virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
        
    protected:
        BaseSceneTransition();
        
        virtual ~BaseSceneTransition();
        
        virtual void onEnter() override;
        
        virtual void onExit() override;
        
        virtual cocos2d::FiniteTimeAction* getInAction() { return nullptr; }
        
        virtual cocos2d::FiniteTimeAction* getOutAction() { return nullptr; }
        
        virtual bool isDimLayerEnable() { return true; }
        
        virtual void updateProgress(const float progress);
        
        void updateComplete();
        
        virtual bool isNewSceneEnter()  = 0;
        
    protected:
        enum MenuDrawType {
            ON_ON,
            ON_OFF,
            OFF_ON,
            OFF_OFF
        };
        
        MenuDrawType _menuDrawType;
        
        float _lastProgress;
        
        ShapeSolidRect* _dimLayer;
        
        cocos2d::Node* _menuDrawStub;
        
        class ProgressUpdater;
    };
    
    //---------------------------------------------------------------------------
    // Swipe Back Transition
    //---------------------------------------------------------------------------
    class SwipeBack : public BaseSceneTransition
    {
    public:
        static SwipeBack* create(Scene *scene);
        
        void cancel();
        
        virtual void onEnter() override;
        
        virtual void onExit() override;
        
        virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
        
    protected:
        SwipeBack();
        
        virtual ~SwipeBack();
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
        
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void updateProgress(const float progress) override;

    protected:
        bool _isCanceled;
        
        void cancelNewScene(float dt);
    };
    
    //---------------------------------------------------------------------------
    // Swipe Dismiss Transition
    //---------------------------------------------------------------------------
    class SwipeDismiss : public SwipeBack
    {
    public:
        static SwipeDismiss* create(Scene *scene);
        
        virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
    };
    
    //---------------------------------------------------------------------------
    // CrossFade
    //---------------------------------------------------------------------------
    class CrossFade : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(CrossFade);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        
        virtual cocos2d::FiniteTimeAction* getOutAction() override;

    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
    };


    //---------------------------------------------------------------------------
    // SlideInToTop
    //---------------------------------------------------------------------------
    class SlideInToTop : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(SlideInToTop);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
    };
    
    //---------------------------------------------------------------------------
    // SlideOutToBottom
    //---------------------------------------------------------------------------
    class SlideOutToBottom : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(SlideOutToBottom);
        
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
    };
    
    //---------------------------------------------------------------------------
    // SlideInToLeft
    //---------------------------------------------------------------------------
    class SlideInToLeft : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(SlideInToLeft);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
    };
    
    //---------------------------------------------------------------------------
    // SlideOutToRight
    //---------------------------------------------------------------------------
    class SlideOutToRight : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(SlideOutToRight);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
    };
    
    //---------------------------------------------------------------------------
    // EditMainIn
    //---------------------------------------------------------------------------
    class EditMainIn : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(EditMainIn);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
        
        virtual bool isDimLayerEnable() override { return false; }
        
    };
    
    //---------------------------------------------------------------------------
    // EditMainOut
    //---------------------------------------------------------------------------
    class EditMainOut : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(EditMainOut);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
        virtual void onExit() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
        
        virtual bool isDimLayerEnable() override { return false; }
    };
    
    //---------------------------------------------------------------------------
    // EditSubInOut
    //---------------------------------------------------------------------------
    class EditSubInOut : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(EditSubInOut);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
        
        virtual bool isDimLayerEnable() override { return false; }
        
    };
    
    //---------------------------------------------------------------------------
    // HoldIn
    //---------------------------------------------------------------------------
    class HoldIn : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(HoldIn);
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
    };
    
    //---------------------------------------------------------------------------
    // HoldOut
    //---------------------------------------------------------------------------
    class HoldOut : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(HoldOut);
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
    };
    
    
    //---------------------------------------------------------------------------
    // HoldOn
    //---------------------------------------------------------------------------
    class HoldOn : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(HoldOn);
        
        virtual bool isDimLayerEnable() override { return false; }
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
    };
    
    //---------------------------------------------------------------------------
    // SignComplete
    //---------------------------------------------------------------------------
    class SignComplete : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(SignComplete);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return true; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = true; }
        
        virtual bool isDimLayerEnable() override { return false; }
        
    };
    
    class FadeIn : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(FadeIn);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
        
        virtual bool isDimLayerEnable() override { return false; }
    private:
//        cocos2d::Sprite
    };

    class FadeOut : public BaseSceneTransition
    {
    public:
        CREATE_SCENE_TRANSITION(FadeOut);
        
        virtual cocos2d::FiniteTimeAction* getInAction() override;
        virtual cocos2d::FiniteTimeAction* getOutAction() override;
        
    protected:
        virtual bool isNewSceneEnter() override { return false; }
        
        virtual void sceneOrder() override { _isInSceneOnTop = false; }
        
        virtual bool isDimLayerEnable() override { return false; }
    };
    
}


#endif /* SceneTransition_h */
