//
//  DashedRoundRect.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#ifndef DashedRoundRect_h
#define DashedRoundRect_h

#include <2d/CCSprite.h>


class DashedRoundRect : public cocos2d::Sprite
{
public:
    DashedRoundRect();
    virtual ~DashedRoundRect();
    
    static DashedRoundRect* create(const std::string& dashedImageFileName="") {
        DashedRoundRect * dashedRect = new (std::nothrow)DashedRoundRect();
        std::string fileName = dashedImageFileName;
        if (fileName.length()==0) {
            fileName = "images/dash_line_s.png";
        }
        if (dashedRect && dashedRect->initWithFile(fileName)) {
            dashedRect->initMesh();
            dashedRect->autorelease();
            dashedRect->_bInit = true;
        } else {
            CC_SAFE_DELETE(dashedRect);
        }
        
        return dashedRect;
    };
    
    bool initMesh();
    void setLineWidth(const float lineWidth);
    void setCornerRadius(const float radius);
    virtual void setContentSize(const cocos2d::Size& size) override;
    
protected:
    void applyMesh();
private:
    float _thickness;
    float _cornerRadius;
    bool _bInit;
    cocos2d::TrianglesCommand::Triangles _triangles;
};


#endif /* DashedRoundRect_h */
