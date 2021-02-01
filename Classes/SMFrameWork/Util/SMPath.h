//
//  SMPath.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 3..
//

#ifndef SMPath_h
#define SMPath_h

// Bezier 따라 만든거

#include <cocos2d.h>
#include <vector>

class SMPath
{
public:
    SMPath();
    virtual ~SMPath();
    
    inline int size() {return (int)_vertices.size();}
    
    inline void reset() {_vertices.clear(); _length=0;}
    
    inline float getLength() {return _length;}
    
    void addBack(const cocos2d::Vec2& pt);

    void addFront(const cocos2d::Vec2& pt);
    
    const cocos2d::Vec2& getVertex(int index);
    
    const std::vector<cocos2d::Vec2>& getVertices() {return _vertices;}
    
    const cocos2d::Vec2& getLastPosition() { return getVertex(size()-1); }
    
    const float getLastAngle();
    
    SMPath getSubPath(float t);
    
    void setLine(const cocos2d::Vec2& origin, const cocos2d::Vec2& destination);

    void setRect(const cocos2d::Vec2& origin, const cocos2d::Vec2& destination);
    
    void setCircle(const cocos2d::Vec2& center, float radius, float angle, int segments);
    
    void setQuadBezier(const cocos2d::Vec2& origin, const cocos2d::Vec2& control, const cocos2d::Vec2& destination, int segments);
    
    void setCubicBezier(const cocos2d::Vec2& origin, const cocos2d::Vec2& control1, const cocos2d::Vec2& control2, const cocos2d::Vec2& destination, int segments);
    
    
protected:
    void postProcess();
    
    float calculateLength(const std::vector<cocos2d::Vec2>& vertices);
    
private:
    std::vector<cocos2d::Vec2> _vertices;
    
    float _length;
};


#endif /* SMPath_h */
