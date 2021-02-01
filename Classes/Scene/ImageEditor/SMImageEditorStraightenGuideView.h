//
//  SMImageEditorStraightenGuideView.h
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#ifndef SMImageEditorStraightenGuideView_h
#define SMImageEditorStraightenGuideView_h

#include "../../SMFrameWork/Base/SMView.h"
#include <cocos2d.h>


class SMImageEditorStraightenGuideView : public SMView
{
public:
    SMImageEditorStraightenGuideView();
    virtual ~SMImageEditorStraightenGuideView();
    
    static SMImageEditorStraightenGuideView * create(int tag, float x, float y, float width, float height, cocos2d::Rect rect);
    
    cocos2d::Rect imageRect;
    cocos2d::Rect straightenRect;
    
protected:
    virtual bool init() override;
    
    float lineAlpha;
    
    cocos2d::DrawNode * _outLine;
    
    cocos2d::DrawNode * _guideLine1;
    cocos2d::DrawNode * _guideLine2;
    cocos2d::DrawNode * _guideLine3;
    cocos2d::DrawNode * _guideLine4;
    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
    
};

#endif /* SMImageEditorStraightenGuideView_h */
