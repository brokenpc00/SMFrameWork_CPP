//
//  SMViewConstValue.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "SMViewConstValue.h"

#include <cocos2d.h>

/*************************************************************************
 
 공통 Constant
 
 *************************************************************************/
const cocos2d::Color4F SMViewConstValue::Const::COLOR4F_TRANSPARENT = cocos2d::Color4F(0, 0, 0, 0);
const cocos2d::Color3B SMViewConstValue::Const::LOADING_SPRITE_COLOR = cocos2d::Color3B(0xee, 0xef, 0xf1);
const cocos2d::Color4F SMViewConstValue::Const::KNOB_ON_BGCOLOR = cocos2d::Color4F(0x66/255.0f, 0x66/255.0f, 0x66/255.0f, 1.0f);
const cocos2d::Color4F SMViewConstValue::Const::KNOB_OFF_BGCOLOR = cocos2d::Color4F(0xdb/255.0f, 0xdc/255.0f, 0xdf/255.0f, 1.0f);
const cocos2d::Color4F SMViewConstValue::Const::KNOB_COLOR = cocos2d::Color4F(1, 1, 1, 1);

/*************************************************************************
 
 레이어별 로컬 Z Order 정의
 
 *************************************************************************/
const int SMViewConstValue::ZOrder::USER   = 0;
const int SMViewConstValue::ZOrder::BG     = INT_MIN+1;

// 버튼
const int SMViewConstValue::ZOrder::BUTTON_NORMAL = INT_MIN+100;
const int SMViewConstValue::ZOrder::BUTTON_PRESSED = BUTTON_NORMAL+1;
const int SMViewConstValue::ZOrder::BUTTON_TEXT = BUTTON_PRESSED+1;
const int SMViewConstValue::ZOrder::BUTTON_ICON_NORMAL = BUTTON_TEXT+1;
const int SMViewConstValue::ZOrder::BUTTON_ICON_PRESSED = BUTTON_ICON_NORMAL+1;


/*************************************************************************
 
 뷰 세팅
 
 *************************************************************************/
const float SMViewConstValue::Config::DEFAULT_FONT_SIZE = 12;

const float SMViewConstValue::Config::TAP_TIMEOUT         = 0.5f;
const float SMViewConstValue::Config::DOUBLE_TAP_TIMEOUT  = 0.3f;
const float SMViewConstValue::Config::LONG_PRESS_TIMEOUT  = 0.5f;
const float SMViewConstValue::Config::SCALED_TOUCH_SLOPE = 100.0f;
const float SMViewConstValue::Config::SCALED_DOUBLE_TAB_SLOPE = 100.0f;

const float SMViewConstValue::Config::SMOOTH_DIVIDER       = 3.0;
const float SMViewConstValue::Config::TOLERANCE_POSITION   = 0.01;
const float SMViewConstValue::Config::TOLERANCE_ROTATE     = 0.01;
const float SMViewConstValue::Config::TOLERANCE_SCALE      = 0.005;

const float SMViewConstValue::Config::MIN_VELOCITY     = 500.0;
const float SMViewConstValue::Config::MAX_VELOCITY     = 15000.0;
//const float SMViewConstValue::Config::MAX_VELOCITY     = 10000.0;

const float SMViewConstValue::Config::SCROLL_TOLERANCE = 10.0;
const float SMViewConstValue::Config::SCROLL_HORIZONTAL_TOLERANCE = 20.0;

const float SMViewConstValue::Config::BUTTON_PUSHDOWN_PIXELS = -10.0f;
const float SMViewConstValue::Config::BUTTON_PUSHDOWN_SCALE = 0.9f;
const float SMViewConstValue::Config::BUTTON_STATE_CHANGE_PRESS_TO_NORMAL_TIME = .25f;
const float SMViewConstValue::Config::BUTTON_STATE_CHANGE_NORMAL_TO_PRESS_TIME = .15f;

const float SMViewConstValue::Config::ZOOM_SHORT_TIME = .1f;
const float SMViewConstValue::Config::ZOOM_NORMAL_TIME = .30f;
const float SMViewConstValue::Config::LIST_HIDE_REFRESH_TIME = .1f;



/*************************************************************************
 
 뷰 크기
 
 *************************************************************************/

float SMViewConstValue::Size::getStatusHeight() {
    if (cocos2d::Application::getInstance()->getStatusBarHeight()==0) {
        return TOP_STATUS_HEIGHT;
    } else {
        float statusHeight = cocos2d::Application::getInstance()->getStatusBarHeight();
//        CCLOG("[[[[[ status height : %f", statusHeight);
        return statusHeight;
    }
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
const float SMViewConstValue::Size::TOP_STATUS_HEIGHT = 24;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
const float SMViewConstValue::Size::TOP_STATUS_HEIGHT = 44;
#else
const float SMViewConstValue::Size::TOP_STATUS_HEIGHT = 0;
#endif
//const float SMViewConstValue::Size::TOP_MENU_HEIGHT = 166;
const float SMViewConstValue::Size::TOP_MENU_HEIGHT = 206;
//const float SMViewConstValue::Size::LEFT_SIDE_MENU_WIDTH = 1080*(320-50)/320;
const float SMViewConstValue::Size::LEFT_SIDE_MENU_WIDTH = 550;
const float SMViewConstValue::Size::RIGHT_SIDE_MENU_WIDTH = 500;

// Swipe하기 위한 좌측영역... 60 pixel
const float SMViewConstValue::Size::EDGE_SWIPE_MENU = 100;
// Swipe하기 위한 상단영역... 100 pixel
const float SMViewConstValue::Size::EDGE_SWIPE_TOP = 240;



/*************************************************************************
 
 액션 태그 정의
 
 *************************************************************************/
const int SMViewConstValue::Tag::USER = (0x10000);
const int SMViewConstValue::Tag::SYSTEM = (0x1000);

const int SMViewConstValue::Tag::ACTION_VIEW_SHOW     = (SYSTEM + 1);
const int SMViewConstValue::Tag::ACTION_VIEW_HIDE     = (SYSTEM + 2);
const int SMViewConstValue::Tag::ACTION_BG_COLOR      = (SYSTEM + 3);
const int SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS  = (SYSTEM + 4);
const int SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_PRESS_TO_NORMAL  = (SYSTEM + 5);
const int SMViewConstValue::Tag::ACTION_VIEW_STATE_CHANGE_DELAY  = (SYSTEM + 6);
const int SMViewConstValue::Tag::ACTION_ZOOM  = (SYSTEM + 7);
const int SMViewConstValue::Tag::ACTION_STICKER_REMOVE  = (SYSTEM + 10);

const int SMViewConstValue::Tag::ACTION_LIST_ITEM_DEFAULT  = (SYSTEM + 100);
const int SMViewConstValue::Tag::ACTION_LIST_HIDE_REFRESH  = (SYSTEM + 101);
const int SMViewConstValue::Tag::ACTION_LIST_JUMP  = (SYSTEM + 102);
const int SMViewConstValue::Tag::ACTION_PROGRESS1  = (SYSTEM + 103);
const int SMViewConstValue::Tag::ACTION_PROGRESS2  = (SYSTEM + 104);
const int SMViewConstValue::Tag::ACTION_PROGRESS3  = (SYSTEM + 105);




