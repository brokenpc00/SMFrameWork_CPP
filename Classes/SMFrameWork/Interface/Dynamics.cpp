//
//  Dynamics.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#include "Dynamics.h"
#include <cocos2d.h>

const float Dynamics::MAX_TIMESTEP = 1.0f / 60.0f;

Dynamics::Dynamics()
{
    reset();
}

Dynamics::~Dynamics()
{
}


void Dynamics::reset()
{
    _position = 0;
    _velocity = 0;
    _maxPosition = FLT_MAX;
    _minPosition = FLT_MIN;
    _lastTime = 0;
    _friction = 0;
    _stiffness = 0;
    _damping = 0;
}

void Dynamics::setState(const float position, const float velocity, const float nowTime)
{
    _velocity = velocity;
    _position = position;
    _lastTime = nowTime;
}

float Dynamics::getPosition() const
{
    return _position;
}

bool Dynamics::isAtRest(const float velocityTolerance, const float positionTolerance, const float range) const
{
    const bool standingStill = std::abs(_velocity) < velocityTolerance;
    
    bool withinLimits;
    if (range==1) {
        withinLimits = ((_position - positionTolerance < _maxPosition) && (_position + positionTolerance > _minPosition));
    } else {
        withinLimits = ((_position*range - positionTolerance < _maxPosition*range) && (_position*range + positionTolerance > _minPosition*range));
    }
    // 수식이 이상해서 수정함...
//    bool withinLimits = ((_position*range - positionTolerance < _maxPosition*range) && (_position*range + positionTolerance > _minPosition*range));
//    bool withinLimits = ((_position - positionTolerance < _maxPosition) && (_position + positionTolerance > _minPosition));
    
    
    // 원래는 이거임.
    //const bool withinLimits = _position - positionTolerance < _maxPosition && _position + positionTolerance > _minPosition;
    
    return standingStill && withinLimits;
}

void Dynamics::setMaxPosition(const float maxPosition)
{
    _maxPosition = maxPosition;
}

void Dynamics::setMinPosition(const float minPosition)
{
    _minPosition = minPosition;
}

void Dynamics::update(float now)
{
    float dt = now - _lastTime;
    if (dt > MAX_TIMESTEP) {
        dt = MAX_TIMESTEP;
    }
    
    // Calculate current acceleration
    float acceleration = calculateAcceleration();
    
    // Calculate next position based on current velocity and acceleration
    _position += _velocity * dt + .5f * acceleration * dt * dt;
    
    // Update velocity
    _velocity += acceleration * dt;
    
    _lastTime = now;
}

float Dynamics::getDistanceToLimit() const
{
    float distanceToLimit = 0;
    
    if (_position > _maxPosition) {
        distanceToLimit = _maxPosition - _position;
    } else if (_position < _minPosition) {
        distanceToLimit = _minPosition - _position;
    }
    
    return distanceToLimit;
}

void Dynamics::setFriction(const float friction)
{
    _friction = friction;
}

void Dynamics::setSpring(const float stiffness, const float dampingRatio)
{
    _stiffness = stiffness;
    _damping = dampingRatio * 2 * std::sqrt(stiffness);
}

float Dynamics::calculateAcceleration()
{
    float acceleration;
    float distanceFromLimit = getDistanceToLimit();
    
    if (distanceFromLimit != 0.0f) {
        acceleration = distanceFromLimit * _stiffness - _damping * _velocity;
    } else {
        acceleration = -_friction * _velocity;
    }
    
    return acceleration;
}
