//
//  Dynamics.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef Dynamics_h
#define Dynamics_h

// Dynamics.java 자바기반 물리엔진... 줌이나 스크롤링에 사용하기 위하여 C++로 변환 함.
class Dynamics {
public:
    Dynamics();
    virtual ~Dynamics();
    
    
    void reset();
    
    void setState(const float position, const float velocity, const float nowTime);
    
    float getPosition() const;
    
//    bool isAtRest(const float velocityTolerance, const float positionTolerance) const;
    bool isAtRest(const float velocityTolerance, const float positionTolerance, const float range=1.0f) const;
    
    void setMaxPosition(const float maxPosition);
    
    void setMinPosition(const float minPosition);
    
    void update(const float now);
    
    float getDistanceToLimit() const;
    
    void setFriction(const float friction);
    
    void setSpring(float stiffness, float dampingRatio);
    
    float calculateAcceleration();
    
private:
    static const float MAX_TIMESTEP;
    
    float _position;
    
    float _velocity;
    
    float _maxPosition;
    
    float _minPosition;
    
    float _lastTime;
    
    float _friction;
    
    float _stiffness;
    
    float _damping;
};


#endif /* Dynamics_h */
