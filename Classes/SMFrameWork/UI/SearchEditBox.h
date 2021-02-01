//
//  SearchEditBox.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 21..
//
//

#ifndef SearchEditBox_h
#define SearchEditBox_h

#include "../Base/SMView.h"
#include <2d/CCLabel.h>
#include <ui/UIEditBox/UIEditBox.h>

class SearchEditBoxListener;

class SearchEditBox : public SMView, protected OnClickListener, public cocos2d::ui::EditBoxDelegate, public cocos2d::IMEDelegate {
public:
    static SearchEditBox* create(SearchEditBoxListener* l);
    void closeKeyboard();
    bool isKeyboardOpened() { return _keybordOpened; }
    std::string getText();
    void onCloseKeyboard();
    void setActive(bool active);
    void setText(const std::string& text);
    void cancelInput();
    bool isActive() { return _isActive; }
    virtual bool containsPoint(const cocos2d::Vec2& point) override;
    
protected:
    SearchEditBox();
    virtual ~SearchEditBox();
    bool initWithListener(SearchEditBoxListener* l);
    virtual void onClick(SMView* view) override;
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch* touch, const cocos2d::Vec2* point, MotionEvent* event) override;
    
    
    // editbox delegate
    virtual void editBoxEditingDidBegin(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxEditingDidEnd(cocos2d::ui::EditBox* editBox) override;
    virtual void editBoxTextChanged(cocos2d::ui::EditBox* editBox, const std::string& text) override;
    virtual void editBoxReturn(cocos2d::ui::EditBox* editBox) override;
    
    // ime delegate
    virtual void keyboardWillShow(cocos2d::IMEKeyboardNotificationInfo& info) override;
    virtual void keyboardWillHide(cocos2d::IMEKeyboardNotificationInfo& info) override;
    
    void enableEditBox(bool enabled);
    
private:
    void onActiveEditBox(cocos2d::Node* target, int tag);

private:
    class Container;
    
    class ActiveAction;
    
    Container* _container;
    
    cocos2d::ui::EditBox* _editBox;
    
    bool _keybordOpened;
    
    bool _isActive;
    
    SearchEditBoxListener* _listener;    
};


class SearchEditBoxListener {
public:
    virtual void onEditInputChanged(SearchEditBox* editBox) = 0;
    
    virtual void onEditReturn(SearchEditBox* editBox) = 0;
    
    virtual void onEditActive(SearchEditBox* editBox, bool active) = 0;
    
};


#endif /* SearchEditBox_h */
