//
//  PulsActionView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 6..
//
// Circle Puls Action View

#ifndef PulsActionView_h
#define PulsActionView_h

#include "../Base/SMView.h"
#include "../Base/ShaderNode.h"
#include <2d/CCActionInterval.h>


class PulsActionView : public SMView
{
public:
    static PulsActionView* puls(SMView * baseView, cocos2d::Node* parent, float x, float y);
    
    virtual void onExit() override;
    
    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t flags) override;
    
    void disconnect();
    
protected:
    PulsActionView();
    virtual ~PulsActionView();
    
    virtual bool initWithParam(SMView* baseView, cocos2d::Node* parent, float x, float y);
    
private:
    bool _removeSelfOnExit;
    SMView * _baseView;
    
    cocos2d::Action* _actionSequence;
};

#endif /* PulsActionView_h */
