//
//  MaskSprite.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#ifndef MaskSprite_h
#define MaskSprite_h

#include <cocos2d.h>
#include <vector>

class NanoWrapper;

class MaskSprite : public cocos2d::Sprite
{
public:
    static MaskSprite * createWithTexture(cocos2d::Texture2D * texture, uint8_t* maskData = nullptr, int maskStride=0);
    
    void enableMaskDraw(bool enabled);
    
    void clearMask(GLubyte clearValue=0xFF);
    
    void flush();
    
    //
    void addFill(const std::vector<cocos2d::Vec2>& points);
    void subFill(const std::vector<cocos2d::Vec2>& points);
    
    void addStroke(const std::vector<cocos2d::Vec2>& points, float strokeSize);
    void subStroke(const std::vector<cocos2d::Vec2>& points, float strokeSize);
    
    cocos2d::Sprite* cropSprite(const cocos2d::Rect& cropRect, cocos2d::Rect& outRect);
    bool cropData(const cocos2d::Rect& cropRect, uint8_t** outPixelData, uint8_t** outMaskData, cocos2d::Rect& outRect);
    cocos2d::Sprite* extractSprite(const cocos2d::Rect& cropRect, cocos2d::Rect& outRect, float& outScale, int padding);
    
    
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    
    void readPixels(uint8_t** pixelData);
    
    uint8_t* getMaskData();
    uint8_t* cloneMaskData();
    
    int getMaskPixelsWide();
    
    void updateMask(uint8_t* maskData=nullptr);
    
    void setDrawColor(const cocos2d::Color4F& strokeColor, const cocos2d::Color4F& fillColor);
    
protected:
    MaskSprite();
    virtual ~MaskSprite();
    
    bool initWithMask(cocos2d::Texture2D* texture, uint8_t* maskData, int maskStride);
    
private:
    void initRenderTexture();
    void releaseRenderTexture();
    
    void onDrawMask();
    void onUpdateMask(uint8_t** pixelData, int operation);
    
private:
    enum Command {
        ADD_FILL,
        SUB_FILL,
        ADD_STROKE,
        SUB_STROKE,
    };
    
    struct CommandItem {
        Command command;
        float strokeSize;
        std::vector<cocos2d::Vec2> points;
        
        CommandItem(Command command, const std::vector<cocos2d::Vec2>& points, float strokeSize) {
            this->command = command;
            this->strokeSize = strokeSize;
            this->points = points;
        };
    };
    
    cocos2d::Texture2D* _maskTexture;
    
    uint8_t* _maskData;
    
    int _maskPixelsWide;
    int _maskPixelsHigh;
    
    NanoWrapper* _nano;
    cocos2d::RenderTexture* _renderTexture;
    cocos2d::CustomCommand* _renderCommand;
    
    std::vector<CommandItem> _commandQueue;
    
    bool _dirtyDraw;
    
    bool _flushDraw;
    
    int _operation;
    
    cocos2d::Vec2 _maskAlign;
    
    int _uniformDrawTexture;
    int _uniformMaskTexture;
    int _uniformMaskAlign;
    int _uniformOperation;
    cocos2d::Color4F _strokeColor;
    cocos2d::Color4F _fillColor;
};

#endif /* MaskSprite_h */
