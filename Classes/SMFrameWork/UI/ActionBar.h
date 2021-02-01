//
//  ActionBar.h
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 6..
//
//

#ifndef ActionBar_h
#define ActionBar_h

#include "../Base/SMView.h"
#include <2d/CCLabel.h>
#include <string>
#include <vector>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define STATUS_BAR_HEIGHT       (48.0)
#define MENU_BAR_HEIGHT (88.0)
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#define STATUS_BAR_HEIGHT       (80.0)
#define MENU_BAR_HEIGHT (132.0)
#else
#define STATUS_BAR_HEIGHT       (48.0)
#define MENU_BAR_HEIGHT (88.0)
#endif

#define ACTION_BAR_HEIGHT       (STATUS_BAR_HEIGHT+MENU_BAR_HEIGHT)

class ShapeRoundLine;
class ShapeSolidCircle;
class SMButton;

class ActionBarListener {
public:
    virtual bool onActionBarClick(SMView * view) {return false;}
    virtual void onActionBarTouch() {}
    virtual void goHome() = 0;
};

class ActionBar : public SMView, public OnClickListener
{
public:
    
    enum DropDown {
        NOTHING = 0,
        UP,
        DOWN,
    };
    
    enum MenuType {
        NONE = 0,
        // drop down menu (for gallery group selection)
        DROPDOWN = 0x900,
        
        // left menu
        MENU_MENU = 0x1000,
        MENU_BACK,
        MENU_CLOSE,
        MENU_DOT,
        
        // right menu (ICON)
        CART,
        SEARCH,
        CONFIRM,
        DELETE,
        CAMERA,
        ALBUM,
        
        // right menu (TEXT)
        NEXT,
        DONE,
        CANCEL,
    };
    
    enum TextTransition {
        FADE = 0,
        ELASTIC,
        SWIPE
    };

    enum ButtonTransition {
        BTN_FADE = 0,
        BTN_ELASTIC,
        BTN_SWIPE
    };

    struct ColorSet {
        cocos2d::Color4F bg;
        cocos2d::Color4F text;
        cocos2d::Color4F normal;
        cocos2d::Color4F press;
        
        ColorSet();
        
        ColorSet(const cocos2d::Color4F& bg, const cocos2d::Color4F& text, const cocos2d::Color4F& normal, const cocos2d::Color4F& press);
        
        ColorSet& operator=(const ColorSet& rhs);
        bool operator==(const ColorSet& rhs) const;
        
        static const ColorSet WHITE;
        static const ColorSet BLACK;
        static const ColorSet NONE;
        static const ColorSet WHITE_TRANSLUCENT;
        static const ColorSet TRANSLUCENT;
        static const ColorSet VIOLET;
    };
    
    struct DotPosition;
    
    static ActionBar* create();
    
    void setMenuButtonType(MenuType menuButtonType, bool immediate, bool swipe=false);
    void setColorSet(const ColorSet& colorSet, bool immediate);
    
    void useSystemFont();
    
    void setText(const std::string& textString, bool immediate, bool dropdown=false);
    std::string getText();
    void setTextTransitionType(TextTransition type);
    void setTextWithDropDown(const std::string& textString, bool immediate);
    
    void setButtonTransitionType(ButtonTransition type);
    
    void setOneButton(MenuType buttonType, bool immediate, bool swipe=false);
    void setTwoButton(MenuType buttonType1, MenuType buttonType2, bool immediate, bool swipe=false);
    
    void setDropDown(DropDown dropdown, bool immediate, float delay=0);
    void showButton(bool show, bool immediate);
    void showActionButtonWithDelay(bool show, float delay);
    void showMenuButton(bool show, bool immediate);
    
    DropDown getDropDownState() { return _dropdown; }
    
    void setActionBarListener(ActionBarListener* l);
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    void setTextOffsetY(float textOffsetY);
    void setOverlapChild(SMView* child);
    
    SMView* getOverlapChild() { return _overlapChild; }
    
    virtual bool containsPoint(const cocos2d::Vec2& point) override;
    
    ActionBarListener* getActionBarListener() { return _listener; }
    
    MenuType getMenuButtonType() {return _menuButtonType;}
    
    std::string getMenuText() {return _textString;}
    
    std::vector<MenuType> getButtonTypes();
    
    void onSwipeStart();

    void onSwipeUpdate(float t);

    void onSwipeComplete();

    void onSwipeCancel();

    
protected:
    ActionBar();
    virtual ~ActionBar();
    virtual bool init() override;
    
    virtual void onClick(SMView* view) override;
    
    cocos2d::Sprite* _dropdownButton;
    class TextContainer;
    TextContainer* _textContainer;
    
private:
    void applyColorSet(const ColorSet& colorSet);
    void updateTextPosition(bool dropdown);
    
private:
    class MenuTransform;
    class ColorTransform;
    class TextTransform;
    
    class ButtonAction;
    class ButtonFadeAction;
    class DropDownAction;
    
    cocos2d::Node* _buttonContainer;
    SMButton* _menuButton;
    SMButton* _actionButton[2][2];
    
    ShapeRoundLine* _menuLine[4];
    ShapeSolidCircle* _menuCircle[4];
    
    MenuTransform* _menuTransform;
    TextTransform* _textTransform;
    ColorTransform* _colorTransform;
    
    MenuType _menuButtonType;
    ActionBarListener* _listener;
    
    uint32_t _bgColor;
    ColorSet _colorSet;
    ColorSet _activeColorSet;
    
    cocos2d::Label* _textLabel[2];
    std::string _textString;
    int _textIndex;
    int _buttonIndex;
  
    DropDown _dropdown;
    DropDownAction* _dropdownAction;
    TextTransition _textTransType;
    ButtonTransition _buttonTransType;
    SMView* _overlapChild;
    
    bool _useSystemFont;
};


#endif /* ActionBar_h */
