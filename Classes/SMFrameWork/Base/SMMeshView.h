//
//  SMMeshView.hpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 14..
//

#ifndef SMMeshView_h
#define SMMeshView_h

#include "SMView.h"
#include <cocos2d.h>
#include <renderer/CCTrianglesCommand.h>
#include <renderer/CCCustomCommand.h>
#include <2d/CCAutoPolygon.h>

class SMMeshView : public SMView
{
public:
    static SMMeshView * createMeshView(float x, float y, float width, float height, float anchorX, float anchorY, float gridWidth, float gridHeight)
    {
        auto view = new (std::nothrow)SMMeshView();
        if (view) {
            view->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
            view->setPosition(cocos2d::Vec2(x, y));
            view->setContentSize(cocos2d::Size(width, height));
            view->setGridSize(cocos2d::Size(gridWidth, gridHeight));
            if (view->init()) {
                view->autorelease();
            } else {
                CC_SAFE_DELETE(view);
                view = nullptr;
            }
        }
        
        return view;
    }
    
    void setGridSize(cocos2d::Size size) {
        _gridSize = size;
    }
    
    cocos2d::Size getGridSize() {return _gridSize;}
    
    virtual bool init() override;
    
    void setCurtain(cocos2d::Vec2 point);
    
    void convertToMesh(const float meshSize=0, const bool isFlip=false);
    
    void convertToRect();
    
    bool isMesh() const {return _isMesh;}

    void grow(float px, float py, float value, float step, float radius);
    
    
//    virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
    virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
protected:
    cocos2d::CustomCommand _meshCommand;
    cocos2d::BlendFunc        _blendFunc;
    
    GLint _viewProjectionMatrixUniform;
    GLint _normalMatrixUniform;
    GLint _lightDirectionUniform;
    GLint _diffuseFactorUniform;
    GLint _drawPointUniform;
//    GLint _texSamplerUniform;


protected:
    SMMeshView();
    virtual ~SMMeshView();
    
    cocos2d::Size _gridSize;
    
private:
    bool _isMesh;
    float _meshSize;
    int _rows, _cols;
    cocos2d::TrianglesCommand::Triangles _triangles;

};


#endif /* SMMeshView_h */
