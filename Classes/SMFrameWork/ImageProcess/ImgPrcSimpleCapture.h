//
//  ImgPrcSimpleCapture.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 5..
//
//

#ifndef ImgPrcSimpleCapture_h
#define ImgPrcSimpleCapture_h

#include "ImageProcessFunction.h"

class ImgPrcSimpleCapture : public ImageProcessFunction {
public:
    ImgPrcSimpleCapture(float scale=1.0f)
    {
        setCaptureOnly();
        _scale = scale;
    }
    
    virtual bool onPreProcess(cocos2d::Node* node) override {
        auto canvasSize = node->getContentSize();
        return startProcess(node, canvasSize, canvasSize/2, cocos2d::Vec2::ANCHOR_MIDDLE, _scale, _scale);
    }
    
    virtual bool onProcessInBackground() override {
        // capture only
        return true;
    }
    
private:
    float _scale;
};

#endif /* ImgPrcSimpleCapture_h */
