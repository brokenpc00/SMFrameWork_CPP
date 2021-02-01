//
//  SideMenu.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 14..
//
//

#include "SideMenu.h"
#include "../Base/EdgeSwipeLayer.h"
#include "../Const/SMFontColor.h"
#include "../Const/SMViewConstValue.h"
#include "../Base/Intent.h"
//#include "../../Scene/Login/LoginScene.h"
#include "../Util/ViewUtil.h"
#include "../Util/StringUtil.h"
#include <cocos2d.h>

#define TOP_MENU_HEIGHT SMViewConstValue::Size::TOP_MENU_HEIGHT

#define CELL_HEIGHT 112.0f

static SideMenu * _instance = nullptr;

SideMenu::SideMenu()
{
    _sideMenuUpdateCallback = nullptr;
    _state = kSideMenuStateClose;
    _lastPosition = 0;
}

SideMenu::~SideMenu()
{
//    CCLOG("[[[[[ side menu destructor");
    _instance = nullptr;
}

SideMenu * SideMenu::getInstance()
{
    if (!_instance) {
        auto s = cocos2d::Director::getInstance()->getWinSize();
        _instance = SideMenu::create(s.width, 0, SMViewConstValue::Size::LEFT_SIDE_MENU_WIDTH, s.height);
        _instance->setBackgroundColor4F(cocos2d::Color4F(0, 0, 0, 1.0f));
    }
    
    return _instance;
}

bool SideMenu::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setVisible(false);
    auto s = cocos2d::Director::getInstance()->getWinSize();
    
    _contentsView = SMView::create(0, 0, 0, getContentSize().width, s.height);
    _contentsView->setBackgroundColor4F(cocos2d::Color4F(0xf4/255.0f, 0xf5/255.0f, 0xf6/255.0f, 1.0f));
    addChild(_contentsView);
    
//    float menuWidth = getContentSize().width;
//
//    float topBgHeight = TOP_MENU_HEIGHT;
//    auto topBgView = SMView::create(0, 0, _contentsView->getContentSize().height-topBgHeight, menuWidth, topBgHeight);
//    //topBgView->setBackgroundColor4F(cocos2d::Color4F(0xef/255.0f, 0x4c/255.0f, 0x55/255.0f, 1.0f));
//    topBgView->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
//    _contentsView->addChild(topBgView, 10);
//
//    auto topBgBottomLine = SMView::create(0, 0, 0, _contentsView->getContentSize().width, 2.0f);
//    topBgBottomLine->setBackgroundColor4F(MAKE_COLOR4D(74, 74, 88, 1.0f));
//    topBgView->addChild(topBgBottomLine);
//
//
//    _profileButton = SMButton::create(99, SMButton::Style::DEFAULT, 0, 0, 0, 0);
//    _profileButton->setIcon(SMButton::State::NORMAL, "images/my.png");
//    _profileButton->setTextSystemFont("로그인해주세요.", SMFontConst::SystemFontRegular, 34);
//    _profileButton->setIconAlign(SMButton::Align::LEFT);
//    _profileButton->setIconPadding(23.0f);
//    _profileButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x000000, 1.0f));
//    _profileButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
//    _profileButton->setContentSize(cocos2d::Size(38+_profileButton->getTextLabel()->getContentSize().width, 44));
//    _profileButton->setPosition(cocos2d::Vec2(45, 18));
//    _profileButton->setOnClickListener(this);
//    topBgView->addChild(_profileButton);
//
//
//    _signInButton = SMButton::create(0, SMButton::Style::SOLID_RECT, 209*3, topBgHeight/2-(26*3)/2, 55*3, 26*3);
//    _signInButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F(0xef/255.0f, 0x4c/255.0f, 0x55/255.0f, 1.0f));
//    _signInButton->setButtonColor(SMButton::State::PRESSED, cocos2d::Color4F(0xef/255.0f, 0x4c/255.0f, 0x55/255.0f, 1.0f));
//    _signInButton->setOutlineWidth(2.0f);
//    _signInButton->setOutlineColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
//    _signInButton->setOutlineColor(SMButton::State::PRESSED, cocos2d::Color4F(0xee/255.0f, 0xef/255.0f, 0xf1/255.0f, 1.0f));
//    _signInButton->setTextSystemFont("로그인", SMFontConst::NotoSansLight, 11*3);
//    _signInButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
//    _signInButton->setTextColor(SMButton::State::PRESSED, cocos2d::Color4F(0xee/255.0f, 0xef/255.0f, 0xf1/255.0f, 1.0f));
//    topBgView->addChild(_signInButton);
//
//    _signInButton->setOnClickListener(this);
//
//    _menuNames.push_back("공지사항");
//    _menuNames.push_back("견적계약관리");
//    _menuNames.push_back("납기지연현황");
//    _menuNames.push_back("교환반품관리");
//    _menuNames.push_back("이미지등록");
//    _menuNames.push_back("구매담당자 : ");
//    _menuNames.push_back("콜센터 : ");
//    _menuNames.push_back("로그아웃");
//
//    _sideMenuTableView = SMTableView::createMultiColumn(SMTableView::Orientation::VERTICAL, 1, 0, 0, getContentSize().width, getContentSize().height-topBgHeight);
//    _sideMenuTableView->setBackgroundColor4F(cocos2d::Color4F::WHITE);
//    _contentsView->addChild(_sideMenuTableView);
//
//    _sideMenuTableView->cellForRowAtIndexPath = CC_CALLBACK_1(SideMenu::cellForRowAtIndexPath, this);
//    _sideMenuTableView->numberOfRowsInSection = [&](int section)->int{
//        return (int)_menuNames.size();
//    };
//
//
//    setUserInfo();
    
    return true;
}

cocos2d::Node * SideMenu::cellForRowAtIndexPath(const IndexPath &indexPath)
{
    int index = indexPath.getIndex();
    std::string cellID = cocos2d::StringUtils::format("SIDECELL%02d", index);
    cocos2d::Node * convertView = _sideMenuTableView->dequeueReusableCellWithIdentifier(cellID);
    SideMenuCell * cell;
    if (convertView) {
        cell = (SideMenuCell*)convertView;
    } else {
        cell = SideMenuCell::create(0, 0, 0, getContentSize().width, CELL_HEIGHT);
        cell->contentsView = SMView::create(0, 0, 0, cell->getContentSize().width, cell->getContentSize().height);
        cell->addChild(cell->contentsView);
        cell->contentsView->setBackgroundColor4F(cocos2d::Color4F::WHITE);

        cell->cellTitle = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontRegular, 30);
        cell->cellTitle->setTextColor(cocos2d::Color4B(51, 51, 51, 0xff));
        cell->cellTitle->setAnchorPoint(cocos2d::Point::ZERO);
        cell->contentsView->addChild(cell->cellTitle);
        cell->cellTitle->setPosition(cocos2d::Point(44, cell->contentsView->getContentSize().height/2-cell->cellTitle->getContentSize().height/2));
        
        /*
        if (index==0) {
            cell->cartCountButton = SMButton::create(0, SMButton::Style::SOLID_CIRCLE, 0, 0, 15*3, 15*3);
            cell->cartCountButton->setTextSystemFont("", SMFontConst::NotoSansLight, 9*3);
            cell->cartCountButton->setTextColor(SMButton::State::NORMAL, cocos2d::Color4F::WHITE);
            cell->cartCountButton->setShapeCornerRadius(15*3/2);
            cell->cartCountButton->setButtonColor(SMButton::State::NORMAL, cocos2d::Color4F(0x33/255.0f, 0x33/255.0f, 0x33/255.0f, 1.0f));
            cell->contentsView->addChild(cell->cartCountButton);
        }
         */
        
        if (index==5 || index==6) {
            //auto iconSprite = cocos2d::Sprite::create("images/mp_icon_phone.png");
            //cell->cellRightIcon = SMImageView::create(iconSprite);
            //cell->contentsView->addChild(cell->cellRightIcon);
            
            cell->cellRightLabel = cocos2d::Label::createWithSystemFont("", SMFontConst::SystemFontRegular, 30);
            cell->cellRightLabel->setTextColor(cocos2d::Color4B(51, 51, 51, 0xff));
            cell->cellRightLabel->setAnchorPoint(cocos2d::Point::ZERO);
            cell->contentsView->addChild(cell->cellRightLabel);
        }
        
        auto bottomLine = SMView::create(0, 0, 0, getContentSize().width, 2.0f);
        bottomLine->setBackgroundColor4F(MAKE_COLOR4D(232, 232, 232, 1.0f));
        cell->contentsView->addChild(bottomLine);
        
        cell->contentsView->setTag(index);
        cell->contentsView->setOnClickListener(cell);
        cell->contentsView->setOnStateChangeCallback([&](SMView * view, SMView::State state){
            if (state==SMView::State::PRESSED) {
                view->setBackgroundColor4F(cocos2d::Color4F(0xee/255.0f, 0xef/255.0f, 0xf1/255.0f, 1.0f));
            } else {
                view->setBackgroundColor4F(cocos2d::Color4F::WHITE);
            }
        });
    }
    
    cell->parent = this;
    cell->setTag(index);
    
    std::string dmName;
    if (index==_menuNames.size()-3) {
        // last-1
        dmName = _menuNames[_menuNames.size()-3] + "홍길동";
    } else {
        dmName = _menuNames[index];
    }
    cell->cellTitle->setString(dmName);
    cell->cellTitle->setPosition(cocos2d::Point(20*3, cell->contentsView->getContentSize().height/2-cell->cellTitle->getContentSize().height/2));

    if (index==5) {
        cell->cellRightLabel->setString("02.3708.5678");
        cell->cellRightLabel->setPosition(cocos2d::Point(cell->contentsView->getContentSize().width-cell->cellRightLabel->getContentSize().width-40, cell->contentsView->getContentSize().height/2-cell->cellRightLabel->getContentSize().height/2));
    } else if (index==6) {
        cell->cellRightLabel->setString("02.3708.5964");
        cell->cellRightLabel->setPosition(cocos2d::Point(cell->contentsView->getContentSize().width-cell->cellRightLabel->getContentSize().width-40, cell->contentsView->getContentSize().height/2-cell->cellRightLabel->getContentSize().height/2));
    }
    
    return cell;
    
}

void SideMenu::setUserInfo()
{
    _profileButton->setText("N150005");
    float buttonWidth = _profileButton->getIconNode(SMButton::State::NORMAL)->getContentSize().width + 10 + _profileButton->getTextLabel()->getContentSize().width;
    float buttonHeight = MAX(_profileButton->getIconNode(SMButton::State::NORMAL)->getContentSize().height, _profileButton->getTextLabel()->getContentSize().height);
    _profileButton->setContentSize(cocos2d::Size(buttonWidth, buttonHeight*3));
    float topBgHeight = TOP_MENU_HEIGHT;
    float profilePosY = topBgHeight-96-buttonHeight;
    _profileButton->setPosition(cocos2d::Size(45, profilePosY));
    
    _signInButton->setText("로그아웃");
    _signInButton->setPosition(cocos2d::Size(getContentSize().width-_signInButton->getContentSize().width-100, _signInButton->getPosition().y));
    
}

void SideMenu::openMenu(SMScene *mainScenem, const SideMenuOpenClose & callback)
{
    SideMenu::getInstance()->_callback = callback;
    SideMenu::getInstance()->_swipeLayer->open(false);
}

void SideMenu::closeMenu(const SideMenuOpenClose & callback)
{
    SideMenu::getInstance()->_callback = callback;
    SideMenu::getInstance()->_swipeLayer->close(false);
}

void SideMenu::setOpenPosition(const float position)
{
    float f = 0;
    
    if (position >= _contentSize.width) {
        // 완전 열림
        if (_state != kSideMenuState::kSideMenuStateOpen) {
            _state = kSideMenuState::kSideMenuStateOpen;
            if (_callback) {
                _callback();
            }
            if (!isVisible()) {
                setVisible(true);
            }
        }
        f = 1.0;
    } else if (position <= 0) {
        // 완전 닫힘
        if (_state != kSideMenuState::kSideMenuStateClose) {
            _state = kSideMenuState::kSideMenuStateClose;
            if (_swipeLayer) {
                _swipeLayer->closeComplete();
            }
            if (_callback) {
                _callback();
            }
            if (isVisible()) {
                setVisible(false);
//                if (_visibleListener) {
//                    _visibleListener->onSideMenuVisible(false);
//                }
            }
        }
        f = 0.0;
    } else {
        // 이동중.
        if (_state != kSideMenuState::kSideMenuStateMoving) {
            _state = kSideMenuState::kSideMenuStateMoving;
            if (!isVisible()) {
                setVisible(true);
//                if (_visibleListener) {
//                    _visibleListener->onSideMenuVisible(true);
//                }
            }
        }
        
        f = position / _contentSize.width;
        if (f < 0) f = 0;
        else if (f > 1) f = 1;
    }
    
    float x =  -0.3 * (1.0 - f) * _contentSize.width;
    setPositionX(x);
    
    if (_sideMenuUpdateCallback) {
        _sideMenuUpdateCallback(_state, position);
    }
    
    _lastPosition = position;
}

void SideMenu::onClick(SMView *view)
{
    // menu acion & close
    if (view==_profileButton) {
//        onLoginMenu();
    } else if (view==_signInButton) {
        CCLOG("[[[[[ 로그 아웃!!!!");
//        SMComm::getInstance()->signIn(this, "V22114", "q1w2e3r4", "Y", CC_CALLBACK_2(SideMenu::onSignComplete, this));
    }
}

void SideMenu::SideMenuCell::onClick(SMView *view)
{
    
    switch (view->getTag()) {
        case 0:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onNoticeMenu();
            });
            
        }
            break;
        case 1:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onContractMenu();
            });
        }
            break;
        case 2:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onDelayMenu();
            });
        }
            break;
        case 3:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onExchangeMenu();
            });
        }
            break;
        case 4:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onUploadMenu();
            });
        }
            break;
        case 5:
        {
            cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&]{
//                parent->onLogOutMenu();
            });
        }
            break;
        case 6:
        {
            CCLOG("구매담당자");
        }
            break;
        case 7:
        {
            CCLOG("콜센터");
        }
            break;
        case 8:
        {
            CCLOG("현재버전");
        }
            break;
        default:
            break;
    }
}

//void SideMenu::onNoticeMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    
////    SMComm::getInstance()->getNotiList(this, CC_CALLBACK_2(SideMenu::onGetNotiList, this));
//
//    CCLOG("공지사항");
//    auto sceneParam = Intent::create();
//    auto scene = NotiListScene::create(sceneParam, SMScene::SwipeType::BACK);
//    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
//    cocos2d::Director::getInstance()->pushScene(pScene);
//}
//
//void SideMenu::onContractMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    CCLOG("견적계약관리");
//    auto sceneParam = Intent::create();
//    auto scene = EstimateSearchScene::create(sceneParam, SMScene::SwipeType::BACK);
//    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
//    cocos2d::Director::getInstance()->pushScene(pScene);
//}
//
//void SideMenu::onDelayMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    CCLOG("납기지연현황");
//    auto sceneParam = Intent::create();
//    auto scene = DelayScene::create(sceneParam, SMScene::SwipeType::BACK);
//    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
//    cocos2d::Director::getInstance()->pushScene(pScene);
//}
//
//
//void SideMenu::onExchangeMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    CCLOG("교환반품관리");
//    auto sceneParam = Intent::create();
//    auto scene = ExchangeScene::create(sceneParam, SMScene::SwipeType::BACK);
//    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
//    cocos2d::Director::getInstance()->pushScene(pScene);
//}
//
//
//void SideMenu::onUploadMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    CCLOG("이미지등록");
//    auto sceneParam = Intent::create();
//    auto scene = UploadScene::create(sceneParam, SMScene::SwipeType::BACK);
//    auto pScene = SceneTransition::SlideInToLeft::create(SceneTransition::Time::NORMAL, scene);
//    cocos2d::Director::getInstance()->pushScene(pScene);
//}
//
//
//void SideMenu::onLogOutMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
//    CCLOG("로그아웃");
//}
//
//void SideMenu::onLoginMenu()
//{
//    if (_state!=kSideMenuStateClose) {
//        closeMenu();
//    }
////    CCLOG("Login Scene");
////    auto sceneParam = Intent::create();
////    auto scene = LoginScene::create(sceneParam, SMScene::SwipeType::DISMISS);
////    auto pScene = SceneTransition::SlideInToTop::create(SceneTransition::Time::NORMAL, scene);
////    cocos2d::Director::getInstance()->pushScene(pScene);
//}
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif


