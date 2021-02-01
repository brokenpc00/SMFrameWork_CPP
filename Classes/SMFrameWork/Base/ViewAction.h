//
//  ViewAction.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef ViewAction_h
#define ViewAction_h

#include "SMView.h"
#include "../Util/ViewUtil.h"
#include "ShaderNode.h"
#include "ShaderUtil.h"
#include <2d/CCActionInterval.h>
#include <2d/CCTweenFunction.h>
#include <2d//CCActionGrid3D.h>

namespace ViewAction {
    
    class BGColorTo : public cocos2d::ActionInterval {
        
    public:
        BGColorTo(){}
        virtual ~BGColorTo() {}
        
        static BGColorTo* create(float duration, const cocos2d::Color4F& color) {
            auto bgColorTo = new (std::nothrow) BGColorTo();
            
            if (bgColorTo != nullptr && bgColorTo->initWithDuration(duration)) {
                bgColorTo->autorelease();
                bgColorTo->_toColor = cocos2d::Vec4(color.r, color.g, color.b, color.a);
            }
            
            return bgColorTo;
        }
        
        virtual void startWithTarget(cocos2d::Node *target) override {
            auto view = (SMView*)target;
            if (view) {
                cocos2d::ActionInterval::startWithTarget(target);
                cocos2d:: Color4F color = view->getBackgroundColor();
                _startColor = cocos2d::Vec4(color.r, color.g, color.b, color.a);
                _deltaColor = _toColor - _startColor;
            }
        }
        
        virtual void update(float t)  override {
            auto view = (SMView*)_target;
            if (view) {
                cocos2d::Vec4 color = _startColor + _deltaColor * t;
                view->setBackgroundColor4F(cocos2d::Color4F(color.x, color.y, color.z, color.w));
            }
        }
        
    protected:
        cocos2d::Vec4 _startColor;
        cocos2d::Vec4 _toColor;
        cocos2d::Vec4 _deltaColor;
        
    private:
        CC_DISALLOW_COPY_AND_ASSIGN(BGColorTo);
    };
    
    class FontColorTo : public cocos2d::ActionInterval {
    public:
        FontColorTo(){}
        virtual ~FontColorTo() {}
        
        static FontColorTo* create(float duration, const cocos2d::Color4B& toColor, cocos2d::Label* target = nullptr) {
            auto action = new FontColorTo();
            
            if (action && action->initWithDuration(duration)) {
                action->autorelease();
                action->_toColor = ViewUtil::colorToVec4(toColor);
                action->_label = target;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            if (_label == nullptr) {
                _label = (cocos2d::Label*)target;
            }
            _startColor = ViewUtil::colorToVec4(_label->getTextColor());
            _deltaColor = _toColor - _startColor;
        }
        
        virtual void update(float t)  override {
            cocos2d::Vec4 color = _startColor + _deltaColor * t;
            _label->setTextColor(ViewUtil::vec4ToColor4B(color));
        }
        
        void setValue(const cocos2d::Color4B& toColor) {
            _toColor = ViewUtil::colorToVec4(toColor);
        }
        
        
    protected:
        cocos2d::Vec4 _startColor;
        cocos2d::Vec4 _toColor;
        cocos2d::Vec4 _deltaColor;
        cocos2d::Label * _label;
        
    private:
        CC_DISALLOW_COPY_AND_ASSIGN(FontColorTo);
    };
    
    class ColorTo : public cocos2d::ActionInterval {
    public:
        ColorTo(){}
        virtual ~ColorTo() {}
        
        static ColorTo* create(float duration, const cocos2d::Color4F& toColor, cocos2d::Node* target = nullptr) {
            auto action = new ColorTo();
            
            if (action && action->initWithDuration(duration)) {
                action->autorelease();
                action->_toColor = ViewUtil::colorToVec4(toColor);
                action->_node = target;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            if (_node == nullptr) {
                _node = target;
            }
            _startColor = ViewUtil::colorToVec4(_node->getColor(), _node->getOpacity());
            _deltaColor = _toColor - _startColor;
        }
        
        virtual void update(float t)  override {
            cocos2d::Vec4 color = _startColor + _deltaColor * t;
            _node->setColor(ViewUtil::vec4ToColor3B(color));
            _node->setOpacity(ViewUtil::vec4ToOpacity(color));
        }
        
        void setValue(const cocos2d::Color4F& toColor) {
            _toColor = ViewUtil::colorToVec4(toColor);
        }
        
        
    protected:
        cocos2d::Vec4 _startColor;
        cocos2d::Vec4 _toColor;
        cocos2d::Vec4 _deltaColor;
        cocos2d::Node* _node;
        
    private:
        CC_DISALLOW_COPY_AND_ASSIGN(ColorTo);
    };
    
    class AlphaTo : public cocos2d::ActionInterval {
    public:
        AlphaTo(){}
        virtual ~AlphaTo() {}
        
        static AlphaTo* create(float duration, const float toAlpha, cocos2d::Node* target = nullptr) {
            auto action = new AlphaTo();
            
            if (action && action->initWithDuration(duration)) {
                action->autorelease();
                action->_toAlpha = toAlpha;
                action->_node = target;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            if (_node == nullptr) {
                _node = target;
            }
            _startAlpha = _node->getOpacity() / 255.0;
            _deltaAlpha = _toAlpha - _startAlpha;
        }
        
        virtual void update(float t)  override {
            float a = _startAlpha + _deltaAlpha * t;
            _node->setOpacity((GLubyte)(0xFF * a));
        }
        
        void setValue(float toAlpha) {
            _toAlpha = toAlpha;
        }
        
    protected:
        float _startAlpha;
        float _toAlpha;
        float _deltaAlpha;
        
        cocos2d::Node* _node;
        
    private:
        CC_DISALLOW_COPY_AND_ASSIGN(AlphaTo);
    };
    
    
    class Bounce : public cocos2d::ActionInterval {
    public:
        Bounce(){}
        virtual ~Bounce() {}
        
        static Bounce* create(float duration, const float bounceRate, int bounceCount, float maxScale=1.0, cocos2d::Node* target = nullptr) {
            auto action = new Bounce();
            
            if (action && action->initWithDuration(duration)) {
                action->autorelease();
                action->_bounceRate = bounceRate;
                action->_bounceCount = bounceCount;
                action->_maxScale = maxScale;
                action->_node = target;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            if (_node == nullptr) {
                _node = target;
            }
        }
        
        virtual void update(float t)  override {
            float a = std::cos(t * M_PI_2);
            float s = (float)(a * _bounceRate * std::abs(std::sin(t * _bounceCount * M_PI)));
            _target->setScale((1.0 + s)*_maxScale);
        }
        
    protected:
        float _bounceRate;
        int _bounceCount;
        float _maxScale;
        cocos2d::Node* _node;
        
    };
    
    typedef std::function<void()> OnRingWaveComplete;
    
    class RingWave : public SMView {
    public:
        static RingWave* show(cocos2d::Node* parent, float x, float y, float size, float duration, float delay, cocos2d::Color4F* color = nullptr, OnRingWaveComplete ringWaveComplete=nullptr);
        
        void setWaveColor(cocos2d::Color4F color);
        
        OnRingWaveComplete _ringWaveComplete;
        
    protected:
        RingWave();
        virtual ~RingWave();
        
    private:
        bool initWithParam(float size, float duration, float delay, cocos2d::Color4F* color);
        
        ShapeCircle* _circle;
    };
    
    class RingWave2 : public SMView {
    public:
        static RingWave2* create(float minRadius, float maxRadius);
        
        void hide();
        
    private:
        class WaveAction;
        
        bool initWithParam(float minRadius, float maxRadius);
        
        ShapeCircle* _ring;
    };
    
    
    class ScaleSine : public cocos2d::ActionInterval {
    public:
        ScaleSine(){}
        virtual ~ScaleSine() {}
        
        static ScaleSine* create(float duration, float maxScale) {
            auto action = new ScaleSine();
            
            if (action && action->initWithDuration(duration)) {
                action->autorelease();
                action->_deltaScale = maxScale - 1.0;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            _baseScale = target->getScale();
        }
        
        
        virtual void update(float t)  override {
            _target->setScale(_baseScale*(1.0 + _deltaScale * std::sin(t * M_PI)));
        }
        
    protected:
        float _deltaScale;
        float _baseScale;
        
    };
    
    
#define CREATE_DELAY_ACTION(__TYPE__) \
static __TYPE__* create(bool autorelease=true) {\
__TYPE__* action = new (std::nothrow)__TYPE__();\
if (action == nullptr || !action->initWithDuration(0)) {\
return nullptr;\
}\
if (autorelease) action->autorelease();\
return action;\
}
    
    class DelayBaseAction: public cocos2d::ActionInterval {
    public:
        
        virtual void update(float t)  override {
            float time = t * getDuration();
            if (time < _delay)
                return;
            
            if (!_started) {
                _started = true;
                _ended = false;
                onStart();
            }
            
            float tt = (time - _delay) / _duration;
            if (_reverse) tt = 1 - tt;
            onUpdate(tt);
            
            if (t >= 1.0 && !_ended) {
                onEnd();
                _ended = true;
            }
        }
        
        virtual void onStart() {}
        virtual void onEnd() {}
        virtual void onUpdate(float t) = 0;
        
        void reverse() {
            _reverse = true;
        }
        
        void setTimeValue(float duration, float delay) {
            _duration = duration;
            _delay = delay;
            setDuration(duration+delay);
            _started = false;
        }
        
    protected:
        DelayBaseAction() : _reverse(false) {}
        
        float _duration, _delay;
        
    private:
        bool _started;
        bool _ended;
        bool _reverse;
    };
    
    
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    // Transform Action
    //-------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------
    class TransformAction : public DelayBaseAction {
    public:
        CREATE_DELAY_ACTION(TransformAction);
        
        TransformAction& toScale(float scale);
        
        TransformAction& toPositionX(float positionX);
        
        TransformAction& toPositionY(float positionY);
        
        TransformAction& toPosition(const cocos2d::Vec2& position);
        
        TransformAction& toAlpha(const float alpha);
        
        TransformAction& removeOnFinish();
        
        TransformAction& enableOnFinish();
        
        TransformAction& enableSmooth();

        TransformAction& disableOnFinish();
        
        TransformAction& invisibleOnFinish();
        
        TransformAction& runFuncOnFinish(std::function<void(cocos2d::Node* target, int tag)> callback);
        
        TransformAction& runActionOnFinish(cocos2d::Action* action);
        
        TransformAction& setTweenFunc(cocos2d::tweenfunc::TweenType type, float easingParam = 0);
        
        TransformAction& setUpdateCallback(std::function<void(cocos2d::Node* target, int tag, float t)> callback);
        
    protected:
        TransformAction();
        
        virtual ~TransformAction();
        
        virtual void onStart() override;
        
        virtual void onUpdate(float t) override;
        
        virtual void onEnd() override;
        
    private:
        bool _scaleAction;
        bool _positionXAction;
        bool _positionYAction;
        bool _positionAction;
        bool _alphaAction;
        bool _rotateAction;
        
        float _fromScale, _toScale;
        float _fromAlpha, _toAlpha;
        float _fromAngle, _toAngle;
        cocos2d::Vec2 _fromPosition, _toPosition;
        
        bool _removeOnFinish;
        bool _enableOnFinish;
        bool _disableOnFinish;
        bool _invisibleOnFinish;
        
        bool _smooth;
        
        cocos2d::tweenfunc::TweenType _tweenType;
        float _easingParam;
        
        std::function<void(cocos2d::Node* target, int tag)> _callbackOnFinish;
        
        std::function<void(cocos2d::Node* target, int tag, float t)> _updateCallback;
        
        cocos2d::Action* _action;
        
        SMView* _view;
    };
    
    
    class GenieAction : public cocos2d::ActionInterval {
    public:
        GenieAction() {}
        virtual ~GenieAction() {}
        
        static GenieAction* create(float duration, cocos2d::Sprite* sprite, const cocos2d::Vec2& removeAnchor) {
            auto action = new (std::nothrow)GenieAction();
            
            if (action && action->initWithDuration(duration)) {
                action->_sprite = sprite;
                action->_removeAnchor = removeAnchor;
                action->autorelease();
            } else {
                CC_SAFE_DELETE(action);
                action = nullptr;
            }
            
            return action;
        }
        
        virtual void startWithTarget(cocos2d::Node* target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            
            ShaderUtil::setGenieEffectSpriteShader(_sprite);
            
            auto state = _sprite->getGLProgramState();
            state->setUniformVec2("u_anchor", _removeAnchor);
        }
        
        virtual void update(float t)  override {
            if (t < 0) t *= 0.1;
            
            auto state = _sprite->getGLProgramState();
            state->setUniformFloat("u_progress", M_PI_2 * t);
        }
        
    private:
        cocos2d::Sprite* _sprite;
        
        cocos2d::Vec2 _removeAnchor;
    };
    
    class PageTurnAction : public cocos2d::Grid3DAction
    {
    public:
        static PageTurnAction * create(float duration, const cocos2d::Size& gridSize, bool directionLeft, bool forward);
        
    public:
        virtual cocos2d::GridBase* getGrid() override;
        virtual PageTurnAction* clone() const override;
        virtual void update(float t) override;

    protected:
        PageTurnAction();
        virtual ~PageTurnAction();
    private:
        bool _directionLeft;
        bool _forward;
    };
    
    
    class CurtainAction : public cocos2d::Grid3DAction
    {
    public:
        static CurtainAction * create(float duration, const cocos2d::Size& gridSize, const cocos2d::Vec2 pos);
        
    public:
        virtual cocos2d::GridBase* getGrid() override;
        virtual CurtainAction* clone() const override;
        virtual void update(float t) override;
        
    protected:
        CurtainAction();
        virtual ~CurtainAction();
        
    private:
        cocos2d::Vec2 _pos;
    };

}



#endif /* ViewAction_h */
