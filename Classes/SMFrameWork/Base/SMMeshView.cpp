//
//  SMMeshView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 9. 14..
//

#include "SMMeshView.h"
#include "../Util/SpriteUtilFn.h"

#define DEFAULT_MESH_SIZE 20



SMMeshView::SMMeshView() : _isMesh(false)
{
    _triangles.verts = nullptr;
    _triangles.indices = nullptr;

    _blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
}

SMMeshView::~SMMeshView()
{
    
}

bool SMMeshView::init()
{
    if (!SMView::init()) {
        return false;
    }
    

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    std::string shaderName = "mobileShader/mesh.fsh";
#else
    std::string shaderName = "shader/mesh.fsh";
#endif
    
    auto program = cocos2d::GLProgramCache::getInstance()->getGLProgram(shaderName);

    if (program == nullptr) {
        auto fileUtils = cocos2d::FileUtils::getInstance();
        
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        auto vertexFilePath = fileUtils->fullPathForFilename("mobileShader/mesh.vsh");
#else
        auto vertexFilePath = fileUtils->fullPathForFilename("shader/mesh.vsh");
#endif
        auto vertexSource = fileUtils->getStringFromFile(vertexFilePath);
        
        auto fragmentShaderName = fileUtils->fullPathForFilename(shaderName);
        auto fragmentSource =fileUtils->getStringFromFile(fragmentShaderName);
        
        program = cocos2d::GLProgram::createWithByteArrays(vertexSource.c_str(), fragmentSource.c_str());
        cocos2d::GLProgramCache::getInstance()->addGLProgram(program, shaderName);
    }


    auto programState = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
    setGLProgramState(programState);

    _viewProjectionMatrixUniform = program->getUniformLocation("viewProjectionMatrix");
//    _normalMatrixUniform = program->getUniformLocation("normalMatrix");
    _lightDirectionUniform = program->getUniformLocation("lightDirection");
    _diffuseFactorUniform = program->getUniformLocation("diffuseFactor");
//    _texSamplerUniform = program->getUniformLocation("texSampler");

    
    return true;
}


void SMMeshView::draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags)
{
    SMView::draw(renderer, transform, flags);

    _meshCommand.init(_globalZOrder, transform, flags);
    _meshCommand.func = CC_CALLBACK_0(SMMeshView::onDraw, this, transform, flags);
    renderer->addCommand(&_meshCommand);
    
}

void SMMeshView::onDraw(const cocos2d::Mat4 &transform, uint32_t flags)
{
    CCLOG("[[[[[ mesh view on draw");
    
    cocos2d::Mat4 viewProjectionMatrix = transform;
    const float ZFlattenScale = 0.0005f;
    viewProjectionMatrix.scale(1.0f, 1.0f, ZFlattenScale, &viewProjectionMatrix);
    
    cocos2d::Mat4 mvInverse = transform;
    mvInverse.m[12] = mvInverse.m[13] = mvInverse.m[14] = 0.0f;
    mvInverse.inverse();
    mvInverse.transpose();
    
//    GLfloat normalMat[9];
//    normalMat[0] = mvInverse.m[0];normalMat[1] = mvInverse.m[1];normalMat[2] = mvInverse.m[2];
//    normalMat[3] = mvInverse.m[4];normalMat[4] = mvInverse.m[5];normalMat[5] = mvInverse.m[6];
//    normalMat[6] = mvInverse.m[8];normalMat[7] = mvInverse.m[9];normalMat[8] = mvInverse.m[10];

    cocos2d::Mat4 normatrix = mvInverse;
    normatrix.m[0] = mvInverse.m[0]; normatrix.m[1] = mvInverse.m[1]; normatrix.m[2] = mvInverse.m[2];
    normatrix.m[3] = mvInverse.m[4]; normatrix.m[4] = mvInverse.m[5]; normatrix.m[5] = mvInverse.m[6];
    normatrix.m[6] = mvInverse.m[8]; normatrix.m[7] = mvInverse.m[9]; normatrix.m[8] = mvInverse.m[10];
    
    
    auto state = getGLProgramState();
    state->setUniformVec3(_lightDirectionUniform, cocos2d::Vec3(0.0, 0.0, 1.0f));
    state->setUniformFloat(_diffuseFactorUniform, 1.0f);
    state->setUniformMat4(_viewProjectionMatrixUniform, viewProjectionMatrix);
  
    
    
}

void SMMeshView::setCurtain(cocos2d::Vec2 point)
{
    
    setGridSize(cocos2d::Size(20, 50));
    
    convertToMesh(15, false);
 
    if (isMesh()) {
        const float Frills = 3;
        cocos2d::Size boundsSize = getContentSize();
        point.x = MIN(point.x, boundsSize.width);
        
        cocos2d::Vec2 np = cocos2d::Vec2(point.x/boundsSize.width, point.y/boundsSize.height);
        auto verts = _triangles.verts;
        for (int i=0; i<_triangles.vertCount; i++) {

            auto vertex = verts->vertices;
            float dy = vertex.y - np.y;
            float bend = 0.25f * (1.0f - expf(-dy*dy*10.0f));
            
            float x = vertex.x;
            
            vertex.z = 0.1 + 0.1f * sin(-1.4f * cos(x * x * Frills * 2.0 * M_PI)) * (1.0 - np.x);
            vertex.x = (vertex.x) * np.x + vertex.x * bend * (1.0 - np.x);
            
            verts->vertices = vertex;
            
            verts++;
        }
        
        
    }
    
    
}


void SMMeshView::convertToRect()
{
    // mesh sprite를 일반 sprite로 변환
    if (!_isMesh) {
        // mesh sprite가 아니면 바로 리턴
        return;
    }
    
    // 기존에 설정된 size 및 origin을 무시하고 실제 texture의 사이즈를 설정
    // 실제 Pixel단위 사이즈
    cocos2d::Rect rect(cocos2d::Vec2::ZERO, getContentSize());

//    setTextureRect(rect);
    
    CC_SAFE_DELETE_ARRAY(_triangles.verts);
    CC_SAFE_DELETE_ARRAY(_triangles.indices);
    _triangles.verts = nullptr;
    _triangles.indices = nullptr;
    
    // 이제 일반 sprite가 되었다.
    _isMesh = false;
}

void SMMeshView::convertToMesh(const float meshSize, const bool flipTexure)
{
    // 일반 sprite를 mesh sprite로 변환
    if (_isMesh) {
        // 이미 mesho sorite면 바로 리턴
        return;
    }
    
    auto size = getContentSize();
    auto origin = getPosition();
//    auto size = getTextureRect().size;
//    auto origin = getTextureRect().origin;
    
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
    
    
//    _polyInfo.setTriangles(_triangles);
    
    _rows = rows;
    _cols = cols;
    // 이제 이 sprite는 mesh sprite이다.
    _isMesh = true;
}

void SMMeshView::grow(float px, float py, float value, float step, float radius)
{
    if (!isMesh()) {
        return;
    }
    // fish eye에 썼던 grow method
    float sx, sy, dx, dy, r;
    float growStep = std::abs(value*step);
    
    auto size = getContentSize();
    
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
