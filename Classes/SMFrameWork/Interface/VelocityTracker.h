//
//  VelocityTracker.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef VelocityTracker_h
#define VelocityTracker_h

#include "MotionEvent.h"
#include <base/ccTypes.h>

#define _MAX_TRACKING_POINTERS 1

class VelocityTrackerStrategy;

struct BitSet32 {
    uint32_t value;
    
    inline BitSet32() : value(0) { }
    explicit inline BitSet32(uint32_t value) : value(value) { }
    
    // Gets the value associated with a particular bit index.
    static inline uint32_t valueForBit(uint32_t n) { return 0x80000000 >> n; }
    
    // Clears the bit set.
    inline void clear() { value = 0; }
    
    // Returns the number of marked bits in the set.
    inline uint32_t count() const { return __builtin_popcount(value); }
    
    // Returns true if the bit set does not contain any marked bits.
    inline bool isEmpty() const { return ! value; }
    
    // Returns true if the bit set does not contain any unmarked bits.
    inline bool isFull() const { return value == 0xffffffff; }
    
    // Returns true if the specified bit is marked.
    inline bool hasBit(uint32_t n) const { return value & valueForBit(n); }
    
    // Marks the specified bit.
    inline void markBit(uint32_t n) { value |= valueForBit(n); }
    
    // Clears the specified bit.
    inline void clearBit(uint32_t n) { value &= ~ valueForBit(n); }
    
    // Finds the first marked bit in the set.
    // Result is undefined if all bits are unmarked.
    inline uint32_t firstMarkedBit() const { return __builtin_clz(value); }
    
    // Finds the first unmarked bit in the set.
    // Result is undefined if all bits are marked.
    inline uint32_t firstUnmarkedBit() const { return __builtin_clz(~ value); }
    
    // Finds the last marked bit in the set.
    // Result is undefined if all bits are unmarked.
    inline uint32_t lastMarkedBit() const { return 31 - __builtin_ctz(value); }
    
    // Finds the first marked bit in the set and clears it.  Returns the bit index.
    // Result is undefined if all bits are unmarked.
    inline uint32_t clearFirstMarkedBit() {
        uint32_t n = firstMarkedBit();
        clearBit(n);
        return n;
    }
    
    // Finds the first unmarked bit in the set and marks it.  Returns the bit index.
    // Result is undefined if all bits are marked.
    inline uint32_t markFirstUnmarkedBit() {
        uint32_t n = firstUnmarkedBit();
        markBit(n);
        return n;
    }
    
    // Finds the last marked bit in the set and clears it.  Returns the bit index.
    // Result is undefined if all bits are unmarked.
    inline uint32_t clearLastMarkedBit() {
        uint32_t n = lastMarkedBit();
        clearBit(n);
        return n;
    }
    
    // Gets the index of the specified bit in the set, which is the number of
    // marked bits that appear before the specified bit.
    inline uint32_t getIndexOfBit(uint32_t n) const {
        return __builtin_popcount(value & ~(0xffffffffUL >> n));
    }
    
    inline bool operator== (const BitSet32& other) const { return value == other.value; }
    inline bool operator!= (const BitSet32& other) const { return value != other.value; }
};

class VelocityTracker {
    
public:
    struct Estimator {
        static const size_t MAX_DEGREE = 4;
        
        // Estimator time base.
        int64_t time;
        
        // Polynomial coefficients describing motion in X and Y.
        float xCoeff[MAX_DEGREE + 1], yCoeff[MAX_DEGREE + 1];
        
        // Polynomial degree (number of coefficients), or zero if no information is
        // available.
        uint32_t degree;
        
        // Confidence (coefficient of determination), between 0 (no fit) and 1 (perfect fit).
        float confidence;
        
        inline void clear() {
            time = 0;
            degree = 0;
            confidence = 0;
            for (size_t i = 0; i <= MAX_DEGREE; i++) {
                xCoeff[i] = 0;
                yCoeff[i] = 0;
            }
        }
    };
    
    // Creates a velocity tracker using the specified strategy.
    // If strategy is NULL, uses the default strategy for the platform.
    VelocityTracker();
    
    ~VelocityTracker();
    
    // Resets the velocity tracker state.
    void clear();
    
    // Resets the velocity tracker state for specific pointers.
    // Call this method when some pointers have changed and may be reusing
    // an id that was assigned to a different pointer earlier.
    void clearPointers(BitSet32 idBits);
    
    // Adds movement information for a set of pointers.
    // The idBits bitfield specifies the pointer ids of the pointers whose positions
    // are included in the movement.
    // The positions array contains position information for each pointer in order by
    // increasing id.  Its size should be equal to the number of one bits in idBits.
    void addMovement(int64_t eventTime, BitSet32 idBits, cocos2d::Vec2& position);
    
    // for multi-touch
//    void addMovement(int64_t eventTime, BitSet32 idBits, const std::vector<cocos2d::Vec2>& positions);
    
    // Adds movement information for all pointers in a MotionEvent, including historical samples.
    void addMovement(MotionEvent* event);
    
    void addMovement(int action, const float x, const float y, const float eventSec);
    
    // Gets the velocity of the specified pointer id in position units per second.
    // Returns false and sets the velocity components to zero if there is
    // insufficient movement information for the pointer.
    bool getVelocity(uint32_t id, float* outVx, float* outVy) const;
    
    // Gets an estimator for the recent movements of the specified pointer id.
    // Returns false and clears the estimator if there is no information available
    // about the pointer.
    bool getEstimator(uint32_t id, Estimator* outEstimator) const;
    
    // Gets the active pointer id, or -1 if none.
    inline int32_t getActivePointerId() const { return mActivePointerId; }
    
    // Gets a bitset containing all pointer ids from the most recent movement.
    inline BitSet32 getCurrentPointerIdBits() const { return mCurrentPointerIdBits; }
    
private:
    
    int64_t mLastEventTime;
    BitSet32 mCurrentPointerIdBits;
    int32_t mActivePointerId;
    VelocityTrackerStrategy* mStrategy;
};

/*
 * Velocity tracker algorithm based on least-squares linear regression.
 */
class VelocityTrackerStrategy {
public:
    enum Weighting {
        // No weights applied.  All data points are equally reliable.
        WEIGHTING_NONE,
        
        // Weight by time delta.  Data points clustered together are weighted less.
        WEIGHTING_DELTA,
        
        // Weight such that points within a certain horizon are weighed more than those
        // outside of that horizon.
        WEIGHTING_CENTRAL,
        
        // Weight such that points older than a certain amount are weighed less.
        WEIGHTING_RECENT,
    };
    
    // Degree must be no greater than Estimator::MAX_DEGREE.
    VelocityTrackerStrategy(uint32_t degree, Weighting weighting = WEIGHTING_NONE);
    virtual ~VelocityTrackerStrategy();
    
    virtual void clear();
    virtual void clearPointers(BitSet32 idBits);
    virtual void addMovement(int64_t eventTime, BitSet32 idBits, cocos2d::Vec2& position);
    virtual bool getEstimator(uint32_t id, VelocityTracker::Estimator* outEstimator) const;
    
private:
    // Sample horizon.
    // We don't use too much history by default since we want to react to quick
    // changes in direction.
    static const int64_t HORIZON = 100 * 1000000; // 100 ms
    
    // Number of samples to keep.
    static const uint32_t HISTORY_SIZE = 20;
    
    struct Movement {
        int64_t eventTime;
        BitSet32 idBits;
        cocos2d::Vec2 positions[_MAX_TRACKING_POINTERS];
        
        inline const cocos2d::Vec2& getPosition(uint32_t id) const {
            return positions[idBits.getIndexOfBit(id)];
        }
    };
    
    float chooseWeight(uint32_t index) const;
    
    const uint32_t mDegree;
    const Weighting mWeighting;
    uint32_t mIndex;
    Movement mMovements[HISTORY_SIZE];
};


#endif /* VelocityTracker_h */
