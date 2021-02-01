//
//  MeshSprite.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 2..
//
// Mesh Sprite

#include "MeshSprite.h"
#include <math.h>
#include <cmath>
#include "../Util/SpriteUtilFn.h"

#define DEFAULT_MESH_SIZE 20

MeshSprite * MeshSprite::create(cocos2d::Sprite *src)
{
    if (src==nullptr || src->getTexture()==nullptr) {
        return nullptr;
    }
    
    auto sprite = new (std::nothrow)MeshSprite();
    if (sprite && sprite->initWithSprite(src)) {
        sprite->autorelease();
    } else {
        CC_SAFE_DELETE(sprite);
    }
    
    return sprite;
}

MeshSprite::MeshSprite() :
_isMesh(false)
{
    _triangles.verts = nullptr;
    _triangles.indices = nullptr;
}

MeshSprite::~MeshSprite()
{
    CC_SAFE_DELETE_ARRAY(_triangles.verts);
    CC_SAFE_DELETE_ARRAY(_triangles.indices);
}

bool MeshSprite::initWithSprite(cocos2d::Sprite *sprite)
{
    if (sprite==nullptr) {
        return false;
    }
    
    auto texture = sprite->getTexture();
    if (texture==nullptr) {
        return false;
    }
    
    if (!initWithTexture(texture, sprite->getTextureRect(), sprite->isTextureRectRotated())) {
        return false;
    }
    
    return true;
}

void MeshSprite::convertToRect()
{
    // mesh sprite를 일반 sprite로 변환
    if (!_isMesh) {
        // mesh sprite가 아니면 바로 리턴
        return;
    }
    
    // 기존에 설정된 size 및 origin을 무시하고 실제 texture의 사이즈를 설정
    // 실제 Pixel단위 사이즈
    cocos2d::Rect rect(cocos2d::Vec2::ZERO, getTexture()->getContentSizeInPixels());
    setTextureRect(rect);
    
    CC_SAFE_DELETE_ARRAY(_triangles.verts);
    CC_SAFE_DELETE_ARRAY(_triangles.indices);
    _triangles.verts = nullptr;
    _triangles.indices = nullptr;
    
    // 이제 일반 sprite가 되었다.
    _isMesh = false;
}

void MeshSprite::convertToMesh(const float meshSize, const bool flipTexure)
{
    // 일반 sprite를 mesh sprite로 변환
    if (_isMesh) {
        // 이미 mesho sorite면 바로 리턴
        return;
    }
    
    auto size = getTextureRect().size;
    auto origin = getTextureRect().origin;
    
    if (meshSize<=9) {
        // 기본 값또는 잘못된 값이면
        // 대충 긴쪽을 기준으로 default만큼 나눈수.
        if (size.width>size.height) {
            _meshSize = size.width/DEFAULT_MESH_SIZE;
        } else {
            _meshSize = size.height/DEFAULT_MESH_SIZE;
        }
        if (_meshSize<10) {
            _meshSize = 10;
        }
    } else {
        // 정상 값이면 그걸로 하자.
        _meshSize = meshSize;
    }
    
    int cols = (int)std::ceil(size.width/_meshSize);
    int rows = (int)std::ceil(size.height/_meshSize);
    
    int numVertices = (cols+1) * (rows+1);
    int numQuads = cols*rows;
    int numFaces = numQuads * 2;
    int numIndices = numFaces * 3;
    
    // vetext와 texture buffer 생성
    _triangles.verts = new cocos2d::V3F_C4B_T2F[numVertices];
    _triangles.indices = new unsigned short[numIndices];
    _triangles.vertCount = numVertices;
    _triangles.indexCount = numIndices;
    
    // index buffer 설정
    unsigned short* indices = _triangles.indices;
    unsigned short ll, lr, ul, ur;
    for (int index=0; index<numQuads; index++) {
        int rowNum = index/cols;
        int colNum = index % cols;
        ll = (short)(rowNum * (cols+1) + colNum);
        lr = (short)(ll + 1);
        ul = (short)((rowNum + 1) * (cols+1) + colNum);
        ur = (short)(ul + 1);
        QuadToTrianglesWindCCWSet(indices, ul, ur, ll, lr);
        indices += 6;
    }
    
    // vertex buffer 설정
    cocos2d::V3F_C4B_T2F* vertices = _triangles.verts;
    float xx, yy, uu, vv;
    
    if (flipTexure) {
        for (int y = 0; y <= rows; y++) {
            if (y == rows) {
                yy = size.height;
                vv = 1.0;
            } else {
                yy = _meshSize * y;
                vv = (float)y * _meshSize / size.height;
            }
            for (int x = 0; x <= cols; x++) {
                if (x == cols) {
                    xx = size.width;
                    uu = 1.0;//_w/tw;
                } else {
                    xx = _meshSize * x;
                    uu = (float)x * _meshSize / size.width;
                }
                vertices->vertices.x = xx; // x coord
                vertices->vertices.y = yy; // y coord
                vertices->texCoords.u = uu;
                vertices->texCoords.v = vv;
                vertices->colors = cocos2d::Color4B::WHITE;
                
                vertices++;
            }
        }
    } else {
        for (int y = 0; y <= rows; y++) {
            if (y == rows) {
                yy = size.height;
                vv = 0.0;
            } else {
                yy = _meshSize * y;
                vv = 1.0-(float)y * _meshSize / size.height;
            }
            for (int x = 0; x <= cols; x++) {
                if (x == cols) {
                    xx = size.width;
                    uu = 1.0;//_w/tw;
                } else {
                    xx = _meshSize * x;
                    uu = (float)x * _meshSize / size.width;
                }
                vertices->vertices.x = xx; // x coord
                vertices->vertices.y = yy; // y coord
                vertices->texCoords.u = uu;
                vertices->texCoords.v = vv;
                vertices->colors = cocos2d::Color4B::WHITE;
                
                vertices++;
            }
        }
    }
    
    _polyInfo.setTriangles(_triangles);
    
    _rows = rows;
    _cols = cols;
    // 이제 이 sprite는 mesh sprite이다.
    _isMesh = true;
}

void MeshSprite::grow(float px, float py, float value, float step, float radius)
{
    if (!isMesh()) {
        return;
    }
    // fish eye에 썼던 grow method
    float sx, sy, dx, dy, r;
    float growStep = std::abs(value*step);
    
    auto size = getTextureRect().size;
    
    cocos2d::V3F_C4B_T2F* vertices = _triangles.verts;
    for (int y = 0; y <= _rows; y++) {
        if (y == _rows) {
            sy = size.height;
        } else {
            sy = _meshSize * y;
        }
        for (int x = 0; x <= _cols; x++) {
            if (x == _cols) {
                sx = size.width;
            } else {
                sx = _meshSize * x;
            }
            
            dx = sx - px;
            dy = sy - py;
            r = (float)std::sqrt(dx*dx+dy*dy)/radius;
            r = (float)std::pow(r, growStep);
            
            if (value > 0 && r > .001) {
                vertices->vertices.x = px + dx/r;
                vertices->vertices.y = py + dy/r;
            } else if (value < 0 && r > .001) {
                vertices->vertices.x = px + dx*(r);
                vertices->vertices.y = py + dy*(r);
            } else {
                vertices->vertices.x = sx;
                vertices->vertices.y = sy;
            }
            vertices++;
        }
    }
}

//struct SMMeshVertex {
//    cocos2d::Vec2 from;
//    cocos2d::Vec3 to;
//};
//
//struct SMVertex {
//    cocos2d::Vec3 position;
//    cocos2d::Vec3 normal;
//    cocos2d::Vec2 uv;
//};
//
//struct SMMeshFace {
//    unsigned int indices[4];
//};
//
//void MeshSprite::curtain(cocos2d::Vec2 point)
//{
//    if (!isMesh()) {
//        return;
//    }
//    
//    return;
//    
//    cocos2d::Size boundsSize = getTextureRect().size;
//    const float Frills = 3;
//    
//    point.x = 1079;
//    
//    point.x = MIN(point.x, boundsSize.width);
//
//    cocos2d::Vec2 np = cocos2d::Vec2(point.x/boundsSize.width, point.y/boundsSize.height);
//    
//    const int IndexesPerFace = 4;
//    int vertexCount = _triangles.vertCount;
//    int indexCount = _triangles.indexCount;
//    int faceCount = indexCount/IndexesPerFace;
//
//    
//    cocos2d::V3F_C4B_T2F* vertices = _triangles.verts;
//    SMMeshVertex vert[vertexCount];
//    for (int i=0; i<vertexCount; i++) {
//        vert[i].to = vertices->vertices;
//        vert[1].from = cocos2d::Vec2(vert[i].to.x, vert[i].to.y);
//        vertices++;
//    }
//    
////    unsigned short *indexData = new unsigned short[indexCount];
//    
//    unsigned short* indices = _triangles.indices;
//    SMMeshFace face[faceCount];
//    for (int i=0; i<faceCount; i++) {
//        for (int j=0; j<4; j++) {
//            face[i].indices[j] = indices[j];
//        }
//        
//        indices++;
//    }
//    
//    for (int i = 0; i < _triangles.vertCount; i++) {
//        SMMeshVertex vertex = vert[i];
//        
//        float dy = vertex.to.y - np.y;
//        float bend = 0.25f * (1.0f - expf(-dy * dy * 10.0f));
//        
//        float x = vertex.to.x;
//        
//        vertex.to.z = 0.1 + 0.1f * sin(-1.4f * cos(x * x * Frills * 2.0 * M_PI)) * (1.0 - np.x);
//        vertex.to.x = (vertex.to.x) * np.x + vertex.to.x * bend * (1.0 - np.x);
//        vert[i] = vertex;
//    }
//    
//
//    vertices = _triangles.verts;
//    
//    for (int i = 0; i < _triangles.vertCount; i++) {
//        SMMeshVertex meshVertex = vert[i];
//        cocos2d::Vec2 uv = meshVertex.from;
//        
//        vertices->vertices = cocos2d::Vec3(meshVertex.to.x, meshVertex.to.y, meshVertex.to.z);
//        vertices->texCoords.u = uv.x;
//        vertices->texCoords.v = 1.0 - uv.y;
//
//        vertices++;
//    }
//    
//
//    indices = _triangles.indices;
//    vertices = _triangles.verts;
//    
//    for (int i = 0; i < faceCount; i++) {
//        SMMeshFace ff = face[i];
//        cocos2d::Vec3 weightedFaceNormal = cocos2d::Vec3(0.0f, 0.0f, 0.0f);
//        const int Winding[2][3] = {
//            {0, 1, 2},
//            {2, 3, 0}
//        };
//
//        cocos2d::Vec3 vertic[4];
//        for (int j = 0; j < 4; j++) {
//            unsigned int faceIndex = ff.indices[j];
//            if (faceIndex >= vertexCount) {
//                CCLOG("Vertex index %u in face %d is out of bounds!", faceIndex, i);
//                return;
//            }
//
//            cocos2d::Vec3 a = vertices[faceIndex].vertices;
//
//            float xScale = boundsSize.width;
//            float yScale = boundsSize.height;
//            float zScale = 0.5*(boundsSize.width+boundsSize.height);
//            cocos2d::Vec3 b = cocos2d::Vec3(xScale, yScale, zScale);
//
//            vertic[j] = cocos2d::Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
//        }
//        
//        for (int triangle = 0; triangle < 2; triangle++) {
//            
//            int aIndex = ff.indices[Winding[triangle][0]];
//            int bIndex = ff.indices[Winding[triangle][1]];
//            int cIndex = ff.indices[Winding[triangle][2]];
//            
//            indices[IndexesPerFace * i + triangle * 3 + 0] = aIndex;
//            indices[IndexesPerFace * i + triangle * 3 + 1] = bIndex;
//            indices[IndexesPerFace * i + triangle * 3 + 2] = cIndex;
//            
//            cocos2d::Vec3 a = vertic[Winding[triangle][0]];
//            cocos2d::Vec3 b = vertic[Winding[triangle][1]];
//            cocos2d::Vec3 c = vertic[Winding[triangle][2]];
//            
//            cocos2d::Vec3 ab = a;
//            ab.subtract(b);
//            cocos2d::Vec3 cb = c;
//            cb.subtract(b);
//            
//            
//            cocos2d::Vec3 weightedNormal = ab;
//            weightedNormal.cross(cb);
//            
//            weightedFaceNormal.add(weightedNormal);
//        }
//
//        
////        // accumulate weighted normal over all faces
////
//        for (int i = 0; i < 4; i++) {
//            int vertexIndex = ff.indices[i];
//            cocos2d::Vec3 a = cocos2d::Vec3(vertices[vertexIndex].colors.r/0xff, vertices[vertexIndex].colors.g/0xff, vertices[vertexIndex].colors.b/0xff);
//            a.add(weightedFaceNormal);
//            vertices[vertexIndex].colors = cocos2d::Color4B(a.x*0xff, a.y*0xff, a.z*0xff, vertices[vertexIndex].colors.a);
//        }
//
//    }
//    
//}
