//
//  TransitionView.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 28..
//

#ifndef TransitionView_h
#define TransitionView_h

#include "../Base/SMView.h"

class SMImageView;

class TransitionView : public SMView
{
public:
    static TransitionView* create(const cocos2d::Size& imageSize, SMView* thumbView, const cocos2d::Rect& editRect, bool initThumbnail);
    
protected:
    bool init(const cocos2d::Size& imageSize, SMView* thumbView, const cocos2d::Rect& editRect, bool initThumbnail);
    
public:
    void setImageSprite(cocos2d::Sprite* sprite);
    
    void startTransitionIn();
    
    void startTrnasitionOut();
    
    void fadeToImage();
    
    cocos2d::Sprite* getSprite();
    
    cocos2d::Rect getImageBounds() {return _screenBounds;}
    
private:
    SMView * _thumbView;
    
    SMImageView * _imageView;
    
    cocos2d::Vec2 _fromPos, _toPos;
    
    float _fromScale, _toScale;
    
    cocos2d::Rect _screenBounds;
};


#endif /* TransitionView_h */
