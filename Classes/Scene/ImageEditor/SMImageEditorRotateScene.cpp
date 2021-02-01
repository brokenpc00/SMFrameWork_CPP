//
//  SMImageEditorRotateScene.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 8..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "SMImageEditorRotateScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMTableView.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"
#include "SMImageEditorDrawScene.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f


SMImageEditorRotateScene::SMImageEditorRotateScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _mainImageView(nullptr)
, _currentImageSprite(nullptr)
, _rotateMenuTableView(nullptr)
, _isRotateAnimation(false)
, _listener(nullptr)
{
    
}

SMImageEditorRotateScene::~SMImageEditorRotateScene()
{
    
}

bool SMImageEditorRotateScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
    
    auto programBundle = getSceneParam();
    if (programBundle) {
        _fromPicker = programBundle->getBool("FROM_PICKER", false);
        _callIndex = programBundle->getInt("CALL_INDEX", -1);
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
            applyRotateImage();
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
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);

    _rotateMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->addChild(_rotateMenuTableView);
    _rotateMenuTableView->numberOfRowsInSection = [&] (int section) {
        return kRotateMenuCount;
    };
    _rotateMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * convertView = _rotateMenuTableView->dequeueReusableCellWithIdentifier("MENU_CELL");
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
            case kRotateMenuLeft:
            {
                cell->menuButton->setTextSystemFont("좌 90도", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kRotateMenuRight:
            {
                cell->menuButton->setTextSystemFont("우 90도", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kRotateMenuFlipX:
            {
                cell->menuButton->setTextSystemFont("세로반전", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
            case kRotateMenuFlipY:
            {
                cell->menuButton->setTextSystemFont("가로반전", SMFontConst::SystemFontRegular, 24.0f);
            }
                break;
        }
        cell->setTag(index);
        cell->menuButton->setTag(index);
        
        return cell;
    };
    
    return true;
}

void SMImageEditorRotateScene::onClick(SMView *view)
{
    if (isScheduled(schedule_selector(SMImageEditorRotateScene::doRotate))) {
//        return;
        unschedule(schedule_selector(SMImageEditorRotateScene::doRotate));
    }
    
    _currentRotateType = (kRotateMenu)view->getTag();
    schedule(schedule_selector(SMImageEditorRotateScene::doRotate), 0.1f);
}

void SMImageEditorRotateScene::doRotate(float dt)
{
    
    if (_isRotateAnimation) {
        return;
    }
    
    _isRotateAnimation = true;
    
    // rotate animation
    /*
     - rotation 3D로 rotate 진행
     1. flipX는 Vec3의 x로 0 or 180
     2. flipY는 Vec3의 y로 0 or 180
     3. left, right rotate는 Vec3의 z로 현재 angle +- 90
     */
    
    cocos2d::Size s = _director->getWinSize();
    cocos2d::Rect menuFrame = cocos2d::Rect(0, BOTTOM_MENU_HEIGHT, s.width, BOTTOM_MENU_HEIGHT);
    cocos2d::Rect photoViewFrame = cocos2d::Rect(PHOTO_MARGIN, BOTTOM_MENU_HEIGHT+PHOTO_MARGIN, s.width-PHOTO_MARGIN*2, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT-PHOTO_MARGIN*2);
    
    cocos2d::Vec3 v3 = _currentImageSprite->getRotation3D();
    int portrait = (int)(fabs(v3.z / 90.0f));
    if (portrait>3) {
        portrait = 0;
    }
    bool bPortrait = (portrait%2) == 0;
    switch (_currentRotateType) {
        case kRotateMenuLeft:
        {
            v3.z -= 90.0f;
        }
            break;
        case kRotateMenuRight:
        {
            v3.z += 90.0f;
        }
            break;
        case kRotateMenuFlipX:
        {
            if (bPortrait) {
                v3.x = (v3.x==0.0f) ? 180.0f : 0.0f;
            } else {
                v3.y = (v3.y==0.0f) ? 180.0f : 0.0f;
            }
        }
            break;
        case kRotateMenuFlipY:
        {
            if (bPortrait) {
                v3.y = (v3.y==0.0f) ? 180.0f : 0.0f;
            } else {
                v3.x = (v3.x==0.0f) ? 180.0f : 0.0f;
            }
        }
            break;
        default:
            break;
    }
    
    
    auto rotateTo = cocos2d::RotateTo::create(0.15f, v3);
    
    auto rotateAction = cocos2d::EaseSineOut::create(rotateTo);
    
    _currentImageSprite->runAction(cocos2d::Sequence::create(rotateAction, cocos2d::DelayTime::create(0.15f), cocos2d::CallFunc::create([&]{
        _isRotateAnimation = false;
        unschedule(schedule_selector(SMImageEditorRotateScene::doRotate));
        _director->getScheduler()->performFunctionInCocosThread([&]{
        });
    }), NULL));
    
    
    // rotate
    
    float rotateScale = 1.0f;
    
    cocos2d::Sprite * calcSprite = cocos2d::Sprite::createWithTexture(_currentImageSprite->getTexture());
    calcSprite->setRotation3D(v3);
    
    cocos2d::Rect boundingBox = calcSprite->getBoundingBox();
    auto imageSize = boundingBox.size;
    
    _mainImageView->setScale(1.0f);
    
    
    // scale
    
    if (imageSize.width<imageSize.height) {
        // 세로
        rotateScale = photoViewFrame.size.height / imageSize.height;
    } else if (imageSize.width>imageSize.height) {
        // 가로
        rotateScale = photoViewFrame.size.width / imageSize.width;
    }
    
    
    auto scaleTo = cocos2d::ScaleTo::create(0.15f, rotateScale);
    _currentImageSprite->runAction(scaleTo);
    
}

void SMImageEditorRotateScene::applyRotateImage()
{
    cocos2d::Vec3 v3 = _currentImageSprite->getRotation3D();
    float scale = _currentImageSprite->getScale();
    _currentImageSprite->setScale(1.0f);
    cocos2d::Rect boundingBox = _currentImageSprite->getBoundingBox();
    _currentImageSprite->setScale(scale);
    
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
    cv::warpAffine(src, dst, rot, bbox.size(), cv::INTER_CUBIC);

    // for 가로반전
    if (v3.y==180) {
        cv::Mat flipDst;
        cv::flip(dst, flipDst, 1);
        dst = flipDst.clone();
        flipDst.release();
    }
    
    // for 세로반전
    if (v3.x==180) {
        cv::Mat flipDst;
        cv::flip(dst, flipDst, 0);
        dst = flipDst.clone();
        flipDst.release();
    }
    
    cocos2d::Image * rotateccImage = cvImageUtil::cvMat2ccImage(dst);
    rotateccImage->retain();
    dst.release();
    rot.release();
    src.release();
    
    Intent * intent = Intent::create();
    intent->putRef("EDIT_IMAGE", rotateccImage);

    if (_fromPicker) {
        intent->putBool("FROM_PICKER", true);
        intent->putInt("CALL_INDEX", _callIndex);
        // draw로 보낸다.
        auto scene = SMImageEditorDrawScene::create(intent, SMScene::SwipeType::NONE);
        scene->setOnImageSelectedListener(_listener);
        auto pScene = SceneTransition::FadeIn::create(SceneTransition::Time::NORMAL, scene);
        _director->replaceScene(pScene);
        CC_SAFE_RELEASE_NULL(rotateccImage);

    } else {
    setSceneResult(intent);
    auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
    _director->popSceneWithTransition(pScene);

    }
    
}
