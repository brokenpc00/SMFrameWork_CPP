//
//  DrawCropView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 29..
//

#include "DrawCropView.h"
#include "../Base/ShaderNode.h"
#include "../Util/ViewUtil.h"
#include "../Popup/LoadingPopup.h"
#include "FuncCutout.h"

#define SMOOTH_STEPS        (4)
#define SMOOTH_AMOUNT       (0.5)
#define SMOOTH_WINDOW_SIZE  (8)
#define MIN_POINT_DISTANCE  (10.0)
#define POINT_RESOLUTION    (5.0)

#define DOT_SIZE    (10.0)
#define DOT_RESOLUTION  (20.0)



DrawCropView::DrawCropView() : _maskSprite(nullptr)
, _boundRect(cocos2d::Rect::ZERO)
//, _commandCursor(0)
, _pointAdded(false)
, _cropBounds(cocos2d::Rect::ZERO)
, onSelectCallback(nullptr)
, grabcutCallback(nullptr)
{
    
}

DrawCropView::~DrawCropView()
{
    
}

DrawCropView * DrawCropView::create(cocos2d::Sprite *sprite, const cocos2d::Rect& canvasRect, const cocos2d::Rect &boundRect)
{
    auto view = new (std::nothrow)DrawCropView();
    
    if (view!=nullptr) {
        if (view->init(sprite, canvasRect, boundRect)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

DrawCropView * DrawCropView::create(uint8_t* pixelData, const cocos2d::Size& imageSize, uint8_t* maskData, int maskPixelsWide, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect)
{
    auto view = new (std::nothrow)DrawCropView();
    if (view!=nullptr) {
        if (view->init(pixelData, imageSize, maskData, maskPixelsWide, canvasRect, boundRect)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    return view;
}

bool DrawCropView::init(cocos2d::Sprite *sprite, const cocos2d::Rect& canvasRect, const cocos2d::Rect &boundRect)
{
    if (!SMView::init()) {
        return false;
    }
    _canvasRect = canvasRect;

    auto s = _canvasRect.size;
    
    setContentSize(s);
    
    _boundRect = boundRect;
    
    setBackgroundColor4F(MAKE_COLOR4F(0x000000, 0.2f));
    
    auto texture = sprite->getTexture();
    _textureSize = texture->getContentSizeInPixels();
    
    
    _maskSprite = MaskSprite::createWithTexture(texture);
    _maskSprite->setScale(_boundRect.size.width / _textureSize.width);
    _maskSprite->setPosition(_boundRect.getMidX(), _boundRect.getMidY());
    _maskSprite->enableMaskDraw(true);
    _maskSprite->clearMask(0);
    addChild(_maskSprite);

    _uiLayer = SMView::create();
    _uiLayer->setContentSize(s);
    _uiLayer->setLocalZOrder(99);
    addChild(_uiLayer);

    setOnTouchListener(this);
    
    _smoothWeights.resize(SMOOTH_STEPS);
    
    for (int i = 1; i < SMOOTH_STEPS; i++) {
        _smoothWeights[i] = 1.0 + (SMOOTH_AMOUNT - 1.0) * i / SMOOTH_STEPS;
    }
    
    return true;
}

bool DrawCropView::init(uint8_t *pixelData, const cocos2d::Size &imageSize, uint8_t *maskData, int maskPixelsWide, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect)
{
    
    if (!SMView::init()) {
        return false;
    }
    

    _canvasRect = canvasRect;
    
    auto s = _canvasRect.size;
    
    setContentSize(s);
    
    _boundRect = boundRect;
    
    setBackgroundColor4F(MAKE_COLOR4F(0x000000, 0.2f));
    
    
    auto texture = new cocos2d::Texture2D();
    int pixelsWide = (int)imageSize.width;
    int pixelsHigh = (int)imageSize.height;
    texture->initWithData(pixelData,
                          pixelsWide*pixelsHigh,
                          cocos2d::Texture2D::PixelFormat::RGBA8888, pixelsWide, pixelsHigh, imageSize);
    
    _textureSize = texture->getContentSizeInPixels();

    _maskSprite = MaskSprite::createWithTexture(texture, maskData, maskPixelsWide);
    _maskSprite->setScale(_boundRect.size.width / _textureSize.width);
    _maskSprite->setPosition(_boundRect.getMidX(), _boundRect.getMidY());
    _maskSprite->enableMaskDraw(true);
    _maskSprite->clearMask(0);
    addChild(_maskSprite);
    
    
    _uiLayer = SMView::create();
    _uiLayer->setContentSize(s);
    _uiLayer->setLocalZOrder(99);
    addChild(_uiLayer);
    
    setOnTouchListener(this);
    
    _smoothWeights.resize(SMOOTH_STEPS);
    
    for (int i = 1; i < SMOOTH_STEPS; i++) {
        _smoothWeights[i] = 1.0 + (SMOOTH_AMOUNT - 1.0) * i / SMOOTH_STEPS;
    }
    
    return true;
}

float DrawCropView::getSpriteScale()
{
    return _maskSprite->getScale();
}

cocos2d::Rect DrawCropView::getBoundRect()
{
    return _boundRect;
}

void DrawCropView::setInitCropBound()
{
    auto size = _maskSprite->getTexture()->getContentSizeInPixels();
//
//    float l = FLT_MAX;
//    float r = FLT_MIN;
//    float t = FLT_MIN;
//    float b = FLT_MAX;
//
//    cocos2d::Vec2 pt = cocos2d::Vec2(4, 4);
//
//    l = std::min(l, pt.x);
//    r = std::max(r, size.width-5);
//    t = std::max(t, size.height-5);
//    b = std::min(b, pt.y);
//
//    l -= 2;
//    r += 2;
//    t += 2;
//    b -= 2;
//    _cropBounds.setRect(l, size.height-t, r-l, t-b);
    
    _cropBounds.setRect(1, 1, size.width-1, size.height-1);
}

cocos2d::Sprite * DrawCropView::cropSprite(cocos2d::Rect &outRect)
{
    if (_cropBounds.size.width <= 0 || _cropBounds.size.height <= 0)
        return nullptr;
    
    return _maskSprite->cropSprite(_cropBounds, outRect);
}

bool DrawCropView::cropData(uint8_t **outPixelData, uint8_t **outMaskData, cocos2d::Rect &outRect)
{
    if (_cropBounds.size.width <= 0 || _cropBounds.size.height <= 0)
        return false;
    
    return _maskSprite->cropData(_cropBounds, outPixelData, outMaskData, outRect);
}

void DrawCropView::drawDots(const std::vector<cocos2d::Vec2>& points) {
    int numDots = (int)_dots.size();
    
    for (int i = 0; i < points.size(); i++) {
        auto pt = convertToNodeSpace(_maskSprite->convertToWorldSpace(points[i]));
        
        if (i < numDots) {
            _dots[i]->setPosition(pt);
        } else {
            auto dot = ShapeSolidCircle::create();
            dot->setContentSize(cocos2d::Size(DOT_SIZE, DOT_SIZE));
            dot->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            dot->setPosition(pt);
            _uiLayer->addChild(dot);
            _dots.push_back(dot);
        }
    }
}



std::vector<cocos2d::Vec2> DrawCropView::getSmoothPoints(const std::vector<cocos2d::Vec2>& contour, int start, int end) {
    int n = (int)contour.size();
    std::vector<cocos2d::Vec2> result;
    result.push_back(contour[start]);
    
    start  = std::min(n-1,  std::max(0, start));
    end    = std::min(n-1,  std::max(0,  end));
    
    n = end-start+1;
    if (n <= 2 + SMOOTH_STEPS) {
        for (int i = start+1; i <= end; i++) {
            result.push_back(contour[i]);
        }
        
        return result;
    }
    
    std::vector<cocos2d::Vec2> points(n), foward(n-2), reverse(n-2);
    
    for (int i = 0; i < n; i++) {
        auto& pt = contour[start+i];
        points[i] = cocos2d::Vec2(pt.x, pt.y);
    }
    for (int i = 0; i < n-2; i++) {
        foward[i] = reverse[i] = points[i+1];
    }
    
    for(int i = 1; i < n-1; i++) {
        
        float sum0 = 1.0, sum1 = 1.0;
        auto& pt0 = foward [i-1];
        auto& pt1 = reverse[n-1-i-1];
        
        for(int j = 1; j < SMOOTH_STEPS; j++) {
            cocos2d::Vec2 cur0(0, 0), cur1(0, 0);
            float weight = _smoothWeights[j];
            
            int l = i - j;
            int r = i + j;
            
            if (l >= 0) {
                cur0 += points[l];
                sum0 += weight;
                
                cur1 += points[n-1-l];
                sum1 += weight;
            }
            
            if (r < n) {
                cur0 += points[r];
                sum0 += weight;
                
                cur1 += points[n-1-r];
                sum1 += weight;
            }
            
            pt0 += cur0 * weight;
            pt1 += cur1 * weight;
        }
        
        pt0.x /= sum0;
        pt0.y /= sum0;
        
        pt1.x /= sum1;
        pt1.y /= sum1;
    }
    
    for (int i = 0; i < n-2; i++) {
        auto& pt0 = foward[i];
        auto& pt1 = reverse[i];
        result.push_back(cocos2d::Vec2((int)(0.5*(pt0.x+pt1.x)), (int)(0.5*(pt0.y+pt1.y))));
    }
    
    result.push_back(contour[end]);
    
    return result;
}

std::vector<cocos2d::Vec2> DrawCropView::getSlicePoints(const std::vector<cocos2d::Vec2>& points, float resolution) {
    std::vector<cocos2d::Vec2> result;
    
    if (points.size() < 2) {
        return points;
    }
    
    result.push_back(points[0]);
    
    cocos2d::Vec2 p0 = points[0], p1 = points[1];
    
    int index = 1;
    float remain = 0;
    
    for (;index < points.size();) {
        float dist = p0.distance(p1);
        if (remain + dist >= resolution) {
            float t = resolution / dist;
            float xx = p0.x + t * (p1.x - p0.x);
            float yy = p0.y + t * (p1.y - p0.y);
            p0 = cocos2d::Vec2(xx, yy);
            result.push_back(p0);
            remain = 0;
        } else {
            remain = dist;
            p1 = points[++index];
        }
    }
    
    return result;
}

void DrawCropView::reset() {
    _maskSprite->clearMask(0);
    
    for (auto dot : _dots) {
        _uiLayer->removeChild(dot);
    }
    _dots.clear();
    _points.clear();
//    _command.clear();
//    _commandCursor = 0;
}

int DrawCropView::onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) {
    
    auto pt = _maskSprite->convertToNodeSpace(convertToWorldSpace(*point));
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
            
            reset();
            _points.clear();
            _points.push_back(pt);
            _pointAdded = true;
            
            break;
        case MotionEvent::ACTION_MOVE:
            if (_pointAdded) {
                _points.push_back(pt);
                _pointAdded = false;
            } else {
                int n = (int)_points.size();
                if (n > 1) {
                    _points[n-1] = pt;
                    
                    auto p0 = _points.at(n-2);
                    auto p1 = _points.at(n-1);
                    
                    float dist = p1.distance(p0);
                    if (dist > MIN_POINT_DISTANCE) {
                        if (dist > POINT_RESOLUTION * 2) {
                            // 너무 먼 거리에 있으면 중간점 추가
                            int connect = std::min(4, (int)(dist / POINT_RESOLUTION));
                            if (connect > 1) {
                                auto d = p1-p0;
                                for (int i = 1; i < connect; i++) {
                                    if (i == 1) {
                                        _points[n-1] = p0 + i*d/connect;
                                    } else {
                                        _points.push_back(p0 + i*d/connect);
                                    }
                                }
                                _points.push_back(p1);
                            }
                        }
                        _pointAdded = true;
                    }
                }
            }
            
            if (_points.size() >= SMOOTH_WINDOW_SIZE) {
                int start = (int)_points.size()-SMOOTH_WINDOW_SIZE;
                auto smoothed = getSmoothPoints(_points, start, (int)_points.size());
                for (int i = 0; i < 8; i++) {
                    _points[start+i] = smoothed[i];
                }
            }
            break;
        case MotionEvent::ACTION_UP:
        {
            _maskSprite->flush();
            
            bool canceled = true;
            if (_points.size() >= 2) {
                _points.push_back(_points[0]);
                
                // area
                float area = 0;
                for(int i = 0; i < (int)_points.size()-1; i++){
                    area += _points[i].x * _points[i+1].y - _points[i+1].x * _points[i].y;
                }
                area += _points[_points.size()-1].x * _points[0].y - _points[0].x * _points[_points.size()-1].y;
                area *= 0.5;
                
                if (std::abs(area) >= 100 * 100) {
                    canceled = false;
//                    _command.erase(_command.begin()+_commandCursor, _command.end());
//                    _command.push_back(DrawCommand(Mode::SUB_STROKE, _points, 20));
//                    _commandCursor++;
                    
                    if (onSelectCallback) {
                        onSelectCallback(true);
                    }
                }
                
                float l = FLT_MAX;
                float r = FLT_MIN;
                float t = FLT_MIN;
                float b = FLT_MAX;
                
                for (auto& pt : _points) {
                    l = std::min(l, pt.x);
                    r = std::max(r, pt.x);
                    t = std::max(t, pt.y);
                    b = std::min(b, pt.y);
                }
                
                l -= 2;
                r += 2;
                t += 2;
                b -= 2;
                auto size = _maskSprite->getTexture()->getContentSizeInPixels();
                _cropBounds.setRect(l, size.height-t, r-l, t-b);
            }
            
            if (canceled) {
                reset();
                
                if (onSelectCallback) {
                    onSelectCallback(false);
                }
                
                _cropBounds = cocos2d::Rect::ZERO;
            }
        }
            break;
        default:
            break;
    }
    if (_points.size()) {
        _maskSprite->addFill(_points);
        auto slicePoints = getSlicePoints(_points, DOT_RESOLUTION);
        drawDots(slicePoints);
    }
    
    return TOUCH_FALSE;
}

#define INFALTE_CHUNK_SIZE  (16*1024)
static bool decompressData(uint8_t* outData, uint8_t* inData, size_t inSize) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit(&zs) != Z_OK) {
        CCLOG("[PRELOADER] inflateInit failed while decompressing.");
        return false;
    }
    
    int ret, readBytes, totalBytes = 0;
    uint8_t buffer[INFALTE_CHUNK_SIZE];
    
    zs.next_in = (Bytef*)inData;
    zs.avail_in = (uInt)inSize;
    
    do {
        zs.avail_out = INFALTE_CHUNK_SIZE;
        zs.next_out = (Bytef*)buffer;
        
        ret = inflate(&zs, Z_NO_FLUSH);
        
        switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                break;
        }
        
        readBytes = INFALTE_CHUNK_SIZE - zs.avail_out;
        if (readBytes > 0) {
            memcpy(outData + totalBytes, buffer, readBytes);
            totalBytes += readBytes;
        }
    } while (zs.avail_out == 0);
    
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        CCLOG("[PRELOADER] Error during zlib decompression : %d", ret);
        return false;
    }
    
    return true;
}

void DrawCropView::onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite *sprite, Intent *result)
{
    LoadingPopup::close();

    uint8_t * retData = nullptr;
    if (result!=nullptr) {
        auto dataHolder = (DataHolder*)result->getRef("NO_COMPRESS_DATA");
        if (dataHolder!=nullptr) {
            retData = dataHolder->getData();
        }
    }
    
    if (grabcutCallback) {
        
        grabcutCallback(retData);
        
    }

//
//    auto compressedData = dataHolder->getData();
//    auto compressedSize = dataHolder->getDataLen();
//
//    if (decompressData(_maskSprite->getMaskData(), compressedData, compressedSize)) {
//        _maskSprite->updateMask();
//    }

//    cocos2d::Rect outRect;
//    float outScale;
//    auto size = _maskSprite->getTexture()->getContentSizeInPixels();
//    
//    uint8_t* maskData = _maskSprite->getMaskData();
//    auto maskSprite = MaskSprite::createWithTexture(_maskSprite->getTexture(), maskData);
//    
//    auto newSprite = maskSprite->extractSprite(cocos2d::Rect(cocos2d::Vec2::ZERO, size), outRect, outScale, 2);
//
//
//
//    auto layer = (SMView*)cocos2d::Director::getInstance()->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
//    layer->addChild(newSprite);
//    newSprite->setPosition(layer->getContentSize()/2);
//    return;
//
////    auto newSprite = _maskSprite->extractSprite(cocos2d::Rect(cocos2d::Vec2::ZERO, size), outRect, outScale, 2);
//    if (newSprite) {
//        auto pt = _maskSprite->convertToWorldSpace(cocos2d::Vec2(outRect.getMidX(), outRect.getMidY()));
//        float scale = _maskSprite->getScale();
//        for (auto parent = _maskSprite->getParent();  parent != nullptr; parent = parent->getParent()) {
//            scale *= parent->getScale();
//        }
//
//        newSprite->setPosition(pt);
//        newSprite->setScale(scale * (1.0/outScale));
//    }
    
    
//    if (grabcutCallback) {
//
//        grabcutCallback();
//
//    }
}

cocos2d::Sprite* DrawCropView::extractSprite()
{
    cocos2d::Rect outRect;
    float outScale;
    auto size = _maskSprite->getTexture()->getContentSizeInPixels();
    auto sprite = _maskSprite->extractSprite(cocos2d::Rect(cocos2d::Vec2::ZERO, size), outRect, outScale, 2);

    if (sprite) {
        auto pt = _maskSprite->convertToWorldSpace(cocos2d::Vec2(outRect.getMidX(), outRect.getMidY()));
        float scale = _maskSprite->getScale();
        for (auto parent = _maskSprite->getParent();  parent != nullptr; parent = parent->getParent()) {
            scale *= parent->getScale();
        }
        
        sprite->setPosition(pt);
        sprite->setScale(scale * (1.0/outScale));
    }
    
    return sprite;

}

void DrawCropView::grabcut(uint8_t* pixelData, const cocos2d::Size& imageSize, uint8_t* maskData, int maskPixelsWide, std::function<void(uint8_t* newMaskData)> callback)
{
    grabcutCallback = callback;
    
    uint8_t* newPixelData;
    uint8_t* newMaskData;
    auto dataLength = (size_t)(imageSize.width * imageSize.height * 4);
    newPixelData = (uint8_t*)malloc(dataLength);
    memcpy(newPixelData, pixelData, dataLength);
    
    newMaskData = (uint8_t*)malloc(imageSize.width*imageSize.height);
    memcpy(newMaskData, maskData, imageSize.width * imageSize.height);
    
    
//    if (maskPixelsWide%8 != 0) {
//        // 강제로 맞춘다.
//        maskPixelsWide = (maskPixelsWide/8 + 1)*8;
//    }

    
    ImageProcessor::getInstance().executeImageProcess(this, nullptr,
                                                      new FuncCutout(newPixelData,
                                                                     (int)imageSize.width, (int)imageSize.height, 4,
                                                                     newMaskData,
                                                                     maskPixelsWide));
    LoadingPopup::showLoading();
}

//void DrawCropView::grabcut(std::function<void(uint8_t* newMaskData)> callback)
//{
//    reset();
//    grabcutCallback = callback;
//
//    uint8_t* pixelData = nullptr;
//    _maskSprite->readPixels(&pixelData);
//    if (pixelData == nullptr)
//        return;
//
//    auto size = _maskSprite->getTexture()->getContentSizeInPixels();
//
//    ImageProcessor::getInstance().executeImageProcess(this, nullptr,
//                                                      new FuncCutout(pixelData,
//                                                                     (int)size.width, (int)size.height, 4,
//                                                                     _maskSprite->cloneMaskData(),
//                                                                     _maskSprite->getMaskPixelsWide()));
//    LoadingPopup::showLoading();
//}
