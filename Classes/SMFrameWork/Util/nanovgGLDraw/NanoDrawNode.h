//
//  NanoDrawNode.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 27..
//

#ifndef NanoDrawNode_h
#define NanoDrawNode_h

#include <cocos2d.h>
#include "../use_opencv.h"

class NanoVG;

class NanoDrawNode : public cocos2d::Node
{
public:
    static NanoDrawNode* create();
    
    const cocos2d::BlendFunc& getBlendFunc() const;
    
    void setBlendFunc(const cocos2d::BlendFunc& blendFunc);
    
    void clear();
    
    void drawPath(const std::vector<cocos2d::Vec2>& pointArray, float strokeWidth, bool close);
    
protected:
    NanoDrawNode();
    virtual ~NanoDrawNode();
    
    virtual bool init() override;

    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;

    
    
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
private:
    bool loadShaderVertex();
    
protected:
    NanoVG* _nanoCore;
    float _strokeWidth;
    
    // vertex array object
    GLuint _vao;
    // vertex buffer object
    GLuint _vbo;
    
    cocos2d::BlendFunc _blendFunc;
    cocos2d::CustomCommand _customCommand;
    
    GLint _uniformColor;
    GLint _uniformStroke;

    bool _dirty;
};



#endif /* NanoDrawNode_h */
