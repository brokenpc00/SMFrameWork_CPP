//
//  DrawCropView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 29..
//

#ifndef DrawCropView_h
#define DrawCropView_h

#include "../Base/SMView.h"
#include "../Base/MaskSprite.h"
#include "../ImageProcess/ImageProcessor.h"
#include "../Base/Intent.h"

class ShapeSolidCircle;

class DrawCropView : public SMView, public OnTouchListener, public ImageProcessProtocol
{
public:
    static DrawCropView * create(cocos2d::Sprite* sprite, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect);
    static DrawCropView * create(uint8_t* pixelData, const cocos2d::Size& imageSize, uint8_t* maskData, int maskPixelsWide, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect);
    
    std::function<void(bool)> onSelectCallback;

    float getSpriteScale();
    cocos2d::Rect getBoundRect();
    cocos2d::Sprite * cropSprite(cocos2d::Rect& outRect);
    bool cropData(uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect);
    
//    void grabcut(std::function<void(uint8_t* newMaskData)> callback=nullptr);
    void grabcut(uint8_t* pixelData, const cocos2d::Size& imageSize, uint8_t* maskData, int maskPixelsWide, std::function<void(uint8_t* newMaskData)> callback=nullptr);
    
    cocos2d::Sprite* extractSprite();
    
    const cocos2d::Rect getCropBound() {return _cropBounds;}
    void setInitCropBound();
    
protected:
    DrawCropView();
    virtual ~DrawCropView();
    bool init(cocos2d::Sprite* sprite, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect);
    bool init(uint8_t* pixelData, const cocos2d::Size& imageSize, uint8_t* maskData, int maskPixelsWide, const cocos2d::Rect& canvasRect, const cocos2d::Rect& boundRect);
    
    // touch listener
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;

    // image process callback
    virtual void onImageProcessComplete(const int tag, const bool success, cocos2d::Sprite* sprite, Intent* result) override;

    
protected:
    MaskSprite* _maskSprite;
    cocos2d::Rect _canvasRect;
    cocos2d::Rect _boundRect;
    cocos2d::Size _textureSize;
    cocos2d::Rect _textureRect;
    
private:
    std::vector<cocos2d::Vec2> getSmoothPoints(const std::vector<cocos2d::Vec2>& contour, int start, int end);
    std::vector<cocos2d::Vec2> getSlicePoints(const std::vector<cocos2d::Vec2>& contour, float resolution);
    void drawDots(const std::vector<cocos2d::Vec2>& points);
    void reset();
    
private:
    bool _pointAdded;
    enum class Mode {
        ADD_FILL = 0,
        SUB_FILL,
        ADD_STROKE,
        SUB_STROKE
    };
    
    std::vector<float> _smoothWeights;
    std::vector<cocos2d::Vec2> _points;
    std::vector<ShapeSolidCircle*> _dots;
    cocos2d::Rect _cropBounds;
    
    // draw bg (점 붙이는 뷰)
    SMView* _uiLayer;
    std::function<void(uint8_t*)> grabcutCallback;
};



#endif /* DrawCropView_h */
