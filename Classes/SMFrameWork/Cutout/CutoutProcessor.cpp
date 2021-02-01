//
//  CutoutProcessor.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#include "CutoutProcessor.h"
#include <algorithm>

#define MIN_IMAGE_SIZE  (30)
#define PADDING         (4)

#define MIN_GRABCUT_SIZE    (100)
#define MAX_GRABCUT_SIZE    (512)
#define FAST_BASE_SIZE      (160)


CutoutProcessor::CutoutProcessor(uint8_t* pixelData, int width, int height, int bpp, uint8_t* maskData, int maskPixelWide)  :
_imagePixelData(pixelData),
_imageWidth(width),
_imageHeight(height),
_imageBpp(bpp),
_maskPixelData(maskData),
_maskPixelWide(maskPixelWide)
{
}

CutoutProcessor::~CutoutProcessor() {
    release();
}

void CutoutProcessor::release() {
    CC_SAFE_FREE(_imagePixelData);
    CC_SAFE_FREE(_maskPixelData);
    _srcColorChannel.release();
    _srcAlphaChannel.release();
    _fastMask.release();
}


bool CutoutProcessor::init() {
    if (_imagePixelData == nullptr || _imageWidth <= MIN_IMAGE_SIZE || _imageHeight < MIN_IMAGE_SIZE) {
        return false;
    }
    
    if (_imageBpp == 4) {
        // 알파채널 체크
        int length = _imageWidth * _imageHeight;
        auto src = _imagePixelData + 3;
        
        _srcHasAlphaChannel = false;
        for (int i = 0; i < length; i++) {
            if (*src < 0xFF) {
                _srcHasAlphaChannel = true;
                src += 4;
                break;
            }
        }
        
        // 알파채널 있으면 분리
        if (_srcHasAlphaChannel) {
            _srcAlphaChannel = cv::Mat(_imageHeight, _imageWidth, CV_8UC1);
            src = _imagePixelData + 3;
            auto dst = _srcAlphaChannel.data;
            for (int i = 0; i < length; i++) {
                *dst++ = *src;
                src += 4;
            }
        }
        
        // RGB로 변환
        cv::cvtColor(cv::Mat(_imageHeight, _imageWidth, CV_8UC4, _imagePixelData), _srcColorChannel, CV_RGBA2RGB);
        CC_SAFE_FREE(_imagePixelData);
    } else if (_imageBpp == 3) {
        // RGB
        _srcColorChannel = cv::Mat(_imageHeight, _imageWidth, CV_8UC3);
        _srcHasAlphaChannel = false;
    } else {
        // ERROR
        CC_ASSERT(0);
        return false;
    }
    
    return true;
}

bool CutoutProcessor::initFastMask(cv::Mat* alphaMask) {
    cv::Mat image;
    
    float scale = std::sqrt((float)FAST_BASE_SIZE * FAST_BASE_SIZE / (_imageWidth * _imageHeight));
    _fastWidth  = ((int)(_imageWidth  * scale)/8) * 8;
    _fastHeight = ((int)(_imageHeight * scale)/8) * 8;
    cv::resize(_srcColorChannel, image, cv::Size(_fastWidth, _fastHeight), 0, 0, CV_INTER_LINEAR);
    
    cv::Point pt1(0, 0);
    cv::Point pt2(_fastWidth-1, _fastHeight-1);
    cv::Rect  rect(pt1, pt2);
    
    cv::Mat bgdModel, fgdModel;
    cv::grabCut(image, _fastMask, rect, bgdModel, fgdModel, 0, cv::GC_INIT_WITH_RECT);
    if (bgdModel.empty() || fgdModel.empty()) {
        return false;
    }
    
    _fastMask.setTo(cv::GC_PR_BGD);
    cv::ellipse(_fastMask, cv::Point(_fastWidth*0.5, _fastHeight*0.5), cv::Size(_fastWidth*0.45, _fastHeight*0.45), 0, 0, 360, cv::GC_PR_FGD, CV_FILLED);
    cv::rectangle(_fastMask, cv::Point(0, 0), cv::Point(_fastWidth-1, _fastHeight-1), cv::GC_BGD, 2);
    
    cv::grabCut(image, _fastMask, rect, bgdModel, fgdModel, 10, cv::GC_INIT_WITH_MASK);
    if (bgdModel.empty() || fgdModel.empty()) {
        return false;
    }
    
    cv::compare(_fastMask, cv::GC_PR_FGD, _fastMask, cv::CMP_EQ);
    
    const int   erosionType = cv::MORPH_RECT;
    cv::Size    erosionSize(5, 5);
    cv::Mat     erorionElement = cv::getStructuringElement(
                                                           erosionType,
                                                           erosionSize);
    
    cv::rectangle(_fastMask, pt1, pt2, 0, 1);
    cv::erode(_fastMask, _fastMask, erorionElement);
    
    
    int numPixels = 0;
    auto src = _fastMask.data;
    int length = _fastWidth * _fastHeight;
    for (int i = 0; i < length; i++) {
        if (*src++ > 100) {
            numPixels++;
        }
    }
    
    if (numPixels < 20 * 20) {
        _fastMask.setTo(0);
        cv::ellipse(_fastMask, cv::Point(_fastWidth*0.5, _fastHeight*0.5), cv::Size(_fastWidth*0.45, _fastHeight*0.45), 0, 0, 360, 0xFF, CV_FILLED);
    }
    
    return true;
}

uint8_t*  CutoutProcessor::cutout() {
    if (!init()) {
        return nullptr;
    }
    
    cv::Mat grabImage, grabMask;
    int     width, height;
    
    cv::Mat bgdModel, fgdModel;
    
    //------------------------------------------------------
    // Grabcut 이미지 / 입력 마스크 생성
    if (_imageWidth * _imageHeight < MIN_GRABCUT_SIZE * MIN_GRABCUT_SIZE) {
        // 이미지가 너무 작으면 키운다 (100 * 100)
        float scale = std::sqrt((float)MIN_GRABCUT_SIZE * MIN_GRABCUT_SIZE / (_imageWidth * _imageHeight));
        width  = (int)(_imageWidth * scale);
        height = (int)(_imageHeight * scale);
        
        cv::resize(_srcColorChannel, grabImage, cv::Size(width, height), CV_INTER_LINEAR);
        
    } else if (_imageWidth * _imageHeight > MAX_GRABCUT_SIZE * MAX_GRABCUT_SIZE) {
        // 이미지가 너무 크면 줄인다. (512*512)
        float scale = std::sqrt((float)MAX_GRABCUT_SIZE * MAX_GRABCUT_SIZE / (_imageWidth * _imageHeight));
        width  = (int)(_imageWidth * scale);
        height = (int)(_imageHeight * scale);
        
        cv::resize(_srcColorChannel, grabImage, cv::Size(width, height), CV_INTER_LINEAR);
        
    } else {
        // 원본 사이즈 사용
        width  = _imageWidth;
        height = _imageHeight;
        
        grabImage = _srcColorChannel;
    }
    
    //------------------------------------------------------
    // Grabcut Init Rect
    cv::grabCut(grabImage, grabMask, cv::Rect(0, 0, width-1, height-1), bgdModel, fgdModel, 0, cv::GC_INIT_WITH_RECT);
    if (bgdModel.empty() || fgdModel.empty()) {
        return nullptr;
    }
    grabMask.setTo(cv::GC_PR_BGD);
    
    //------------------------------------------------------
    // 4) 마스크에 alpha 있는지
    bool inputMaskHasAlphaChannel = false;
    for (int y = 0; y < _imageHeight && inputMaskHasAlphaChannel == false; y++) {
        auto m = _maskPixelData + y * _maskPixelWide;
        for (int x = 0; x < _imageWidth; x++) {
            if (*m < 0xFF) {
                inputMaskHasAlphaChannel = true;
                break;
            }
        }
    }
    
    //------------------------------------------------------
    // 4) 마스크 합성
    cv::Mat alphaMask;
    bool hasAlphaMask = true;
    
    if (inputMaskHasAlphaChannel) {
        cv::resize(cv::Mat(_imageHeight, _maskPixelWide, CV_8UC1, _maskPixelData), alphaMask, cv::Size(width, height), CV_INTER_LINEAR);
        
        if (_srcHasAlphaChannel) {
            // alphaMask + inputMask 합성
            cv::Mat alphaTemp;
            cv::resize(_srcAlphaChannel, alphaTemp, cv::Size(width, height), CV_INTER_LINEAR);
            
            for (int y = 0; y < height; y++) {
                auto m0 = alphaMask.row(y).data;
                auto m1 = alphaTemp.row(y).data;
                
                for (int x = 0; x < width; x++) {
                    *m0 = std::min(*m0, *m1);
                    m0++;
                    m1++;
                }
            }
        }
    } else {
        if (_srcHasAlphaChannel) {
            cv::resize(_srcAlphaChannel, alphaMask, cv::Size(width, height), CV_INTER_LINEAR);
        } else {
            hasAlphaMask = false;
        }
    }
    
    // BGD 픽셀 세팅
    if (hasAlphaMask) {
        
//        const int   erosionType = cv::MORPH_RECT;
//        cv::Size    erosionSize(width*0.2, height*0.2);
//        cv::Mat     erorionElement = cv::getStructuringElement(
//                                                               erosionType,
//                                                               erosionSize);
//        cv::Mat subMask;
//        cv::rectangle(alphaMask, cv::Point(0, 0), cv::Point(width-1, height-1), cv::Scalar(0), 1);
//        cv::erode(compoMask, subMask, erorionElement);
//        cv::subtract(compoMask, subMask, compoMask);
        
//        if (inputMaskHasAlphaChannel) {
        if (initFastMask()) {
            cv::resize(_fastMask, _fastMask, cv::Size(width, height), cv::INTER_LINEAR);
            
            for (int y = 0; y < height; y++) {
                auto mm = grabMask.row(y).data;
                auto m0 = alphaMask.row(y).data;
                auto mf = _fastMask.row(y).data;
                
                
                for (int x = 0; x < width; x++) {
                    if (*m0 == 0) {
                        *mm = cv::GC_BGD;
                    } else if (*m0 == 0xFF || *mf > 100) {
                        
                        *mm = cv::GC_PR_FGD;
                    } else {
                        *mm = cv::GC_PR_BGD;
                    }
                    mm++;
                    m0++;
                }
            }
            
            _fastMask.release();
        } else {
            for (int y = 0; y < height; y++) {
                auto mm = grabMask.row(y).data;
                auto m0 = alphaMask.row(y).data;
                
                for (int x = 0; x < width; x++) {
                    if (*m0 == 0) {
                        *mm = cv::GC_BGD;
                    } else if (*m0 == 0xFF) {
                        *mm = cv::GC_PR_FGD;
                    } else {
                        *mm = cv::GC_PR_BGD;
                    }
                    mm++;
                    m0++;
                }
            }
        }
        
//        } else {
//            for (int y = 0; y < height; y++) {
//                auto mm = grabMask.row(y).data;
//                auto m0 = alphaMask.row(y).data;
//
//                for (int x = 0; x < width; x++) {
//                    if (*m0 == 0) {
//                        *mm = cv::GC_BGD;
//                    } else if (*m0 == 0xFF) {
//                        *mm = cv::GC_PR_FGD;
//                    } else {
//                        *mm = cv::GC_PR_BGD;
//                    }
//                    mm++;
//                    m0++;
//                }
//            }
//        }
        alphaMask.release();
    } else {
        // Fast Mask 적용
        //------------------------------------------------------
        // FastMask Init
        if (initFastMask()) {
            cv::resize(_fastMask, _fastMask, cv::Size(width, height), cv::INTER_LINEAR);
            
            for (int y = 0; y < height; y++) {
                auto* mm = grabMask.row(y).data;
                auto* mf = _fastMask.row(y).data;
                
                for (int x = 0; x < width; x++) {
                    if (*mf > 100) {
                        *mm = cv::GC_PR_FGD;
                    }
                    mf++;
                    mm++;
                }
            }
        }
        _fastMask.release();
        
        cv::rectangle(grabMask, cv::Point(0, 0), cv::Point(width-1, height-1), cv::GC_BGD);
    }
    _srcColorChannel.release();
    
    //------------------------------------------------------
    // Grabcut
    cv::grabCut(grabImage, grabMask, cv::Rect(0, 0, width-1, height-1), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);
    if (bgdModel.empty() || fgdModel.empty()) {
        return nullptr;
    }
    grabImage.release();
    
    cv::compare(grabMask, cv::GC_PR_FGD, grabMask, cv::CMP_EQ);
    if (_imageWidth != width || _imageHeight != height) {
        cv::resize(grabMask, grabMask, cv::Size(_imageWidth, _imageHeight), CV_INTER_LINEAR);
    }
    
    //------------------------------------------------------
    // 모양 다듬기
    simplifyShape(grabMask, _imageWidth, _imageHeight, .1, 10*10);
    
    //------------------------------------------------------
    // 알파채널 합성
    if (_srcHasAlphaChannel) {
        for (int y = 0; y < _imageHeight; y++) {
            auto mm = _maskPixelData + y * _maskPixelWide;
            auto m0 = grabMask.row(y).data;
            auto m1 = _srcAlphaChannel.row(y).data;
            
            for (int x = 0; x < _imageWidth; x++) {
                *mm = std::min(*m0, *m1);
                mm++;
                m0++;
                m1++;
            }
        }
    } else {
        for (int y = 0; y < _imageHeight; y++) {
            auto mm = _maskPixelData + y * _maskPixelWide;
            auto m0 = grabMask.row(y).data;
            memcpy(mm, m0, _imageWidth);
        }
    }
    
    auto ret = _maskPixelData;
    _maskPixelData = nullptr;
    
    return ret;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Bounds Rect
//-------------------------------------------------------------------------
cv::Rect CutoutProcessor::getBoundsRect(const std::vector<cocos2d::Vec2>& points, int width, int height) {
    if (points.size() < 3) {
        return cv::Rect(0, 0, 0, 0);
    }
    
    int minX = width;
    int minY = height;
    int maxX = 0;
    int maxY = 0;
    
    for (auto& pt : points) {
        int x = (int)pt.x;
        int y = (int)pt.y;
        
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }
    
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, width);
    maxY = std::min(maxY, height);
    
    int w = maxX - minX;
    int h = maxY - minY;
    
    if (w <= 0 || h <= 0) {
        return cv::Rect(0, 0, 0, 0);
    }
    
    return cv::Rect(minX, minY, w, h);
}

cv::Rect CutoutProcessor::getBoundsRect(const cv::Mat& mask, int width, int height) {
    int minX = width;
    int minY = height;
    int maxX = 0;
    int maxY = 0;
    
    for (int y = 0; y < mask.rows; y++) {
        auto* m = mask.row(y).data;
        for (int x = 0; x < mask.cols; x++) {
            if (*m) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
            m++;
        }
    }
    
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, width);
    maxY = std::min(maxY, height);
    
    int w = maxX - minX;
    int h = maxY - minY;
    
    if (w <= 0 || h <= 0) {
        return cv::Rect(0, 0, 0, 0);
    }
    
    return cv::Rect(minX, minY, w, h);
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// 라인 세그먼트 Smooth 처리
//-------------------------------------------------------------------------
std::vector<cv::Point> CutoutProcessor::getSmoothSegment(const std::vector<cv::Point>& contour, int steps, int start, int end, float* weights) {
    int n = (int)contour.size();
    std::vector<cv::Point> result;
    
    start  = std::min(n-1,  std::max(0, start));
    end    = std::min(n-1,  std::max(0,  end));
    steps  = std::min(n,    std::max(0,    steps));
    
    n = end-start+1;
    if (n <= 2 + steps) {
        for (int i = start+1; i < end; i++) {
            result.push_back(contour[i]);
        }
        
        return result;
    }
    
    std::vector<cv::Point2f> points(n), foward(n-2), reverse(n-2);
    
    for (int i = 0; i < n; i++) {
        auto& pt = contour[start+i];
        points[i] = cv::Point2f(pt.x, pt.y);
    }
    for (int i = 0; i < n-2; i++) {
        foward[i] = reverse[i] = points[i+1];
    }
    
    for(int i = 1; i < n-1; i++) {
        
        float sum0 = 1.0, sum1 = 1.0;
        auto& pt0 = foward [i-1];
        auto& pt1 = reverse[n-1-i-1];
        
        for(int j = 1; j < steps; j++) {
            cv::Point2f cur0(0, 0), cur1(0, 0);
            float weight = weights[j];
            
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
        result.push_back(cv::Point((int)(0.5*(pt0.x+pt1.x)), (int)(0.5*(pt0.y+pt1.y))));
    }
    
    return result;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Polygon에서 Corner Elements를 찾는다.
//-------------------------------------------------------------------------
std::vector<int> CutoutProcessor::findCornerElements(const std::vector<cv::Point>& points) {
    
    std::vector<int> cornerIndices;
    
    // 1) Polygon의 무게중심과 각 정점의 거리로 1d Array 생성
    std::vector<float> values;
    
    float minValue = FLT_MAX;
    float maxValue = FLT_MIN;
    
    int n = (int)points.size();
    
    //-------------------------------------------------------------------------
    const float delta = 3;
    
    auto mu = cv::moments(points);
    auto mc = cv::Point2f(mu.m10/mu.m00 , mu.m01/mu.m00); // 무게중심
    
    for (int i = 0; i < n; i++) {
        auto& p0 = points[i];
        
        float dx = mc.x - p0.x;
        float dy = mc.y - p0.y;
        float dist = dx*dx + dy*dy;
        
        minValue = std::min(minValue, dist);
        maxValue = std::max(maxValue, dist);
        
        values.push_back(dist);
    }
    
    //-------------------------------------------------------------------------
    float range = maxValue - minValue;
    float scale = 100.0 / range;
    for (auto& v : values) {
        v *= scale; // rescale
    }
    
    // 2) 생상된 1d Array에서 peak, valley 찾는다
    float maximum = FLT_MIN;
    float minimum = FLT_MAX;
    int maximumPos = 0;
    int minimumPos = 0;
    
    bool lookForMax = true;
    
    for (int i = 0; i < n; i++) {
        float value = values[i];
        
        if (value > maximum) {
            maximum = value;
            maximumPos = i;
        }
        
        if (value < minimum) {
            minimum = value;
            minimumPos = i;
        }
        
        if (lookForMax) {
            if (value < maximum - delta) {
                cornerIndices.push_back(maximumPos);
                minimum = value;
                minimumPos = i;
                lookForMax = false;
            }
        } else {
            if (value > minimum + delta) {
                cornerIndices.push_back(minimumPos);
                maximum = value;
                maximumPos = i;
                lookForMax = true;
            }
        }
    }
    
    //-------------------------------------------------------------------------
    // 3) Convex / Concave 포인트 추가
    std::vector<int> hull;
    std::vector<cv::Vec4i> defect;
    
    cv::convexHull(cv::Mat(points), hull, false);
    if (hull.size() > 3) {
        cv::convexityDefects(points, hull, defect);
    }
    for (auto index : hull) {
        cornerIndices.push_back(index);
    }
    for (auto& v : defect) {
        float depth = v[3] / 256;
        if (depth > 10) {
            int farIndex = v[2];
            cornerIndices.push_back(farIndex);
        }
    }
    
    //-------------------------------------------------------------------------
    // 4) 처음과 마지막 포인트 추가.
    cornerIndices.push_back(0);
    cornerIndices.push_back(n-1);
    
    //-------------------------------------------------------------------------
    // 5) 소팅 / 중복제거
    std::sort(cornerIndices.begin(), cornerIndices.end(), std::less<int>());
    auto iter = std::unique(cornerIndices.begin(), cornerIndices.end());
    cornerIndices.erase(iter, cornerIndices.end());
    
    return cornerIndices;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Polygon에서 Noise 제거
//-------------------------------------------------------------------------
void CutoutProcessor::simplifyShape(cv::Mat& mask, int width, int height, double epsilon, float minArea) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
    cv::Mat erordeElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(2, 2));
    cv::erode(mask, mask, erordeElement);
    
    
    // 외곽선 검출
    cv::findContours(mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) {
        return;
    }
    
    // 검출된 외곽선 정리
    std::vector<std::vector<cv::Point>> approx(contours.size());
    
    static const int   smoothSteps = 4;
    static const float smoothAmount = 0.5;
    
    std::vector<float> weights(smoothSteps);
    for (int i = 1; i < smoothSteps; i++) {
        weights[i] = 1.0 + (smoothAmount - 1.0) * i / smoothSteps;
    }
    
    for (int i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        
        if (area < minArea) {
            // 크기가 작은 영역은 제거
            contours[i].resize(1);
            approx[i] = contours[i];
            
        } else {
            // 외곽선 Noise 정리
            
            // 코너 인덱스 찾는다.
            auto peakIndices = findCornerElements(contours[i]);
            int start = 0;
            int end = 0;
            
            for (int index: peakIndices) {
                end = index;
                if (start == end) {
                    continue;
                }
                
                // 코너 시작점 추가
                approx[i].push_back(contours[i][start]);
                
                // 다음 시작점까지 세그먼트 분리해서 Smooth처리
                auto segment = getSmoothSegment(contours[i], 4, start, end, weights.data());
                approx[i].insert(approx[i].end(), segment.begin(), segment.end());
                
                start = end;
            }
            
            // 남은 포인트 추가
            if (end < contours[i].size()-1) {
                approx[i].insert(approx[i].end(), contours[i].begin()+end, contours[i].end());
            }
            
            // 단순화
            cv::approxPolyDP(cv::Mat(approx[i]), approx[i], 0.1, true);
        }
    }
    
    // 마스크 생성
    mask.setTo(0);
    for (int i = 0 ; i >= 0; i = hierarchy[i][0]) {
        if (approx[i].size() > 1) {
            cv::drawContours(mask, approx, i, cv::Scalar(0xFF), CV_FILLED, CV_AA, hierarchy);
        }
    }
    
    // 마스크 테두리 픽셀 정리
    erordeElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(4, 4));
    cv::erode(mask, mask, erordeElement);
    
    cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(2, 2));
    cv::dilate(mask, mask, dilateElement);
    
    
    // AntiAlise
    cv::blur(mask, mask, cv::Size(1, 1));
}


