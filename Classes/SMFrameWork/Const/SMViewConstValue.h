//
//  SMViewConstValue.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef SMViewConstValue_h
#define SMViewConstValue_h

#include <base/ccTypes.h>


class SMViewConstValue {
public:
    
    /*************************************************************************
     
     레이어별 로컬 Z Order 정의
     
     *************************************************************************/
    class ZOrder {
    public:
        static const int BG;
        static const int USER;
        static const int BUTTON_NORMAL;
        static const int BUTTON_PRESSED;
        static const int BUTTON_ICON_NORMAL;
        static const int BUTTON_ICON_PRESSED;
        static const int BUTTON_TEXT;
    };

    /*************************************************************************
     
     뷰 세팅
     
     *************************************************************************/
    class Config {
    public:
        static const float DEFAULT_FONT_SIZE;
        
        static const float TAP_TIMEOUT;
        static const float DOUBLE_TAP_TIMEOUT;
        static const float LONG_PRESS_TIMEOUT;
        
        static const float SCALED_TOUCH_SLOPE;
        static const float SCALED_DOUBLE_TAB_SLOPE;
        
        static const float SMOOTH_DIVIDER;
        static const float TOLERANCE_POSITION;
        static const float TOLERANCE_ROTATE;
        static const float TOLERANCE_SCALE;
        
        static const float MIN_VELOCITY;
        static const float MAX_VELOCITY;
        
        static const float SCROLL_TOLERANCE;
        static const float SCROLL_HORIZONTAL_TOLERANCE;
        
        static const float BUTTON_PUSHDOWN_PIXELS;
        static const float BUTTON_PUSHDOWN_SCALE;
        static const float BUTTON_STATE_CHANGE_PRESS_TO_NORMAL_TIME;
        static const float BUTTON_STATE_CHANGE_NORMAL_TO_PRESS_TIME;
        
        static const float ZOOM_SHORT_TIME;
        static const float ZOOM_NORMAL_TIME;
        static const float LIST_HIDE_REFRESH_TIME;
        
    };
    
    
    /*************************************************************************
     
     태그 정의
     
     *************************************************************************/
    class Tag {
        
    public:
        static const int USER;
        static const int SYSTEM;
        
        static const int ACTION_VIEW_SHOW;
        static const int ACTION_VIEW_HIDE;
        static const int ACTION_BG_COLOR;
        static const int ACTION_VIEW_STATE_CHANGE_NORMAL_TO_PRESS;
        static const int ACTION_VIEW_STATE_CHANGE_PRESS_TO_NORMAL;
        static const int ACTION_VIEW_STATE_CHANGE_DELAY;
        static const int ACTION_ZOOM;
        static const int ACTION_STICKER_REMOVE;
        static const int ACTION_LIST_ITEM_DEFAULT;
        static const int ACTION_LIST_HIDE_REFRESH;
        static const int ACTION_LIST_JUMP;
        static const int ACTION_PROGRESS1;
        static const int ACTION_PROGRESS2;
        static const int ACTION_PROGRESS3;
    };
    
    class Const {
        
    public:
        static const cocos2d::Color4F COLOR4F_TRANSPARENT;
        static const cocos2d::Color3B LOADING_SPRITE_COLOR;
        static const cocos2d::Color4F KNOB_ON_BGCOLOR;
        static const cocos2d::Color4F KNOB_OFF_BGCOLOR;
        static const cocos2d::Color4F KNOB_COLOR;
    };
    
    class Size {
    public:
        static float getStatusHeight();
        static const float TOP_STATUS_HEIGHT;
        static const float TOP_MENU_HEIGHT;
        static const float LEFT_SIDE_MENU_WIDTH;
        static const float RIGHT_SIDE_MENU_WIDTH;
        static const float EDGE_SWIPE_MENU;
        static const float EDGE_SWIPE_TOP;
    };
    
};

#endif /* SMViewConstValue_h */
