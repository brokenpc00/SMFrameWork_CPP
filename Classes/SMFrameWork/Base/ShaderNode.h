//
//  ShaderNode.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef ShaderNode_h
#define ShaderNode_h

#include <string>
#include <cocos2d.h>

class BlendFunc;

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Abstract Shader Base Class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShaderNode : public cocos2d::Node {
    
public:
    enum Quadrant {
        ALL              = 0,
        LEFT_HALF,      // 1
        RIGHT_HALF,     // 2
        TOP_HALF,       // 3
        BOTTOM_HALF,    // 4
        LEFT_TOP,       // 5
        LEFT_BOTTOM,    // 6
        RIGHT_TOP,      // 7
        RIGHT_BOTTOM    // 8
    };
    
    static float DEFAULT_ANTI_ALIAS_WIDTH;
    
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void setColor4F(const cocos2d::Color4F& color);
    virtual cocos2d::Color4F getColor4F(void);
    
    
protected:
    ShaderNode();
    virtual ~ShaderNode() = 0;
    
    virtual bool init() override = 0 ;
    virtual bool initWithShaderKey(const int shaderKey);
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState);
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags);
    
private:
    bool loadShaderVertex(const std::string &cacheKey, const std::string &vert, const std::string &frag);
    bool setupShader(const int shaderKey);
    
    int _shaderKey;
    cocos2d::CustomCommand _customCommand;
    
protected:
    GLint _uniformColor;
    cocos2d::BlendFunc _blendFunc;
    int _quadrant;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Abstract Shape Node
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeNode : public ShaderNode {
    
public:
    virtual void setCornerRadius(float cornerRadius);
    virtual void setAntiAliasWidth(float aaWidth);
    virtual void setLineWidth(float lineWidth);
    virtual void setCornerQuadrant(Quadrant quadrant);
    
    const float getCornerRadius()   { return _cornerRadius; }
    const float getAntiAliasWidth() { return _aaWidth; }
    const float getLineWidth()      { return _lineWidth; }
    
protected:
    ShapeNode();
    virtual ~ShapeNode() = 0;
    
    static cocos2d::Vec2 getQuadDimen(int quadrant, const cocos2d::Size& size);
    
    
protected:
    float _cornerRadius;
    float _lineWidth;
    float _aaWidth;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeSolidRect : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeSolidRect);
    
protected:
    ShapeSolidRect();
    virtual ~ShapeSolidRect();
    
    virtual bool init() override;
    
private:
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Gaussian Blur Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeGaussianBlurRect : public ShapeNode {
public:
    CREATE_FUNC(ShapeGaussianBlurRect);
    
protected:
    ShapeGaussianBlurRect();
    virtual ~ShapeGaussianBlurRect();
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;

private:
//    GLint _uniformTexelWidth;
//    GLint _uniformTexelHeight;;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Vignette Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeVignetteRect : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeVignetteRect);
    
    void setCenterPoint(float centerX, float centerY);
    
    void setStrength(float strength);
    
protected:
    ShapeVignetteRect();
    virtual ~ShapeVignetteRect();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformAspect;
    GLint _uniformCenter;
    GLint _uniformStrength;
    
    float _centerX;
    float _centerY;
    float _strength;
};


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Gradient Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeGradientRect : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeGradientRect);
    
protected:
    ShapeGradientRect();
    virtual ~ShapeGradientRect();
    
    virtual bool init() override;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeRect : public ShapeSolidRect {
    
public:
    CREATE_FUNC(ShapeRect);
    
protected:
    ShapeRect();
    virtual ~ShapeRect();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformDimension;
    GLint _uniformLineWidth;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Circle
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeSolidCircle : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeSolidCircle);
    
    void setCircleAnchorPoint(const cocos2d::Vec2& circleAnchor);
    
protected:
    ShapeSolidCircle();
    virtual ~ShapeSolidCircle();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformRadius;
    GLint _uniformAAWidth;
    GLint _uniformCircleAnchor;
    
    cocos2d::Vec2 _circleAnchor;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Circle Hole
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeCircleHole : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeCircleHole);
    
    void setCenterPoint(float centerX, float centerY);
    
    void setRadius(float radius);
    
protected:
    ShapeCircleHole();
    virtual ~ShapeCircleHole();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformRadius;
    GLint _uniformAAWidth;
    GLint _uniformCenter;
    GLint _uniformDimension;
    
    float _centerX;
    float _centerY;
    float _radius;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Circle (Ring)
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeCircle : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeCircle);
    
protected:
    ShapeCircle();
    virtual ~ShapeCircle();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformRadius;
    GLint _uniformAAWidth;
    GLint _uniformLineWidth;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Arc Ring
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeArcRing : public ShapeCircle {
    
public:
    CREATE_FUNC(ShapeArcRing);
    
    void setArcRatio(double ratio);
    
    void draw(const float startAngle, const float sweepAngle);
    
protected:
    ShapeArcRing();
    virtual ~ShapeArcRing();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformTheta;
    
    float _theta;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Rounded Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeSolidRoundRect : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeSolidRoundRect);
    
protected:
    ShapeSolidRoundRect();
    virtual ~ShapeSolidRoundRect();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformDimension;
    GLint _uniformCornerRadius;
    GLint _uniformAAWidth;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Rounded Rect
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeRoundedRect : public ShapeSolidRoundRect {
    
public:
    CREATE_FUNC(ShapeRoundedRect);
    
protected:
    ShapeRoundedRect();
    virtual ~ShapeRoundedRect();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformLineWidth;
    
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Solid Triangle
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeSolidTriangle : public ShapeNode {
    
public:
    CREATE_FUNC(ShapeSolidTriangle);
    
    void setTriangle(cocos2d::Vec2 p0, cocos2d::Vec2 p1, cocos2d::Vec2 p2);
    
protected:
    ShapeSolidTriangle();
    virtual ~ShapeSolidTriangle();
    
    virtual bool init() override;
    virtual void onInitShaderParams(cocos2d::GLProgramState* programState) override;
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    GLint _uniformP0;
    GLint _uniformP1;
    GLint _uniformP2;
    
    cocos2d::Vec2 _p0, _p1, _p2;
    
    bool _dirty;
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Shape Rounded Line
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
class ShapeRoundLine : public ShapeSolidRoundRect {
    
public:
    CREATE_FUNC(ShapeRoundLine);
    
    void line(float x1, float y1, float x2, float y2);
    
    void line(cocos2d::Vec2 from, cocos2d::Vec2 to);
    
    void moveTo(float x, float y);
    
    void moveBy(float dx, float dy);
    
    void setLengthScale(float lineScale);
    
    virtual void setLineWidth(float lineWidth) override;
    
    cocos2d::Vec2 getFromPosition() { return cocos2d::Vec2(_x1, _y1); }
    
    cocos2d::Vec2 getToPosition() { return cocos2d::Vec2(_x2, _y2); }
    
protected:
    ShapeRoundLine();
    virtual ~ShapeRoundLine();
    
    virtual void onDraw(const cocos2d::Mat4& transform, uint32_t flags) override;
    
private:
    
    void updateLineShape();
    
    GLint _uniformDimension;
    GLint _uniformCornerRadius;
    GLint _uniformAAWidth;
    
    float _x1, _x2, _y1, _y2, _lineScale;
    bool _dirty;
    
    // hide
    virtual void setCornerRadius(float cornerRadius) override;
};



#endif /* ShaderNode_h */
