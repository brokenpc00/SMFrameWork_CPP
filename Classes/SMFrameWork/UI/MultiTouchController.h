//
//  MultiTouchController.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 18..
//  Copyright © 2017년 KimSteve. All rights reserved.
// reference : https://github.com/osmdroid/osmdroid/blob/master/osmdroid-android/src/main/java/org/metalev/multitouch/controller/MultiTouchController.java



#ifndef MultiTouchController_h
#define MultiTouchController_h

#include "../Interface/MotionEvent.h"
#include <cocos/2d/CCNode.h>
#include <cocos/base/CCEvent.h>
#include <cmath>

class SMView;

namespace MultiTouch {

    class PointInfo {
    public:
        PointInfo() : _numPoints(0), _xMid(0), _yMid(0),
        _dx(0), _dy(0), _diameter(0), _diameterSq(0), _angle(0),
        _isDown(false), _isMultiTouch(false),
        _diameterSqIsCalculated(false), _diameterIsCalculated(false), _angleIsCalculated(false),
        _action(0), _eventTime(0)
        {
            
        }
        
    private:
        // Multitouch information
        int _numPoints;
        
        float _xs[cocos2d::EventTouch::MAX_TOUCHES];
        float _ys[cocos2d::EventTouch::MAX_TOUCHES];
        int _pointerIds[cocos2d::EventTouch::MAX_TOUCHES];
        
        // Midpoint of pinch operations
        float _xMid, _yMid;
        
        // Width/diameter/angle of pinch operations
        float _dx, _dy, _diameter, _diameterSq, _angle;
        
        // Whether or not there is at least one finger down (isDown) and/or at
        // least two fingers down (isMultiTouch)
        bool _isDown, _isMultiTouch;
        
        // Whether or not these fields have already been calculated, for caching purposes
        bool _diameterSqIsCalculated, _diameterIsCalculated, _angleIsCalculated;
        
        // Event action code and event time
        int _action;
        
        float _eventTime;
        
    public:
        /** Set all point info */
        void set(int numPoints, float* x, float* y, int* pointerIds, int action, bool isDown, float eventTime) {
            
            _eventTime = eventTime;
            _action = action;
            _numPoints = numPoints;
            
            for (int i = 0; i < numPoints; i++) {
                _xs[i] = x[i];
                _ys[i] = y[i];
                _pointerIds[i] = pointerIds[i];
            }
            
            _isDown = isDown;
            _isMultiTouch = numPoints >= 2;
            
            if (_isMultiTouch) {
                _xMid = (x[0] + x[1]) * .5f;
                _yMid = (y[0] + y[1]) * .5f;
                
                _dx = std::abs(x[1] - x[0]);
                _dy = std::abs(y[1] - y[0]);
                
            } else {
                // Single-touch event
                _xMid = x[0];
                _yMid = y[0];
                _dx = _dy = 0.0f;
            }
            
            // Need to re-calculate the expensive params if they're needed
            _diameterSqIsCalculated = _diameterIsCalculated = _angleIsCalculated = false;
        }

    public:
        /**
         * Copy all fields from one PointInfo class to another. PointInfo objects
         * are volatile so you should use this if you want to keep track of the last
         * touch event in your own code.
         */
        void set(const PointInfo& other) {
            _numPoints = other._numPoints;
            
            for (int i = 0; i < _numPoints; i++) {
                _xs[i] = other._xs[i];
                _ys[i] = other._ys[i];
                _pointerIds[i] = other._pointerIds[i];
            }
            
            _xMid = other._xMid;
            _yMid = other._yMid;
            _dx = other._dx;
            _dy = other._dy;
            _diameter = other._diameter;
            _diameterSq = other._diameterSq;
            _angle = other._angle;
            _isDown = other._isDown;
            _action = other._action;
            _isMultiTouch = other._isMultiTouch;
            _diameterIsCalculated = other._diameterIsCalculated;
            _diameterSqIsCalculated = other._diameterSqIsCalculated;
            _angleIsCalculated = other._angleIsCalculated;
            _eventTime = other._eventTime;
        }
        
        float getDistance(PointInfo& other) {
            float deltaX = getX() - other.getX();
            float deltaY = getY() - other.getY();
            return (float)std::sqrt(deltaX*deltaX + deltaY*deltaY);
        }
        
        // ---------------------------------------------------------------------------
        
        /** True if number of touch points >= 2. */
        bool isMultiTouch() {
            return _isMultiTouch;
        }
        
        /** Difference between x coords of touchpoint 0 and 1. */
        float getMultiTouchWidth() {
            return _isMultiTouch ? _dx : 0.0f;
        }
        
        /** Difference between y coords of touchpoint 0 and 1. */
        float getMultiTouchHeight() {
            return _isMultiTouch ? _dy : 0.0f;
        }
        
        /** Calculate the squared diameter of the multitouch event, and cache it.
         * Use this if you don't need to perform the sqrt. */
        float getMultiTouchDiameterSq() {
            if (!_diameterSqIsCalculated) {
                _diameterSq = (_isMultiTouch ? _dx * _dx + _dy * _dy : 0.0f);
                _diameterSqIsCalculated = true;
            }
            return _diameterSq;
        }
        
        /** Calculate the diameter of the multitouch event, and cache it. Uses fast
         * int sqrt but gives accuracy to 1/16px. */
        float getMultiTouchDiameter() {
            if (!_diameterIsCalculated) {
                if (!_isMultiTouch) {
                    _diameter = 0.0f;
                } else {
                    float diamSq = getMultiTouchDiameterSq();
                    _diameter = (diamSq == 0.0f ? 0.0f : std::sqrt(diamSq));
                    // Make sure diameter is never less than dx or dy, for trig purposes
                    if (_diameter < _dx)
                        _diameter = _dx;
                    if (_diameter < _dy)
                        _diameter = _dy;
                }
                _diameterIsCalculated = true;
            }
            return _diameter;
        }
        
        /**
         * Calculate the angle of a multitouch event, and cache it.
         * Actually gives the smaller of the two angles between the x axis and the line
         * between the two touchpoints, so range is [0,Math.PI/2]. Uses Math.atan2().
         */
        float getMultiTouchAngle() {
            if (!_angleIsCalculated) {
                if (!_isMultiTouch)
                    _angle = 0.0f;
                else
                    _angle = (float) std::atan2(_ys[1] - _ys[0], _xs[1] - _xs[0]);
                _angleIsCalculated = true;
            }
            return _angle;
        }
        
        // ---------------------------------------------------------------------------
        
        /** Return the total number of touch points */
        int getNumTouchPoints() {
            return _numPoints;
        }
        
        /** Return the X coord of the first touch point if there's only one,
         * or the midpoint between first and second touch points if two or more. */
        float getX() {
            return _xMid;
        }
        
        /** Return the array of X coords -- only the first getNumTouchPoints()
         * of these is defined. */
        float* getXs() {
            return _xs;
        }
        
        /** Return the X coord of the first touch point if there's only one,
         * or the midpoint between first and second touch points if two or more. */
        float getY() {
            return _yMid;
        }
        
        /** Return the array of Y coords -- only the first getNumTouchPoints()
         * of these is defined. */
        float* getYs() {
            return _ys;
        }
        
        /**
         * Return the array of pointer ids -- only the first getNumTouchPoints()
         * of these is defined. These don't have to be all the numbers from 0 to
         * getNumTouchPoints()-1 inclusive, numbers can be skipped if a finger is
         * lifted and the touch sensor is capable of detecting that that
         * particular touch point is no longer down. Note that a lot of sensors do
         * not have this capability: when finger 1 is lifted up finger 2
         * becomes the new finger 1.  However in theory these IDs can correct for
         * that.  Convert back to indices using MotionEvent.findPointerIndex().
         */
        int* getPointerIds() {
            return _pointerIds;
        }
        
        // ---------------------------------------------------------------------------
        
        bool isDown() {
            return _isDown;
        }
        
        int getAction() {
            return _action;
        }
        
        float getEventTime() {
            return _eventTime;
        }
    };
    
    
    /**
     * A class that is used to store scroll offsets and scale information for
     * objects that are managed by the multitouch controller
     */
    class PositionAndScale {
    protected:
        float _xOff, _yOff, _scale, _scaleX, _scaleY, _angle;
        bool _updateScale, _updateScaleXY, _updateAngle;
        
        
    public:
        
        PositionAndScale() : _xOff(0.0), _yOff(0.0), _scale(1.0), _scaleX(1.0), _scaleY(1.0), _angle(0.0),
        _updateScale(false), _updateScaleXY(false), _updateAngle(false) {
        }
        
        /**
         * Set position and optionally scale, anisotropic scale, and/or angle.
         * Where if the corresponding "update" flag is set to false, the field's
         * value will not be changed during a pinch operation. If the value is
         * not being updated *and* the value is not used by the client
         * application, then the value can just be zero. However if the value is
         * not being updated but the value *is* being used by the client
         * application, the value should still be specified and the update flag
         * should be false (e.g. angle of the object being dragged should still
         * be specified even if the program is in "resize" mode rather than "rotate"
         * mode).
         */
        void set(float xOff, float yOff, bool updateScale, float scale,
                 bool updateScaleXY, float scaleX, float scaleY,
                 bool updateAngle, float angle) {
            _xOff = xOff;
            _yOff = yOff;
            _updateScale = updateScale;
            _scale = scale == 0.0f ? 1.0f : scale;
            _updateScaleXY = updateScaleXY;
            _scaleX = scaleX == 0.0f ? 1.0f : scaleX;
            _scaleY = scaleY == 0.0f ? 1.0f : scaleY;
            _updateAngle = updateAngle;
            _angle = angle;
        }
        
    protected:
        /** Set position and optionally scale, anisotropic scale, and/or angle,
         * without changing the "update" flags. */
        void set(float xOff, float yOff, float scale,
                 float scaleX, float scaleY, float angle) {
            _xOff = xOff;
            _yOff = yOff;
            _scale = scale == 0.0f ? 1.0f : scale;
            _scaleX = scaleX == 0.0f ? 1.0f : scaleX;
            _scaleY = scaleY == 0.0f ? 1.0f : scaleY;
            _angle = angle;
        }
        
    public:
        float getXOff() {
            return _xOff;
        }
        
        float getYOff() {
            return _yOff;
        }
        
        float getScale() {
            return !_updateScale ? 1.0f : _scale;
        }
        
        /** Included in case you want to support anisotropic scaling */
        float getScaleX() {
            return !_updateScaleXY ? 1.0f : _scaleX;
        }
        
        /** Included in case you want to support anisotropic scaling */
        float getScaleY() {
            return !_updateScaleXY ? 1.0f : _scaleY;
        }
        
        float getAngle() {
            return !_updateAngle ? 0.0f : _angle;
        }
        
        friend class Controller;
        
    };
    
    class ObjectCanvas {
        
    public:
        ObjectCanvas() {}
        ~ObjectCanvas() {}
        /**
         * See if there is a draggable object at the current point. Returns the
         * object at the point, or null if nothing to drag. To start a multitouch
         * drag/stretch operation, this routine must return some non-null reference
         * to an object. This object is passed into the other methods in this interface
         * when they are called.
         *
         * @param touchPoint
         *            The point being tested (in object coordinates). Return the
         *            topmost object under this point, or if dragging/stretching
         *            the whole canvas, just return a reference to the canvas.
         * @return a reference to the object under the point being tested, or
         *            null to cancel the drag operation. If dragging/stretching the whole
         *            canvas (e.g. in a photo viewer), always return non-null, otherwise
         *            the stretch operation won't work.
         */
        virtual cocos2d::Node* getDraggableObjectAtPoint(PointInfo& touchPoint) { return nullptr; }
        
        /**
         * TODO: document
         */
        virtual bool pointInObjectGrabArea(PointInfo& touchPoint, cocos2d::Node* obj) { return false; }
        
        /**
         * Get the screen coords of the dragged object's origin, and scale
         * multiplier to convert screen coords to obj coords. The job of this routine
         * is to call the .set() method on the passed PositionAndScale object to
         * record the initial position and scale of the object (in object coordinates)
         * before any dragging/stretching takes place.
         *
         * @param obj
         *            The object being dragged/stretched.
         * @param objPosAndScaleOut
         *            Output parameter: You need to call objPosAndScaleOut.set()
         *            to record the current position and scale of obj.
         */
        virtual void getPositionAndScale(cocos2d::Node* obj, PositionAndScale& objPosAndScaleOut) {}
        
        /**
         * Callback to update the position and scale (in object coords) of the
         * currently-dragged object.
         *
         * @param obj
         *            The object being dragged/stretched.
         * @param newObjPosAndScale
         *            The new position and scale of the object, in object
         *            coordinates. Use this to move/resize the object before returning.
         * @param touchPoint
         *            Info about the current touch point, including multitouch
         *            information and utilities to calculate and cache multitouch pinch
         *            diameter etc. (Note: touchPoint is volatile, if you want to
         *            keep any fields of touchPoint, you must copy them before the method
         *            body exits.)
         * @return true
         *            if setting the position and scale of the object was successful,
         *            or false if the position or scale parameters are out of range
         *            for this object.
         */
        virtual bool setPositionAndScale(cocos2d::Node* obj, PositionAndScale& newObjPosAndScale,
                                         PointInfo& touchPoint) { return false; }
        
        /**
         * Select an object at the given point. Can be used to bring the object to
         * top etc. Only called when first touchpoint goes down, not when multitouch
         * is initiated. Also called with null on touch-up.
         *
         * @param obj
         *            The object being selected by single-touch, or null on touch-up.
         * @param touchPoint
         *            The current touch point.
         */
        virtual void selectObject(cocos2d::Node*, PointInfo& touchPoint) {}
        
        virtual void doubleClickObject(cocos2d::Node*, PointInfo& touchPoint) {}
        
        virtual void touchModeChanged(const int touchMode, PointInfo& touchPoint) {}
        
        virtual cocos2d::Vec2 toWorldPoint(const cocos2d::Vec2& canvasPoint) = 0;
        
        virtual cocos2d::Vec2 toCanvasPoint(const cocos2d::Vec2& worldPoint) = 0;
        
    };
    
    
    class Controller {
        
        
    public:
        Controller(ObjectCanvas* objectCanvas);
        ~Controller();
        
    private:
        /**
         * Time in ms required after a change in event status (e.g. putting down
         * or lifting off the second finger) before events actually do anything --
         * helps eliminate noisy jumps that happen on change of status
         */
        static const float EVENT_SETTLE_TIME_INTERVAL;
        
        /**
         * The biggest possible abs val of the change in x or y between multitouch
         * events (larger dx/dy events are ignored) -- helps eliminate jumps in
         * pointer position on finger 2 up/down.
         */
        static const float MAX_MULTITOUCH_POS_JUMP_SIZE;
        
        /**
         * The biggest possible abs val of the change in multitouchWidth or
         * multitouchHeight between multitouch events (larger-jump events are ignored) --
         * helps eliminate jumps in pointer position on finger 2 up/down.
         */
        static const float MAX_MULTITOUCH_DIM_JUMP_SIZE;
        
        /** The smallest possible distance between multitouch points (used to
         * avoid div-by-zero errors and display glitches)
         */
        static const float MIN_MULTITOUCH_SEPARATION;
        
        /** The distance selectedObject must move before registering a drag taking place */
        static const float THRESHOLD;
        
        static const float SETTLE_THRESHOLD;
        
        // ---------------------------------------------------------------------------
    public:
        /** No touch points down. */
        static const int MODE_NOTHING;
        
        /** One touch point down, dragging an object. */
        static const int MODE_DRAG;
        
        /** Two or more touch points down, stretching/rotating an object using the
         * first two touch points. */
        static const int MODE_PINCH;
        
        static const int MODE_ST_GRAB;
        
        // ---------------------------------------------------------------------------
        
    private:
        ObjectCanvas* _objectCanvas;
        
        /** The current touch point */
        PointInfo mCurrPt;
        
        /** The previous touch point */
        PointInfo mPrevPt;
        
        /** The initial touch point */
        PointInfo mInitPt;
        
        /** The initial touch point */
        PointInfo mLastUpPt;
        
        /** Fields extracted from mCurrPt */
        float mCurrPtX, mCurrPtY, mCurrPtDiam, mCurrPtWidth, mCurrPtHeight, mCurrPtAng;
        
        // ---------------------------------------------------------------------------
    private:
        
        float _xVals[cocos2d::EventTouch::MAX_TOUCHES];
        float _yVals[cocos2d::EventTouch::MAX_TOUCHES];
        int _pointerIds[cocos2d::EventTouch::MAX_TOUCHES];
        
        /** The object being dragged/stretched */
        cocos2d::Node* _selectedObject;
        
        cocos2d::Node* _prevSelectedObject;
        
        /** Current position and scale of the dragged object */
        PositionAndScale mCurrXform;
        
        /** Drag/pinch start time and time to ignore spurious events until
         * (to smooth over event noise) */
        float mSettleStartTime, mSettleEndTime;
        
        /** Conversion from object coords to screen coords */
        float _startPosX, _startPosY;
        double _startAngle, _startRadius;
        
        /** Conversion between scale and width, and object angle and start pinch angle */
        float _startScaleOverPinchDiam, _startAngleMinusPinchAngle;
        
        /** Conversion between X scale and width, and Y scale and height */
        float _startScaleXOverPinchWidth, _startScaleYOverPinchHeight;
        
        /** Whether the current object has moved beyond THRESHOLD */
        bool mDragOccurred;
        
        /** Current drag mode */
        int mMode;
        
        // ---------------------------------------------------------------------------
        
    protected:
        /**
         * Extract fields from mCurrPt, respecting the update* fields of mCurrPt.
         * This just avoids code duplication. I hate that Java doesn't support
         * higher-order functions, tuples or multiple return values from functions.
         */
        void extractCurrPtInfo();
        
    public:
        bool dragOccurred();
        
        int getMode();
        
        PointInfo& getCurrentPoint();
        
        // ---------------------------------------------------------------------------
        
        /** Process incoming touch events */
        bool onTouchEvent(SMView* target, MotionEvent* event);
        
        
    private:
        void decodeTouchEvent(int pointerCount, float* x, float* y, int* pointerIds, int action, bool down, float eventTime);
        // ---------------------------------------------------------------------------
        /** Start dragging/pinching, or reset drag/pinch to current point if something
         * goes out of range */
        void anchorAtThisPositionAndScale();
        
        
        /** Drag/stretch/rotate the selected object using the current touch
         * position(s) relative to the anchor position(s). */
        void performDragOrPinch();
        
        /**
         * Returns true if selectedObject has moved passed the movement THRESHOLD,
         * otherwise false.
         * This serves to help avoid small jitters in the object when the user
         * places their finger on the object without intending to move it.
         */
        bool pastThreshold(float deltaX, float deltaY, float newScale);
        
        /**
         * State-based controller for tracking switches between no-touch,
         * single-touch and multi-touch situations. Includes logic for cleaning up the
         * event stream, as events around touch up/down are noisy at least on
         * early Synaptics sensors.
         */
        void multiTouchController();
        
    };
    
}


#endif /* MultiTouchController_h */
