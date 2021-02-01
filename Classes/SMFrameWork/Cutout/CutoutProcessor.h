//
//  CutoutProcessor.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#ifndef CutoutProcessor_h
#define CutoutProcessor_h

#include <math/Vec2.h>
#include "../Util/use_opencv.h"
#include <vector>

class CutoutProcessor {
public:

    CutoutProcessor(uint8_t* pixelData, int width, int height, int bpp, uint8_t* maskData, int maskPixelWide);
    ~CutoutProcessor();

    uint8_t* cutout();
    uint8_t* cutout2();
    void release();
    
protected:
    bool init();
    
private:
    bool initFastMask(cv::Mat* alphaMask=nullptr);
    
    cv::Rect getBoundsRect(const std::vector<cocos2d::Vec2>& points, int width, int height);
    cv::Rect getBoundsRect(const cv::Mat& mask, int width, int height);
    
    std::vector<cv::Point> getSmoothSegment(const std::vector<cv::Point>& points, int steps, int start, int end, float* weights);
    
    std::vector<int> findCornerElements(const std::vector<cv::Point>& points);
    
    void simplifyShape(cv::Mat& mask, int width, int height, double epsilon, float minArea);
    
private:
    cv::Mat _srcColorChannel, _srcAlphaChannel;
    cv::Mat _fastMask;
    
    cv::Rect _bounds;
    uint8_t* _imagePixelData;
    
    int _imageWidth;
    int _imageHeight;
    int _imageBpp;
    
    uint8_t* _maskPixelData;
    
    int _maskPixelWide;
    bool _srcHasAlphaChannel;
    int _fastWidth, _fastHeight;
    
    
    
    
};

#endif /* CutoutProcessor_h */
