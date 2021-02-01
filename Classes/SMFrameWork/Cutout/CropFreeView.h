//
//  CropFreeView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#ifndef CropFreeView_h
#define CropFreeView_h

#include "BaseCropView.h"
#include <vector>
#include <functional>

class ShapeSolidCircle;

class CropFreeView : public BaseCropView, public OnTouchListener
{
public:
    static CropFreeView* create(cocos2d::Sprite* sprite, const cocos2d::Rect& boundRect);
    std::function<void(bool)> onSelectCallback;
    
protected:
    CropFreeView();
    
    virtual bool init(cocos2d::Sprite* sprite, const cocos2d::Rect& boundRect) override;
    
    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    virtual cocos2d::Sprite* cropSpriteInternal(cocos2d::Rect& outRect) override;
    
    virtual bool cropDataInternal(uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect) override;
    
private:
    // like draw
    std::vector<cocos2d::Vec2> getSmoothPoints(const std::vector<cocos2d::Vec2>& contour, int start, int end);
    
    std::vector<cocos2d::Vec2> getSlicePoints(const std::vector<cocos2d::Vec2>& contour, float resolution);
    
    void drawDots(const std::vector<cocos2d::Vec2>& points);
    
    void reset();
    
private:
    int _commandCursor;
    
    bool _pointAdded;

    // draw type
    enum class Mode {
        ADD_FILL = 0,
        SUB_FILL,
        ADD_STROKE,
        SUB_STROKE
    };

    // like MaksSprite CommandItem
    struct DrawCommand {
        Mode mode;
        
        float strokeSize;
        
        std::vector<cocos2d::Vec2> points;
        
        DrawCommand(Mode mode, const std::vector<cocos2d::Vec2>& points, float strokeSize) {
            this->mode = mode;
            this->points = points;
            this->strokeSize = strokeSize;
        }
    };
    
    std::vector<DrawCommand> _command;
    
    std::vector<float> _smoothWeights;
    
    std::vector<cocos2d::Vec2> _points;
    
    // 그려지는 패턴은 점으로 하자... 그려서 따는거니까..
    std::vector<ShapeSolidCircle*> _dots;
    
    cocos2d::Rect _cropBounds;
};


#endif /* CropFreeView_h */
