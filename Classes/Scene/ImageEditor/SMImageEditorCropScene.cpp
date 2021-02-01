//
//  SMImageEditorCropScene.cpp
//  ePubCheck
//
//  Created by KimSteve on 2017. 4. 19..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorCropScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMZoomView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMTableView.h"
#include "../../SMFrameWork/Base/SMPageView.h"
#include "../../SMFrameWork/Base/SMCircularListView.h"
#include "../../SMFrameWork/Base/SMSlider.h"
#include "../../SMFrameWork/Base/MeshSprite.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Base/ViewAction.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Util/use_opencv.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f

SMImageEditorCropScene::SMImageEditorCropScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _mainImageView(nullptr)
//, _meshSprite(nullptr)
, _originImageSprite(nullptr)
, _imageRect(cocos2d::Rect::ZERO)
, _cropGuideView(nullptr)
, _cropMenuTableView(nullptr)
//, _listener(nullptr)
, _onEditImageResultCallback(nullptr)
//, _tmpImageView(nullptr)
, _mainccImage(nullptr)
, _freeCropView(nullptr)
{

}

SMImageEditorCropScene::~SMImageEditorCropScene()
{
    
}

bool SMImageEditorCropScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
 
    
    auto programBundle = getSceneParam();
    if (programBundle) {
        _mainccImage = (cocos2d::Image*)programBundle->getRef("EDIT_IMAGE");
        if (_mainccImage) {
            auto tmpccImage = new cocos2d::Image;
            tmpccImage->initWithRawData(_mainccImage->getData(), _mainccImage->getDataLen(), _mainccImage->getWidth(), _mainccImage->getHeight(), _mainccImage->getBitPerPixel());
            auto texture = new cocos2d::Texture2D;
            texture->initWithImage(tmpccImage);
            tmpccImage->autorelease();
            
            _originImageSprite = cocos2d::Sprite::createWithTexture(texture);
            texture->autorelease();
        }
    }
    
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    _contentView->setBackgroundColor4F(SMColorConst::COLOR_F_DBDCDF);
    addChild(_contentView);
    
    _topMenuView = SMView::create(0, 0, s.height-TOP_MENU_HEIGHT, s.width, TOP_MENU_HEIGHT);
    _topMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    auto topMenuBottomLine = SMView::create(0, 0, 0, s.width, 1);
    topMenuBottomLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _topMenuView->addChild(topMenuBottomLine);
    _contentView->addChild(_topMenuView);
    topMenuBottomLine->setLocalZOrder(10);
    
    _drawCropButton = SMButton::create(0, SMButton::Style::DEFAULT, s.width/2-TOP_MENU_HEIGHT/2, 0, TOP_MENU_HEIGHT, TOP_MENU_HEIGHT);
    _topMenuView->addChild(_drawCropButton);
    _drawCropButton->setIcon(SMButton::State::NORMAL, "images/modle_cutout_auto.png");
    _drawCropButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
    _drawCropButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xff5635, 1.0f));
//    _drawCropButton->setVisible(false);
    _drawCropButton->setIconScale(2.0f);
    _drawCropButton->setOnClickCallback([this](SMView* view){
        auto s = _director->getWinSize();

        uint8_t* maskData;
        uint8_t* pixelData;
        cocos2d::Rect outRect;
        // 현재 pixel과 mask data를 가져온다.
        bool bUserCrop = true;
        if (_freeCropView->getCropBound().equals(cocos2d::Rect::ZERO)) {
            _freeCropView->setInitCropBound();
            bUserCrop = false;
        }
        _freeCropView->cropData(&pixelData, &maskData, outRect);

        
        if (!bUserCrop) {
            size_t length = outRect.size.width*outRect.size.height;
            maskData = (uint8_t*)malloc(length);
            memset(maskData, 0x00, length);
            
            for (int i=5; i<outRect.size.width-6; i++) {
                for (int j=5; j<outRect.size.height-6; j++) {

                    int index = (outRect.size.height-1)*i + j;
                    maskData[index] = 0xFF;
                }
            }
        }

        // 가져온 data로 grabcut 시도
        _freeCropView->grabcut(pixelData, outRect.size, maskData, outRect.size.width, [this, pixelData, outRect, maskData, bUserCrop](uint8_t* newMaskData){
            // 성공했으면 그걸로 새로운 이미지를
            if (!bUserCrop) {
                free(maskData);
            }
            cocos2d::Image * newImage;
            if (newMaskData) {

                auto image = new cocos2d::Image;
                image->initWithRawData(pixelData, outRect.size.width*outRect.size.height*4, outRect.size.width, outRect.size.height, 32);
                auto texture = new cocos2d::Texture2D;
                texture->initWithImage(image);

                auto maskSprite = MaskSprite::createWithTexture(texture, newMaskData, outRect.size.width);
                float outScale;
                auto size = maskSprite->getTexture()->getContentSizeInPixels();
                cocos2d::Rect newOutRect;
                auto sprite = maskSprite->extractSprite(cocos2d::Rect(cocos2d::Vec2::ZERO, size), newOutRect, outScale, 2);

                if (sprite) {
                    auto rt = cocos2d::RenderTexture::create(outRect.size.width, outRect.size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
                    rt->beginWithClear(0, 0, 0, 0);
                    sprite->setAnchorPoint(cocos2d::Vec2::ZERO);
                    sprite->setPosition(cocos2d::Vec2::ZERO);
                    sprite->visit();
                    rt->end();
                    cocos2d::Director::getInstance()->getRenderer()->render();
                    newImage = rt->newImage();
                } else {
                    newImage = new cocos2d::Image;
                    newImage->initWithRawData(image->getData(), image->getDataLen(), image->getWidth(), image->getHeight(), 32);
                }
                image->release();
                texture->release();

                // for Test
//                if (sprite) {
//                    auto s = _director->getWinSize();
//                    auto layer = _director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP);
//                    auto bgView = SMView::create(0, 0, 0, s.width, s.height);
//                    bgView->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
//                    layer->addChild(bgView);
//
//                    auto imageView = SMImageView::create(sprite);
//                    imageView->setContentSize(s);
//                    bgView->addChild(imageView);
//
//                    bgView->setOnClickCallback([bgView,this](SMView* view){
//                        _director->getScheduler()->performFunctionInCocosThread([bgView, this]{
//                            bgView->removeFromParent();
//                        });
//                    });
//
//                }

            } else {
                newImage = new cocos2d::Image;
                newImage->initWithRawData(_mainccImage->getData(), _mainccImage->getDataLen(), _mainccImage->getWidth(), _mainccImage->getHeight(), 32);
            }
            
            auto intent = Intent::create();
            intent->putRef("EDIT_IMAGE", newImage);
            setSceneResult(intent);
            
            auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
            newImage->autorelease();
        });
    });
    
    auto closeButton = SMButton::create(0, SMButton::Style::DEFAULT, 0, 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    closeButton->setIcon(SMButton::State::NORMAL, "images/popup_close.png");
    closeButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    closeButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    closeButton->setPushDownScale(0.9f);
    _topMenuView->addChild(closeButton);
    closeButton->setOnClickCallback([&](SMView*view){
        auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
        _director->popSceneWithTransition(pScene);
    });
    
    auto applyButton = SMButton::create(0, SMButton::Style::DEFAULT, s.width-(TOP_MENU_HEIGHT-20), 0, TOP_MENU_HEIGHT-20, TOP_MENU_HEIGHT-20);
    applyButton->setIcon(SMButton::State::NORMAL, "images/ic_titlebar_check.png");
    applyButton->setIconColor(SMButton::State::NORMAL, cocos2d::Color4F(0x22/255.0f, 0x22/255.0f, 0x22/255.0f, 1.0f));
    applyButton->setIconColor(SMButton::State::PRESSED, cocos2d::Color4F(0x99/255.0f, 0x99/255.0f, 0x99/255.0f, 1.0f));
    applyButton->setPushDownScale(0.9f);
    _topMenuView->addChild(applyButton);
    applyButton->setOnClickCallback([this](SMView*view){
        // image crop
        cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
        float hRatio = _originImageSprite->getContentSize().width / _imageRect.size.width;
        float vRatio = _originImageSprite->getContentSize().height / _imageRect.size.height;

        kCropMenu type = _cropGuideView->cropType;
        
        if (type==kCropMenuFree && _drawCropButton->isVisible()) {
            cocos2d::Rect cropRect;
            
            auto sprite = _freeCropView->cropSprite(cropRect);

            auto rt = cocos2d::RenderTexture::create(cropRect.size.width, cropRect.size.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
            rt->beginWithClear(0, 0, 0, 0);
            sprite->setAnchorPoint(cocos2d::Vec2::ZERO);
            sprite->setPosition(cocos2d::Vec2::ZERO);
            sprite->visit();
            rt->end();
            _director->getRenderer()->render();
            
            auto newImage = rt->newImage();
            newImage->autorelease();

            auto intent = Intent::create();
            intent->putRef("EDIT_IMAGE", newImage);
            setSceneResult(intent);
            
            auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);

        } else {
        
        cocos2d::Rect guideRect = _cropGuideView->cropRect;
        
        cocos2d::Rect croppedRect = cocos2d::Rect((guideRect.origin.x - _imageRect.origin.x) * hRatio,
                                                                                ((guideRect.origin.y - _imageRect.origin.y)) * vRatio,
                                                                                guideRect.size.width * hRatio,
                                                                              guideRect.size.height * vRatio);
        auto size = _originImageSprite->getContentSize();
        
        
        if (_mainccImage) {
            croppedRect.origin.y = size.height - (croppedRect.origin.y + croppedRect.size.height);
            
            cv::Mat src = cvImageUtil::ccImage2cvMat(_mainccImage);
            cv::Rect myROI(roundf(croppedRect.origin.x), roundf(croppedRect.origin.y), roundf(croppedRect.size.width), roundf(croppedRect.size.height));
            
            cv::Mat croppedImage = src(myROI).clone();
            
            cocos2d::Image * cropccImage = cvImageUtil::cvMat2ccImage(croppedImage);
                cropccImage->retain();
            src.release();
            croppedImage.release();
            auto intent = Intent::create();
            intent->putRef("EDIT_IMAGE", cropccImage);
            setSceneResult(intent);
                
            auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
        }
            
        }

    });
 
    _innerContentView = SMView::create(0, 0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
    _contentView->addChild(_innerContentView);
 
    cocos2d::Rect cropGuideViewFrame = cocos2d::Rect(0, 0, s.width, _innerContentView->getContentSize().height);
    
    cocos2d::Rect photoViewFrame = cocos2d::Rect(PHOTO_MARGIN, PHOTO_MARGIN, s.width-PHOTO_MARGIN*2, _innerContentView->getContentSize().height-PHOTO_MARGIN*2);
    
    _mainImageView = SMImageView::create();
    _mainImageView->setSprite(_originImageSprite);
    _mainImageView->setPosition(photoViewFrame.origin);
    _mainImageView->setContentSize(photoViewFrame.size);
    _mainImageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _innerContentView->addChild(_mainImageView);
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    
    _cropMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->addChild(_cropMenuTableView);
    _cropMenuTableView->numberOfRowsInSection = [&] (int section) {
        return kCropMenuCount;
    };
    
    _cropMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * convertView = _cropMenuTableView->dequeueReusableCellWithIdentifier("MENU_CELL");
        MenuCell * cell = nullptr;
        if (convertView) {
            cell = (MenuCell*)convertView;
        } else {
            cell = MenuCell::create(0, 0, 0, BOTTOM_MENU_HEIGHT, BOTTOM_MENU_HEIGHT);
            cell->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, BOTTOM_MENU_HEIGHT-20, BOTTOM_MENU_HEIGHT-20);
            cell->addChild(cell->menuButton);
            cell->menuButton->setShapeCornerRadius(10);
            cell->menuButton->setOutlineWidth(2.0f);
            cell->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            cell->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            cell->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            cell->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            cell->menuButton->setPushDownScale(0.9f);
            cell->menuButton->setOnClickListener(this);
        }
        
        int index = indexPath.getIndex();
        switch (index) {
            case kCropMenuRect:
            {
                cell->menuButton->setTextSystemFont("사각형", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenuOriginal:
            {
                cell->menuButton->setTextSystemFont("원본비율", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenuSquare:
            {
                cell->menuButton->setTextSystemFont("정사각형", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu16_9:
            {
                cell->menuButton->setTextSystemFont("16:9", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu3_2:
            {
                cell->menuButton->setTextSystemFont("3:2", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu4_3:
            {
                cell->menuButton->setTextSystemFont("4:3", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu4_6:
            {
                cell->menuButton->setTextSystemFont("4:6", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu5_7:
            {
                cell->menuButton->setTextSystemFont("5:7", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu8_10:
            {
                cell->menuButton->setTextSystemFont("8:10", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenu14_8:
            {
                cell->menuButton->setTextSystemFont("14:8", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kCropMenuFree:
            {
                cell->menuButton->setTextSystemFont("자유롭게", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        cell->setTag(index);
        cell->menuButton->setTag(index);
        
        return cell;
    };
    
    cocos2d::Rect offsetRect = photoViewFrame;
    offsetRect.origin.y -= cropGuideViewFrame.origin.y;
    
    _imageRect = ViewUtil::frameForImageInTargetFrame(_mainImageView->getSprite()->getContentSize(), offsetRect);
    _cropGuideView = SMImageEditorCropGuideView::create(0, cropGuideViewFrame.origin.x, cropGuideViewFrame.origin.y, cropGuideViewFrame.size.width, cropGuideViewFrame.size.height, _imageRect);
    _innerContentView->addChild(_cropGuideView);
    
    cocos2d::Rect canvasRect = cocos2d::Rect(0, 0, s.width, _innerContentView->getContentSize().height);
    _freeCropView = DrawCropView::create(_originImageSprite, canvasRect, _mainImageView->getBoundingBox());
    _freeCropView->onSelectCallback = [this](bool selected) {
        if (selected) {
            CCLOG("[[[[[ selected!!!!");
            _drawCropButton->setVisible(true);
        } else {
            CCLOG("[[[[[ no selected!!!!!");
            _drawCropButton->setVisible(false);
        }
            
    };
    _freeCropView->setVisible(false);
    _innerContentView->addChild(_freeCropView);
    
    return true;
}

void SMImageEditorCropScene::onClick(SMView *view)
{
    int tag = view->getTag();
    switch (tag) {
        case kCropMenuRect:
        {
            CCLOG("[[[[[ 사각형 자르기");
        }
            break;
        case kCropMenuOriginal:
        {
            CCLOG("[[[[[ 원본비율 자르기");
        }
            break;
        case kCropMenuSquare:
        {
            CCLOG("[[[[[ 정사각형 자르기");
        }
            break;
        case kCropMenu16_9:
        {
            CCLOG("[[[[[ 16:9 자르기");
        }
            break;
        case kCropMenu3_2:
        {
            CCLOG("[[[[[ 3:2 자르기");
        }
            break;
        case kCropMenu4_3:
        {
            CCLOG("[[[[[ 4:3 자르기");
        }
            break;
        case kCropMenu4_6:
        {
            CCLOG("[[[[[ 4:6 자르기");
        }
            break;
        case kCropMenu5_7:
        {
            CCLOG("[[[[[ 5:7 자르기");
        }
            break;
        case kCropMenu8_10:
        {
            CCLOG("[[[[[ 8:10 자르기");
        }
            break;
        case kCropMenu14_8:
        {
            CCLOG("[[[[[ 14:8 자르기");
        }
            break;
        case kCropMenuFree:
        {
            CCLOG("[[[[[ 자유롭게 자르기");
        }
            break;
            
        default:
        {
            CCLOG("[[[[[ 뭐지? 자르기");
            return;
        }
            break;
    }
    
    _cropGuideView->setCropType((kCropMenu)tag);
    showFreeCropView(tag!=kCropMenuFree);
}

void SMImageEditorCropScene::showFreeCropView(bool bShow)
{
    _cropGuideView->setVisible(bShow);
    _freeCropView->setVisible(!bShow);
    if (bShow) {
        
    } else {
        
    }
    return;
    auto a = ViewAction::TransformAction::create();
    if (bShow) {
        if (_cropGuideView->getOpacity()==0xff) {
            return;
        }
        _cropGuideView->setVisible(true);
        a->toAlpha(1).setTimeValue(0.3, 0.2);
        _cropGuideView->setOpacity(0);
    } else {
        if (_cropGuideView->getOpacity()==0) {
            return;
        }
        a->toAlpha(0).runFuncOnFinish([&](cocos2d::Node* node, int tag){
            _cropGuideView->setVisible(false);
        }).setTimeValue(0.3, 0.2);
        _cropGuideView->setOpacity(0xff);
    }
    _cropGuideView->runAction(a);
}
