//
//  SMImageEditorStraightenScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 16..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorStraightenScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"


#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f

#define RADIANS_TO_DEGREES(radians) ((radians) * (180.0 / M_PI))
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0 * M_PI)


SMImageEditorStraightenScene::SMImageEditorStraightenScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _sliderView(nullptr)
, _straightenGuideView(nullptr)
, _mainImageView(nullptr)
, _currentImageSprite(nullptr)
, _mainccImage(nullptr)
, _imageRect(cocos2d::Rect::ZERO)
, _initScale(0.0f)
, _currentDgrees(0.0f)
, _currentScale(1.0f)
{
    
}

SMImageEditorStraightenScene::~SMImageEditorStraightenScene()
{
    
}

bool SMImageEditorStraightenScene::init()
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
            
            _currentImageSprite = cocos2d::Sprite::createWithTexture(texture);
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
    _topMenuView->setLocalZOrder(10);
    
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
    applyButton->setOnClickCallback([&](SMView * view){
        _director->getScheduler()->performFunctionInCocosThread([&]{
            applyStraightenImage();
        });
    });
    
    cocos2d::Rect cropGuideViewFrame = cocos2d::Rect(0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
    cocos2d::Rect photoViewFrame = cocos2d::Rect(PHOTO_MARGIN, BOTTOM_MENU_HEIGHT+PHOTO_MARGIN, s.width-PHOTO_MARGIN*2, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT-PHOTO_MARGIN*2);
    
    
    _mainImageView = SMImageView::create();
    _mainImageView->setSprite(_currentImageSprite);
    _mainImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _mainImageView->setPosition(cocos2d::Vec2(photoViewFrame.origin.x+photoViewFrame.size.width/2, photoViewFrame.origin.y+photoViewFrame.size.height/2));
    _mainImageView->setContentSize(photoViewFrame.size);
    _mainImageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _contentView->addChild(_mainImageView);
    
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    _bottomMenuView->setLocalZOrder(10);
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    
    cocos2d::Rect offsetRect = photoViewFrame;
    offsetRect.origin.y -= cropGuideViewFrame.origin.y;
    
    _imageRect = ViewUtil::frameForImageInTargetFrame(_mainImageView->getSprite()->getContentSize(), offsetRect);
    _straightenGuideView = SMImageEditorStraightenGuideView::create(0, cropGuideViewFrame.origin.x, cropGuideViewFrame.origin.y, cropGuideViewFrame.size.width, cropGuideViewFrame.size.height, _imageRect);
    _contentView->addChild(_straightenGuideView);

    //static SMSlider * create(const Type type = ZERO_TO_ONE, const InnerColor& initColor=InnerColor::LIGHT);
    _sliderView = SMSlider::create(SMSlider::Type::MINUS_ONE_TO_ONE, SMSlider::InnerColor(MAKE_COLOR4F(0xe94253, 1), MAKE_COLOR4F(0x222222, 1), MAKE_COLOR4F(0xffffff, 1), MAKE_COLOR4F(0xeeeff1, 1)));
    _sliderView->setContentSize(cocos2d::Size(s.width-160, 80));
    _sliderView->setPosition(cocos2d::Vec2(80, 40));
    _bottomMenuView->addChild(_sliderView);
    _sliderView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _sliderView->onSliderValueChange = [&] (SMSlider* slider, float value) {
        changeStraighten(value);
    };
    
    return true;
}

void SMImageEditorStraightenScene::changeStraighten(float value)
{
//    CCLOG("[[[[[ slider value : %f", value);
    
    if (_initScale==0.0f) {
        _initScale = _currentImageSprite->getScale();
        CCLOG("[[[[[ init scale : %f", _initScale);
    }
    
    int degrees = (int)(45.f * value);
    
    double radian = DEGREES_TO_RADIANS(value < 0 ? degrees : -degrees);

    double w = _currentImageSprite->getContentSize().width / 2.f;
    double h = _currentImageSprite->getContentSize().height / 2.f;
    
    double x1, y1, x2, y2;
    
    x1 = cos(radian);
    y1 = sin(radian);
    x2 = -cos(radian + M_PI_2);
    y2 = -sin(radian + M_PI_2);

    double dist1 = calculateShortestDistance(-w, -h, -w+x1, -h+y1, 0, 0);
    double dist2 = calculateShortestDistance(-w, h, -w+x2, h+y2, 0, 0);
    
    float scale = MAX(dist1 / h, dist2 / w);
//    float angle = DEGREES_TO_RADIANS(degrees);
    
    cocos2d::Vec3 v3 = _currentImageSprite->getRotation3D();
    v3.z = degrees;
    _currentImageSprite->setRotation3D(v3);
    
    _currentImageSprite->setScale(_initScale*scale);
    
    _currentScale = scale;
    _currentDgrees = degrees;
}

double SMImageEditorStraightenScene::calculateShortestDistance(double x1, double y1, double x2, double y2, double x, double y)
{
    double segment_mag = pow((x2-x1), 2) + pow((y2-y1), 2);
    
    if (segment_mag != 0) {
        double u = ((x-x1) * (x2-x1) + (y-y1) * (y2-y1)) / segment_mag;
        double xp = x1 + u * (x2-x1);
        double yp = y1 + u * (y2-y1);
        
        return sqrt(pow((xp-x), 2) + pow((yp-y), 2));
    }
    
    return sqrt(pow((x-x1), 2) + pow((y-y1), 2));
}

void SMImageEditorStraightenScene::applyStraightenImage()
{
    cocos2d::Vec3 v3 = _currentImageSprite->getRotation3D();
    
    float oldScale = _currentImageSprite->getScale();
    _currentImageSprite->setScale(_currentScale);
    
    cocos2d::Rect boundingBox = _currentImageSprite->getBoundingBox();

    _currentImageSprite->setScale(oldScale);
    
    double angle = v3.z;
    cv::Mat src = cvImageUtil::ccImage2cvMat(_mainccImage);
    
    // v3.x = 세로반전
    // v3.y =  가로반전
    // v3.z = 좌우회전
    cv::Point2f center((float)(src.cols)/2.0f, (float)(src.rows)/2.0f);
    
    // for 좌우 회전
    cv::Mat rot = cv::getRotationMatrix2D(center, -angle, 1.0f);
    cv::RotatedRect rr = cv::RotatedRect(cv::Point2f(center.x, center.y), cv::Size2f(src.cols, src.rows), -angle);
    cv::Rect bbox = cv::Rect(0, 0, boundingBox.size.width, boundingBox.size.height);
    
    cv::Point2f pt[4];
    rr.points(pt);
    std::vector<cv::Point> pts;
    pts.push_back(pt[0]);
    pts.push_back(pt[1]);
    pts.push_back(pt[2]);
    pts.push_back(pt[3]);
    
    rot.at<double>(0,2) += bbox.width/2.0 - center.x;
    rot.at<double>(1,2) += bbox.height/2.0 - center.y;
    
    cv::Mat dst;
    cv::warpAffine(src, dst, rot, bbox.size(), 8);
    
    cv::Size scaleSize(bbox.size().width*_currentScale, bbox.size().height*_currentScale);
    cv::resize(dst, dst, scaleSize);
    
//    CCLOG("[[[[[ origin : %d, %d, new : %d, %d", src.cols, src.rows, dst.cols, dst.rows);
    
    cv::Mat crop = dst(cv::Rect(dst.cols/2-src.cols/2, dst.rows/2-src.rows/2, src.cols, src.rows)).clone();
    
    cocos2d::Image * rotateccImage = cvImageUtil::cvMat2ccImage(crop);
    rotateccImage->retain();
    crop.release();
    dst.release();
    rot.release();
    src.release();
    
    auto intent = Intent::create();
    intent->putRef("EDIT_IMAGE", rotateccImage);
    setSceneResult(intent);
    
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);
    
}
