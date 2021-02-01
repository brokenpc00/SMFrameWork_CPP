//
//  SMPath.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 3..
//

#include "SMPath.h"
#include <cmath>

SMPath::SMPath() : _length(0)
{
    _vertices.clear();
}

SMPath::~SMPath()
{
    
}

void SMPath::addBack(const cocos2d::Vec2 &point)
{
    if (size()) {
        if (point == _vertices[size()-1]) {
            
            return;
        }
        
        _length += point.distance(_vertices[size()-1]);
    }
    
    _vertices.push_back(point);
}

void SMPath::addFront(const cocos2d::Vec2 &point)
{
    if (size()) {
        if (point == _vertices[0]) {
            
            return;
        }
        
        _length += point.distance(_vertices[0]);
    }
    
    _vertices.insert(_vertices.begin(), point);
}

const cocos2d::Vec2& SMPath::getVertex(int index) {
    if (size() == 0 || index < 0 || index >= size()) {
        return cocos2d::Vec2::ZERO;
    }
    
    return _vertices[index];
}

void SMPath::postProcess()
{
    _length = calculateLength(_vertices);
}

void SMPath::setLine(const cocos2d::Vec2 &origin, const cocos2d::Vec2 &destination)
{
    _vertices.resize(2);
    
    _vertices[0] = origin;
    _vertices[1] = destination;
    
    postProcess();
}

void SMPath::setRect(const cocos2d::Vec2 &origin, const cocos2d::Vec2 &destination)
{
    _vertices.resize(5);
    
    _vertices[0] = origin;
    _vertices[2] = cocos2d::Vec2(destination.x, origin.y);
    _vertices[3] = destination;
    _vertices[4] = cocos2d::Vec2(origin.x, destination.y);
    // close shape
    _vertices[5] = origin;
    
    postProcess();
}

void SMPath::setCircle(const cocos2d::Vec2 &center, float radius, float angle, int segments)
{
    const float coef = 2.0f * (float)M_PI/segments;
    
    _vertices.resize(segments+1);
    
    for (int i = 0;i <= segments; i++) {
        float rads = i*coef;
        GLfloat j = radius * cosf(rads + angle) + center.x;
        GLfloat k = radius * sinf(rads + angle) + center.y;
        
        _vertices[i].x = j;
        _vertices[i].y = k;
    }
    
    postProcess();
}

void SMPath::setQuadBezier(const cocos2d::Vec2 &origin, const cocos2d::Vec2 &control, const cocos2d::Vec2 &destination, int segments)
{
    _vertices.resize(segments + 1);
    
    float t = 0.0f;
    for (int i = 0; i < segments; i++) {
        _vertices[i].x = powf(1 - t, 2) * origin.x + 2.0f * (1 - t) * t * control.x + t * t * destination.x;
        _vertices[i].y = powf(1 - t, 2) * origin.y + 2.0f * (1 - t) * t * control.y + t * t * destination.y;
        t += 1.0f / segments;
    }
    _vertices[segments].x = destination.x;
    _vertices[segments].y = destination.y;
    
    postProcess();
}

void SMPath::setCubicBezier(const cocos2d::Vec2 &origin, const cocos2d::Vec2 &control1, const cocos2d::Vec2 &control2, const cocos2d::Vec2 &destination, int segments)
{
    _vertices.resize(segments + 1);
    
    float t = 0;
    for (int i = 0; i < segments; i++) {
        _vertices[i].x = powf(1 - t, 3) * origin.x + 3.0f * powf(1 - t, 2) * t * control1.x + 3.0f * (1 - t) * t * t * control2.x + t * t * t * destination.x;
        _vertices[i].y = powf(1 - t, 3) * origin.y + 3.0f * powf(1 - t, 2) * t * control1.y + 3.0f * (1 - t) * t * t * control2.y + t * t * t * destination.y;
        t += 1.0f / segments;
    }
    _vertices[segments].x = destination.x;
    _vertices[segments].y = destination.y;
    
    postProcess();
}

float SMPath::calculateLength(const std::vector<cocos2d::Vec2> &vertices )
{
    int numVertices = (int)vertices.size();
    if (numVertices < 2) {
        return 0;
    }
    
    float length = 0;
    
    for (int i = 1; i < numVertices; i++) {
        length += vertices[i-1].distance(vertices[i]);
    }
    
    return length;
    
}

const float SMPath::getLastAngle()
{
    if (size() < 2) {
        return 0;
    }
    
    auto& p0 = _vertices[size()-2];
    auto& p1 = _vertices[size()-1];
    
    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    
    return std::atan2(dy, dx);
}

SMPath SMPath::getSubPath(float t)
{
    if (size() == 0) {
        return SMPath();
    }
    
    if (t <= 0 || size() == 1) {
        SMPath path;
        path.addBack(_vertices[0]);
        return path;
    }
    
    float targetLength = _length * t;
    float l = 0;
    
    SMPath newPath;
    newPath.addBack(_vertices[0]);
    
    for (int i = 1; i < size(); i++) {
        auto& p0 = _vertices[i-1];
        auto& p1 = _vertices[i];
        float d = p0.distance(p1);
        
        if (l + d > targetLength) {
            float f = 1.0 - (l + d - targetLength) / d;
            float xx = p0.x + f * (p1.x - p0.x);
            float yy = p0.y + f * (p1.y - p0.y);
            
            newPath.addBack(cocos2d::Vec2(xx, yy));
            return newPath;
        }
        
        l += d;
        newPath.addBack(p1);
    }
    
    return newPath;
}
