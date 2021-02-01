//
//  MeshSprite.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 2..
//
// mesh sprite

#ifndef MeshSprite_h
#define MeshSprite_h

#include <2d/CCSprite.h>

class MeshSprite : public cocos2d::Sprite
{
public:
    static MeshSprite* create(cocos2d::Sprite* src);
    
    virtual bool initWithSprite(cocos2d::Sprite* sprite);
    
    void convertToMesh(const float meshSize=0, const bool isFlip=false);
    
    void convertToRect();
    
    bool isMesh() const {return _isMesh;}
    
    cocos2d::TrianglesCommand::Triangles& getTriangles() {return _triangles;}
    
    // fish eye
    // http://www.codeproject.com/Articles/182242/Transforming-Images-for-Fun-A-Local-Grid-based-Ima
    void grow(float px, float py, float value, float step, float radius);
    
//    void curtain(cocos2d::Vec2 point);
    
protected:
    MeshSprite();
    virtual ~MeshSprite();

private:
    bool _isMesh;
    float _meshSize;
    int _rows, _cols;
    cocos2d::TrianglesCommand::Triangles _triangles;
    bool _bSetCurtainFlag;
};


#endif /* MeshSprite_h */
