//
//  MultiTouchController.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "MultiTouchController.h"
#include "../Base/SMView.h"
#include <algorithm>

using namespace MultiTouch;


const float Controller::EVENT_SETTLE_TIME_INTERVAL = SMViewConstValue::Config::TAP_TIMEOUT;

const float Controller::MAX_MULTITOUCH_POS_JUMP_SIZE = 30.0f;
const float Controller::MAX_MULTITOUCH_DIM_JUMP_SIZE = 40.0f;
const float Controller::MIN_MULTITOUCH_SEPARATION = 10.0f;
const float Controller::THRESHOLD = 0.001f;
const float Controller::SETTLE_THRESHOLD = 5.0f;


const int Controller::MODE_NOTHING = 0;
const int Controller::MODE_DRAG = 1;
const int Controller::MODE_PINCH = 2;
const int Controller::MODE_ST_GRAB = 3;


Controller::Controller(ObjectCanvas* objectCanvas) :
_objectCanvas(objectCanvas)
, _selectedObject(nullptr)
, _prevSelectedObject(nullptr)
, mMode(MODE_NOTHING)
, mDragOccurred(false)
{
    
}

Controller::~Controller()
{
    
}

void Controller::extractCurrPtInfo()
{
    // Get new drag/pinch params. Only read multitouch fields that are needed,
    // to avoid unnecessary computation (diameter and angle are expensive operations).
    mCurrPtX = mCurrPt.getX();
    mCurrPtY = mCurrPt.getY();
    
    mCurrPtDiam = std::max(MIN_MULTITOUCH_SEPARATION * .71f,
                           !mCurrXform._updateScale ? 0.0f : mCurrPt.getMultiTouchDiameter());
    mCurrPtWidth = std::max(MIN_MULTITOUCH_SEPARATION,
                            !mCurrXform._updateScaleXY ? 0.0f : mCurrPt.getMultiTouchWidth());
    mCurrPtHeight = std::max(MIN_MULTITOUCH_SEPARATION,
                             !mCurrXform._updateScaleXY ? 0.0f : mCurrPt.getMultiTouchHeight());
    mCurrPtAng = !mCurrXform._updateAngle ? 0.0f : mCurrPt.getMultiTouchAngle();
}

bool Controller::dragOccurred()
{
    return mDragOccurred;
}

/** Process incoming touch events */
bool Controller::onTouchEvent(SMView* target, MotionEvent* event) {
    int pointerCount = (int)event->getTouchCount();
    
    if (pointerCount == 1) {
        cocos2d::Vec2 pt = _objectCanvas->toCanvasPoint(event->getTouch(0)->getLocation());
        //        cocos2d::Vec2 pt = target->convertToNodeSpace3D(event->getTouch(0)->getLocation());
        _xVals[0] = pt.x;
        _yVals[0] = pt.y;
        
    } else {
        
        for (int ptrIdx = 0; ptrIdx < pointerCount; ptrIdx++) {
            int ptrId = event->getTouch(ptrIdx)->getID();
            
            _pointerIds[ptrIdx] = ptrId;
            
            // N.B. if pointerCount == 1, then the following methods throw
            // an array index out of range exception, and the code above
            // is therefore required not just for Android 1.5/1.6 but
            // also for when there is only one touch point on the screen --
            // pointlessly inconsistent :(
            
            //            cocos2d::Vec2 pt = target->convertToNodeSpace3D(event->getTouch(ptrIdx)->getLocation());
            cocos2d::Vec2 pt = _objectCanvas->toCanvasPoint(event->getTouch(ptrIdx)->getLocation());
            
            _xVals[ptrIdx] = pt.x;
            _yVals[ptrIdx] = pt.y;
        }
        
    }
    
    int action = event->getAction();
    bool isDown;
    
    switch (action) {
        case MotionEvent::ACTION_UP:
        case MotionEvent::ACTION_POINTER_UP:
        case MotionEvent::ACTION_CANCEL:
            isDown = false;
            break;
        default:
            isDown = true;
            break;
    }
    
    
    // Decode event
    decodeTouchEvent(pointerCount, _xVals, _yVals, _pointerIds,
                     action,
                     isDown,
                     event->getEventTime());
    
    return (_selectedObject != nullptr);
}

void Controller::decodeTouchEvent(int pointerCount, float* x, float* y, int* pointerIds, int action, bool down, float eventTime) {
    mPrevPt.set(mCurrPt);
    mCurrPt.set(pointerCount, x, y, pointerIds, action, down, eventTime);
    
    multiTouchController();
}

// ---------------------------------------------------------------------------

/** Start dragging/pinching, or reset drag/pinch to current point if something
 * goes out of range */
void Controller::anchorAtThisPositionAndScale() {
    if (_selectedObject == nullptr)
        return;
    
    // Get selected object's current position and scale
    _objectCanvas->getPositionAndScale(_selectedObject, mCurrXform);
    
    // Figure out the object coords of the drag start point's screen coords.
    // All stretching should be around this point in object-coord-space.
    // Also figure out out ratio between object scale factor and multitouch
    // diameter at beginning of drag; same for angle and optional anisotropic
    // scale.
    float currScaleInv = 1.0f / (!mCurrXform._updateScale ? 1.0f
                                 : mCurrXform._scale == 0.0f ? 1.0f : mCurrXform._scale);
    extractCurrPtInfo();
    _startPosX = (mCurrPtX - mCurrXform._xOff) * currScaleInv;
    _startPosY = (mCurrPtY - mCurrXform._yOff) * currScaleInv;
    _startAngle = mCurrPtAng + std::atan2(_startPosY, -_startPosX);
    _startRadius = std::sqrt(_startPosX * _startPosX + _startPosY * _startPosY);
    _startScaleOverPinchDiam = mCurrXform._scale / mCurrPtDiam;
    _startScaleXOverPinchWidth = mCurrXform._scaleX / mCurrPtWidth;
    _startScaleYOverPinchHeight = mCurrXform._scaleY / mCurrPtHeight;
    _startAngleMinusPinchAngle = mCurrXform._angle - mCurrPtAng;
}

/** Drag/stretch/rotate the selected object using the current touch
 * position(s) relative to the anchor position(s). */
void Controller::performDragOrPinch() {
    // Don't do anything if we're not dragging anything
    if (_selectedObject == nullptr)
        return;
    
    // Calc new position of dragged object
    float currScale = !mCurrXform._updateScale ? 1.0f : mCurrXform._scale == 0.0f ? 1.0f : mCurrXform._scale;
    extractCurrPtInfo();
    //-------------------------------------------------------------
    //        float newPosX = mCurrPtX - startPosX * currScale;
    //        float newPosY = mCurrPtY - startPosY * currScale;
    //-------------------------------------------------------------
    // 핀치의 중앙에서 회전하도록 수정.
    double diffAngle = _startAngle - mCurrPtAng;
    float newPosX = (float) (mCurrPtX + _startRadius * currScale * std::cos(diffAngle));
    float newPosY = (float) (mCurrPtY - _startRadius * currScale * std::sin(diffAngle));
    //-------------------------------------------------------------
    
    
    float deltaX = mCurrPt.getX() - mPrevPt.getX();
    float deltaY = mCurrPt.getY() - mPrevPt.getY();
    
    // Calc new scale of object, if any
    float newScale = mCurrXform._scale;
    if (mMode == MODE_ST_GRAB) {
        if (deltaX < 0.0f || deltaY < 0.0f) {
            newScale = mCurrXform._scale - 0.04f;
        } else {
            newScale = mCurrXform._scale + 0.04f;
        }
        if (newScale < 0.35f) return;
    } else {
        newScale = _startScaleOverPinchDiam * mCurrPtDiam;
    }
    
    if (!mDragOccurred) {
        if (!pastThreshold(std::abs(deltaX), std::abs(deltaY), newScale)) {
            return;
        }
    }
    
    float newScaleX = _startScaleXOverPinchWidth * mCurrPtWidth;
    float newScaleY = _startScaleYOverPinchHeight * mCurrPtHeight;
    float newAngle = _startAngleMinusPinchAngle + mCurrPtAng;
    
    // Set the new obj coords, scale, and angle as appropriate
    // (notifying the subclass of the change).
    mCurrXform.set(newPosX, newPosY, newScale, newScaleX, newScaleY, newAngle);
    
    bool success = _objectCanvas->setPositionAndScale(_selectedObject,
                                                      mCurrXform, mCurrPt);
    if (!success)
        ; // If we could't set those params, do nothing currently
    mDragOccurred = true;
}

/**
 * Returns true if selectedObject has moved passed the movement THRESHOLD,
 * otherwise false.
 * This serves to help avoid small jitters in the object when the user
 * places their finger on the object without intending to move it.
 */
bool Controller::pastThreshold(float deltaX, float deltaY, float newScale) {
    if (deltaX < THRESHOLD && deltaY < THRESHOLD) {
        if (newScale == mCurrXform._scale) {
            mDragOccurred = false;
            return false;
        }
    }
    mDragOccurred = true;
    return true;
}


/**
 * State-based controller for tracking switches between no-touch,
 * single-touch and multi-touch situations. Includes logic for cleaning up the
 * event stream, as events around touch up/down are noisy at least on
 * early Synaptics sensors.
 */
void Controller::multiTouchController() {
    
    switch (mMode) {
        case MODE_NOTHING:
            // Not doing anything currently
            if (mCurrPt.isDown()) {
                mInitPt.set(mCurrPt);
                // Start a new single-point drag
                _selectedObject = _objectCanvas->getDraggableObjectAtPoint(mCurrPt);
                
                if (_selectedObject != nullptr) {
                    if (_objectCanvas->pointInObjectGrabArea(mCurrPt, _selectedObject)) {
                        // Started a new single-point scale/rotate
                        mMode = MODE_ST_GRAB;
                        _objectCanvas->touchModeChanged(mMode, mCurrPt);
                        _objectCanvas->selectObject(_selectedObject, mCurrPt);
                        anchorAtThisPositionAndScale();
                        mSettleStartTime = mCurrPt.getEventTime();
                        mSettleEndTime = mSettleStartTime + EVENT_SETTLE_TIME_INTERVAL;
                    } else {
                        // Started a new single-point drag
                        mMode = MODE_DRAG;
                        _objectCanvas->touchModeChanged(mMode, mCurrPt);
                        _objectCanvas->selectObject(_selectedObject, mCurrPt);
                        anchorAtThisPositionAndScale();
                        // Don't need any settling time if just placing one finger,
                        // there is no noise
                        mSettleStartTime = mCurrPt.getEventTime();
                        mSettleEndTime = mSettleStartTime + EVENT_SETTLE_TIME_INTERVAL;
                    }
                }
                if (_prevSelectedObject != _selectedObject) {
                    _prevSelectedObject = nullptr;
                }
            }
            break;
            
        case MODE_ST_GRAB:
            _prevSelectedObject = nullptr;
            // Currently in a single-point drag
            if (!mCurrPt.isDown()) {
                // First finger was released, stop scale/rotate
                mMode = MODE_NOTHING;
                _objectCanvas->touchModeChanged(mMode, mCurrPt);
                _prevSelectedObject = _selectedObject;
                _objectCanvas->selectObject((_selectedObject = nullptr), mCurrPt);
                mDragOccurred = false;
            } else {
                // Point 1 is still down, do scale/rotate
                performDragOrPinch();
            }
            break;
            
        case MODE_DRAG:
            // Currently in a single-point drag
            if (!mCurrPt.isDown()) {
                // First finger was released, stop dragging
                mMode = MODE_NOTHING;
                _objectCanvas->touchModeChanged(mMode, mCurrPt);
                
                // check doubleTab
                bool performDoubleTab = false;
                if (_prevSelectedObject != nullptr && _prevSelectedObject == _selectedObject) {
                    if (mCurrPt.getEventTime() - mLastUpPt.getEventTime() < SMViewConstValue::Config::DOUBLE_TAP_TIMEOUT) {
                        
                        auto p1 = _objectCanvas->toWorldPoint(cocos2d::Vec2(mCurrPt.getX(), mCurrPt.getY()));
                        auto p2 = _objectCanvas->toWorldPoint(cocos2d::Vec2(mLastUpPt.getX(), mLastUpPt.getY()));
                        float dist = p1.distance(p2);
                        //                        float dist = mLastUpPt.getDistance(mCurrPt);
                        if (dist >= 0 && dist < SMViewConstValue::Config::SCALED_DOUBLE_TAB_SLOPE) {
                            // 더블탭
                            performDoubleTab = true;
                        }
                    }
                }
                if (performDoubleTab) {
                    _prevSelectedObject = nullptr;
                    _objectCanvas->doubleClickObject(_selectedObject, mCurrPt);
                } else {
                    _prevSelectedObject = _selectedObject;
                    mLastUpPt.set(mCurrPt);
                }
                _prevSelectedObject = _selectedObject;
                mLastUpPt.set(mCurrPt);
                
                _selectedObject = nullptr;
                mDragOccurred = false;
            } else if (mCurrPt.isMultiTouch()) {
                _prevSelectedObject = nullptr;
                // Point 1 was already down and point 2 was just placed down
                mMode = MODE_PINCH;
                _objectCanvas->touchModeChanged(mMode, mCurrPt);
                
                // Restart the drag with the new drag position (that is at the
                // midpoint between the touchpoints)
                anchorAtThisPositionAndScale();
                // Need to let events settle before moving things,
                // to help with event noise on touchdown
                mSettleStartTime = mSettleEndTime = mCurrPt.getEventTime();
            } else {
                // Point 1 is still down and point 2 did not change state,
                // just do single-point drag to new location
                if (mCurrPt.getEventTime() < mSettleEndTime) {
                    float deltaX = mCurrPt.getX() - mInitPt.getX();
                    float deltaY = mCurrPt.getY() - mInitPt.getY();
                    if (std::sqrt(deltaX*deltaX + deltaY*deltaY) > SETTLE_THRESHOLD) {
                        performDragOrPinch();
                        mSettleEndTime = mSettleStartTime;
                    } else {
                        // Ignore the first few events if we just stopped stretching,
                        // because if finger 2 was kept down while
                        // finger 1 is lifted, then point 1 gets mapped to finger 2.
                        // Restart the drag from the new position.
                        anchorAtThisPositionAndScale();
                    }
                } else {
                    // Keep dragging, move to new point
                    performDragOrPinch();
                }
            }
            break;
            
        case MODE_PINCH:
            _prevSelectedObject = nullptr;
            // Two-point pinch-scale/rotate/translate
            if (!mCurrPt.isMultiTouch() || !mCurrPt.isDown()) {
                // Dropped one or both points, stop stretching
                
                if (!mCurrPt.isDown()) {
                    // Dropped both points, go back to doing nothing
                    mMode = MODE_NOTHING;
                    _objectCanvas->touchModeChanged(mMode, mCurrPt);
                    _prevSelectedObject = _selectedObject;
                    _objectCanvas->selectObject((_selectedObject = nullptr), mCurrPt);
                    
                } else {
                    // Just dropped point 2, downgrade to a single-point drag
                    mMode = MODE_DRAG;
                    _objectCanvas->touchModeChanged(mMode, mCurrPt);
                    // Restart the pinch with the single-finger position
                    anchorAtThisPositionAndScale();
                    // Ignore the first few events after the drop, in case we
                    // dropped finger 1 and left finger 2 down
                    mSettleStartTime = mSettleEndTime = mCurrPt.getEventTime();
                }
                
            } else {
                // Still pinching
                // 이거 조금 이상함... 빼고 해야 될 수도...
                if (fabs(mCurrPt.getX() - mPrevPt.getX()) > MAX_MULTITOUCH_POS_JUMP_SIZE
                    || fabs(mCurrPt.getY() - mPrevPt.getY()) > MAX_MULTITOUCH_POS_JUMP_SIZE
                    || fabs(mCurrPt.getMultiTouchWidth() - mPrevPt.getMultiTouchWidth()) * .5f > MAX_MULTITOUCH_DIM_JUMP_SIZE
                    || fabs(mCurrPt.getMultiTouchHeight() - mPrevPt.getMultiTouchHeight()) * .5f > MAX_MULTITOUCH_DIM_JUMP_SIZE) {
                    // Jumped too far, probably event noise, reset and ignore events for a bit
                    anchorAtThisPositionAndScale();
                    mSettleStartTime = mCurrPt.getEventTime();
                    mSettleEndTime = mSettleStartTime + EVENT_SETTLE_TIME_INTERVAL;

                } else if (mCurrPt.getEventTime() < mSettleEndTime) {
//                if (mCurrPt.getEventTime() < mSettleEndTime) {    // 뺄거면 여기까지
                    
                    // Events have not yet settled, reset
                    anchorAtThisPositionAndScale();
                } else {
                    // Stretch to new position and size
                    performDragOrPinch();
                }
            }
            break;
    }
}

int Controller::getMode() {
    return mMode;
}

PointInfo& Controller::getCurrentPoint() {
    return mCurrPt;
}

