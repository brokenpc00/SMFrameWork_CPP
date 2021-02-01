//
//  DrawView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#ifndef DrawView_h
#define DrawView_h

#include "../Base/SMView.h"
#include <cocos2d.h>

class MaskSprite;
class SMZoomView;
class NanoDrawNode;

class DrawView : public SMView, public OnTouchListener
{
public:
    CREATE_VIEW(DrawView);

    cocos2d::Sprite * getCaptureImage();
    
    void setDrawColor(const cocos2d::Color3B& color);
    
    void clearCanvas();
    
protected:
    DrawView();
    virtual ~DrawView();
    
    virtual bool init() override;

    virtual int onTouch(SMView* view, const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    std::vector<cocos2d::Vec2> getSmoothPoints(const std::vector<cocos2d::Vec2>& contour, int start, int end);
private:
    SMView * _contentView;
    
    bool _touchFocused;
    
    cocos2d::Color3B _strokeColor;
    
    std::vector<float> _smoothWeights;

    std::vector<cocos2d::Vec2> _points;
    
    std::vector<NanoDrawNode*> _drawNodes;
    
    bool _pointAdded;
};



#endif /* DrawView_h */
