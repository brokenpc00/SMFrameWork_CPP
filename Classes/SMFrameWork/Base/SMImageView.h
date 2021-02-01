//
//  SMImageView.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#ifndef SMImageView_h
#define SMImageView_h

#include "SMView.h"
#include <2d/CCSprite.h>
#include <renderer/CCTexture2D.h>
#include <platform/CCImage.h>
#include "../Util/ImageDownloader.h"
#include "../Util/DownloadProtocol.h"
#include <string>

class SMImageView;

typedef std::function<void(cocos2d::Sprite * sprite)> OnImageLoadCompleteCallback;

class SMImageView : public _UIContainerView, public DownloadProtocol
{
public:
    static const uint8_t GRAVITY_LEFT;  // 좌측으로 붙인다.
    static const uint8_t GRAVITY_CENTER_HORIZONTAL; // 좌우 가운데
    static const uint8_t GRAVITY_RIGHT; // 우측으로 붙인다.

    static const uint8_t GRAVITY_TOP;   // 위로 붙인다.
    static const uint8_t GRAVITY_CENTER_VERTICAL;   // 상하 가운데
    static const uint8_t GRAVITY_BOTTOM;    // 아래로 붙인다.
    
    // lt, lc, lb, ct, cc, cb, rt, rc, rb
    
    // setCotentGravity(SMImageView::GRAVITY_LEFT | SMImageView::GRAVITY_CENTER_VERTICAL);
    
    enum class ScaleType {
        CENTER, // 원본 가운데
        CENTER_INSIDE, // 가운데 안쪽으로
        CENTER_CROP, // 가운데 정렬하고 Clipping
        FIT_XY, // 뷰 크기에 비율 상관 없이 때려 맞춤.
        FIT_CENTER  // 비율을 유지하고 뷰 크기에 맞추면서 가운데 정렬... 이거를 기본값으로 하자
    };
    
    static SMImageView * createWithSpriteFileName(const std::string& spriteFileName) {
        SMImageView * imageView = new (std::nothrow)SMImageView();
        if (imageView && imageView->init()) {
            imageView->autorelease();
            imageView->_isDownloadImageView = false;
            cocos2d::Sprite * sprite = cocos2d::Sprite::create(spriteFileName);
            if (sprite!=nullptr) {
                imageView->setSprite(sprite, true);
            }
        } else {
            CC_SAFE_DELETE(imageView);
        }
        
        return imageView;
    }
    
    static SMImageView * create(cocos2d::Sprite * sprite=nullptr) {
        SMImageView * imageView = new (std::nothrow)SMImageView();
        if (imageView && imageView->init()) {
            imageView->autorelease();
            imageView->_isDownloadImageView = false;
            if (sprite!=nullptr) {
                imageView->setSprite(sprite, true);
            }
        } else {
            CC_SAFE_DELETE(imageView);
        }
        
        return imageView;
    }
    
    static SMImageView* create(float x, float y, float width, float height, float anchorX=0.0f, float anchorY=0.0f) {
        SMImageView * imageView =  SMImageView::create();
        if (imageView!=nullptr) {
            imageView->_isDownloadImageView = false;
            imageView->setPosition(x, y);
            imageView->setContentSize(cocos2d::Size(width, height));
            imageView->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
        }
        
        return imageView;
    }
    
    static SMImageView* createWithDownloadImage(const std::string& imagePath, float x, float y, float width, float height, float anchorX=0.5f, float anchorY=0.5f, float iconScale = -1) {
        SMImageView * imageView =  SMImageView::create();
        if (imageView!=nullptr) {
            imageView->_isDownloadImageView = true;
            imageView->setPosition(x, y);
            imageView->setContentSize(cocos2d::Size(width, height));
            imageView->setAnchorPoint(cocos2d::Vec2(anchorX, anchorY));
            imageView->setScaleType(ScaleType::FIT_CENTER);
            imageView->_iconScale = iconScale;
            imageView->setImagePath(imagePath);
        } else {
            CC_SAFE_DELETE(imageView);
        }
        
        return imageView;
    }
    
    virtual void setContentSize(const cocos2d::Size& size) override;
    
    virtual void setScaleType(ScaleType scaleType);
    
    virtual void setSprite(cocos2d::Sprite * sprite, bool fitToSpriteSize=false);
    
    void enableClipping(bool clipEnable);
    
    cocos2d::Sprite * getSprite() {return _sprite;}
    
    void setContentGravity(uint8_t gravity);

    void clearContentGravity() { setContentGravity(0); }

    void updateData();
    
    void setMaxAreaRatio(float ratio) {_maxAreaRatio = ratio;}
    
    virtual void onEnter() override;
    
    virtual void onExit() override;
    
public:
    // for image download
    virtual void onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct) override;
    
    virtual void onImageLoadStart(DownloadProtocol::DownloadStartState state) override;

    void setDownloadConfig(const DownloadConfig& config);
    
//    std::function<void(SMImageView* sender, cocos2d::Sprite* sprite)> _onSpriteLoadedCallback;
    std::function<cocos2d::Sprite*(SMImageView* sender, cocos2d::Sprite* sprite)> _onSpriteLoadedCallback;
    
    std::function<void(SMImageView* sender, cocos2d::Sprite* sprite)> _onSpriteSetCallback;
    
    void setImagePath(const std::string& imagePath, bool loadImmediate = true, OnImageLoadCompleteCallback callback=nullptr);
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    void setNSUrlPathForIOS(const std::string &absoluteString, bool smallThumbnail);
    void setPHAssetObject(void* phAssetObject, bool isThumbnail);
#else
    void setImageFilePath(const std::string& imagePath, bool bThumb=false);
#endif
    
    void setLoadingIconOffset(const cocos2d::Vec2& offset);
    
    void setLoadingIconColor(const cocos2d::Color3B& iconColor);
    
    void setLoadingBgColor(const cocos2d::Color3B& bgColor);
    
    void disableFadeIn() { _imgActionEnable = false; }
    
    float getImageScale() {return _sprite->getScale();}
    
protected:
    SMImageView();
    virtual ~SMImageView();
    
    virtual void onUpdateOnVisit() override;
    
    virtual void computeContentSize();
    
private:
    bool _clipping;

    ScaleType _scaleType;
    
    cocos2d::Sprite* _sprite;
    
    cocos2d::Rect _rect;
    #if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    void* _assetObject;
    #endif
    
    uint8_t _gravity;
    
    float _maxAreaRatio;
    
    bool _isDownloadImageView;
    
    std::string _imgPath;
    
    std::string _nsurlPath;
    
    class LoadingBg;
    class ShowImgAction;
    
    LoadingBg* showBgNode(bool progress);
    ShowImgAction* _imgShowAction;

    bool _imgActionEnable;
    bool _iconColorSet;
    bool _bgColorSet;

    cocos2d::Color3B _loadingIconColor;
    cocos2d::Color3B _loadingBgColor;
    cocos2d::Vec2 _loadingIconOffset;

    float _iconScale;
    
    DownloadProtocol::DownloadStartState _startState;
    
    DownloadConfig* _dlConfig;
    
    bool _isLocalFilePath;
    
    CC_DISALLOW_COPY_AND_ASSIGN(SMImageView);
    
    OnImageLoadCompleteCallback _onImageLoadCompleteCallback;
};

#endif /* SMImageView_h */
