//
//  SMView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "SMView.h"
#include "ShaderNode.h"
#include "../Util/ViewUtil.h"
#include <2d/CCActionInterval.h>
#include <renderer/CCCustomCommand.h>
#include <cmath>
#include <math.h>
#include <cocos2d.h>

const uint32_t SMView::VIEWFLAG_POSITION         = (1<<0);
const uint32_t SMView::VIEWFLAG_SCALE            = (1<<1);
const uint32_t SMView::VIEWFLAG_ROTATE           = (1<<2);
const uint32_t SMView::VIEWFLAG_ANCHOR_SIZE      = (1<<3);
const uint32_t SMView::VIEWFLAG_COLOR            = (1<<4);
const uint32_t SMView::VIEWFLAG_ANIM_OFFSET      = (1<<5);
const uint32_t SMView::VIEWFLAG_ANIM_SCALE       = (1<<6);
const uint32_t SMView::VIEWFLAG_ANIM_ROTATE      = (1<<7);

const uint32_t SMView::VIEWFLAG_USER_SHIFT       = (8);

const uint32_t SMView::TOUCH_MASK_CLICK          = (1<<0);
const uint32_t SMView::TOUCH_MASK_DOUBLECLICK    = (1<<1);
const uint32_t SMView::TOUCH_MASK_LONGCLICK      = (1<<2);
const uint32_t SMView::TOUCH_MASK_TOUCH          = (1<<3);


class BackgroundColorTo : public cocos2d::ActionInterval {
    
public:
    BackgroundColorTo(){}
    virtual ~BackgroundColorTo() {}
    
    static BackgroundColorTo* create(float duration, const cocos2d::Color4F& color) {
        auto bgColorTo = new (std::nothrow) BackgroundColorTo();
        
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
    CC_DISALLOW_COPY_AND_ASSIGN(BackgroundColorTo);
};


SMView::SMView() :

_enabled(true),
_touchMask(0),
_smoothFlags(0),
_updateFlags(0),

_pressState(State::NORMAL),
_onSmoothUpdateCallback(nullptr),
_onClickValidateCallback(nullptr),
_onLongClickValidateCallback(nullptr),

_cancelIfTouchOutside(false),
_onClickListener        (nullptr),
_onLongClickListener    (nullptr),
_onDoubleClickListener  (nullptr),
_onTouchListener        (nullptr),
_onStateChangeListener  (nullptr),

_touchEventTime(0),
_touchMotionTarget(nullptr),
_touchStartPosition(cocos2d::Vec2::ZERO),
_touchLastPosition(cocos2d::Vec2::ZERO),
_touchStartTime(0),
_touchHasFirstClicked(false),
_touchTargeted(false),
_lastTouchLocation(cocos2d::Vec2::ZERO),
_ignoreTouchBounds(false),

_bgNode(nullptr),
_bgColor(SMViewConstValue::Const::COLOR4F_TRANSPARENT),

_realPosition(_position.x, _position.y, _positionZ),
_newPosition(_realPosition),
_animOffset(cocos2d::Vec3::ZERO),
_newAnimOffset(cocos2d::Vec3::ZERO),

_realScale(cocos2d::Vec3(_scaleX, _scaleY, _scaleZ)),
_newScale(_realScale),
_animScale(cocos2d::Vec3::ONE),
_newAnimScale(cocos2d::Vec3::ONE),

_realRotation(cocos2d::Vec3(_rotationX, _rotationY, _rotationZ_X)),
_newRotation(_realRotation),
_animRotation(cocos2d::Vec3::ZERO),
_newAnimRotation(cocos2d::Vec3::ZERO),

_newAnchorPoint(cocos2d::Vec2(_anchorPoint)),
_newContentSize(cocos2d::Size(_contentSize)),

_beforeVisitCmdScissor(nullptr),
_afterVisitCmdScissor(nullptr),

_scissorEnable(false),
_scissorRect(nullptr),

_onClickCallback(nullptr),
_onLongClickCallback(nullptr),
_onDoubleClickCallback(nullptr),
_onStateChangeCallback(nullptr)
{
    setCascadeOpacityEnabled(true);
}

SMView::~SMView() {
    if (_bgNode != nullptr) {
        cocos2d::Node::removeChild(_bgNode);
    }
    
    CC_SAFE_DELETE(_beforeVisitCmdScissor);
    CC_SAFE_DELETE(_afterVisitCmdScissor);
    CC_SAFE_DELETE(_scissorRect);
    
    unscheduleAllCallbacks();
    _eventDispatcher->removeEventListenersForTarget(this);
}

void SMView::onEnter() {
    cocos2d::Node::onEnter();
}

void SMView::onExit() {
    cocos2d::Node::onExit();
}

void SMView::cleanup() {
    cocos2d::Node::cleanup();
    // Interpolation 초기화
    if (_smoothFlags) {
        _smoothFlags = 0;
        
        _newAnimOffset = _animOffset = cocos2d::Vec3::ZERO;
        _newAnimRotation = _animRotation = cocos2d::Vec3::ZERO;
        _newAnimScale = _animScale = cocos2d::Vec3::ONE;
        
        _position.x = _realPosition.x = _newPosition.x;
        _position.y = _realPosition.y = _newPosition.y;
        _positionZ = _realPosition.z = _newPosition.z;
        
        _rotationX = _realRotation.x = _newRotation.x;
        _rotationY = _realRotation.y = _newRotation.y;
        _rotationZ_Y = _rotationZ_X = _realRotation.z = _newRotation.z;
        updateRotationQuat();
        
        _scaleX = _realScale.x = _newScale.x;
        _scaleY = _realScale.y = _newScale.y;
        _scaleZ = _realScale.z = _newScale.z;
        
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }
}


bool SMView::init() {
    if (cocos2d::Node::init()) {
        return true;
    }
    return false;
}

void SMView::setTouchMask(const uint32_t mask) {
    _touchMask |= mask;
}

inline void SMView::clearTouchMask(const uint32_t mask) {
    _touchMask &= ~mask;
}

inline bool SMView::isTouchMasked(const uint32_t mask) const {
    return (_touchMask & mask) != 0;
}

void SMView::removeChild(Node* child, bool cleanup) {
    cocos2d::Node::removeChild(child, cleanup);
    if (child != nullptr) {
        if (child == _bgNode) {
            _bgNode = nullptr;
        } else if (child == _touchMotionTarget) {
            _touchMotionTarget = nullptr;
        }
    }
}

void SMView::removeAllChildrenWithCleanup(bool cleanup) {
    if (_bgNode != nullptr) {
        _bgNode->retain();
        cocos2d::Node::removeChild(_bgNode);
    }
    
    cocos2d::Node::removeAllChildrenWithCleanup(cleanup);
    
    if (_bgNode != nullptr) {
        cocos2d::Node::addChild(_bgNode, SMViewConstValue::ZOrder::BG, "");
        _bgNode->release();
    }
    
    _touchMotionTarget = nullptr;
}

void SMView::removeFromParent() {
    if (getParent()) {
        auto parent = dynamic_cast<SMView*>(getParent());
        if (parent && parent->_touchMotionTarget == this) {
            parent->_touchMotionTarget = nullptr;
        }
    }
    cocos2d::Node::removeFromParent();
}


bool SMView::isTouchEnable() const {
    return _touchMask > 0;
}

void SMView::setOnClickListener(OnClickListener* l) {
    _onClickListener = l;
    
    if (l != nullptr) {
        setTouchMask(TOUCH_MASK_CLICK);
    } else {
        clearTouchMask(TOUCH_MASK_CLICK);
    }
}

void SMView::setOnDoubleClickListener(OnDoubleClickListener* l) {
    if (l == nullptr) {
        if (_onDoubleClickListener != nullptr) {
            unscheduleClickValidator();
        }
    }
    _onDoubleClickListener = l;
    
    if (l != nullptr) {
        setTouchMask(TOUCH_MASK_DOUBLECLICK);
    } else {
        clearTouchMask(TOUCH_MASK_DOUBLECLICK);
    }
}

void SMView::setOnLongClickListener(OnLongClickListener* l) {
    if (l == nullptr) {
        if (_onLongClickListener != nullptr) {
            unscheduleLongClickValidator();
        }
    }
    _onLongClickListener = l;
    
    if (l != nullptr) {
        setTouchMask(TOUCH_MASK_LONGCLICK);
    } else {
        clearTouchMask(TOUCH_MASK_LONGCLICK);
    }
}

void SMView::setOnTouchListener(OnTouchListener* l) {
    _onTouchListener = l;
    
    if (l != nullptr) {
        setTouchMask(TOUCH_MASK_TOUCH);
    } else {
        clearTouchMask(TOUCH_MASK_TOUCH);
    }
}

void SMView::setOnStateChangeListener(OnStateChangeListener* l) {
    _onStateChangeListener = l;
}


void SMView::setOnClickCallback(std::function<void (SMView *)> callback) {
    _onClickCallback = callback;
    
    if (callback != nullptr) {
        setTouchMask(TOUCH_MASK_CLICK);
    } else {
        clearTouchMask(TOUCH_MASK_CLICK);
    }
}

void SMView::setOnDoubleClickCallback(std::function<void (SMView *)> callback) {
    if (callback == nullptr) {
        if (_onDoubleClickCallback != nullptr) {
            unscheduleClickValidator();
        }
    }
    _onDoubleClickCallback = callback;
    
    if (callback != nullptr) {
        setTouchMask(TOUCH_MASK_DOUBLECLICK);
    } else {
        clearTouchMask(TOUCH_MASK_DOUBLECLICK);
    }
}

void SMView::setOnLongClickCallback(std::function<void (SMView *)> callback) {
    if (callback == nullptr) {
        if (_onLongClickCallback != nullptr) {
            unscheduleLongClickValidator();
        }
    }
    _onLongClickCallback = callback;
    
    if (callback != nullptr) {
        setTouchMask(TOUCH_MASK_LONGCLICK);
    } else {
        clearTouchMask(TOUCH_MASK_LONGCLICK);
    }
}

void SMView::setOnStateChangeCallback(std::function<void (SMView *, State)> callback) {
    _onStateChangeCallback = callback;
}

const float SMView::getShortestAngle(float from, float to) {
    return std::fmod((std::fmod(to-from, 360.0)+540), 360.0)-180;
}

bool SMView::smoothInterpolate(float& from, float to, float tolerance, float smoothDivider) {
    if (from != to) {
        from = from + (to-from) / smoothDivider;
        if (std::abs(from-to) < tolerance) {
            from = to;
            return false; // done
        }
        return true; // still need update
    }
    return false;
}

bool SMView::smoothInterpolateRotate(float& from, float to, float tolerance) {
    if (from != to) {
        float diff = getShortestAngle(from, to);
        if (std::abs(diff) < tolerance) {
            from = to;
            return false; // done
        }
        from += diff / SMViewConstValue::Config::SMOOTH_DIVIDER;
        return true; // still need update
    }
    return false;
}


void SMView::scheduleSmoothUpdate(const uint32_t flag) {
    if (_smoothFlags&flag)
        return;
    
    _smoothFlags |= flag;
    
    if (_onSmoothUpdateCallback == nullptr) {
        _onSmoothUpdateCallback = static_cast<cocos2d::SEL_SCHEDULE>(&SMView::onInternalSmoothUpate);
    }
    
    if (!isScheduled(_onSmoothUpdateCallback)) {
        schedule(_onSmoothUpdateCallback);
    }
}

void SMView::unscheduleSmoothUpdate(const uint32_t flag) {
    if (flag == 0) {
        _smoothFlags = 0;
    } else {
        _smoothFlags &= ~flag;
    }
    if (_smoothFlags == 0) {
        if (_onSmoothUpdateCallback != nullptr && isScheduled(_onSmoothUpdateCallback)) {
            unschedule(_onSmoothUpdateCallback);
        }
    }
}

void SMView::scheduleClickValidator() {
    if (_onClickValidateCallback == nullptr) {
        _onClickValidateCallback = static_cast<cocos2d::SEL_SCHEDULE>(&SMView::onClickValidator);
    }
    
    if (!isScheduled(_onClickValidateCallback)) {
        scheduleOnce(_onClickValidateCallback, SMViewConstValue::Config::DOUBLE_TAP_TIMEOUT);
    }
}

void SMView::unscheduleClickValidator() {
    if (_onClickValidateCallback != nullptr && isScheduled(_onClickValidateCallback)) {
        unschedule(_onClickValidateCallback);
    }
}

void SMView::scheduleLongClickValidator() {
    if (_onLongClickValidateCallback == nullptr) {
        _onLongClickValidateCallback = static_cast<cocos2d::SEL_SCHEDULE>(&SMView::onLongClickValidator);
    }
    
    if (!isScheduled(_onLongClickValidateCallback)) {
        scheduleOnce(_onLongClickValidateCallback, SMViewConstValue::Config::LONG_PRESS_TIMEOUT);
    }
    
}

void SMView::unscheduleLongClickValidator() {
    if (_onLongClickValidateCallback != nullptr && isScheduled(_onLongClickValidateCallback)) {
        unschedule(_onLongClickValidateCallback);
    }
}

void SMView::onInternalSmoothUpate(float dt) {
    
    uint32_t flags = 0;
    
    if (isSmoothUpdate(VIEWFLAG_ANCHOR_SIZE)) {
        flags |= VIEWFLAG_ANCHOR_SIZE;
        bool needUpdate = false;
        needUpdate |= smoothInterpolate(_anchorPoint.x, _newAnchorPoint.x, SMViewConstValue::Config::TOLERANCE_SCALE);
        needUpdate |= smoothInterpolate(_anchorPoint.y, _newAnchorPoint.y, SMViewConstValue::Config::TOLERANCE_SCALE);
        
        auto s = _contentSize;
        needUpdate |= smoothInterpolate(s.width, _newContentSize.width, SMViewConstValue::Config::TOLERANCE_POSITION);
        needUpdate |= smoothInterpolate(s.height, _newContentSize.height, SMViewConstValue::Config::TOLERANCE_POSITION);
        cocos2d::Node::setContentSize(s);
        if (_bgNode != nullptr) {
            _bgNode->setContentSize(s);
        }
        
        if (!needUpdate) {
            unscheduleSmoothUpdate(VIEWFLAG_ANCHOR_SIZE);
        }
        _anchorPointInPoints.set(_contentSize.width * _anchorPoint.x, _contentSize.height * _anchorPoint.y);
        _transformUpdated = _transformDirty = _inverseDirty = _contentSizeDirty = true;
    }
    
    bool animOffset = false;
    if (isSmoothUpdate(VIEWFLAG_ANIM_OFFSET)) {
        flags |= VIEWFLAG_ANIM_OFFSET;
        animOffset = true;
        
        bool needUpdate = false;
        needUpdate |= smoothInterpolate(_animOffset.x, _newAnimOffset.x, SMViewConstValue::Config::TOLERANCE_POSITION);
        needUpdate |= smoothInterpolate(_animOffset.y, _newAnimOffset.y, SMViewConstValue::Config::TOLERANCE_POSITION);
        needUpdate |= smoothInterpolate(_animOffset.z, _newAnimOffset.z, SMViewConstValue::Config::TOLERANCE_POSITION);
        if (!needUpdate) {
            unscheduleSmoothUpdate(VIEWFLAG_ANIM_OFFSET);
        }
    }
    
    if (isSmoothUpdate(VIEWFLAG_POSITION) || animOffset) {
        flags |= VIEWFLAG_POSITION;
        bool needUpdate = false;
        needUpdate |= smoothInterpolate(_realPosition.x, _newPosition.x, SMViewConstValue::Config::TOLERANCE_POSITION);
        needUpdate |= smoothInterpolate(_realPosition.y, _newPosition.y, SMViewConstValue::Config::TOLERANCE_POSITION);
        needUpdate |= smoothInterpolate(_realPosition.z, _newPosition.z, SMViewConstValue::Config::TOLERANCE_POSITION);
        if (!needUpdate && !animOffset) {
            unscheduleSmoothUpdate(VIEWFLAG_POSITION);
        }
        _position.x = _realPosition.x + _animOffset.x;
        _position.y = _realPosition.y + _animOffset.y;
        _positionZ = _realPosition.z + _animOffset.z;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }
    
    bool animScale = false;
    if (isSmoothUpdate(VIEWFLAG_ANIM_SCALE)) {
        flags |= VIEWFLAG_ANIM_SCALE;
        animScale = true;
        
        bool needUpdate = false;
        needUpdate |= smoothInterpolate(_animScale.x, _newAnimScale.x, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        needUpdate |= smoothInterpolate(_animScale.y, _newAnimScale.y, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        needUpdate |= smoothInterpolate(_animScale.z, _newAnimScale.z, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        if (!needUpdate) {
            unscheduleSmoothUpdate(VIEWFLAG_ANIM_SCALE);
        }
    }
    
    if (isSmoothUpdate(VIEWFLAG_SCALE) || animScale) {
        flags |= VIEWFLAG_SCALE;
        bool needUpdate = false;
        needUpdate |= smoothInterpolate(_realScale.x, _newScale.x, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        needUpdate |= smoothInterpolate(_realScale.y, _newScale.y, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        needUpdate |= smoothInterpolate(_realScale.z, _newScale.z, SMViewConstValue::Config::TOLERANCE_SCALE, 3.0);
        if (!needUpdate && !animScale) {
            unscheduleSmoothUpdate(VIEWFLAG_SCALE);
        }
        
        _scaleX = _realScale.x * _animScale.x;
        _scaleY = _realScale.y * _animScale.y;
        _scaleZ = _realScale.z * _animScale.z;
        
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }
    
    bool animRotate = false;
    if (isSmoothUpdate(VIEWFLAG_ANIM_ROTATE)) {
        flags |= VIEWFLAG_ANIM_ROTATE;
        animRotate = true;
        
        bool needUpdate = false;
        needUpdate |= smoothInterpolateRotate(_animRotation.x, _newAnimRotation.x, SMViewConstValue::Config::TOLERANCE_ROTATE);
        needUpdate |= smoothInterpolateRotate(_animRotation.y, _newAnimRotation.y, SMViewConstValue::Config::TOLERANCE_ROTATE);
        needUpdate |= smoothInterpolateRotate(_animRotation.z, _newAnimRotation.z, SMViewConstValue::Config::TOLERANCE_ROTATE);
        if (!needUpdate) {
            unscheduleSmoothUpdate(VIEWFLAG_ANIM_ROTATE);
        }
    }
    
    if (isSmoothUpdate(VIEWFLAG_ROTATE) || animRotate) {
        flags |= VIEWFLAG_ROTATE;
        bool needUpdate = false;
        needUpdate |= smoothInterpolateRotate(_realRotation.x, _newRotation.x, SMViewConstValue::Config::TOLERANCE_ROTATE);
        needUpdate |= smoothInterpolateRotate(_realRotation.y, _newRotation.y, SMViewConstValue::Config::TOLERANCE_ROTATE);
        needUpdate |= smoothInterpolateRotate(_realRotation.z, _newRotation.z, SMViewConstValue::Config::TOLERANCE_ROTATE);
        if (!needUpdate && !animRotate) {
            unscheduleSmoothUpdate(VIEWFLAG_ROTATE);
        }
        
        _rotationX = _realRotation.x + _animRotation.x;
        _rotationY = _realRotation.y + _animRotation.y;
        _rotationZ_Y = _rotationZ_X = _realRotation.z + _animRotation.z;
        
        updateRotationQuat();
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }
    
    onSmoothUpdate(flags, dt);
}

void SMView::registerUpdate(const uint32_t flag) {
    if (_updateFlags&flag)
        return;
    
    _updateFlags |= flag;
}

void SMView::unregisterUpdate(const uint32_t flag) {
    if (flag == 0) {
        _updateFlags = 0;
    } else {
        _updateFlags &= ~flag;
    }
}

static bool intersectRectInWindow(cocos2d::Rect& rect, const cocos2d::Size& winSize) {
    float dw = winSize.width;
    float dh = winSize.height;
    float sw = rect.size.width;
    float sh = rect.size.height;
    float x = rect.origin.x;
    float y = rect.origin.y;
    
    if (x+sw <= 0 || x >= dw || y+sh <= 0 || y >= dh || sw <= 0 || sh <= 0) {
        // 화면 밖은 리턴
        return false;
    }
    
    float sx = 0;
    float sy = 0;
    float width = sw;
    float height = sh;
    if (x < 0) {
        sx = -x;
        width -= sx;
    }
    if (y < 0) {
        sy = -y;
        height -= sy;
    }
    if (x + sw > dw) {
        width -= x + sw - dw;
    }
    if (y + sh > dh) {
        height -= y + sh - dh;
    }
    
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    
    rect.setRect(ViewUtil::round(x), ViewUtil::round(y), ViewUtil::round(width), ViewUtil::round(height));
    
    return true;
}

void SMView::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) {
    if (!_visible)
        return;
    
    if (_updateFlags) {
        onUpdateOnVisit();
    }
    
    if (_beforeVisitCmdScissor) {
        float scaleX = _scaleX;
        float scaleY = _scaleY;
        
        auto parent = this->getParent();
        while (parent) {
            scaleX *= parent->getScaleX();
            scaleY *= parent->getScaleY();
            parent = parent->getParent();
        }
        
        if (_scissorRect) {
            auto o = convertToWorldSpace(_scissorRect->origin);
            _targetScissorRect.setRect(o.x, o.y, _scissorRect->size.width*scaleX, _scissorRect->size.height*scaleY);
            
            if (!intersectRectInWindow(_targetScissorRect, _director->getWinSize()))
                return; // 더이상 그릴필요 없음
        } else {
            auto o = convertToWorldSpace(cocos2d::Vec2::ZERO);
            _targetScissorRect.setRect(o.x, o.y, _contentSize.width*scaleX, _contentSize.height*scaleY);
            
            if (!intersectRectInWindow(_targetScissorRect, _director->getWinSize()))
                return; // 더이상 그릴필요 없음
        }
        
        _beforeVisitCmdScissor->init(_globalZOrder);
        renderer->addCommand(_beforeVisitCmdScissor);
    }
    
    cocos2d::Node::visit(renderer, parentTransform, parentFlags);
    
    if (_afterVisitCmdScissor) {
        _afterVisitCmdScissor->init(_globalZOrder);
        renderer->addCommand(_afterVisitCmdScissor);
    }
}

void SMView::setScissorEnable(const bool enable) {
    if (_scissorEnable != enable) {
        if (enable) {
            if (_beforeVisitCmdScissor == nullptr) {
                _beforeVisitCmdScissor = new cocos2d::CustomCommand();
            }
            if (_afterVisitCmdScissor == nullptr) {
                _afterVisitCmdScissor = new cocos2d::CustomCommand();
            }
            
            _beforeVisitCmdScissor->func = CC_CALLBACK_0(SMView::onBeforeVisitScissor, this);
            _afterVisitCmdScissor->func = CC_CALLBACK_0(SMView::onAfterVisitScissor, this);
        } else {
            CC_SAFE_DELETE(_beforeVisitCmdScissor);
            CC_SAFE_DELETE(_afterVisitCmdScissor);
        }
        _scissorEnable = enable;
    }
}

void SMView::setScissorRect(const cocos2d::Rect& rect) {
    if (!_scissorRect) {
        _scissorRect = new cocos2d::Rect(rect);
    } else {
        *_scissorRect = rect;
    }
}

void SMView::onBeforeVisitScissor() {
    glEnable(GL_SCISSOR_TEST);
    _director->getOpenGLView()->setScissorInPoints(
                                                   _targetScissorRect.origin.x,
                                                   _targetScissorRect.origin.y,
                                                   _targetScissorRect.size.width,
                                                   _targetScissorRect.size.height);
}

void SMView::onAfterVisitScissor() {
    glDisable(GL_SCISSOR_TEST);
}

//--------------------------------------------------------------------
// Scale setter
//--------------------------------------------------------------------
void SMView::setScale(float scale, bool immediate) {
    setScale(scale, scale, immediate);
}

void SMView::setScale(float scaleX, float scaleY, bool immediate) {
    if (immediate) {
        _realScale.x = _newScale.x = scaleX;
        _realScale.y = _newScale.y = scaleY;
        cocos2d::Node::setScale(scaleX*_animScale.x, scaleY*_animScale.y);
    } else {
        setScaleX(scaleX, false);
        setScaleY(scaleY, false);
    }
}

void SMView::setScaleX(float scaleX, bool immediate) {
    if (immediate) {
        cocos2d::Node::setScaleX(scaleX*_animScale.x);
        _realScale.x = _newScale.x = scaleX;
    } else {
        if (_newScale.x == scaleX) {
            return;
        }
        _newScale.x = scaleX;
        
        scheduleSmoothUpdate(VIEWFLAG_SCALE);
    }
}

void SMView::setScaleY(float scaleY, bool immediate) {
    if (immediate) {
        cocos2d::Node::setScaleY(scaleY*_animScale.x);
        _realScale.y = _newScale.y = scaleY;
    } else {
        if (_newScale.y == scaleY) {
            return;
        }
        _newScale.y = scaleY;
        
        scheduleSmoothUpdate(VIEWFLAG_SCALE);
    }
}

void SMView::setScaleZ(float scaleZ, bool immediate) {
    if (immediate) {
        cocos2d::Node::setScaleZ(scaleZ*_animScale.x);
        _realScale.z = _newScale.z = scaleZ;
    } else {
        if (_newScale.z == scaleZ) {
            return;
        }
        _newScale.z = scaleZ;
        
        scheduleSmoothUpdate(VIEWFLAG_SCALE);
    }
}

//--------------------------------------------------------------------
// Rotation setter
//--------------------------------------------------------------------
void SMView::setRotationX(float rotationX, bool immediate) {
    if (immediate) {
        _realRotation.x = _newRotation.x = rotationX;
        cocos2d::Node::setRotation3D(_realRotation + _animRotation);
    } else {
        if (_newRotation.x == rotationX) {
            return;
        }
        _newRotation.z = rotationX;
        
        scheduleSmoothUpdate(VIEWFLAG_ROTATE);
    }
}

void SMView::setRotationY(float rotationY, bool immediate) {
    if (immediate) {
        _realRotation.y = _newRotation.y = rotationY;
        cocos2d::Node::setRotation3D(_realRotation + _animRotation);
    } else {
        if (_newRotation.y == rotationY) {
            return;
        }
        _newRotation.y = rotationY;
        
        scheduleSmoothUpdate(VIEWFLAG_ROTATE);
    }
}

void SMView::setRotationZ(float rotationZ, bool immediate) {
    if (immediate) {
        _realRotation.z = _newRotation.z = rotationZ;
        cocos2d::Node::setRotation(rotationZ+_animRotation.z);
    } else {
        if (_newRotation.z == rotationZ) {
            return;
        }
        _newRotation.z = rotationZ;
        
        scheduleSmoothUpdate(VIEWFLAG_ROTATE);
    }
}

void SMView::setRotation3D(const cocos2d::Vec3& rotation, bool immediate) {
    if (immediate) {
        _realRotation = _newRotation = rotation;
        cocos2d::Node::setRotation3D(rotation+_animRotation);
    } else {
        if (_newRotation == rotation) {
            return;
        }
        _newRotation =  rotation;
        
        scheduleSmoothUpdate(VIEWFLAG_ROTATE);
    }
}

//--------------------------------------------------------------------
// Position setter
//--------------------------------------------------------------------
void SMView::setPosition(const cocos2d::Vec2& position, bool immediate) {
    setPosition(position.x, position.y, immediate);
}

void SMView::setPosition(float x, float y, bool immediate) {
    if (immediate) {
        setPositionX(x, true);
        setPositionY(y, true);
    } else {
        setPositionX(x, false);
        setPositionY(y, false);
    }
}

void SMView::setPosition3D(const cocos2d::Vec3& position, bool immediate) {
    if (immediate) {
        cocos2d::Node::setPosition3D(position+_animOffset);
    } else {
        setPositionX(position.x, false);
        setPositionY(position.y, false);
        setPositionZ(position.z, false);
    }
}

void SMView::setPositionX(float x, bool immediate) {
    if (immediate) {
        if (_position.x != x+_animOffset.x) {
            _position.x = x+_animOffset.x;
            _realPosition.x = _newPosition.x = x;
            
            _transformUpdated = _transformDirty = _inverseDirty = true;
            _usingNormalizedPosition = false;
        }
    } else {
        if (_newPosition.x == x) {
            return;
        }
        _newPosition.x = x;
        _usingNormalizedPosition = false;
        
        scheduleSmoothUpdate(VIEWFLAG_POSITION);
    }
}

void SMView::setPositionY(float y, bool immediate) {
    if (immediate) {
        if (_position.y != y+_animOffset.y) {
            _position.y = y+_animOffset.y;
            _realPosition.y = _newPosition.y = y;
            
            _transformUpdated = _transformDirty = _inverseDirty = true;
            _usingNormalizedPosition = false;
        }
    } else {
        if (_newPosition.y == y) {
            return;
        }
        _newPosition.y = y;
        _usingNormalizedPosition = false;
        
        scheduleSmoothUpdate(VIEWFLAG_POSITION);
    }
}

void SMView::setPositionZ(float z, bool immediate) {
    if (immediate) {
        cocos2d::Node::setPositionZ(z+_animOffset.x);
        _realPosition.z = _newPosition.z = z;
    } else {
        if (_newPosition.z == z) {
            return;
        }
        _newPosition.z = z;
        _usingNormalizedPosition = false;
        
        scheduleSmoothUpdate(VIEWFLAG_POSITION);
    }
}

//--------------------------------------------------------------------
// AnimOffset setter
//--------------------------------------------------------------------
void SMView::setAnimOffset(const cocos2d::Vec2& animOffset, bool immediate) {
    if (_newAnimOffset.x != animOffset.x ||
        _newAnimOffset.y != animOffset.y) {
        _newAnimOffset.x = animOffset.x;
        _newAnimOffset.y = animOffset.y;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_OFFSET);
    }
    if (immediate) {
        _animOffset = _newAnimOffset;
    }
}

void SMView::setAnimOffset3D(const cocos2d::Vec3& animOffset, bool immediate) {
    if (_newAnimOffset != animOffset) {
        _newAnimOffset = animOffset;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_OFFSET);
    }
    if (immediate) {
        _animOffset = _newAnimOffset;
    }
}

void SMView::setAnimScale(const float animScale, bool immediate) {
    if (_newAnimScale.x != animScale ||
        _newAnimScale.y != animScale) {
        _newAnimScale.x = animScale;
        _newAnimScale.y = animScale;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_SCALE);
    }
    if (immediate) {
        _animScale = _newAnimScale;
    }
}

void SMView::setAnimScale(const cocos2d::Vec2& animScale, bool immediate) {
    if (_newAnimScale.x != animScale.x ||
        _newAnimScale.y != animScale.y) {
        _newAnimScale.x = animScale.x;
        _newAnimScale.y = animScale.y;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_SCALE);
    }
    if (immediate) {
        _animScale = _newAnimScale;
    }
}

void SMView::setAnimRotate(const float animRotate, bool immediate) {
    if (_newAnimRotation.z != animRotate) {
        _newAnimRotation.z = animRotate;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_ROTATE);
    }
    if (immediate) {
        _animRotation = _newAnimRotation;
    }
}

void SMView::setAnimRotate3D(const cocos2d::Vec3& animRotate, bool immediate) {
    if (_newAnimRotation != animRotate) {
        _newAnimRotation = animRotate;
        
        scheduleSmoothUpdate(VIEWFLAG_ANIM_ROTATE);
    }
    if (immediate) {
        _animRotation = _newAnimRotation;
    }
}


//--------------------------------------------------------------------
// AnchorPoint setter
//--------------------------------------------------------------------
void SMView::setAnchorPoint(const cocos2d::Vec2& point, bool immediate) {
    if (immediate) {
        cocos2d::Node::setAnchorPoint(point);
        _newAnchorPoint = point;
    } else {
        if (!point.equals(_newAnchorPoint)) {
            _newAnchorPoint = point;
            scheduleSmoothUpdate(VIEWFLAG_ANCHOR_SIZE);
        }
    }
}


void SMView::setContentSize(const cocos2d::Size& size, bool immediate) {
    
    if (immediate) {
        cocos2d::Node::setContentSize(size);
        if (_bgNode != nullptr) {
            _bgNode->setContentSize(size);
        }
        _newContentSize = size;
    } else {
        if (!size.equals(_newContentSize)) {
            _newContentSize = size;
            scheduleSmoothUpdate(VIEWFLAG_ANCHOR_SIZE);
        }
    }
}

bool SMView::containsPoint(const cocos2d::Vec2& point) {
    return !(point.x < 0 || point.y < 0 || point.x > _contentSize.width || point.y > _contentSize.height);
}

cocos2d::Vec2 SMView::convertToNodeSpace3D(const cocos2d::Vec2& worldPoint) const
{
    cocos2d::Rect rt = _director->getOpenGLView()->getVisibleRect();
    cocos2d::Mat4 mvpMatrix = _director->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION) * _modelViewTransform;
    mvpMatrix.inverse();
    
    cocos2d::Vec4 in(((worldPoint.x - rt.origin.x) / rt.size.width) * 2 - 1,
                     ((worldPoint.y - rt.origin.y) / rt.size.height) * 2 - 1,
                     0 * 2 - 1,
                     1);
    
    cocos2d::Vec4 start = mvpMatrix * in;
    start.x /= start.w;
    start.y /= start.w;
    start.z /= start.w;
    start.w = 1;
    
    in.z = -1 * 2 - 1;
    cocos2d::Vec4 end = mvpMatrix * in;
    end.x /= end.w;
    end.y /= end.w;
    end.z /= end.w;
    end.w = 1;
    
    cocos2d::Vec4 rayDir = end - start;
    rayDir.normalize();
    const cocos2d::Vec4 normal = cocos2d::Vec4(0, 0, 1, 0);
    
    float rayDirDotNorm = rayDir.dot(normal);
    float P0DotNorm = start.dot(normal);
    
    float t = 0;
    
    if (rayDirDotNorm != 0) {
        t = -P0DotNorm / rayDirDotNorm;
    }
    
    cocos2d::Vec4 result = (rayDir * t) + start;
    
    return cocos2d::Vec2(result.x, result.y);
}

float SMView::getWorldScale() const {
    float worldScale = getScale();
    
    for (auto parent = getParent();  parent != nullptr; parent = parent->getParent()) {
        worldScale *= parent->getScale();
    }
    
    return worldScale;
}

void SMView::cancelTouchEvent(SMView* targetView, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    
    if (targetView != nullptr) {
        if (targetView->_touchMotionTarget != nullptr) {
            cancelTouchEvent(targetView->_touchMotionTarget, touch, point, event);
            targetView->_touchMotionTarget = nullptr;
        } else {
            targetView->_touchTargeted = false;
            targetView->dispatchTouchEvent(MotionEvent::ACTION_CANCEL, touch, point, event);
            targetView->_touchHasFirstClicked = false;
            targetView->unscheduleClickValidator();
            targetView->unscheduleLongClickValidator();
            targetView->stateChangePressToNormal();
        }
    }
}

int SMView::dispatchTouchEvent(MotionEvent* event, SMView* view, bool checkBounds) {
    auto action = event->getAction();
    auto touch = event->getTouch(0);
    auto point = view->convertToNodeSpace(touch->getLocation());
    auto isContain = view->containsPoint(point) || (!checkBounds);
    
    _lastTouchLocation = touch->getLocation();
    
    if (action == MotionEvent::ACTION_DOWN) {
        view->_touchStartPosition = point;
        view->_touchStartTime = _director->getGlobalTime();
    } else {
        view->_touchLastPosition = point;
    }
    
    if (view->_cancelIfTouchOutside &&
        action == MotionEvent::ACTION_DOWN && !isContain) {
        view->cancel();
    }
    
    int ret = TOUCH_FALSE;
    if (!checkBounds || isContain || view == _touchMotionTarget) {
        ret = view->dispatchTouchEvent(action, touch, &point, event);
    } else if (view->_ignoreTouchBounds && action == MotionEvent::ACTION_DOWN) {
        view->dispatchChildren(event, ret);
    }
    
    return ret;
}

int SMView::dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    
    _lastTouchLocation = touch->getLocation();
    
    if (_onTouchListener != nullptr) {
        int ret = _onTouchListener->onTouch(this, action, touch, point, event);
        if (ret != TOUCH_FALSE)
            return ret;
    }
    
    if (_touchMotionTarget != nullptr) {
        if (action == MotionEvent::ACTION_DOWN) {
            _touchMotionTarget = nullptr;
        } else {
            int ret = dispatchTouchEvent(event, _touchMotionTarget, false);
            if (_touchMotionTarget && _touchMotionTarget->_touchTargeted && _touchMotionTarget->isTouchEnable()) {
                _touchMotionTarget->unscheduleLongClickValidator();
                // 터치 가능
                if (action == MotionEvent::ACTION_UP) {
                    // TOUCH UP
                    if (event->getEventTime()-_touchMotionTarget->_touchEventTime < SMViewConstValue::Config::TAP_TIMEOUT) {
                        // 유효한 클릭 시간
                        float distance = touch->getLocation().distance(touch->getStartLocation());
                        if (_touchMotionTarget->isTouchMasked(TOUCH_MASK_DOUBLECLICK)) {
                            // 더블클릭 가능한 상태
                            if (_touchMotionTarget->_touchHasFirstClicked) {
                                // 두번째 클릭
                                _touchMotionTarget->_touchHasFirstClicked = false;
                                if (distance < SMViewConstValue::Config::SCALED_DOUBLE_TAB_SLOPE) {
                                    // 더블클릭 실행
                                    _touchMotionTarget->performDoubleClick(_lastTouchLocation);
                                }
                                // 클릭 확인 취소
                                _touchMotionTarget->unscheduleClickValidator();
                            } else {
                                if (distance < SMViewConstValue::Config::SCALED_TOUCH_SLOPE) {
                                    // 첫번째 클릭 성공
                                    _touchMotionTarget->_touchTargeted = false;
                                    _touchMotionTarget->_touchHasFirstClicked = true;
                                    _touchMotionTarget->_touchEventTime = event->getEventTime();
                                    _touchMotionTarget->scheduleClickValidator();
                                } else {
                                    // 첫번째 클릭 실패
                                    _touchMotionTarget->_touchTargeted = false;
                                    _touchMotionTarget->_touchHasFirstClicked = false;
                                    _touchMotionTarget->unscheduleClickValidator();
                                }
                            }
                            _touchMotionTarget->stateChangePressToNormal();
                        } else {
                            // 싱글 클릭만 가능한 상태
                            _touchMotionTarget->_touchTargeted = false;
                            _touchMotionTarget->_touchHasFirstClicked = false;
                            if (ret == TOUCH_FALSE && distance < SMViewConstValue::Config::SCALED_TOUCH_SLOPE) {
                                // 싱글 클릭 실행
                                _touchMotionTarget->performClick(_lastTouchLocation);
                            }
                            _touchMotionTarget->stateChangePressToNormal();
                        }
                    } else {
                        // 클릭 시간 타임아웃
                        _touchMotionTarget->_touchTargeted = false;
                        _touchMotionTarget->_touchHasFirstClicked = false;
                        _touchMotionTarget->unscheduleClickValidator();
                        _touchMotionTarget->unscheduleLongClickValidator();
                        _touchMotionTarget->stateChangePressToNormal();
                        _touchMotionTarget = nullptr;
                    }
                } else if (action == MotionEvent::ACTION_MOVE) {
                    float distance = touch->getLocation().distance(touch->getStartLocation());
                    if (distance > SMViewConstValue::Config::SCALED_TOUCH_SLOPE) {
                        // 허용된 이동범위 벗어남.
                        _touchMotionTarget->_touchHasFirstClicked = false;
                        _touchMotionTarget->unscheduleClickValidator();
                        _touchMotionTarget->unscheduleLongClickValidator();
                    }
                } else if (action == MotionEvent::ACTION_CANCEL) {
                    // 터치 취소됨
                    _touchMotionTarget->_touchHasFirstClicked = false;
                    _touchMotionTarget->unscheduleClickValidator();
                    _touchMotionTarget->unscheduleLongClickValidator();
                }
            }
            
            if (action == MotionEvent::ACTION_CANCEL ||
                action == MotionEvent::ACTION_UP) {
                _touchMotionTarget = nullptr;
            }
            
            if (ret == TOUCH_INTERCEPT) {
                return TOUCH_INTERCEPT;
            }
            
            return TOUCH_TRUE;
        }
    } else {
        // if motionTarget is null
        if (action == MotionEvent::ACTION_DOWN) {
            int ret;
            if (dispatchChildren(event, ret))
                return ret;
        }
    }
    
    if (isTouchEnable()) {
        switch (action) {
            case MotionEvent::ACTION_UP:
            case MotionEvent::ACTION_CANCEL:
                return TOUCH_FALSE;
            default:
                return TOUCH_TRUE;
        }
    }
    
    return TOUCH_FALSE;
}

bool SMView::dispatchChildren(MotionEvent* event, int& ret) {
    
    ssize_t numChild = _children.size();
    for (ssize_t i = numChild-1; i >= 0; i--) {
        if (SMView* view = dynamic_cast<SMView*>(_children.at(i))) {
            if (!view->isVisible() || !view->isEnabled())
                continue;
            if ((ret = dispatchTouchEvent(event, view, true)) != TOUCH_FALSE) {
                _touchMotionTarget = view;
                if (view->_touchMotionTarget == nullptr) {
                    _touchMotionTarget->_touchTargeted = true;
                    _touchMotionTarget->_touchEventTime = event->getEventTime();
                    _touchMotionTarget->stateChangeNormalToPress();
                    
                    if (_touchMotionTarget->_touchHasFirstClicked) {
                        _touchMotionTarget->unscheduleClickValidator();
                    }
                    
                    _touchHasFirstClicked = false;
                    if (_touchMotionTarget->isTouchMasked(TOUCH_MASK_LONGCLICK)) {
                        _touchMotionTarget->scheduleLongClickValidator();
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}


void SMView::onClickValidator(float dt) {
    if (isTouchMasked(TOUCH_MASK_DOUBLECLICK) && _touchHasFirstClicked) {
        _touchHasFirstClicked = false;
        _touchTargeted = false;
        performClick(_lastTouchLocation);
    }
}

void SMView::onLongClickValidator(float dt) {
    if (isTouchMasked(TOUCH_MASK_LONGCLICK) && !_touchHasFirstClicked) {
        _touchHasFirstClicked = false;
        _touchTargeted = false;
        stateChangePressToNormal();
        performLongClick(_lastTouchLocation);
    }
}


void SMView::performClick(const cocos2d::Vec2& worldPoint) {
    if (_onClickListener != nullptr) {
        _onClickListener->onClick(this);
    }
    if (_onClickCallback) {
        _onClickCallback(this);
    }
}

void SMView::performLongClick(const cocos2d::Vec2& worldPoint) {
    if (_onLongClickListener != nullptr) {
        _onLongClickListener->onLongClick(this);
    }
    if (_onLongClickCallback) {
        _onLongClickCallback(this);
    }
}

void SMView::performDoubleClick(const cocos2d::Vec2& worldPoint) {
    if (_onDoubleClickListener != nullptr) {
        _onDoubleClickListener->onDoubleClick(this);
    }
    if (_onDoubleClickCallback) {
        _onDoubleClickCallback(this);
    }
}

void SMView::cancel() {
    if (_touchTargeted) {
        _touchTargeted = false;
        _touchHasFirstClicked = false;
        unscheduleClickValidator();
        unscheduleLongClickValidator();
    }
    
    ssize_t numChild = _children.size();
    for (ssize_t i = numChild-1; i >= 0; i--) {
        if (SMView* view = dynamic_cast<SMView*>(_children.at(i))) {
            view->cancel();
        }
    }
}

uint32_t SMView::USER_VIEW_FLAG(int flagId) {
    return (1<<(VIEWFLAG_USER_SHIFT+flagId));
}

void SMView::setColor4F(const cocos2d::Color4F& color) {
    setColor(cocos2d::Color3B((GLubyte)(color.r*255), (GLubyte)(color.g*255), (GLubyte)(color.b*255)));
    setOpacity((GLubyte)(color.a*255));
}

void SMView::setBackgroundColor4F(const cocos2d::Color4F& color) {
    if (_bgColor == color) {
        return;
    }
    
    _bgColor = color;
    
    if (_bgNode == nullptr) {
        if (color.a == 0.0)
            return;
        
        _bgNode = createBackgroundNode();
        _bgNode->setContentSize(_contentSize);
        _bgNode->setPosition(cocos2d::Vec2::ZERO);
        
        cocos2d::Node::addChild(_bgNode, SMViewConstValue::ZOrder::BG, "");
    }
    
    _bgNode->setColor(cocos2d::Color3B(_bgColor));
    _bgNode->setOpacity((GLubyte)(_bgColor.a*255.0));
    _bgNode->setVisible(_bgColor.a != 0);
}

void SMView::setBackgroundColor4F(const cocos2d::Color4F& color, float changeDurationTime) {
    cocos2d::Action* action = getActionByTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
    if (action != nullptr) {
        stopAction(action);
    }
    
    if (changeDurationTime > 0) {
        action = BackgroundColorTo::create(changeDurationTime, color);
        action->setTag(SMViewConstValue::Tag::ACTION_BG_COLOR);
        runAction(action);
        
        return;
    } else {
        setBackgroundColor4F(color);
    }
}

void SMView::setBackgroundColor(const cocos2d::Color3B& color, float changeDurationTime) {
    setBackgroundColor4F(cocos2d::Color4F(color.r/255.0, color.g/255.0, color.b/255.0, _bgColor.a), changeDurationTime);
}

void SMView::setBackgroundOpacity(GLubyte opacity, float changeDurationTime) {
    setBackgroundColor4F(cocos2d::Color4F(_bgColor.r, _bgColor.g, _bgColor.b, opacity/255.0), changeDurationTime);
}

cocos2d::Node* SMView::createBackgroundNode() {
    return ShapeSolidRect::create();
}

cocos2d::Node* SMView::getBackgroundNode() {
    return _bgNode;
}

void SMView::setBackgroundNode(cocos2d::Node* node) {
    
    if (_bgNode != nullptr) {
        cocos2d::Node::removeChild(_bgNode);
        _bgNode = nullptr;
    }
    
    if (node != nullptr) {
        _bgNode = node;
        
        _bgNode->setContentSize(_contentSize);
        _bgNode->setPosition(cocos2d::Vec2::ZERO);
        _bgNode->setColor(cocos2d::Color3B((GLubyte)(_bgColor.r*255.0), (GLubyte)(_bgColor.g*255.0), (GLubyte)(_bgColor.b*255.0)));
        _bgNode->setOpacity((GLubyte)(_bgColor.a*255.0));
        _bgNode->setVisible(_bgColor.a != 0);
        
        cocos2d::Node::addChild(_bgNode, SMViewConstValue::ZOrder::BG, "");
    }
}

void SMView::stateChangePressToNormal() {
    if (_pressState == State::PRESSED) {
        _pressState = State::NORMAL;
        onStateChangePressToNormal();
        if (_onStateChangeListener != nullptr) {
            _onStateChangeListener->onStateChanged(this, _pressState);
        }
        if (_onStateChangeCallback) {
            _onStateChangeCallback(this, _pressState);
        }
    }
}

void SMView::stateChangeNormalToPress() {
    if (_pressState == State::NORMAL) {
        _pressState = State::PRESSED;
        onStateChangeNormalToPress();
        if (_onStateChangeListener != nullptr) {
            _onStateChangeListener->onStateChanged(this, _pressState);
        }
        if (_onStateChangeCallback) {
            _onStateChangeCallback(this, _pressState);
        }
    }
}

void SMView::changeParent(SMView *newParent)
{
    if (newParent==nullptr || getParent()==nullptr) {
        return;
    }

    auto parent = getParent();

    this->retain();
    parent->removeChild(this, false);
    newParent->addChild(this, _localZOrder);
    this->release();
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Abstract UI Container View
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

_UIContainerView::_UIContainerView() :
_uiContainer(nullptr),
_paddingLeft(0),
_paddingRight(0),
_paddingTop(0),
_paddingBottom(0)
{
    _uiContainer = SMView::create();
    _uiContainer->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    cocos2d::Node::addChild(_uiContainer, 0, "");
}

_UIContainerView::~_UIContainerView() {
}

void _UIContainerView::setPadding(const float left, const float top, const float right, const float bottom) {
    _paddingLeft   = left;
    _paddingTop    = top;
    _paddingRight  = right;
    _paddingBottom = bottom;
    setContentSize(getContentSize());
}

void _UIContainerView::setContentSize(const cocos2d::Size& size) {
    auto innerSize = cocos2d::Size(size.width-_paddingLeft-_paddingRight, size.height-_paddingTop-_paddingBottom);
    _uiContainer->setPosition((_paddingLeft + (size.width - _paddingRight))/2, (_paddingBottom + (size.height - _paddingTop))/2);
    _uiContainer->setContentSize(innerSize);
    SMView::setContentSize(size);
}




