//
//  SMImageView.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//

#include "SMImageView.h"
#include "../Util/ViewUtil.h"
#include <cocos2d.h>
#include <algorithm>
#include "ShaderNode.h"
#include "../UI/CircularProgress.h"
#include "../Const/SMViewConstValue.h"

#define FLAG_CONTENT_SIZE   (1<<0)

#define ACTION_TAG_SHOW (SMViewConstValue::Tag::USER+1)
#define ACTION_TAG_DIM  (SMViewConstValue::Tag::USER+2)


const uint8_t SMImageView::GRAVITY_LEFT    = (1<<0);
const uint8_t SMImageView::GRAVITY_RIGHT   = (1<<1);
const uint8_t SMImageView::GRAVITY_CENTER_HORIZONTAL = SMImageView::GRAVITY_LEFT | SMImageView::GRAVITY_RIGHT;

const uint8_t SMImageView::GRAVITY_TOP     = (1<<2);
const uint8_t SMImageView::GRAVITY_BOTTOM  = (1<<3);
const uint8_t SMImageView::GRAVITY_CENTER_VERTICAL = SMImageView::GRAVITY_TOP | SMImageView::GRAVITY_BOTTOM;

class SMImageView::LoadingBg : public cocos2d::Node
{
public:
    static LoadingBg* create(cocos2d::Color3B* iconColor, cocos2d::Color3B* bgColor, const cocos2d::Vec2& iconOffset) {
        LoadingBg* view = new (std::nothrow)LoadingBg(iconColor, bgColor, iconOffset);
        
        if (view != nullptr) {
            if (view->init()) {
                view->autorelease();
            } else {
                CC_SAFE_DELETE(view);
            }
        }
        
        return view;
    }
    
    LoadingBg(cocos2d::Color3B* iconColor, cocos2d::Color3B* bgColor, const cocos2d::Vec2& iconOffset) :
    _icon(nullptr), _iconScale(-1), _baseScale(1.0), _bg(nullptr), _iconColor(iconColor), _bgColor(bgColor), _iconOffset(iconOffset)
    {
    }
    
    virtual bool init() override {
        
        if (_bgColor != nullptr) {
            _bg = ShapeSolidRect::create();
            _bg->setColor(*_bgColor);
            addChild(_bg);
        }
        
        setCascadeOpacityEnabled(true);
        
        return true;
    }
    
    virtual void onEnter() override {
        cocos2d::Node::onEnter();
        if (_icon && _icon->isVisible()) {
            if (dynamic_cast<CircularProgress*>(_icon)) {
                ((CircularProgress*)_icon)->start();
            }
        }
    }
    
    virtual void onExit() override {
        cocos2d::Node::onExit();
        if (_icon && _icon->isVisible()) {
            if (dynamic_cast<CircularProgress*>(_icon)) {
                ((CircularProgress*)_icon)->stop();
            }
        }
    }
    
    void transformBgColor(float t) {
        if (_bgColor) {
            _bg->setColor(ViewUtil::interpolateColor3B(*_bgColor, cocos2d::Color3B::WHITE, t));
        }
    }
    
    void setLoadingIconScale(float iconScale) {
        _iconScale = iconScale;
    }
    
    void showLoadingIcon(bool show) {
        if (_iconColor) {
            if (show) {
                // show
                if (_icon == nullptr || dynamic_cast<CircularProgress*>(_icon) == nullptr) {
                    if (_icon) {
                        removeChild(_icon);
                    }
                    auto progress = CircularProgress::createIndeterminate();
                    addChild(progress);
                    _icon = progress;
                }
                
                auto progress = dynamic_cast<CircularProgress*>(_icon);
                if (progress) {
                    progress->setStartAngle(cocos2d::random(0.0f, 360.0f));
                    progress->start();
                    
                    // 자동 스케일
                    if (_iconScale < 0) {
                        // 자동 스케일
                        float maxSize = _contentSize.width * 0.3;
                        _iconScale = std::min(1.0f, maxSize / 100);
                    }

                    progress->setContentSize(cocos2d::Size(100*_iconScale, 100*_iconScale));
                }
                
                _icon->setColor(*_iconColor);
                _icon->setPosition(_contentSize.width/2+_iconOffset.x, _contentSize.height/2+_iconOffset.y);
                _icon->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
                _icon->setVisible(true);
                _icon->setOpacity(0xFF);
                _icon->setScale(_baseScale);
                
            } else {
                // hide
                auto progress = dynamic_cast<CircularProgress*>(_icon);
                if (progress) {
                    progress->stop();
                }
                
                _icon->setVisible(false);
            }
        }
        
        if (_bg && _bgColor) {
            _bg->setColor(*_bgColor);
        }
    }
    
    virtual void setContentSize(const cocos2d::Size& contentSize) override {
        cocos2d::Node::setContentSize(contentSize);
        if (_icon) {
            _icon->setPosition(contentSize.width/2+_iconOffset.x, contentSize.height/2+_iconOffset.y);
        }
        if (_bg) {
            _bg->setContentSize(contentSize);
        }
    }
    
    cocos2d::Node* getLoadingIcon() {
        return _icon;
    }
    
    void setIconScale(float scale) {
        if (_icon) {
            _icon->setScale(_baseScale*scale);
        }
    }
    
    void startProgress() {
        if (_icon) {
            auto progress = dynamic_cast<CircularProgress*>(_icon);
            if (progress && progress->isVisible()) {
                progress->start();
            }
        }
    }
    
private:
    ShapeSolidRect* _bg;
    
    cocos2d::Node* _icon;
    
    float _iconScale;
    
    float _baseScale;
    
    cocos2d::Vec2 _iconOffset;
    
public:
    cocos2d::Color3B* _iconColor;
    
    cocos2d::Color3B* _bgColor;
};

class SMImageView::ShowImgAction : public cocos2d::ActionInterval {
public:
    static ShowImgAction* create(float duration, cocos2d::Sprite* image, LoadingBg* bg) {
        auto action = new ShowImgAction();
        action->initWithDuration(duration);
        action->autorelease();
        
        action->_image = image;
        action->_bg = bg;
        
        return action;
    }
    
    virtual void update(float t) override {
        if (!_image || !_bg)
            return;
        
        auto icon = _bg->getLoadingIcon();
        
        if (icon && icon->isVisible()) {
            float scale = std::max((1.0-t*3), 0.0);
            _bg->setIconScale(scale);
        }
        if (_bg->_bgColor) {
            auto color = ViewUtil::interpolateColor3B(*_bg->_bgColor, cocos2d::Color3B::WHITE, t);
            _image->setColor(color);
            _bg->transformBgColor(t);
        }
        _image->setOpacity((GLubyte)(0xFF*t));
        
        if (t >= 1) {
            complete();
        }
    }
    
    void complete() {
        if (!_image || !_bg)
            return;
        
        auto icon = _bg->getLoadingIcon();
        if (icon) {
            icon->setOpacity(0xFF);
            icon->setVisible(false);
            
            auto progress = dynamic_cast<CircularProgress*>(icon);
            if (progress) {
                progress->stop();
            }
        }
        _bg->setVisible(false);
        _bg->transformBgColor(0);
        _bg->setOpacity(0xFF);
        _image->setColor(cocos2d::Color3B::WHITE);
        _image->setOpacity(0xFF);
        
        _image = nullptr;
        _bg = nullptr;
    }
    
    void setValue(cocos2d::Sprite* image, LoadingBg* bg) {
        _image = image;
        _bg = bg;
    }
    
private:
    cocos2d::Sprite* _image;
    LoadingBg* _bg;
};


SMImageView::SMImageView() :
_scaleType(ScaleType::FIT_CENTER)
,_clipping(false)
,_sprite(nullptr)
,_gravity(0)
,_maxAreaRatio(0)
,_dlConfig(nullptr)
,_onSpriteLoadedCallback(nullptr)
,_onSpriteSetCallback(nullptr)
,_bgColorSet(true)
,_iconColorSet(true)
,_iconScale(-1)
,_imgShowAction(nullptr)
,_imgActionEnable(true)
,_loadingIconColor(cocos2d::Color3B::BLACK)
,_loadingBgColor(cocos2d::Color3B::WHITE)
,_loadingIconOffset(cocos2d::Vec2::ANCHOR_MIDDLE)
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
, _assetObject(nullptr)
#endif
{
    _isDownloadImageView = false;
}

SMImageView::~SMImageView()
{
    CC_SAFE_DELETE(_dlConfig);
}

void SMImageView::onEnter()
{
    _UIContainerView::onEnter();
    if (_isDownloadImageView) {
        if (getSprite()) {
            getSprite()->setColor(cocos2d::Color3B::WHITE);
            getSprite()->setOpacity(0xFF);
        } else {
            if (_isLocalFilePath) {
            #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//                ImageDownloader::getInstance().loadThumbnailFromLocalIdentifier(this, _imgPath, 0, _dlConfig);
                if (_assetObject) {
                    ImageDownloader::getInstance().loadImagePHAssetThumbnail(this, _assetObject, 0, _dlConfig);
                }
                
            #elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                // android
                ImageDownloader::getInstance().loadImageFromAndroidThumbnail(this, _imgPath, 0, _dlConfig);
            #endif
            } else {
                ImageDownloader::getInstance().loadImageFromNetwork(this, _imgPath, 0, _dlConfig);
            }
        }
    }
}

void SMImageView::onExit()
{
    _UIContainerView::onExit();
    
    if (_isDownloadImageView) {
        resetDownload();
        setColor(cocos2d::Color3B::WHITE);
        setOpacity(0xFF);
        setSprite(nullptr);
    }
}

void SMImageView::setDownloadConfig(const DownloadConfig &config)
{
    if (_dlConfig==nullptr) {
        _dlConfig = new DownloadConfig();
    }
    *_dlConfig = config;
}

void SMImageView::setContentSize(const cocos2d::Size &size)
{
    _UIContainerView::setContentSize(size);
    registerUpdate(FLAG_CONTENT_SIZE);
}

void SMImageView::setSprite(cocos2d::Sprite* sprite, bool fitToSpriteSize)
{
    if (_sprite!=sprite) {
        // new sprite?
        if (_sprite!=nullptr) {
            _uiContainer->removeChild(_sprite);
        }
        
        if (sprite!=nullptr) {
            _uiContainer->addChild(sprite);
            _sprite = sprite;
            _rect = _sprite->getTextureRect();
            
            registerUpdate(FLAG_CONTENT_SIZE);
        } else {
            _sprite = nullptr;
        }
    }
    
    if (fitToSpriteSize && _sprite!=nullptr) {
        _sprite->setScale(1.0f);
        setContentSize(_sprite->getContentSize());
    }
    
    if (_onSpriteSetCallback != nullptr) {
        _onSpriteSetCallback(this, sprite);
    }    
}

void SMImageView::setScaleType(SMImageView::ScaleType scaleType)
{
    if (_scaleType!=scaleType) {
        // new scale type
        _scaleType = scaleType;
        registerUpdate(FLAG_CONTENT_SIZE);
    }
}

void SMImageView::enableClipping(bool clipEnable)
{
    if (_clipping==clipEnable) {
        return;
    }
    
    _clipping = clipEnable;
    if (_sprite!=nullptr) {
        registerUpdate(FLAG_CONTENT_SIZE);
    }
}

void SMImageView::onUpdateOnVisit()
{
    if (isUpdate(FLAG_CONTENT_SIZE)) {
        computeContentSize();
        unregisterUpdate(FLAG_CONTENT_SIZE);
    }
}

void SMImageView::setContentGravity(uint8_t gravity)
{
    if (_gravity!=gravity) {
        _gravity = gravity;
        registerUpdate(FLAG_CONTENT_SIZE);
    }
}

void SMImageView::updateData()
{
    _rect = _sprite->getTextureRect();
    computeContentSize();
}

void SMImageView::computeContentSize()
{
    if (_sprite==nullptr) {
        return;
    }
    
    cocos2d::Size vsize = _uiContainer->getContentSize();   // 이미지 뷰 크기
    
    if (vsize.width<=0 || vsize.height<=0) {
        return;
    }
    
    cocos2d::Size ssize = _rect.size;   // 이미지(스프라이트) 크기
    if (ssize.width<=0 || ssize.height<=0) {
        return;
    }
    
    float scaleX, scaleY;
    
    switch (_scaleType) {
        case ScaleType::CENTER:
        {
            scaleX = scaleY = 1.0f; // 본래 크기대로 가운데
        }
            break;
        case ScaleType::CENTER_INSIDE:
        {
            scaleX = scaleY = std::min(1.0f, std::min(vsize.width/ssize.width, vsize.height/ssize.height)); // 작은 쪽 비율로 가운데 정렬 1보다 클경우 1로
        }
            break;
        case ScaleType::CENTER_CROP:
        {
            scaleX = scaleY = std::max(vsize.width/ssize.width, vsize.height/ssize.height); // 큰쪽 비율로 정렬... 그리고 clip하면 됨.
        }
            break;
        case ScaleType::FIT_XY:
        {
            // 가로 세로 각각 비율로 줄임. 이미지 뷰 크기에 때려 맞춤.
            scaleX = vsize.width/ssize.width;
            scaleY = vsize.height/ssize.height;
        }
            break;
        default:
        {
            scaleX = scaleY = std::min(vsize.width/ssize.width, vsize.height/ssize.height); // 작은 쪽 비율로 맞춤.  이미지 뷰 크기에 fit 됨.
        }
            break;
    }
    
    float sw = ssize.width * scaleX;
    float sh = ssize.height * scaleY;
    
    cocos2d::Vec2 origin(vsize.width/2-sw/2, vsize.height/2-sh/2);  // 이미지 뷰 안에 이미지의 센터 정렬 좌표
    
    if (_gravity) {
        if (_gravity & GRAVITY_CENTER_HORIZONTAL) {
            // 가로 정렬
            if (_gravity & GRAVITY_LEFT && !(_gravity & GRAVITY_RIGHT)) {
                // 왼쪽 정렬이냐
                origin.x = 0;
            } else if (_gravity & GRAVITY_RIGHT && !(_gravity & GRAVITY_LEFT)) {
                // 오른쪽 정렬이냐
                origin.x = vsize.width - sw;
            }
        }
        
        if (_gravity & GRAVITY_CENTER_VERTICAL) {
            // 세로 정렬
            if (_gravity & GRAVITY_TOP && !(_gravity & GRAVITY_BOTTOM)) {
                // 위쪽 정렬이냐
                // 여기에서는 좌측 하단이 기본좌표이므로 상단은 0이 아닌 vsize.height가 됨
                origin.y = vsize.height - sh;
            } else if (_gravity & GRAVITY_BOTTOM && !(_gravity & GRAVITY_TOP)) {
                // 아래쪽 정렬이냐
                // 여기에서는 좌측 하단이 기본좌표이므로 하단은 vsize.height가 아닌 0이 됨
                origin.y = 0;
            }
        }
    }
    
    if (_clipping & (sw > vsize.width || sh > vsize.height)) {
        // scale 적용된 이미지가 뷰 크기를 넘어선다면 넘어서는 부분은 잘라낸다.
        cocos2d::Rect dsRect(origin.x, origin.y, sw, sh);
        float cl=0, ct=0, cr=0, cb=0;
        
        // 왼쪽 벗어나는 지점
        if (dsRect.getMinX()<0) {
            float clipSize = -dsRect.getMinX();
            cl = clipSize / scaleX;
            origin.x += clipSize;
        }
        
        // 오른쪽 벗어나는 지점
        if (dsRect.getMaxX()>vsize.width) {
            cr = (dsRect.getMaxX() - vsize.width) / scaleX;
        }
        
        // 위쪽 벗어나는 지점
        if (dsRect.getMinY() < 0) {
            float clipSize = -dsRect.getMinY();
            cb = clipSize / scaleY;
            origin.y += clipSize;
        }
        
        // 아래쪽 벗어나는 지점
        if (dsRect.getMaxY() > vsize.height) {
            ct = (dsRect.getMaxY() - vsize.height) / scaleY;
        }
 
        cocos2d::Rect clipRect(_rect.origin.x + cl,
                                            _rect.origin.y + ct,
                                            _rect.size.width - cl - cr,
                                       _rect.size.height - ct - cb);
        
        _sprite->setTextureRect(clipRect);
    } else {
        auto polyInfo = _sprite->getPolygonInfo();
        if (polyInfo.getVertCount() <= 6) {
            _sprite->setTextureRect(_rect);
        }
    }
    
    auto anchor = _sprite->getAnchorPoint();
    auto rect = _sprite->getTextureRect();
    
    float w = rect.size.width * scaleX;
    float h = rect.size.height * scaleY;
    
    float x = origin.x + w*anchor.x;
    float y = origin.y + h*anchor.y;
    
    if (_maxAreaRatio>0 && _maxAreaRatio<1) {
        float ratio = (w*h) / (_contentSize.width*_contentSize.height);
        if (ratio > _maxAreaRatio) {
            float newScale = _maxAreaRatio / ratio;
            scaleX *= newScale;
            scaleY *= newScale;
        }
    }
    
    _sprite->setPosition(x, y);
    _sprite->setScale(scaleX, scaleY);
}

void SMImageView::setImagePath(const std::string &imagePath, bool immediate, OnImageLoadCompleteCallback callback)
{
    _onImageLoadCompleteCallback = callback;
    
    if (imagePath=="") {
        setSprite(nullptr);
        return;
    }
    _isDownloadImageView = true;
    _isLocalFilePath = false;
    _imgPath = imagePath;

    if (immediate) {
        ImageDownloader::getInstance().loadImageFromNetwork(this, _imgPath, 0, _dlConfig);
    }
}
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
void SMImageView::setNSUrlPathForIOS(const std::string &absoluteString, bool smallThumbnail)
{
    _isDownloadImageView = true;
    _isLocalFilePath = true;
    _imgPath = absoluteString;
    
    if (smallThumbnail) {
        DownloadConfig config = ImageDownloader::DC_IMAGE_ONLY;
        config.setSmallThumbnail();
        // deprecated
//        ImageDownloader::getInstance().loadThumbnailFromNSURL(this, _imgPath, 0, &config);
        ImageDownloader::getInstance().loadThumbnailFromLocalIdentifier(this, _imgPath, 0, &config);
    } else {
        ImageDownloader::getInstance().loadThumbnailFromLocalIdentifier(this, _imgPath, 0, _dlConfig);
    }
}

void SMImageView::setPHAssetObject(void* phAssetObject, bool isThumbnail) {
    _assetObject = phAssetObject;
    _isDownloadImageView = true;
    _isLocalFilePath = true;
    
    if (isThumbnail) {
        ImageDownloader::getInstance().loadImagePHAssetThumbnail(this, phAssetObject, 0, _dlConfig);
    } else {
        ImageDownloader::getInstance().loadImagePHAsset(this, phAssetObject, 0, _dlConfig);
    }
}

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
void SMImageView::setImageFilePath(const std::string &imagePath, bool bThumb)
{
    _isDownloadImageView = true;
    _isLocalFilePath = true;
    _imgPath = imagePath;
    if (bThumb) {
        ImageDownloader::getInstance().loadImageFromAndroidThumbnail(this, _imgPath, 0, _dlConfig);
    } else {
        ImageDownloader::getInstance().loadImageFromAndroidLocalFile(this, _imgPath, 0, _dlConfig);
    }

}
#endif


SMImageView::LoadingBg* SMImageView::showBgNode(bool progress)
{
    LoadingBg* bg = (LoadingBg*)getBackgroundNode();
    
    if (bg == nullptr) {
        cocos2d::Color3B* iconColor = nullptr;
        cocos2d::Color3B* bgColor = nullptr;
        if (_iconColorSet) {
            iconColor = &_loadingIconColor;
        }
        if (_bgColorSet) {
            bgColor = &_loadingBgColor;
        }
        
        bg = LoadingBg::create(iconColor, bgColor, _loadingIconOffset);
        bg->setContentSize(_contentSize);
        bg->setLoadingIconScale(_iconScale);
        setBackgroundNode(bg);
    }
    bg->setVisible(true);
    bg->setOpacity(0xFF);
    auto tmp = dynamic_cast<LoadingBg*>(bg);
    if (tmp) {
    bg->showLoadingIcon(progress);
    } else {
        CCLOG("[[[[[[[[[[[[[ what ??????");
    }
    setBackgroundColor4F(cocos2d::Color4F::WHITE);
    
    return bg;
}

void SMImageView::onImageLoadComplete(cocos2d::Sprite* sprite, int tag, bool direct)
{
    if (sprite==nullptr) {
        // 오류 났네..
        if (_bgNode && _bgNode->isVisible()) {
            ((LoadingBg*)_bgNode)->showLoadingIcon(false);
        }
        
        if (_onSpriteLoadedCallback != nullptr) {
            _onSpriteLoadedCallback(this, nullptr);
        }
        return;
    }
    
    if (_onSpriteLoadedCallback) {
        // 이거는 받은 sprite를 따로 조작했을 경우.... 바꿔치기 위하여...
        auto newSprite = _onSpriteLoadedCallback(this, sprite);
        if (newSprite && newSprite!=sprite) {
            sprite = newSprite;
        }
    }

    auto action = getActionByTag(ACTION_TAG_SHOW);
    if (action) {
        stopAction(_imgShowAction);
    }    

    setSprite(sprite);
    
    if (_onImageLoadCompleteCallback!=nullptr) {
        _onImageLoadCompleteCallback(sprite);
    }
    
    cocos2d::Vec2 p1 = convertToWorldSpace(cocos2d::Vec2::ZERO);
    cocos2d::Vec2 p2 = convertToWorldSpace(cocos2d::Vec2(_contentSize.width, _contentSize.height));
    cocos2d::Size s = _director->getWinSize();

    if (!_imgActionEnable || p1.y > s.height || p2.y < 0 || p1.x > s.width || p2.x < 0) {
        // 현재 cell은 화면 밖에 있음. => 바로 세팅
        if (getBackgroundNode()) {
            LoadingBg* bg = (LoadingBg*)getBackgroundNode();
            bg->setVisible(false);
        }
        setSprite(sprite);
        return;
    }
    
    // 화면 안에서 로딩되었으니... Fade in 같은 애니메이션 하면 좋을 듯...
    switch (_startState) {
        case DownloadProtocol::DownloadStartState::DOWNLOAD:
        case DownloadProtocol::DownloadStartState::MEM_CACHE:
        {
            // 새로 받았거나 메모리 캐시에서 받아온거...라면 애니메이션 할까?
            if (_imgShowAction == nullptr) {
                _imgShowAction = ShowImgAction::create(0.15, sprite, (LoadingBg*)getBackgroundNode());
                _imgShowAction->setTag(ACTION_TAG_SHOW);
                _imgShowAction->retain();
            } else {
                auto action = getActionByTag(ACTION_TAG_SHOW);
                if (action) {
                    stopAction(_imgShowAction);
                }
            }
            _imgShowAction->setValue(sprite, (LoadingBg*)_bgNode);
            runAction(_imgShowAction);
            
            sprite->setOpacity(0);
        }
            break;
        case DownloadProtocol::DownloadStartState::IMAGE_CACHE:
        {
            // 여긴 방금 받았던거... 이미지 캐시에서 받은거라 방금전까지 있었던거... 라면 애니메이션 안하겠지?
            // 즉시 세팅
            if (getBackgroundNode()) {
                LoadingBg* bg = (LoadingBg*)getBackgroundNode();
                bg->setVisible(false);
            }
            sprite->setOpacity(0xFF);
            sprite->setVisible(true);
        }
            break;
        default:
            break;
    }
}

void SMImageView::onImageLoadStart(DownloadProtocol::DownloadStartState state)
{
    _startState = state;
    
    // To Do.
    // 상태에 따라 background나 progress view를 보여주자...
    switch (state) {
        case DownloadProtocol::DownloadStartState::DOWNLOAD:
            // 다운로드 시작 -> 완료까지 시간 걸림. 프로그래스 필요
            showBgNode(true);
            break;
        case DownloadProtocol::DownloadStartState::MEM_CACHE:
            // 디코드 시작 -> 금방 완료됨. 로딩 배경만
            showBgNode(true);
            break;
        case DownloadProtocol::DownloadStartState::IMAGE_CACHE:
            // 바로 완료됨 -> 이미 완료됨. 로딩배경 필요없음
            if (getBackgroundNode()) {
                getBackgroundNode()->setVisible(false);
            }
            break;
    }
}

void SMImageView::setLoadingIconOffset(const cocos2d::Vec2& offset) {
    _loadingIconOffset = offset;
}

void SMImageView::setLoadingIconColor(const cocos2d::Color3B& iconColor) {
    if (iconColor == cocos2d::Color3B::BLACK) {
        _iconColorSet = false;
    } else {
        _iconColorSet = true;
        _loadingIconColor = iconColor;
    }
}

void SMImageView::setLoadingBgColor(const cocos2d::Color3B& bgColor) {
    if (bgColor == cocos2d::Color3B::BLACK) {
        _bgColorSet = false;
    } else {
        _bgColorSet = true;
        _loadingBgColor = bgColor;
    }
}

