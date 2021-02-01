//
//  CropFreeView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#include "CropFreeView.h"
#include "../Base/ShaderNode.h"
#include "../Base/ViewAction.h"
#include "../Const/SMFontColor.h"

#define SMOOTH_STEPS        (4)
#define SMOOTH_AMOUNT       (0.5)
#define SMOOTH_WINDOW_SIZE  (8)
#define MIN_POINT_DISTANCE  (10.0)
#define POINT_RESOLUTION    (5.0)

#define DOT_SIZE    (10.0)
#define DOT_RESOLUTION  (20.0)

#define DIM_OPACITY_EDIT    (0.5)
#define DIM_OPACITY_IDLE    (0.7)


CropFreeView::CropFreeView() : _commandCursor(0)
, _pointAdded(false)
, _cropBounds(cocos2d::Rect::ZERO)
, onSelectCallback(nullptr)
{
    
}

CropFreeView * CropFreeView::create(cocos2d::Sprite *sprite, const cocos2d::Rect &boundRect)
{
    auto view = new (std::nothrow)CropFreeView();
    
    if (view) {
        if (view->init(sprite, boundRect)) {
            view->autorelease();
        } else {
            CC_SAFE_DELETE(view);
        }
    }
    
    return view;
}

bool CropFreeView::init(cocos2d::Sprite *sprite, const cocos2d::Rect &boundRect)
{
    BaseCropView::init(sprite, boundRect);
    
    // 그리는 스프라이트임. 준비하자
    _maskSprite->enableMaskDraw(true);
    _maskSprite->clearMask(0);
    
    setOnTouchListener(this);
    
    _smoothWeights.resize(SMOOTH_STEPS);
    for (int i=0; i<SMOOTH_STEPS; i++) {
        _smoothWeights[i] = 1.0 + (SMOOTH_AMOUNT - 1.0) * i / SMOOTH_STEPS;
    }
    
    setDimOpacity(DIM_OPACITY_IDLE, 0.3, 0);
    
    return true;
}

cocos2d::Sprite* CropFreeView::cropSpriteInternal(cocos2d::Rect &outRect)
{
    if (_cropBounds.size.width <= 0 || _cropBounds.size.height <= 0)
        return nullptr;
    
    return _maskSprite->cropSprite(_cropBounds, outRect);
}

bool CropFreeView::cropDataInternal(uint8_t **outPixelData, uint8_t **outMaskData, cocos2d::Rect &outRect)
{
    if (_cropBounds.size.width <= 0 || _cropBounds.size.height <= 0)
        return false;
    
    return _maskSprite->cropData(_cropBounds, outPixelData, outMaskData, outRect);
}

void CropFreeView::drawDots(const std::vector<cocos2d::Vec2> &points)
{
    // 주어진 point에 SolidCircle로 점을 찍는다.
    int numDots = (int)_dots.size();
    
    for (int i=0; i<points.size(); i++) {
        auto pt = convertToNodeSpace(_maskSprite->convertToWorldSpace(points[i]));
        
        if (i<numDots) {
            // 이미 찍은 점은 위치만 조정...
            _dots[i]->setPosition(pt);
        } else {
            // 안 찍었던 점이면 새로 만들어서 찍어준다.
            auto dot = ShapeSolidCircle::create();
            dot->setContentSize(cocos2d::Size(DOT_SIZE, DOT_SIZE));
            dot->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
            dot->setPosition(pt);
            _uiLayer->addChild(dot);
            _dots.push_back(dot);
        }
    }
}

std::vector<cocos2d::Vec2> CropFreeView::getSmoothPoints(const std::vector<cocos2d::Vec2> &contour, int start, int end)
{
    // 주어진 점 배열을 부드러운 곡선으로 만들고 Slice하여 해당 좌표 배열을 넘긴다.
    int n = (int)contour.size();
    
    std::vector<cocos2d::Vec2> result;
    result.push_back(contour[start]);

    start = std::min(n-1, std::max(0, start));
    end = std::min(n-1, std::max(0, end));;
    
    // 시작점은 제외
    n = end-start+1;
    
    if (n<=2+SMOOTH_STEPS) {
        for (int i=start-1; i<=end; i++) {
            result.push_back(contour[i]);
        }
        
        return result;
    }
    
    std::vector<cocos2d::Vec2> points(n), forward(n-1), reverse(n-2);
    
    for (int i=0; i<n; i++) {
        auto& pt = contour[start+i];
        points[i] = cocos2d::Vec2(pt.x, pt.y);
    }
    for (int i=0; i<n-2; i++) {
        forward[i] = reverse[i] = points[i+1];
    }
    
    for (int i=1; i<n-1; i++) {
        float sum0 = 1.0, sum1 = 1.0;
        auto& pt0 = forward[i-1];
        auto& pt1 = reverse[n-2-i];
        
        for (int j=1; j<SMOOTH_STEPS; j++) {
            cocos2d::Vec2 cur0(0, 0), cur1(0, 0);
            float weight = _smoothWeights[j];
            
            int l = i - j;
            int r = i + j;
            
            if (l>=0) {
                cur0 += points[l];
                sum0 += weight;
                
                cur1 += points[n-1-l];
                sum1 += weight;
            }
            
            if (r<n) {
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
    
    for (int i=0; i<n-2; i++) {
        auto& pt0 = forward[i];
        auto& pt1 = reverse[i];
        result.push_back(cocos2d::Vec2((int)(0.5*(pt0.x+pt1.x)), (int)(0.5*(pt0.y+pt1.y))));
    }
    
    // 마지막넘도 넣어주자.
    result.push_back(contour[end]);
    
    return result;
}

std::vector<cocos2d::Vec2> CropFreeView::getSlicePoints(const std::vector<cocos2d::Vec2> &points, float resolution)
{
    std::vector<cocos2d::Vec2> result;
    

    // 좌표는 2개 이상이어야 한다.
    if (points.size() < 2) {
        return points;
    }

    // 일단 첫번째꺼는 넣어주고...
    result.push_back(points[0]);
    
    // 그 다음 좌표부터 계산
    cocos2d::Vec2 p0 = points[0], p1 = points[1];
    
    int index = 1;
    float remain = 0;

    for (; index<points.size();) {
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

void CropFreeView::reset()
{
    _maskSprite->clearMask(0);
    
    for (auto dot : _dots) {
        _uiLayer->removeChild(dot);
    }
    _dots.clear();
    _points.clear();
    _command.clear();
    _commandCursor = 0;
}

int CropFreeView::onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event)
{
    auto pt = _maskSprite->convertToNodeSpace(convertToWorldSpace(*point));
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
            // 새로 그리기 시작
            
            reset();
            _points.clear();
            _points.push_back(pt);
            _pointAdded = true;
            
            setDimOpacity(DIM_OPACITY_EDIT, 0.3, 0.0);
            
            break;
        case MotionEvent::ACTION_MOVE:
            if (_pointAdded) {
                // 첫 좌표 또는 계산 후 다음 좌표를 넣고
                _points.push_back(pt);
                _pointAdded = false;
            } else {
                // 다음 점은 계산해서 넣고
                int n = (int)_points.size();
                if (n > 1) {
                    _points[n-1] = pt;
                    
                    auto p0 = _points.at(n-2);
                    auto p1 = _points.at(n-1);
                    
                    float dist = p1.distance(p0);
                    if (dist > MIN_POINT_DISTANCE) {
                        if (dist > POINT_RESOLUTION * 2) {
                            // 두 점사이가 너무 멀면 이상하니까 중간에 점을 낑가 놓는다.
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
            // 손을 떼는 순간 점은 날린다.
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
                    _command.erase(_command.begin()+_commandCursor, _command.end());
                    _command.push_back(DrawCommand(Mode::SUB_STROKE, _points, 20));
                    _commandCursor++;
                    
                    setDimOpacity(DIM_OPACITY_IDLE, 0.3, 1.0);
                    
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
                setDimOpacity(DIM_OPACITY_IDLE, 0.3, 0.0);
                
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
        // 아무튼... 있는 점은 그려야제???
        _maskSprite->addFill(_points);
        auto slicePoints = getSlicePoints(_points, DOT_RESOLUTION);
        drawDots(slicePoints);
    }
    
    return TOUCH_FALSE;
}

