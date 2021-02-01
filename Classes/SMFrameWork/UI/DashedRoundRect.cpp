//
//  DashedRoundRect.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#include "DashedRoundRect.h"
#include <math.h>
#include <cmath>
#include "../Util/SpriteUtilFn.h"

#define CONER_SEGMENT   (5)
#define NUM_QUADS       ((CONER_SEGMENT+1)*4)
#define NUM_TRIANGLES   (NUM_QUADS*2)
#define NUM_VERTICES    (NUM_QUADS*2+2)
#define NUM_INDICES     (NUM_TRIANGLES*3)

DashedRoundRect::DashedRoundRect() :
_thickness(16),
_cornerRadius(50),
_bInit(false)
{
    _triangles.verts = nullptr;
    _triangles.indices = nullptr;
}

DashedRoundRect::~DashedRoundRect()
{
    CC_SAFE_DELETE_ARRAY(_triangles.verts);
    CC_SAFE_DELETE_ARRAY(_triangles.indices);
}

bool DashedRoundRect::initMesh()
{
    _triangles.verts = new cocos2d::V3F_C4B_T2F[NUM_VERTICES];
    _triangles.indices = new unsigned short[NUM_INDICES];
    _triangles.vertCount = NUM_VERTICES;
    _triangles.indexCount = NUM_INDICES;
    
    // init texCoords
    cocos2d::V3F_C4B_T2F* vertices = _triangles.verts;
    for (int i = 0; i < NUM_VERTICES; i += 2) {
        vertices[i+0].texCoords.v = 0.0f;
        vertices[i+1].texCoords.v = 1.0f;
        vertices[i+0].texCoords.u = 0.5f;
        vertices[i+1].texCoords.u = 0.5f;
    }
    
    // init indices
    unsigned short* indices = _triangles.indices;
    int idx = 0;
    for (idx = 0; idx < (NUM_QUADS)*2; idx += 2) {
        QuadToTrianglesWindCCWSet(indices, idx+1, idx+3, idx+0, idx+2);
        indices += 6;
    }
    
    _polyInfo.setTriangles(_triangles);
    
    auto texture = getTexture();
    
    if (texture) {
        cocos2d::Texture2D::TexParams texParams;
        texParams.minFilter = GL_LINEAR;
        texParams.magFilter = GL_LINEAR;
        texParams.wrapS = GL_REPEAT;
        texParams.wrapT = GL_REPEAT;
        
        texture->setTexParameters(texParams);
        _thickness = texture->getPixelsWide();
    }
    
    return true;
}

void DashedRoundRect::setLineWidth(const float lineWidth)
{
    _thickness = lineWidth;
}

void DashedRoundRect::setCornerRadius(const float radius)
{
    _cornerRadius = radius;
}

void DashedRoundRect::setContentSize(const cocos2d::Size &size)
{
//    cocos2d::Sprite::setContentSize(size);
    cocos2d::Node::setContentSize(size);
    
    applyMesh();
}

void DashedRoundRect::applyMesh()
{
    if (!_bInit)
        return;
    
    float width = _contentSize.width;
    float height = _contentSize.height;
    //_polyInfo._rect.setRect(0, 0, width, height);
    _polyInfo.setRect(cocos2d::Rect(0, 0, width, height));
    
    float conerRadius = std::min(_cornerRadius, std::min(width/2, height/2));
    
    float inR = conerRadius - _thickness/2.0;
    float outR = conerRadius + _thickness/2.0;
    float w = width-conerRadius;
    float h = height-conerRadius;
    float textureRoundLength = (float)(0.25*2*conerRadius * M_PI)/_thickness;
    float textureWidthLength = (width-2*conerRadius) / _thickness;
    float textureHeightLength = (height-2*conerRadius) / _thickness;
    float stepRoundLength = textureRoundLength / CONER_SEGMENT;
    
    int index = 0;
    float tu = 0;
    
    cocos2d::V3F_C4B_T2F* vertices = _triangles.verts;
    for (int i = 0; i <= CONER_SEGMENT; i++) {
        double rad = i * M_PI * 0.5/CONER_SEGMENT;
        float ca = (float)std::cos(rad);
        float sa = (float)std::sin(rad);
        
        float inA = inR*ca;
        float inB = inR*sa;
        float outA = outR*ca;
        float outB = outR*sa;
        
        // left-bottom
        index = i*2;
        vertices[index+0].vertices.x = conerRadius-inA;
        vertices[index+0].vertices.y = conerRadius-inB;
        vertices[index+1].vertices.x = conerRadius-outA;
        vertices[index+1].vertices.y = conerRadius-outB;
        tu = i*stepRoundLength;
        vertices[index+0].texCoords.u = vertices[index+1].texCoords.u = tu;
        
        // right-bottom
        index += (CONER_SEGMENT+1)*2;
        vertices[index+0].vertices.x = w+inB;
        vertices[index+0].vertices.y = conerRadius-inA;
        vertices[index+1].vertices.x = w+outB;
        vertices[index+1].vertices.y = conerRadius-outA;
        tu += textureWidthLength+textureRoundLength;
        vertices[index+0].texCoords.u = vertices[index+1].texCoords.u = tu;
        
        // right-top
        index += (CONER_SEGMENT+1)*2;
        vertices[index+0].vertices.x = w+inA;
        vertices[index+0].vertices.y = h+inB;
        vertices[index+1].vertices.x = w+outA;
        vertices[index+1].vertices.y = h+outB;
        tu += textureHeightLength+textureRoundLength;
        vertices[index+0].texCoords.u = vertices[index+1].texCoords.u = tu;
        
        // left-top
        index += (CONER_SEGMENT+1)*2;
        vertices[index+0].vertices.x = conerRadius-inB;
        vertices[index+0].vertices.y = h+inA;
        vertices[index+1].vertices.x = conerRadius-outB;
        vertices[index+1].vertices.y = h+outA;
        tu += textureWidthLength+textureRoundLength;
        vertices[index+0].texCoords.u = vertices[index+1].texCoords.u = tu;
    }
    
    // last : left-top -> left-bottom
    vertices[index+2].vertices.x = vertices[0].vertices.x;
    vertices[index+2].vertices.y = vertices[0].vertices.y;
    vertices[index+3].vertices.x = vertices[1].vertices.x;
    vertices[index+3].vertices.y = vertices[1].vertices.y;
    tu += textureHeightLength;
    vertices[index+2].texCoords.u = vertices[index+3].texCoords.u = tu;

}
