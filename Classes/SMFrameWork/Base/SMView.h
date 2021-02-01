//
//  SMView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef SMView_h
#define SMView_h

#include <2d/CCNode.h>
#include <2d/CCAction.h>
#include <base/CCDirector.h>
#include <base/ccTypes.h>
#include <base/CCEventListener.h>
#include <renderer/CCCustomCommand.h>
#include "../Interface/MotionEvent.h"
#include "../Const/SMViewConstValue.h"
#include <functional>

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

class SMScene;
class ShaderNode;
class OnClickListener;
class OnDoubleClickListener;
class OnLongClickListener;
class OnTouchListener;
class OnStateChangeListener;

#define TOUCH_FALSE         (0)
#define TOUCH_TRUE          (1)
#define TOUCH_INTERCEPT     (2)

#define CREATE_VIEW(__TYPE__) \
static __TYPE__* create(float x, float y, float width, float height) {\
__TYPE__* view = new (std::nothrow)__TYPE__();\
if (view != nullptr) {\
view->setPosition(cocos2d::Vec2(x, y));\
view->setContentSize(cocos2d::Size(width, height));\
if (view->init()) {\
view->autorelease();\
} else {\
CC_SAFE_DELETE(view);\
}\
}\
return view;\
}\
\
static __TYPE__* create(int tag, float x, float y, float width, float height, float anchorX=0, float anchorY=0) {\
__TYPE__* view = __TYPE__::create(x, y, width, height);\
if (view != nullptr) {\
view->setTag(tag);\
view->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));\
}\
return view;\
}\
\
static __TYPE__* create(int tag=0) {\
__TYPE__* view = __TYPE__::create(0, 0, 0, 0);\
if (view != nullptr) {\
view->setTag(tag);\
}\
return view;\
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// View Main Class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class SMView : public cocos2d::Node
{
public:
    
    enum class State {
        NORMAL    = 0,
        PRESSED   = 1,
        MAX_STATE = 2
    };
    
    CREATE_VIEW(SMView);
    
    virtual void onEnter() override;
    virtual void onExit() override;
    
    virtual void cleanup() override;
    
    virtual void removeChild(Node* child, bool cleanup = true) override;
    virtual void removeAllChildrenWithCleanup(bool cleanup) override;
    virtual void removeFromParent() override;
    
    virtual bool isTouchEnable() const;
    void setCancelIfTouchOutside(const bool cancelIfTouchOutside) { _cancelIfTouchOutside = cancelIfTouchOutside; }
    
    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
    
    void onBeforeVisitScissor();
    void onAfterVisitScissor();
    
    // Event listener
    void setOnClickListener(OnClickListener* l);
    void setOnLongClickListener(OnLongClickListener* l);
    void setOnDoubleClickListener(OnDoubleClickListener* l);
    void setOnStateChangeListener(OnStateChangeListener* l);
    void setOnTouchListener(OnTouchListener* l);
    
    void setOnClickCallback(std::function<void(SMView* view)> callback);
    void setOnLongClickCallback(std::function<void(SMView* view)> callback);
    void setOnDoubleClickCallback(std::function<void(SMView* view)> callback);
    void setOnStateChangeCallback(std::function<void(SMView* view, State state)> callback);
    
    // Scale setter
    virtual void setScale(float scaleX, float scaleY) override { setScale(scaleX, scaleY, true); }
    virtual void setScaleX(float scaleX) override { setScaleX(scaleX, true); }
    virtual void setScaleY(float scaleY) override { setScaleY(scaleY, true); }
    virtual void setScaleZ(float scaleZ) override { setScaleZ(scaleZ, true); }
    virtual void setScale(float scale) override { setScale(scale, true); }
    
    virtual void setScale(float scaleX, float scaleY, bool immediate);
    virtual void setScaleX(float scaleX, bool immediate);
    virtual void setScaleY(float scaleY, bool immediate);
    virtual void setScaleZ(float scaleZ, bool immediate);
    virtual void setScale(float scale, bool immediate);
    
    // Position & AnimOffset setter
    virtual void setPosition(const cocos2d::Vec2 &position) override { setPosition(position, true); }
    virtual void setPosition(float x, float y) override { setPosition(x, y, true); }
    virtual void setPositionX(float x) override { setPositionX(x, true); }
    virtual void setPositionY(float y) override { setPositionY(y, true); }
    virtual void setPositionZ(float z) override { setPositionZ(z, true); }
    virtual void setPosition3D(const cocos2d::Vec3& position) override { setPosition3D(position, true); }
    
    virtual void setPosition(const cocos2d::Vec2 &position, bool immediate);
    virtual void setPosition(float x, float y, bool immediate);
    virtual void setPositionX(float x, bool immediate);
    virtual void setPositionY(float y, bool immediate);
    virtual void setPositionZ(float positionZ, bool immediate);
    virtual void setPosition3D(const cocos2d::Vec3& position, bool immediate);
    
    virtual void setAnimOffset(const cocos2d::Vec2 &position, bool immediate = false);
    virtual void setAnimOffset3D(const cocos2d::Vec3 &position, bool immediate = false);
    
    virtual void setAnimScale(const float scale, bool immediate = false);
    virtual void setAnimScale(const cocos2d::Vec2& scale, bool immediate = false);
    
    virtual void setAnimRotate(const float rotate, bool immediate = false);
    virtual void setAnimRotate3D(const cocos2d::Vec3& rotate, bool immediate = false);
    
    // Anchor & Size setter
    virtual void setAnchorPoint(const cocos2d::Vec2& anchorPoint) override { setAnchorPoint(anchorPoint, true); }
    virtual void setContentSize(const cocos2d::Size& contentSize) override { setContentSize(contentSize, true); }
    
    virtual void setAnchorPoint(const cocos2d::Vec2& anchorPoint, bool immediate);
    virtual void setContentSize(const cocos2d::Size& contentSize, bool immediate);
    
    // Rotation setter
    virtual void setRotation(float rotation) override { setRotationZ(rotation, true); }
    virtual void setRotation3D(const cocos2d::Vec3& rotation) override { setRotation3D(rotation, true); }
    
    virtual void setRotation(float rotation, bool immediate) { setRotationZ(rotation, immediate); }
    virtual void setRotationX(float rotation, bool immediate);
    virtual void setRotationY(float rotation, bool immediate);
    virtual void setRotationZ(float rotation, bool immediate);
    virtual void setRotation3D(const cocos2d::Vec3& rotation, bool immediate);
    
    static bool smoothInterpolate(float& from, float to, float tolerance, float smoothDivider = SMViewConstValue::Config::SMOOTH_DIVIDER);
    static bool smoothInterpolateRotate(float& from, float to, float tolerance);
    static const float getShortestAngle(float from, float to);
    
    virtual void setColor4F(const cocos2d::Color4F& color);
    
    virtual cocos2d::Node* getBackgroundNode();
    void setBackgroundNode(cocos2d::Node* node);
    
    cocos2d::Color4F getBackgroundColor() { return _bgColor; }
    virtual void setBackgroundColor4F(const cocos2d::Color4F& color);
    virtual void setBackgroundColor(const cocos2d::Color3B& color) { setBackgroundColor(color, 0); }
    virtual void setBackgroundOpacity(GLubyte opacity) { setBackgroundOpacity(opacity, 0); }
    
    virtual void setBackgroundColor4F(const cocos2d::Color4F& color, float changeDurationTime);
    virtual void setBackgroundColor(const cocos2d::Color3B& color, float changeDurationTime);
    virtual void setBackgroundOpacity(GLubyte opacity, float changeDurationTime);
    
    const cocos2d::Vec2& getTouchStartPosition() const { return _touchStartPosition; }
    const cocos2d::Vec2& getTouchLastPosition() const { return _touchLastPosition; }
    
    const cocos2d::Vec3& getNewPosition() const { return _newPosition; }
    const cocos2d::Vec3& getNewRotate() const { return _newRotation; }
    const cocos2d::Vec3& getNewScale() const { return _newScale; }
    
    static inline const int stateToInt(const State& state) { return static_cast<int>(state); }
    
    virtual int dispatchTouchEvent(MotionEvent* event, SMView* view, bool checkBounds);
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event);
    
    void setScissorEnable(const bool enable);
    
    void setScissorRect(const cocos2d::Rect& rect);
    
    void setIgnoreTouchBounds(bool ignore) { _ignoreTouchBounds = ignore; }
    
    const SMView* getMotionTarget() { return _touchMotionTarget; }
    
    const cocos2d::Vec2 getLastTouchLocation() { return _lastTouchLocation; }
    
    virtual bool containsPoint(const cocos2d::Vec2& point);
    
    virtual void setEnabled(const bool enabled) { _enabled = enabled; }
    
    inline bool isEnabled() { return _enabled; }
    
    cocos2d::Node * getBgNode() {return _bgNode;}
    
    void changeParent(SMView * newParent);
    
protected:
    SMView();
    virtual ~SMView();
    
    virtual bool init() override;
    
    virtual void cancelTouchEvent(SMView* target, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event);
    
    virtual void onStateChangeNormalToPress() {}
    virtual void onStateChangePressToNormal() {}
    
    virtual void onUpdateOnVisit() {}
    void registerUpdate(const uint32_t flag);
    void unregisterUpdate(const uint32_t flag);
    inline bool isUpdate(const uint32_t flag) const { return _updateFlags&flag; }
    
    virtual cocos2d::Node* createBackgroundNode();
    
protected:
    
    static const uint32_t VIEWFLAG_POSITION;
    static const uint32_t VIEWFLAG_SCALE;
    static const uint32_t VIEWFLAG_ROTATE;
    static const uint32_t VIEWFLAG_ANCHOR_SIZE;
    static const uint32_t VIEWFLAG_COLOR;
    static const uint32_t VIEWFLAG_ANIM_OFFSET;
    static const uint32_t VIEWFLAG_ANIM_SCALE;
    static const uint32_t VIEWFLAG_ANIM_ROTATE;
    static const uint32_t VIEWFLAG_USER_SHIFT;
    
    static const uint32_t TOUCH_MASK_CLICK;
    static const uint32_t TOUCH_MASK_DOUBLECLICK;
    static const uint32_t TOUCH_MASK_LONGCLICK;
    static const uint32_t TOUCH_MASK_TOUCH;
    
    
    cocos2d::Node* _bgNode;
    
    cocos2d::Color4F _bgColor;
    
    cocos2d::Vec3 _realRotation;
    
    cocos2d::Vec3 _newRotation;
    
    cocos2d::Vec3 _realScale;
    
    cocos2d::Vec3 _newScale;
    
    cocos2d::Vec3 _realPosition;
    
    cocos2d::Vec3 _newPosition;
    
    cocos2d::Vec2 _newAnchorPoint;
    
    cocos2d::Size _newContentSize;
    
    cocos2d::Color4F _newRealColor;
    
    cocos2d::Vec3 _animOffset;
    
    cocos2d::Vec3 _newAnimOffset;
    
    cocos2d::Vec3 _animScale;
    
    cocos2d::Vec3 _newAnimScale;
    
    cocos2d::Vec3 _animRotation;
    
    cocos2d::Vec3 _newAnimRotation;
    
    bool _cancelIfTouchOutside;
    
    OnClickListener*        _onClickListener;
    OnLongClickListener*    _onLongClickListener;
    OnDoubleClickListener*  _onDoubleClickListener;
    OnTouchListener*        _onTouchListener;
    OnStateChangeListener*  _onStateChangeListener;
    
    float   _touchEventTime;
    SMView* _touchMotionTarget;
    bool    _touchHasFirstClicked;
    bool    _touchTargeted;
    bool    _ignoreTouchBounds;
    
    virtual void cancel();
    virtual void performClick(const cocos2d::Vec2& worldPoint);
    virtual void performLongClick(const cocos2d::Vec2& worldPoint);
    virtual void performDoubleClick(const cocos2d::Vec2& worldPoint);
    
    virtual void onSmoothUpdate(const uint32_t flags, float dt) {}
    void scheduleSmoothUpdate(const uint32_t flag);
    void unscheduleSmoothUpdate(const uint32_t flag);
    
    void setTouchMask(const uint32_t mask);
    inline void clearTouchMask(const uint32_t mask);
    inline bool isTouchMasked(const uint32_t mask) const;
    
    static uint32_t USER_VIEW_FLAG(int flagId);
    
    inline bool isSmoothUpdate(uint32_t flag) const { return _smoothFlags&flag; }
    
public:
    /**
     * Converts a Vec2 to node (local) space coordinates. The result is in Points.
     *
     * @param worldPoint A given coordinate.
     * @return A point in node (local) space coordinates.
     */
    cocos2d::Vec2 convertToNodeSpace3D(const cocos2d::Vec2& worldPoint) const;
    
    float getWorldScale() const;
    
private:
    cocos2d::Vec2 _lastTouchLocation;
    uint32_t _touchMask;
    uint32_t _smoothFlags;
    uint32_t _updateFlags;
    bool _enabled;
    
    
    State _pressState;
    cocos2d::Vec2 _touchStartPosition;
    cocos2d::Vec2 _touchLastPosition;
    float _touchStartTime;
    
    bool _scissorEnable;
    
    cocos2d::Rect* _scissorRect;
    cocos2d::Rect _targetScissorRect;
    
    cocos2d::SEL_SCHEDULE _onSmoothUpdateCallback;
    cocos2d::SEL_SCHEDULE _onClickValidateCallback;
    cocos2d::SEL_SCHEDULE _onLongClickValidateCallback;
    
    bool dispatchChildren(MotionEvent* event, int& ret);
    
    void scheduleClickValidator();
    void unscheduleClickValidator();
    
    void scheduleLongClickValidator();
    void unscheduleLongClickValidator();
    
    void onInternalSmoothUpate(float dt);
    void onClickValidator(float dt);
    void onLongClickValidator(float dt);
    
    void stateChangePressToNormal();
    void stateChangeNormalToPress();
    
    cocos2d::CustomCommand* _beforeVisitCmdScissor;
    cocos2d::CustomCommand* _afterVisitCmdScissor;
    
    std::function<void(SMView* view)> _onClickCallback;
    std::function<void(SMView* view)> _onLongClickCallback;
    std::function<void(SMView* view)> _onDoubleClickCallback;
    std::function<void(SMView* view, State state)> _onStateChangeCallback;
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(SMView);
    
    friend class SMScene;
    friend class SMApplication;
};


//-------------------------------------------------------------------------------------
// View Single-Click Event Interface
//-------------------------------------------------------------------------------------
class OnClickListener {
    
public:
    virtual void onClick(SMView* view) = 0;
    
};

//-------------------------------------------------------------------------------------
// View Double-Click Event Interface
//-------------------------------------------------------------------------------------
class OnDoubleClickListener {
    
public:
    virtual void onDoubleClick(SMView* view) = 0;
    
};

//-------------------------------------------------------------------------------------
// View Long-Click Event Interface
//-------------------------------------------------------------------------------------
class OnLongClickListener {
    
public:
    virtual void onLongClick(SMView* view) = 0;
    
};

//-------------------------------------------------------------------------------------
// View Touch Event Interface
//-------------------------------------------------------------------------------------
class OnTouchListener {
    
public:
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) = 0;
    
};

//-------------------------------------------------------------------------------------
// View State Change Interface
//-------------------------------------------------------------------------------------
class OnStateChangeListener {
    
public:
    virtual void onStateChanged(SMView* view, SMView::State state) = 0;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Abstract UI Container View
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class _UIContainerView : public SMView {
    
public:
    SMView* getUIContainer() { return _uiContainer; }
    
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    virtual void setPadding(const float padding) { setPadding(padding, padding, padding, padding); }
    
    virtual void setPadding(const float left, const float top, const float right, const float bottom);
    
    float getPaddingTop() { return _paddingTop; }
    float getPaddingBottom() { return _paddingBottom; }
    float getPaddingLeft() { return _paddingLeft; }
    float getPaddingRight() { return _paddingRight; }
    
protected:
    _UIContainerView();
    
    virtual ~_UIContainerView() = 0;
    
protected:
    SMView* _uiContainer;
    
protected:
    
    float _paddingLeft;
    float _paddingRight;
    float _paddingTop;
    float _paddingBottom;
};

//static const float SystemScale = 3.0f*1080.0f/960.0f;
//static const float SystemScale = 3.0f;


#endif /* SMView_h */
