//
//  VelocityTracker.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "VelocityTracker.h"
#include <base/CCTouch.h>
#include <cocos2d.h>

#define MAX_POINTERS 16

// based on Android framewwork

//static const int64_t ASSUME_POINTER_STOPPED_TIME = 20 * 1000000; // nanosec per millisec
static const int64_t ASSUME_POINTER_STOPPED_TIME = 40 * 1000000; // nanosec per millisec
#define MAX_POINTER_ID 31

VelocityTracker::VelocityTracker() :
mLastEventTime(0),
mCurrentPointerIdBits(0),
mActivePointerId(-1) {
    mStrategy = new VelocityTrackerStrategy(2);
}

VelocityTracker::~VelocityTracker() {
    delete mStrategy;
}

void VelocityTracker::clear() {
    mCurrentPointerIdBits.clear();
    mActivePointerId = -1;
    mStrategy->clear();
}

void VelocityTracker::clearPointers(BitSet32 idBits) {
    BitSet32 remainingIdBits(mCurrentPointerIdBits.value & ~idBits.value);
    mCurrentPointerIdBits = remainingIdBits;
    
    if (mActivePointerId >= 0 && idBits.hasBit(mActivePointerId)) {
        mActivePointerId = !remainingIdBits.isEmpty() ? remainingIdBits.firstMarkedBit() : -1;
    }
    
    mStrategy->clearPointers(idBits);
}



//------------------------------------------------------------------------------------
//*

void VelocityTracker::addMovement(int64_t eventTime, BitSet32 idBits,  cocos2d::Vec2& position) {
    while (idBits.count() > _MAX_TRACKING_POINTERS) {
        idBits.clearLastMarkedBit();
    }
    
    if ((mCurrentPointerIdBits.value & idBits.value)
        && eventTime >= mLastEventTime + ASSUME_POINTER_STOPPED_TIME) {
        // We have not received any movements for too long.  Assume that all pointers
        // have stopped.
        mStrategy->clear();
        //        return;
    }
    mLastEventTime = eventTime;
    
    mCurrentPointerIdBits = idBits;
    if (mActivePointerId < 0 || !idBits.hasBit(mActivePointerId)) {
        mActivePointerId = idBits.isEmpty() ? -1 : idBits.firstMarkedBit();
    }
    
    mStrategy->addMovement(eventTime, idBits, position);
}
 
 
void VelocityTracker::addMovement(MotionEvent* event) {
    // 싱글터치 버전
    int action = event->getAction();
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        case MotionEvent::ACTION_POINTER_DOWN:
            clear();
            break;
        case MotionEvent::ACTION_MOVE:
            //        case MotionEvent::ACTION_UP: // TODO : ACTION_UP 포함해야 하는지 테스트
            break;
        default:
            return;
    }
    
    BitSet32 idBits;
    cocos2d::Touch* touch = (cocos2d::Touch*)event->getTouch(0);
    idBits.markBit(0);
    
    int64_t eventTime = (int64_t)(event->getEventTime()*1000000000); // sec to nanosec
    
    cocos2d::Vec2 position = touch->getLocationInView();
    position = cocos2d::Vec2(position.x, position.y * -1);
    addMovement(eventTime, idBits, position);
}

void VelocityTracker::addMovement(int action, const float x, const float y, const float eventSec) {
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        case MotionEvent::ACTION_POINTER_DOWN:
            clear();
            break;
        case MotionEvent::ACTION_MOVE:
            break;
        default:
            return;
    }
    
    BitSet32 idBits;
    idBits.markBit(0);
    
    int64_t eventTime = (int64_t)(eventSec*1000000000);
    
    cocos2d::Vec2 position(x, y);
    position = cocos2d::Vec2(position.x, position.y);
    addMovement(eventTime, idBits, position);
}

/*/
 //------------------------------------------------------------------------------------
 //------------------------------------------------------------------------------------
 // TODO : 멀티터치 버전으로 수정해야 함.
 //------------------------------------------------------------------------------------
 //------------------------------------------------------------------------------------
void VelocityTracker::addMovement(MotionEvent* event) {
 int32_t actionMasked = event->getAction();
 
 switch (actionMasked) {
 case MotionEvent::ACTION_DOWN:
 case MotionEvent::ACTION_POINTER_DOWN:
 clear();
 break;
 case MotionEvent::ACTION_MOVE:
 break;
 default:
 return;
 }
 
 ssize_t pointerCount = event->getTouchCount();
 if (pointerCount > MAX_POINTERS) {
 pointerCount = MAX_POINTERS;
 }
 
 BitSet32 idBits;
 for (int i = 0; i < pointerCount; i++) {
 idBits.markBit(event->getTouch(i)->getID());
 }
 
 uint32_t pointerIndex[MAX_POINTERS];
 for (int i = 0; i < pointerCount; i++) {
 pointerIndex[i] = idBits.getIndexOfBit(event->getTouch(i)->getID());
 }
 
 int64_t eventTime = (int64_t)(event->getEventTime()*1000000000);
    std::vector<cocos2d::Vec2> positions;
 positions.resize(pointerCount);
 
 for (int i = 0; i < pointerCount; i++) {
 uint32_t index = pointerIndex[i];
 positions[index] = event->getTouch(i)->getLocation();
 }
 
 addMovement(eventTime, idBits, positions);
}
 
void VelocityTracker::addMovement(int64_t eventTime, BitSet32 idBits, const std::vector<cocos2d::Vec2>& positions) {
    while (idBits.count() > MAX_POINTERS) {
        idBits.clearLastMarkedBit();
    }
    if ((mCurrentPointerIdBits.value & idBits.value)
        && eventTime >= mLastEventTime + ASSUME_POINTER_STOPPED_TIME) {
        // We have not received any movements for too long.  Assume that all pointers
        // have stopped.
        mStrategy->clear();
    }
    mLastEventTime = eventTime;
    mCurrentPointerIdBits = idBits;
    if (mActivePointerId < 0 || !idBits.hasBit(mActivePointerId)) {
        mActivePointerId = idBits.isEmpty() ? -1 : idBits.firstMarkedBit();
 }
    mStrategy->addMovement(eventTime, idBits, positions);
}
 
 //------------------------------------------------------------------------------------
 //*/
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
// TODO : 싱글터치 버전
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

bool VelocityTracker::getVelocity(uint32_t id, float* outVx, float* outVy) const {
    Estimator estimator;
    if (getEstimator(id, &estimator) && estimator.degree >= 1) {
        *outVx = estimator.xCoeff[1];
        *outVy = estimator.yCoeff[1];
        return true;
    }
    // TODO : 검증필요
    *outVx = 0;
    *outVy = 0;
    //    *outVx = estimator.xCoeff[0];
    //    *outVy = estimator.xCoeff[0];
    return false;
}

bool VelocityTracker::getEstimator(uint32_t id, Estimator* outEstimator) const {
    return mStrategy->getEstimator(id, outEstimator);
}


const int64_t VelocityTrackerStrategy::HORIZON;
const uint32_t VelocityTrackerStrategy::HISTORY_SIZE;

VelocityTrackerStrategy::VelocityTrackerStrategy(
                                                 uint32_t degree, Weighting weighting) :
mDegree(degree), mWeighting(weighting) {
    clear();
}

VelocityTrackerStrategy::~VelocityTrackerStrategy() {
}

void VelocityTrackerStrategy::clear() {
    mIndex = 0;
    mMovements[0].idBits.clear();
}

void VelocityTrackerStrategy::clearPointers(BitSet32 idBits) {
    BitSet32 remainingIdBits(mMovements[mIndex].idBits.value & ~idBits.value);
    mMovements[mIndex].idBits = remainingIdBits;
}

void VelocityTrackerStrategy::addMovement(int64_t eventTime, BitSet32 idBits,
                                          cocos2d::Vec2& position) {
    if (++mIndex == HISTORY_SIZE) {
        mIndex = 0;
    }
    
    Movement& movement = mMovements[mIndex];
    movement.eventTime = eventTime;
    movement.idBits = idBits;
    //    uint32_t count = idBits.count();
    movement.positions[0] = position;
    //    for (uint32_t i = 0; i < count; i++) {
    //        movement.positions[i] = positions[i];
    //    }
}

/**
 * Solves a linear least squares problem to obtain a N degree polynomial that fits
 * the specified input data as nearly as possible.
 *
 * Returns true if a solution is found, false otherwise.
 *
 * The input consists of two vectors of data points X and Y with indices 0..m-1
 * along with a weight vector W of the same size.
 *
 * The output is a vector B with indices 0..n that describes a polynomial
 * that fits the data, such the sum of W[i] * W[i] * abs(Y[i] - (B[0] + B[1] X[i]
 * + B[2] X[i]^2 ... B[n] X[i]^n)) for all i between 0 and m-1 is minimized.
 *
 * Accordingly, the weight vector W should be initialized by the caller with the
 * reciprocal square root of the variance of the error in each input data point.
 * In other words, an ideal choice for W would be W[i] = 1 / var(Y[i]) = 1 / stddev(Y[i]).
 * The weights express the relative importance of each data point.  If the weights are
 * all 1, then the data points are considered to be of equal importance when fitting
 * the polynomial.  It is a good idea to choose weights that diminish the importance
 * of data points that may have higher than usual error margins.
 *
 * Errors among data points are assumed to be independent.  W is represented here
 * as a vector although in the literature it is typically taken to be a diagonal matrix.
 *
 * That is to say, the function that generated the input data can be approximated
 * by y(x) ~= B[0] + B[1] x + B[2] x^2 + ... + B[n] x^n.
 *
 * The coefficient of determination (R^2) is also returned to describe the goodness
 * of fit of the model for the given data.  It is a value between 0 and 1, where 1
 * indicates perfect correspondence.
 *
 * This function first expands the X vector to a m by n matrix A such that
 * A[i][0] = 1, A[i][1] = X[i], A[i][2] = X[i]^2, ..., A[i][n] = X[i]^n, then
 * multiplies it by w[i]./
 *
 * Then it calculates the QR decomposition of A yielding an m by m orthonormal matrix Q
 * and an m by n upper triangular matrix R.  Because R is upper triangular (lower
 * part is all zeroes), we can simplify the decomposition into an m by n matrix
 * Q1 and a n by n matrix R1 such that A = Q1 R1.
 *
 * Finally we solve the system of linear equations given by R1 B = (Qtranspose W Y)
 * to find B.
 *
 * For efficiency, we lay out A and Q column-wise in memory because we frequently
 * operate on the column vectors.  Conversely, we lay out R row-wise.
 *
 * http://en.wikipedia.org/wiki/Numerical_methods_for_linear_least_squares
 * http://en.wikipedia.org/wiki/Gram-Schmidt
 */

static float vectorDot(const float* a, const float* b, uint32_t m) {
    float r = 0;
    while (m--) {
        r += *(a++) * *(b++);
    }
    return r;
}

static float vectorNorm(const float* a, uint32_t m) {
    float r = 0;
    while (m--) {
        float t = *(a++);
        r += t * t;
    }
    return sqrtf(r);
}

static bool solveLeastSquares(const float* x, const float* y,
                              const float* w, uint32_t m, uint32_t n, float* outB, float* outDet) {
    
    // Expand the X vector to a matrix A, pre-multiplied by the weights.
    float a[n][m]; // column-major order
    for (uint32_t h = 0; h < m; h++) {
        a[0][h] = w[h];
        for (uint32_t i = 1; i < n; i++) {
            a[i][h] = a[i - 1][h] * x[h];
        }
    }
    
    // Apply the Gram-Schmidt process to A to obtain its QR decomposition.
    float q[n][m]; // orthonormal basis, column-major order
    float r[n][n]; // upper triangular matrix, row-major order
    for (uint32_t j = 0; j < n; j++) {
        for (uint32_t h = 0; h < m; h++) {
            q[j][h] = a[j][h];
        }
        for (uint32_t i = 0; i < j; i++) {
            float dot = vectorDot(&q[j][0], &q[i][0], m);
            for (uint32_t h = 0; h < m; h++) {
                q[j][h] -= dot * q[i][h];
            }
        }
        
        float norm = vectorNorm(&q[j][0], m);
        if (norm < 0.000001f) {
            // vectors are linearly dependent or zero so no solution
            return false;
        }
        
        float invNorm = 1.0f / norm;
        for (uint32_t h = 0; h < m; h++) {
            q[j][h] *= invNorm;
        }
        for (uint32_t i = 0; i < n; i++) {
            r[j][i] = i < j ? 0 : vectorDot(&q[j][0], &a[i][0], m);
        }
    }
    
    // Solve R B = Qt W Y to find B.  This is easy because R is upper triangular.
    // We just work from bottom-right to top-left calculating B's coefficients.
    float wy[m];
    for (uint32_t h = 0; h < m; h++) {
        wy[h] = y[h] * w[h];
    }
    for (uint32_t i = n; i-- != 0; ) {
        outB[i] = vectorDot(&q[i][0], wy, m);
        for (uint32_t j = n - 1; j > i; j--) {
            outB[i] -= r[i][j] * outB[j];
        }
        outB[i] /= r[i][i];
    }
    
    // Calculate the coefficient of determination as 1 - (SSerr / SStot) where
    // SSerr is the residual sum of squares (variance of the error),
    // and SStot is the total sum of squares (variance of the data) where each
    // has been weighted.
    float ymean = 0;
    for (uint32_t h = 0; h < m; h++) {
        ymean += y[h];
    }
    ymean /= m;
    
    float sserr = 0;
    float sstot = 0;
    for (uint32_t h = 0; h < m; h++) {
        float err = y[h] - outB[0];
        float term = 1;
        for (uint32_t i = 1; i < n; i++) {
            term *= x[h];
            err -= term * outB[i];
        }
        sserr += w[h] * w[h] * err * err;
        float var = y[h] - ymean;
        sstot += w[h] * w[h] * var * var;
    }
    *outDet = sstot > 0.000001f ? 1.0f - (sserr / sstot) : 1;
    
    return true;
}

bool VelocityTrackerStrategy::getEstimator(uint32_t id,
                                           VelocityTracker::Estimator* outEstimator) const {
    outEstimator->clear();
    
    // Iterate over movement samples in reverse time order and collect samples.
    float x[HISTORY_SIZE];
    float y[HISTORY_SIZE];
    float w[HISTORY_SIZE];
    float time[HISTORY_SIZE];
    uint32_t m = 0;
    uint32_t index = mIndex;
    const Movement& newestMovement = mMovements[mIndex];
    do {
        const Movement& movement = mMovements[index];
        if (!movement.idBits.hasBit(id)) {
            break;
        }
        
        int64_t age = newestMovement.eventTime - movement.eventTime;
        if (age > HORIZON) {
            break;
        }
        
        const cocos2d::Vec2& position = movement.getPosition(id);
        x[m] = position.x;
        y[m] = position.y;
        w[m] = chooseWeight(index);
        time[m] = -age * 0.000000001f;
        index = (index == 0 ? HISTORY_SIZE : index) - 1;
    } while (++m < HISTORY_SIZE);
    
    if (m == 0) {
        return false; // no data
    }
    
    // Calculate a least squares polynomial fit.
    uint32_t degree = mDegree;
    if (degree > m - 1) {
        degree = m - 1;
    }
    if (degree >= 1) {
        float xdet, ydet;
        uint32_t n = degree + 1;
        if (solveLeastSquares(time, x, w, m, n, outEstimator->xCoeff, &xdet)
            && solveLeastSquares(time, y, w, m, n, outEstimator->yCoeff, &ydet)) {
            outEstimator->time = newestMovement.eventTime;
            outEstimator->degree = degree;
            outEstimator->confidence = xdet * ydet;
            return true;
        }
    }
    
    // No velocity data available for this pointer, but we do have its current position.
    outEstimator->xCoeff[0] = x[0];
    outEstimator->yCoeff[0] = y[0];
    outEstimator->time = newestMovement.eventTime;
    outEstimator->degree = 0;
    outEstimator->confidence = 1;
    return true;
}

float VelocityTrackerStrategy::chooseWeight(uint32_t index) const {
    switch (mWeighting) {
        case WEIGHTING_DELTA: {
            // Weight points based on how much time elapsed between them and the next
            // point so that points that "cover" a shorter time span are weighed less.
            //   delta  0ms: 0.5
            //   delta 10ms: 1.0
            if (index == mIndex) {
                return 1.0f;
            }
            uint32_t nextIndex = (index + 1) % HISTORY_SIZE;
            float deltaMillis = (mMovements[nextIndex].eventTime- mMovements[index].eventTime)
            * 0.000001f;
            if (deltaMillis < 0) {
                return 0.5f;
            }
            if (deltaMillis < 10) {
                return 0.5f + deltaMillis * 0.05;
            }
            return 1.0f;
        }
            
        case WEIGHTING_CENTRAL: {
            // Weight points based on their age, weighing very recent and very old points less.
            //   age  0ms: 0.5
            //   age 10ms: 1.0
            //   age 50ms: 1.0
            //   age 60ms: 0.5
            float ageMillis = (mMovements[mIndex].eventTime - mMovements[index].eventTime)
            * 0.000001f;
            if (ageMillis < 0) {
                return 0.5f;
            }
            if (ageMillis < 10) {
                return 0.5f + ageMillis * 0.05;
            }
            if (ageMillis < 50) {
                return 1.0f;
            }
            if (ageMillis < 60) {
                return 0.5f + (60 - ageMillis) * 0.05;
            }
            return 0.5f;
        }
            
        case WEIGHTING_RECENT: {
            // Weight points based on their age, weighing older points less.
            //   age   0ms: 1.0
            //   age  50ms: 1.0
            //   age 100ms: 0.5
            float ageMillis = (mMovements[mIndex].eventTime - mMovements[index].eventTime)
            * 0.000001f;
            if (ageMillis < 50) {
                return 1.0f;
            }
            if (ageMillis < 100) {
                return 0.5f + (100 - ageMillis) * 0.01f;
            }
            return 0.5f;
        }
            
        case WEIGHTING_NONE:
        default:
            return 1.0f;
    }
}



