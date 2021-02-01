//
//  ImageAutoPolygon.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#ifndef ImageAutoPolygon_h
#define ImageAutoPolygon_h

#include <vector>
#include <platform/CCImage.h>
#include <renderer/CCTrianglesCommand.h>
#include <2d/CCAutoPolygon.h>

class ImageAutoPolygon
{
public:
    ImageAutoPolygon(cocos2d::Image* image);
    ~ImageAutoPolygon();
    
    std::vector<cocos2d::Vec2> trace(const cocos2d::Rect& rect, const float& threshold=0.0);
    std::vector<cocos2d::Vec2> reduce(const std::vector<cocos2d::Vec2>& points, const cocos2d::Rect& rect, const float& epsilon=2.0);
    std::vector<cocos2d::Vec2> expand(const std::vector<cocos2d::Vec2>& points, const cocos2d::Rect& rect, const float& epsilon);
    
    cocos2d::TrianglesCommand::Triangles triangulate(const std::vector<cocos2d::Vec2>& points);
    void calculateUV(const cocos2d::Rect& rect, cocos2d::V3F_C4B_T2F* verts, const ssize_t& count);
    cocos2d::PolygonInfo* generateTriangles(const cocos2d::Rect& rect=cocos2d::Rect::ZERO, const float& epsilong=2.0, const float& threshold=0.05);
    
protected:
    cocos2d::Vec2 findFirstNoneTransparentPixel(const cocos2d::Rect& rect, const float& threshold);
    std::vector<cocos2d::Vec2> marchSquare(const cocos2d::Rect& rect, const cocos2d::Vec2& first, const float& threshold);
    unsigned int getSquareValue(const unsigned int&x, const unsigned int& y, const cocos2d::Rect& rect, const float& threshold);
    unsigned char getAlphaByIndex(const unsigned int& i);
    unsigned char getAlphaByPos(const cocos2d::Vec2& pos);
    int getIndexFromPos(const unsigned int& x, const unsigned int& y) {return y*_width + x;}
    cocos2d::Vec2 getPosFromIndex(const unsigned int& i) {return cocos2d::Vec2(i%_width, i/_width);}
    std::vector<cocos2d::Vec2> rdp(std::vector<cocos2d::Vec2> v, const float& optimization);
    float perpendicularDistance(const cocos2d::Vec2& i, const cocos2d::Vec2& start, const cocos2d::Vec2& end);
    cocos2d::Rect getRealRect(const cocos2d::Rect& rect);

protected:
    cocos2d::Image* _image;
    unsigned char* _data;
    unsigned int _width;
    unsigned int _height;
    float _scaleFactor;;
    unsigned int _threshold;
    
};


#endif /* ImageAutoPolygon_h */
