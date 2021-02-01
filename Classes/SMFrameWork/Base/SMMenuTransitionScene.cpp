//
//  SMMenuTransitionScene.cpp
//  FreeTrip
//
//  Created by N15051 on 2020/04/07.
//

#include "SMMenuTransitionScene.h"
#include "../Util/ViewUtil.h"
#include "ViewAction.h"


SMMenuTransitionScene::SMMenuTransitionScene() : _menuBarTitle("")
, _menuBar(nullptr)
, _swipeStarted(false)
, _prevMenuTitle("")
, _toMenuType(ActionBar::MenuType::NONE)
, _fromMenuType(ActionBar::MenuType::NONE)
{
    _menuBarButton[0] = ActionBar::MenuType::NONE;
    _menuBarButton[1] = ActionBar::MenuType::NONE;
}

SMMenuTransitionScene::~SMMenuTransitionScene()
{
    
}

bool SMMenuTransitionScene::initWithMenuBar(ActionBar * menuBar, Intent* sceneParam, SwipeType type)
{
    if (!SMScene::initWithSceneParam(sceneParam, type)) {
        return false;
    }
    
    _fromMenuType = menuBar->getMenuButtonType();
    _menuBar = menuBar;
    
    auto s = cocos2d::Director::getInstance()->getWinSize();
    
    if (_menuBar) {
        SMView * layer = (SMView*)_director->getSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_SCENE_AND_UI);
        if (layer) {
            _menuBar->changeParent(layer);
            
            _prevMenuTitle = _menuBar->getMenuText();
            _prevManuBarButton = _menuBar->getButtonTypes();
            
            
            _menuBar->setTextTransitionType(ActionBar::TextTransition::FADE);
            
            switch (getSwipeType()) {
                case SwipeType::MENU:
                    _toMenuType = ActionBar::MenuType::MENU_MENU;
                    break;
                case SwipeType::DISMISS:
                    _toMenuType = ActionBar::MenuType::MENU_CLOSE;
                    break;
                default:
                    _toMenuType = ActionBar::MenuType::MENU_BACK;
                    break;
            }
            
            _menuBar->setMenuButtonType(_toMenuType, false);
            _menuBar->setButtonTransitionType(ActionBar::ButtonTransition::BTN_FADE);
            _menuBar->setActionBarListener(nullptr);
        }
        return true;
    }
    
    return false;
}

void SMMenuTransitionScene::setActionBarButton(ActionBar::MenuType btn1, ActionBar::MenuType btn2)
{
    _menuBarButton[0] = btn1;
    _menuBarButton[1] = btn2;
}

void SMMenuTransitionScene::setActionBarTitle(const std::string& titleText)
{
    _menuBarTitle = titleText;
}

void SMMenuTransitionScene::setActionBarMenu(ActionBar::MenuType menuButtonType)
{
    _menuBarMenu = menuButtonType;
}

void SMMenuTransitionScene::setActionBarColorSet(const ActionBar::ColorSet& colorSet)
{
    _menuBarColorSet = colorSet;
}


bool SMMenuTransitionScene::onActionBarClick(SMView *view)
{
    return false;
}

void SMMenuTransitionScene::goHome()
{
    
}

void SMMenuTransitionScene::onTransitionStart(const Transition type, const int tag)
{
    if (type==Transition::Transition_IN) {
        if (_menuBar) {
            _menuBar
            ->setColorSet(_menuBarColorSet, false);
            _menuBar->setText(_menuBarTitle, false);
            _menuBar->setTwoButton(_menuBarButton[0], _menuBarButton[1], false);
        }
    }

    if (type==Transition::Transition_OUT || type==Transition::Transition_SWIPE_OUT) {
        if (_menuBar==nullptr) {
            return;
        }
        
        SMView * layer = (SMView*)_director->getSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_SCENE_AND_UI);
        _menuBar->changeParent(layer);
    
    if (type==Transition::Transition_OUT) {
        _menuBar->setTextTransitionType(ActionBar::TextTransition::FADE);
        _menuBar->setText(_prevMenuTitle, false);
            _menuBar->setMenuButtonType(_fromMenuType, false, false);
        _menuBar->setButtonTransitionType(ActionBar::ButtonTransition::BTN_FADE);
        int numButtons = (int)_prevManuBarButton.size();
        if (numButtons == 1) {
                _menuBar->setOneButton(_prevManuBarButton.at(0), false, false);
        } else if (numButtons == 2) {
                _menuBar->setTwoButton(_prevManuBarButton.at(0), _prevManuBarButton.at(1), false, false);
        } else {
            _menuBar->setOneButton(ActionBar::MenuType::NONE, false, false);
        }
    } else {
        _menuBar->setTextTransitionType(ActionBar::TextTransition::SWIPE);
        _menuBar->setText(_prevMenuTitle, false);
            _menuBar->setMenuButtonType(_fromMenuType, false, true);
        _menuBar->setButtonTransitionType(ActionBar::ButtonTransition::BTN_FADE);
        int numButtons = (int)_prevManuBarButton.size();
        if (numButtons == 1) {
                _menuBar->setOneButton(_prevManuBarButton.at(0), true, true);
        } else if (numButtons == 2) {
                _menuBar->setTwoButton(_prevManuBarButton.at(0), _prevManuBarButton.at(1), true, true);
        } else {
            _menuBar->setOneButton(ActionBar::MenuType::NONE, true, true);
        }
        
        _menuBar->onSwipeStart();

    }
    }
    

}

void SMMenuTransitionScene::onTransitionProgress(const Transition type, const int tag, const float progress)
{
    if (type == Transition::Transition_SWIPE_OUT) {
        if (_menuBar) {
            _menuBar->onSwipeUpdate(progress);
        }
        
        _swipeStarted = true;
    }
}

void SMMenuTransitionScene::onTransitionComplete(const Transition type, const int tag)
{
    bool actionBarReturn = false;
    if (_swipeStarted) {
        if (type == Transition::Transition_SWIPE_OUT) {
            _menuBar->onSwipeComplete();
        } else if (type == Transition::Transition_RESUME) {
            _menuBar->onSwipeCancel();
            actionBarReturn = true;
        }
    }
    _swipeStarted = false;
    
    
    if (type == Transition::Transition_IN || actionBarReturn) {
        // 액션바 회수
        auto layer = _director->getSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_SCENE_AND_UI);
        auto children = layer->getChildren();
        for (auto child : children) {
            if (child == _menuBar) {
                ViewUtil::adoptionTo(child, this);
                _menuBar->setActionBarListener(this);
                break;
            }
        }
    }
}

