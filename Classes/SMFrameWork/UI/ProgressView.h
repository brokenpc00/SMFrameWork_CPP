//
//  ProgressView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 1..
//  Progress View... upload나 download 같은거 할때... 진행되는 progress view이다.
//  position을 정하지 않으면 화면 정 가운데 위치한다.

#ifndef ProgressView_h
#define ProgressView_h

#include "../Base/SMView.h"
#include "../UI/CircularProgress.h"

class ShapeCircle;
class ShapeSolidCircle;
class ShapeRoundLine;

class ProgressView : public SMView
{
public:
    static ProgressView * show(cocos2d::Node* parent, float maxProgress = 100.0f,
                               std::function<void(ProgressView* sender)> completeCallback = nullptr,
                               std::function<void(ProgressView* sender, float currentProgress, float maxProgress)> progressCallback=nullptr);
    
    static ProgressView * create(float maxProgress=100.0f);
    
    void hide();
    
    void setOnCompleteCallback(std::function<void(ProgressView* sender)> callback);
    
    void setOnProgressUpdateCallback(std::function<void(ProgressView* sender, float currentProgress, float maxProgress)> callback);
    
    inline void setProgress(const float progress) {_progress->setProgress(progress);}
    
    inline void setMaxProgress(const float maxProgress) {_progress->setMaxProgress(maxProgress);}
    
    inline float getProgress() {return _progress->getProgress();}
    
    inline float getMaxProgress() {return _progress->getMaxProgress();}
    
    inline void setStartAngle(const float startAngle) {return _progress->setStartAngle(startAngle);}
    
    void startShowAction();
    void startDoneAction();
    
    void setWithoutDoneAction(bool without=true) {_withoutDonAction = without;}
    
protected:
    ProgressView();
    virtual ~ProgressView();
    
    virtual bool init() override;
    
private:
    void reset();
    
private:
    std::function<void(ProgressView* sender)> _onCompleteCallback;
    std::function<void(ProgressView* sender, float currentProgress, float maxProgress)> _onProgressUpdateCallback;
    
    void onProgressUpdate(CircularProgress* sender, float currentProgress, float maxProgress);
    
    ShapeCircle* _ring;
    CircularProgress* _progress;
    ShapeSolidCircle* _bgCircle;
    
    // 완료 된후 체크 애니메이션... 양쪽 끝이 라운드처리된 라인으로 그릴거다.
    ShapeRoundLine* _checkLine1;
    ShapeRoundLine* _checkLine2;
    
    class CheckAction;
    class RingExpandAction;
    
    bool _withoutDonAction;
};

#endif /* ProgressView_h */
