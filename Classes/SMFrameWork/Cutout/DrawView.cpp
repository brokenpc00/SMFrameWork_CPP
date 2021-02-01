//
//  DrawView.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#include "DrawView.h"
#include "../Base/MaskSprite.h"
#include "../Base/SMZoomView.h"
#include "../Util/ViewUtil.h"
#include "../Util/nanovgGLDraw/NanoDrawNode.h"
#include "../Util/SMPath.h"
#include <vector>


#define BRUSH_SIZE          (180.0)
#define SMOOTH_STEPS        (4)
#define SMOOTH_AMOUNT       (0.5)
#define SMOOTH_WINDOW_SIZE  (8)
#define MIN_POINT_DISTANCE  (10.0)
#define POINT_RESOLUTION    (5.0)


DrawView::DrawView() : _contentView(nullptr)
, _touchFocused(false)
, _strokeColor(MAKE_COLOR3B(0xffffff))
{
    _drawNodes.clear();
}

DrawView::~DrawView()
{
    
}

bool DrawView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    auto s = getContentSize();
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    addChild(_contentView);
    
    setOnTouchListener(this);
    
    _smoothWeights.resize(SMOOTH_STEPS);
    for (int i = 1; i < SMOOTH_STEPS; i++) {
        _smoothWeights[i] = 1.0 + (SMOOTH_AMOUNT - 1.0) * i / SMOOTH_STEPS;
    }

    
    return true;
}

int DrawView::onTouch(SMView *view, const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    auto pt = _contentView->convertToNodeSpace(convertToWorldSpace(*point));
    
    switch (action) {
        case MotionEvent::ACTION_DOWN:
        {
            _touchFocused = true;
            _points.clear();
            _points.reserve(500);
            _points.push_back(pt);
            _pointAdded = true;
            
            auto drawNode = NanoDrawNode::create();
            _contentView->addChild(drawNode);
            drawNode->setColor(_strokeColor);
            _drawNodes.push_back(drawNode);
        }
            break;
        case MotionEvent::ACTION_POINTER_DOWN:
        {
            _touchFocused = false;
            _points.clear();
            _drawNodes.clear();

        }
            break;
        case MotionEvent::ACTION_MOVE:
        {
            if (_touchFocused) {
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
            } else {
//                CCLOG("[[[[[ 멀티 터치 무브");
            }
        }
            break;
        case MotionEvent::ACTION_UP:
        {
            _touchFocused = false;
        }
            break;
        default:
            break;
    }
    
    if (_points.size()>1 && _drawNodes.size()>0) {
        auto drawNode = _drawNodes[_drawNodes.size()-1];
        if (drawNode) {
            drawNode->drawPath(_points, 10, false);
        }
        
    }

    return TOUCH_FALSE;
}

std::vector<cocos2d::Vec2> DrawView::getSmoothPoints(const std::vector<cocos2d::Vec2>& contour, int start, int end)
{
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

void DrawView::setDrawColor(const cocos2d::Color3B &color)
{
    _strokeColor = color;
}

void DrawView::clearCanvas()
{
    auto children = _contentView->getChildren();
    for (auto view : children) {
        view->removeFromParent();
    }
    _drawNodes.clear();
    _contentView->removeAllChildren();
}
