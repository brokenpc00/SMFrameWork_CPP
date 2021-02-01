//
//  SMImageEditorDrawScene.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 25..
//

#include "SMImageEditorDrawScene.h"
#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMImageView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Base/SMTableView.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Util/cvImageUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Const/SMViewConstValue.h"
#include "../../SMFrameWork/Cutout/DrawView.h"

#define TOP_MENU_HEIGHT (SMViewConstValue::Size::TOP_MENU_HEIGHT+SMViewConstValue::Size::getStatusHeight())
#define PHOTO_MARGIN 20.0f
#define BOTTOM_MENU_HEIGHT 160.0f

SMImageEditorDrawScene::SMImageEditorDrawScene() :
_contentView(nullptr)
, _topMenuView(nullptr)
, _bottomMenuView(nullptr)
, _mainImageView(nullptr)
, _currentImageSprite(nullptr)
, _drawMenuTableView(nullptr)
, _fromPicker(false)
, _listener(nullptr)
, _callIndex(-1)
{
    _colorButtons.clear();
}

SMImageEditorDrawScene::~SMImageEditorDrawScene()
{
    
}

bool SMImageEditorDrawScene::init()
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
            applyDrawImage();
        });
    });
    
    float colorBGWidth = s.width/3*2;
    
    auto colorBG = SMView::create(0, s.width/2-colorBGWidth/2, 0, colorBGWidth, TOP_MENU_HEIGHT);
//    colorBG->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
    _topMenuView->addChild(colorBG);
    
    float posX = 0;
    float buttonWidth = colorBGWidth/6;
    auto redButton = SMButton::create(DrawColorTypeRed, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
    posX += buttonWidth;
    auto blueButton = SMButton::create(DrawColorTypeBlue, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
    posX += buttonWidth;
    auto greenButton = SMButton::create(DrawColorTypeGreen, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
    posX += buttonWidth;
    auto yellowButton = SMButton::create(DrawColorTypeYellow, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
    posX += buttonWidth;
    auto blackButton = SMButton::create(DrawColorTypeBlack, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
    posX += buttonWidth;
    auto whiteButton = SMButton::create(DrawColorTypeWhite, SMButton::Style::DEFAULT, posX, 0, buttonWidth, buttonWidth);
//    redButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::RED);
//    blueButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::BLUE);
//    greenButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::GREEN);
//    yellowButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::YELLOW);
//    blackButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
//    whiteButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);

    colorBG->addChild(redButton);
    colorBG->addChild(blueButton);
    colorBG->addChild(greenButton);
    colorBG->addChild(yellowButton);
    colorBG->addChild(blackButton);
    colorBG->addChild(whiteButton);
    _colorButtons.push_back(redButton);
    _colorButtons.push_back(blueButton);
    _colorButtons.push_back(greenButton);
    _colorButtons.push_back(yellowButton);
    _colorButtons.push_back(blackButton);
    _colorButtons.push_back(whiteButton);
    
    redButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::RED;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    blueButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::BLUE;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    greenButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::GREEN;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    yellowButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::YELLOW;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    blackButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::BLACK;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    whiteButton->setOnClickCallback([this](SMView * view){
        auto color = cocos2d::Color3B::WHITE;
        _drawView->setDrawColor(color);
        changeColorButton((DrawColorType)view->getTag());
    });
    

    cocos2d::Rect cropGuideViewFrame = cocos2d::Rect(0, BOTTOM_MENU_HEIGHT, s.width, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT);
    cocos2d::Rect photoViewFrame = cocos2d::Rect(PHOTO_MARGIN, BOTTOM_MENU_HEIGHT+PHOTO_MARGIN, s.width-PHOTO_MARGIN*2, s.height-BOTTOM_MENU_HEIGHT-TOP_MENU_HEIGHT-PHOTO_MARGIN*2);
    
    cocos2d::Size imgSize = _currentImageSprite->getContentSize();

    _mainImageView = SMImageView::create();
    _mainImageView->setSprite(_currentImageSprite);
    _mainImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _mainImageView->setPosition(cocos2d::Vec2(photoViewFrame.origin.x+photoViewFrame.size.width/2, photoViewFrame.origin.y+photoViewFrame.size.height/2));
    _mainImageView->setContentSize(photoViewFrame.size);
    _mainImageView->setScaleType(SMImageView::ScaleType::FIT_CENTER);
    _contentView->addChild(_mainImageView);
    
    
    // draw view size는 image size에 딱 맞게
    
    _drawContainerView = SMView::create(0, 0, 0, _mainImageView->getContentSize().width, _mainImageView->getContentSize().height);
    _drawContainerView->setLocalZOrder(999);
//    _drawContainerView->setBackgroundColor4F(MAKE_COLOR4F(0xffff00, 0.7f));
    _mainImageView->addChild(_drawContainerView);
    
    _bottomMenuView = SMView::create(0, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
    _contentView->addChild(_bottomMenuView);
    
    _drawView = DrawView::create(0, 0, 0, _drawContainerView->getContentSize().width, _drawContainerView->getContentSize().height);
    _drawView->setAnchorPoint(cocos2d::Vec2::ZERO);
    _drawContainerView->addChild(_drawView);
    
    auto color = cocos2d::Color3B::RED;
    _drawView->setDrawColor(color);
    changeColorButton(DrawColorTypeRed);


    
    
    auto bottomMenuTopLine = SMView::create(0, 0, BOTTOM_MENU_HEIGHT-1, s.width, 1);
    bottomMenuTopLine->setBackgroundColor4F(MAKE_COLOR4F(0xadafb3, 1.0f));
    _bottomMenuView->addChild(bottomMenuTopLine);
    bottomMenuTopLine->setLocalZOrder(10);
    
    _drawMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::HORIZONTAL, 1, 0, 0, s.width, BOTTOM_MENU_HEIGHT);
    _bottomMenuView->addChild(_drawMenuTableView);
    _drawMenuTableView->numberOfRowsInSection = [&] (int section) {
        return 1;
    };
    _drawMenuTableView->cellForRowAtIndexPath = [&] (const IndexPath & indexPath)->cocos2d::Node* {
        cocos2d::Node * convertView = _drawMenuTableView->dequeueReusableCellWithIdentifier("MENU_CELL");
        _clearButton = nullptr;
        if (convertView) {
            _clearButton = (MenuCell*)convertView;
        } else {
            _clearButton = MenuCell::create(0, 0, 0, BOTTOM_MENU_HEIGHT, BOTTOM_MENU_HEIGHT);
            _clearButton->menuButton = SMButton::create(0, SMButton::Style::SOLID_ROUNDEDRECT, 10, 10, BOTTOM_MENU_HEIGHT-20, BOTTOM_MENU_HEIGHT-20);
            _clearButton->addChild(_clearButton->menuButton);
            _clearButton->menuButton->setShapeCornerRadius(10);
            _clearButton->menuButton->setOutlineWidth(2.0f);
            _clearButton->menuButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xadafb3, 1.0f));
            _clearButton->menuButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            _clearButton->menuButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xdbdcdf, 1.0f));
            _clearButton->menuButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xeeeff1, 1.0f));
            _clearButton->menuButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::BLACK);
            _clearButton->menuButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
            _clearButton->menuButton->setPushDownScale(0.9f);
//            _clearButton->menuButton->setOnClickListener(this);
            _clearButton->menuButton->setOnClickCallback([this](SMView* view){
                _drawView->clearCanvas();
            });
        }
        
        int index = indexPath.getIndex();
        _clearButton->menuButton->setTextSystemFont("Clear", SMFontConst::SystemFontRegular, 24.0f);

        _clearButton->setTag(index);
        _clearButton->menuButton->setTag(index);
        
        return _clearButton;
    };
    
    return true;
}

void SMImageEditorDrawScene::onClick(SMView *view)
{

 
}

cocos2d::Color4F SMImageEditorDrawScene::getColor(DrawColorType type)
{
    switch (type) {
        case DrawColorTypeRed:
        {
            return cocos2d::Color4F::RED;
        }
            break;
        case DrawColorTypeBlue:
        {
            return cocos2d::Color4F::BLUE;
        }
            break;
        case DrawColorTypeGreen:
        {
            return cocos2d::Color4F::GREEN;
        }
            break;
        case DrawColorTypeYellow:
        {
            return cocos2d::Color4F::YELLOW;
        }
            break;
        case DrawColorTypeBlack:
        {
            return cocos2d::Color4F::BLACK;
        }
            break;
        case DrawColorTypeWhite:
        {
            return cocos2d::Color4F::WHITE;
        }
            break;

        default:
        {
            return cocos2d::Color4F::WHITE;
        }
            break;
    }
}

void SMImageEditorDrawScene::changeColorButton(DrawColorType type)
{
    for (auto btn : _colorButtons) {
        auto colorSize = 80;
        auto icon = SMView::create(btn->getTag(), 0, 0, colorSize, colorSize);
//        bg->setBackgroundColor4F(MAKE_COLOR4F(0xff000b, 1.0f));
        icon->setContentSize(cocos2d::Size(colorSize, colorSize));
        
        auto shadow = ShapeSolidCircle::create();
        shadow->setContentSize(icon->getContentSize());
        shadow->setColor4F(MAKE_COLOR4F(0x999999, 1.0f));
        shadow->setAnchorPoint(cocos2d::Vec2::ZERO);
        shadow->setPosition(cocos2d::Vec2(2, -2));
        icon->addChild(shadow);

        auto bgCircle = ShapeSolidCircle::create();
        bgCircle->setContentSize(icon->getContentSize());
        bgCircle->setColor4F(MAKE_COLOR4F(0xdbdcdf, 1.0f));
        bgCircle->setAnchorPoint(cocos2d::Vec2::ZERO);
        bgCircle->setPosition(cocos2d::Vec2::ZERO);
        icon->addChild(bgCircle);
        
        auto circle = ShapeCircle::create();
        circle->setContentSize(icon->getContentSize()*0.9);
        circle->setLineWidth(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*6);
        circle->setColor4F(getColor((DrawColorType)btn->getTag()));
        circle->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        circle->setPosition(icon->getContentSize()/2);
        icon->addChild(circle);

        auto color = ShapeSolidCircle::create();
        color->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        color->setColor4F(getColor((DrawColorType)btn->getTag()));
        color->setPosition(icon->getContentSize()/2);
        icon->addChild(color);

        if (btn->getTag()==type) {
            // 눌린거
            color->setContentSize(icon->getContentSize()*0.5);
        } else {
            // 안눌린거
            circle->setVisible(false);
            color->setContentSize(icon->getContentSize()*0.9);
        }
        btn->setIcon(SMButton::State::NORMAL, icon);
    }
}



void SMImageEditorDrawScene::applyDrawImage()
{
    CCLOG("[[[[[ apply Draw Image");
    
    float oldScale = _currentImageSprite->getScale();
    cocos2d::Vec2 oldAnchor = _currentImageSprite->getAnchorPoint();
    cocos2d::Vec2 oldPos = _currentImageSprite->getPosition();
    cocos2d::Size imageSize = _currentImageSprite->getTexture()->getContentSizeInPixels();

    auto imgScale = _mainImageView->getScale();
    auto imgAnchor = _mainImageView->getAnchorPoint();
    auto imagpos = _mainImageView->getPosition();
    
    auto s = _director->getWinSize();
    
    auto rt = cocos2d::RenderTexture::create(imageSize.width, imageSize.height, cocos2d::Texture2D::PixelFormat::RGBA8888);
    rt->beginWithClear(0, 0, 0, 0);
    _mainImageView->setScale(1.0/oldScale);
    _mainImageView->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _mainImageView->setPosition(imageSize/2);
    _mainImageView->visit(_director->getRenderer(), cocos2d::Mat4::IDENTITY, 0);
    rt->end();
    _director->getRenderer()->render();

    _mainImageView->setScale(imgScale);
    _mainImageView->setPosition(imagpos);
    _mainImageView->setAnchorPoint(imgAnchor);
    cocos2d::Image * newImage = rt->newImage();

    
    if (_fromPicker) {
        
        auto texture = new cocos2d::Texture2D;
        texture->initWithImage(newImage);
        newImage->release();
        auto sendSprite = cocos2d::Sprite::createWithTexture(texture);
        texture->release();
        
        if (_listener) {
            _listener->onImageSelected(_callIndex, sendSprite);
        }
        
        _director->getScheduler()->performFunctionInCocosThread([&]{
            auto pScene = SceneTransition::SlideOutToBottom::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
        });
    } else {
        auto intent = Intent::create();
        intent->putRef("EDIT_IMAGE", newImage);
        intent->putBool("FROM_DRAW", true);
        
        setSceneResult(intent);
        
        newImage->autorelease();

        auto pScene = SceneTransition::FadeOut::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
        _director->popSceneWithTransition(pScene);
    }
    
}


