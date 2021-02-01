//
//  CircularProgress.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 1..
//
//  Material CircularProgressView (Android opensource)
// https://github.com/rahatarmanahmed/CircularProgressView



#ifndef CircularProgress_h
#define CircularProgress_h

#include <2d/CCNode.h>
#include <base/ccTypes.h>
#include <2d/CCActionInterval.h>
#include <functional>

// 옆 뚫린 원... 이거를 돌릴거임...
class ShapeArcRing;

class CircularProgress : public cocos2d::Node
{
public:
    static CircularProgress* createDeterminate();
    
    static CircularProgress* createIndeterminate();
    
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    void setLineWidth(const float lineWidth);
    
    float getProgress() { return _currentProgress; }
    
    float getMaxProgress() { return _maxProgress; }
    
    void setProgress(const float progress);
    
    void setMaxProgress(const float maxProgress);
    
    void start();
    
    void stop();
    
    void setStartAngle(const float startAngle);
    
    std::function<void(CircularProgress* sender, float currentProgress, float maxProgress)> _onProgressUpdateCallback;
    
    
protected:
    CircularProgress();
    virtual ~CircularProgress();

    virtual bool init(const bool isIndeterminate);
    
private:
    bool _isIndeterminate;
    bool _started;

    static const float MIN_SWEEP;
    static const float MAX_SWEEP;

    cocos2d::FiniteTimeAction* createIndeterminateAction(const int step);

    class FrontEndExtendAction;
    class BackEndRetractAction;
    class StartRotateAction;
    class ProgressAction;
    
    float _indeterminateRotateOffset;
    float _indeterminateSweep;
    float _startAngle;
    
    float _currentProgress;
    float _maxProgress;
    float _actualProgress;
    float _initialStartAngle;
    
    bool _autoLineWidth;
    
    void drawArc(float startAngle, float sweepAngle);
    
    ShapeArcRing* _circle;
    ProgressAction* _progressAction;
    StartRotateAction* _startRoteteAction;
    cocos2d::Action* _indeterminateAction;
};

#endif /* CircularProgress_h */
