//
//  SMApplication.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 8..
//
//
// applicaiton 기본 세팅과 터치 이벤트를 관리


#include "SMApplication.h"
#include "ShaderNode.h"
#include "SMScene.h"

USING_NS_CC;
#define FIXED_DESIGNED_WIDTH (1080)
//#define FIXED_DESIGNED_WIDTH (750)
//static cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);
//static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
//static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
//static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

#define USING_SIDE_MENU

const int SMApplication::VALID_TOUCH_COUNT = 2;


SMApplication::SMApplication() :
_swipeLayer(nullptr),
_touchHandler(nullptr),
_touchMotionTarget(nullptr),
_menuSwipe(nullptr),
_backSwipe(nullptr),
_dismissSwipe(nullptr),
_dimLayer(nullptr),
_sideMenu(nullptr),
_schemeUrl("")
{
    
}

SMApplication::~SMApplication()
{
    //CC_SAFE_RELEASE(SideMenu::getInstance());
}

void SMApplication::setupScaleFactor(const float designWidth)
{
    auto fileUtils = FileUtils::getInstance();
    auto glView = _director->getOpenGLView();
    
    const float scaleFactor = glView->getFrameSize().width / designWidth;
    std::vector<std::string> resolutionOrder;
    float contentScaleFactor = 1.0;
    resolutionOrder.push_back("resources-iphone");
    if (scaleFactor > 1.7) {
        resolutionOrder.push_back("resources-iphonehd");
        contentScaleFactor = 2.0;
    }
    
    if (scaleFactor > 2.7) {
        resolutionOrder.push_back("resources-ipad");
        contentScaleFactor = 3.0;
    }
    
    if (scaleFactor > 3.7) {
        resolutionOrder.push_back("resources-ipadhd");
        contentScaleFactor = 4.0;
    }
    
    std::reverse(resolutionOrder.begin(),resolutionOrder.end());
    
    _director->setContentScaleFactor(contentScaleFactor);
    fileUtils->setSearchResolutionsOrder(resolutionOrder);
}

bool SMApplication::applicationDidFinishLaunching()
{
    _director = Director::getInstance();
    
    
    auto glview = _director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        // for pc
//        cocos2d::Size designResolutionSize = cocos2d::Size(414, 736);
        cocos2d::Size designResolutionSize = cocos2d::Size(375, 667);
        glview = GLViewImpl::createWithRect("SMFrameWork", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        // for mobile
        glview = GLViewImpl::create("SMFrameWork");
#endif
        _director->setOpenGLView(glview);
    }
    
    _director->setClearColor(cocos2d::Color4F(1, 1, 1,  1));
    _director->setContentScaleFactor(1.0f);
    
    
    auto frameSize = glview->getFrameSize();
    float scaleFactor = FIXED_DESIGNED_WIDTH / frameSize.width;
    float designWidth = FIXED_DESIGNED_WIDTH;
    float designHeight = std::floor(scaleFactor * frameSize.height);
    
//    glview->setDesignResolutionSize(designWidth, designHeight, ResolutionPolicy::FIXED_WIDTH);
    glview->setDesignResolutionSize(designWidth, designHeight, ResolutionPolicy::NO_BORDER);
    _director->initDefaultCamera(designWidth, designHeight);

    Size windowSize = _director->getWinSize();
    
    ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH = windowSize.width / frameSize.width;
    

//    _director->setDisplayStats(true);
    
    // touch handler
    _touchHandler = cocos2d::EventListenerTouchOneByOne::create();
    _touchHandler->retain();
    _touchHandler->onTouchBegan = CC_CALLBACK_2(SMApplication::onTouchBegan, this);
    _touchHandler->onTouchMoved = CC_CALLBACK_2(SMApplication::onTouchMoved, this);
    _touchHandler->onTouchCancelled = CC_CALLBACK_2(SMApplication::onTouchCancelled, this);
    _touchHandler->onTouchEnded = CC_CALLBACK_2(SMApplication::onTouchEnded, this);
    _director->getEventDispatcher()->addEventListenerWithFixedPriority(_touchHandler, 1);
    
    _touchHolder.resize(VALID_TOUCH_COUNT);
    for (int id = 0; id < VALID_TOUCH_COUNT; id++) {
        _touchHolder[id] = nullptr;
    }
    
    _lastTouchDownTime = _director->getGlobalTime();
    
    _dismissSwipe = EdgeSwipeForDismiss::create(0, 0, windowSize.width, windowSize.height);
    _dismissSwipe->setSwipeWidth(windowSize.height);
    _dismissSwipe->setEdgeWidth(SMViewConstValue::Size::EDGE_SWIPE_TOP);
    _dismissSwipe->_swipeUpdateCallback = CC_CALLBACK_2(SMApplication::onEdgeDismissUpdateCallback, this);
    _dismissSwipe->retain();
    _dismissSwipe->onEnter();
    _dismissSwipe->onEnterTransitionDidFinish();
    _dismissSwipe->reset();
    
    _backSwipe = EdgeSwipeForBack::create(0, 0, windowSize.width, windowSize.height);
    _backSwipe->setSwipeWidth(windowSize.width);
    _backSwipe->setEdgeWidth(SMViewConstValue::Size::EDGE_SWIPE_MENU);
    _backSwipe->_swipeUpdateCallback = CC_CALLBACK_2(SMApplication::onEdgeBackUpdateCallback, this);
    _backSwipe->retain();
    _backSwipe->onEnter();
    _backSwipe->onEnterTransitionDidFinish();
    _backSwipe->reset();
    
    _dimLayer = SMView::create(0, 0, windowSize.width, windowSize.height);
    _dimLayer->setBackgroundColor4F(cocos2d::Color4F::BLACK);
    _dimLayer->setOpacity(0.0);
    _dimLayer->setVisible(false);
    
#ifdef USING_SIDE_MENU    
    _sideMenu = SideMenu::getInstance();
    _sideMenu->setSideMenuListener(nullptr);
#endif    
    // shared Layer
    _director->setSharedLayer(cocos2d::Director::SharedLayer::BACKGROUND, SMView::create(0, 0, windowSize.width, windowSize.height));
#ifdef USING_SIDE_MENU
    _director->setSharedLayer(cocos2d::Director::SharedLayer::LEFT_MENU, _sideMenu);
#else
	_director->setSharedLayer(cocos2d::Director::SharedLayer::LEFT_MENU, SMView::create(0, 0, windowSize.width, windowSize.height));
#endif
    _director->setSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_MENU_AND_SCENE, SMView::create(0, 0, windowSize.width, windowSize.height));
    /*------------------ SCENE DISPLAY ------------------*/
    _director->setSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_SCENE_AND_UI, SMView::create(0, 0, windowSize.width, windowSize.height));
    _director->setSharedLayer(cocos2d::Director::SharedLayer::UI, SMView::create(0, 0, windowSize.width, windowSize.height));
    _director->setSharedLayer(cocos2d::Director::SharedLayer::BETWEEN_UI_AND_POPUP, SMView::create(0, 0, windowSize.width, windowSize.height));
    _director->setSharedLayer(cocos2d::Director::SharedLayer::DIM, _dimLayer);
    _director->setSharedLayer(cocos2d::Director::SharedLayer::POPUP, SMView::create(0, 0, windowSize.width, windowSize.height));
    
    _menuSwipe = EdgeSwipeForMenu::create(0, 0, windowSize.width, windowSize.height);
    _menuSwipe->retain();
#ifdef USING_SIDE_MENU
    _menuSwipe->setSwipeWidth(_sideMenu->getContentSize().width);
#else
    _menuSwipe->setSwipeWidth(0);
#endif
    _menuSwipe->setEdgeWidth(SMViewConstValue::Size::EDGE_SWIPE_MENU);
    _menuSwipe->setOnClickCallback([&](SMView* view) {
#ifdef USING_SIDE_MENU
        if (_sideMenu && _sideMenu->getState() == kSideMenuStateOpen) {
            float p1 = _sideMenu->getOpenPosition() + Director::getInstance()->getWinSize().width;
            float p2 = _swipeLayer->getLastTouchLocation().x;
            if (p2 < p1) {
                _sideMenu->closeMenu();
            }
        }
#endif
    });
#ifdef USING_SIDE_MENU    
    _sideMenu->setSwipeLayer(_menuSwipe);
    _sideMenu->_sideMenuUpdateCallback = CC_CALLBACK_2(SMApplication::onSideMenuUpdateCallback, this);
#endif
    
    _swipeLayer = SMView::create();
    _swipeLayer->addChild(_menuSwipe);
    _swipeLayer->retain();
    _swipeLayer->onEnter();
    _swipeLayer->onEnterTransitionDidFinish();
    
    
    return true;
}

void SMApplication::onSideMenuUpdateCallback(int state, float position) {
    
    float f = position / _sideMenu->getContentSize().width;
    if (f <= 0) {
        f = 0;
    } else if (f > 1) {
        f = 1;
    }
    
    if (f > 0) {
        if (!_dimLayer->isVisible()) {
            _dimLayer->setVisible(true);
        }
        _dimLayer->setContentSize(cocos2d::Size(_director->getWinSize().width-position, _director->getWinSize().height));
        _dimLayer->setPositionX(position);
        _dimLayer->setOpacity((GLubyte)(0.5 * 255.0 * f));
    } else {
        if (_dimLayer->isVisible()) {
            _dimLayer->setVisible(false);
        }
    }
    
    Scene* runningScene = dynamic_cast<Scene*>(_director->getRunningScene());
    Scene* inScene = nullptr;
    if (runningScene) {
        auto transitionScene = dynamic_cast<TransitionScene*>(runningScene);
        if (transitionScene) {
            inScene = dynamic_cast<Scene*>(transitionScene->getInScene());
            inScene->setPositionX(0);
            runningScene->setPositionX(0);
            transitionScene->setPositionX(position);
            return;
        }
    }
    if (runningScene) {
        runningScene->setPositionX(position);
    }
    
    if (inScene) {
        runningScene->setPositionX(position);
    }
}

void SMApplication::onEdgeBackUpdateCallback(int state, float position) {
    
    Scene* runningScene = dynamic_cast<Scene*>(_director->getRunningScene());
    auto backScene = dynamic_cast<SceneTransition::SwipeBack*>(runningScene);
    
    if (!backScene) {
        if (position > 0) {
            backScene = SceneTransition::SwipeBack::create(_director->getPreviousScene());
            _director->popSceneWithTransition(backScene);
        }
    } else {
        if (backScene && !_backSwipe->isScrollTargeted()) {
            if (position <= 0) {
                backScene->cancel();
                _backSwipe->reset();
            } else if (position >= _backSwipe->getContentSize().width) {
                backScene->finish();
                _backSwipe->reset();
            } else {
                // 터치
                _director->getEventDispatcher()->setEnabled(false);
            }
        }
    }
    
    if (backScene) {
        backScene->getOutScene()->setPositionX(position);
        backScene->getInScene()->setPositionX(0.3 * (-_backSwipe->getContentSize().width + position));
    }
}

void SMApplication::onEdgeDismissUpdateCallback(int state, float position) {
    
    Scene* runningScene = dynamic_cast<Scene*>(_director->getRunningScene());
    auto dismissScene = dynamic_cast<SceneTransition::SwipeDismiss*>(runningScene);
    
    if (!dismissScene) {
        if (position > 0) {
            dismissScene = SceneTransition::SwipeDismiss::create(_director->getPreviousScene());
            _director->popSceneWithTransition(dismissScene);
        }
    } else {
        if (dismissScene && !_dismissSwipe->isScrollTargeted()) {
            if (position <= 0) {
                dismissScene->cancel();
                _dismissSwipe->reset();
            } else if (position >= _dismissSwipe->getContentSize().height) {
                dismissScene->finish();
                _dismissSwipe->reset();
            } else {
                // 터치
                _director->getEventDispatcher()->setEnabled(false);
            }
        }
    }
    
    if (dismissScene) {
        dismissScene->getOutScene()->setPositionY(-position);
    }
}


void SMApplication::handleTouchEvent(const int action, const std::vector<cocos2d::Touch*>& touches, const int actionIndex)
{
    float nowTime = _director->getGlobalTime();
    
    // 터치 다운과 업의 시간이 같을 경우 오류 방지
    if (action == MotionEvent::ACTION_DOWN) {
        _lastTouchDownTime = nowTime;
    } else if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        if (_lastTouchDownTime == nowTime) {
            nowTime = _lastTouchDownTime + (1.0 / 60.0);
        }
    }
    
    _motionEvent.set(action, &touches, nowTime, actionIndex);
    
    SMView* touchLayer = nullptr;
    SMView* newTouchTarget = nullptr;
    int ret = TOUCH_FALSE;
    
    auto worldPoint = _motionEvent.getTouch(0)->getLocation();
    do {
        
        // top most popup
        touchLayer = dynamic_cast<SMView*>(_director->getSharedLayer(cocos2d::Director::SharedLayer::POPUP));
        if (touchLayer && touchLayer->isVisible() && (action == MotionEvent::ACTION_DOWN || _touchMotionTarget == touchLayer)) {
            ret = touchLayer->dispatchTouchEvent(&_motionEvent, touchLayer, false);
        }
        if (ret != TOUCH_FALSE) {
            newTouchTarget = touchLayer;
            break;
        }
        
        // ui
        touchLayer = dynamic_cast<SMView*>(_director->getSharedLayer(cocos2d::Director::SharedLayer::UI));
        if (touchLayer && touchLayer->isVisible() && (action == MotionEvent::ACTION_DOWN || _touchMotionTarget == touchLayer)) {
            ret = touchLayer->dispatchTouchEvent(&_motionEvent, touchLayer, false);
        }
        if (ret != TOUCH_FALSE) {
            newTouchTarget = touchLayer;
            break;
        }
        
        SMScene* runningScene = dynamic_cast<SMScene*>(_director->getRunningScene());
        // swipe back, menu, dismiss
        SceneTransition::SwipeBack* backScene = nullptr;
        SceneTransition::SwipeDismiss* dismissScene = nullptr;
        if (!runningScene) {
            backScene = dynamic_cast<SceneTransition::SwipeBack*>(_director->getRunningScene());
            dismissScene = dynamic_cast<SceneTransition::SwipeDismiss*>(_director->getRunningScene());
        }
        
        SMScene::SwipeType type = SMScene::SwipeType::NONE;
        // 메뉴나 swipe back, dismiss 등을 처리하기
        if (runningScene) {
            type = runningScene->getSwipeType();
        } else {
            if (backScene) {
                type = SMScene::SwipeType::BACK;
                runningScene = (SMScene*)backScene->getOutScene();
            }
            if (dismissScene) {
                type = SMScene::SwipeType::DISMISS;
                runningScene = (SMScene*)dismissScene->getOutScene();
            }
        }
        
        //
        // Swipe 터치를 위하여 swipe의 경우 하위(Scene내의 view등)로 내려가지 않도록 처리...
        if (runningScene) {
            switch (type) {
                case SMScene::SwipeType::MENU:
#ifdef USING_SIDE_MENU
                    while (_swipeLayer && !dynamic_cast<TransitionScene*>(runningScene)) {
                            if (action == MotionEvent::ACTION_DOWN && _menuSwipe->isScrollArea(worldPoint) && !runningScene->canSwipe(worldPoint, type))
                                break;
                            int ret = _swipeLayer->SMView::dispatchTouchEvent(&_motionEvent, _swipeLayer, false);
                            if (ret == TOUCH_INTERCEPT) {
                            if (_touchMotionTarget && _touchMotionTarget != _sideMenu) {
                                    _touchMotionTarget->cancelTouchEvent(_touchMotionTarget, _motionEvent.getTouch(0), &cocos2d::Vec2::ZERO, &_motionEvent);
                                    _touchMotionTarget = nullptr;
                                }
                            }
                        break;
                    }
#endif
                    break;
                case SMScene::SwipeType::BACK:
                    while (_backSwipe) {
                        if (action == MotionEvent::ACTION_DOWN && _backSwipe->isScrollArea(worldPoint) && !runningScene->canSwipe(worldPoint, type))
                            break;
                        
                        int ret = _backSwipe->SMView::dispatchTouchEvent(&_motionEvent, _backSwipe, false);
                        if (ret == TOUCH_INTERCEPT) {
                            if (_touchMotionTarget && _touchMotionTarget != _backSwipe) {
                                _touchMotionTarget->cancelTouchEvent(_touchMotionTarget, _motionEvent.getTouch(0), &cocos2d::Vec2::ZERO, &_motionEvent);
                                _touchMotionTarget = nullptr;
                            }
                        }
                        break;
                    }
                    break;
                case SMScene::SwipeType::DISMISS:
                    while (_dismissSwipe) {
                        int ret = _dismissSwipe->SMView::dispatchTouchEvent(&_motionEvent, _dismissSwipe, false);
                        if (ret == TOUCH_INTERCEPT) {
                            if (_touchMotionTarget && _touchMotionTarget != _dismissSwipe) {
                                _touchMotionTarget->cancelTouchEvent(_touchMotionTarget, _motionEvent.getTouch(0), &cocos2d::Vec2::ZERO, &_motionEvent);
                                _touchMotionTarget = nullptr;
                            }
                        }
                        break;
                    }
                    break;
                    
                default:
                    break;
            }
            
        }

        // scene... 여기서부터 scene이 터치를 받고 그 하위에 view등등이 터치를 받게 한다.
        if (runningScene) {
            touchLayer = runningScene->getRootView();
        }
        if (!_sideMenu || _sideMenu->getState() == kSideMenuStateClose) {
            if (touchLayer && touchLayer->isVisible() && (action == MotionEvent::ACTION_DOWN || _touchMotionTarget == touchLayer)) {
                ret = touchLayer->dispatchTouchEvent(&_motionEvent, touchLayer, true);
                newTouchTarget = touchLayer;
                
                if (ret == TOUCH_FALSE && action == MotionEvent::ACTION_DOWN) {
                    auto point = touchLayer->convertToNodeSpace(_motionEvent.getTouch(0)->getLocation());
                    if (touchLayer->containsPoint(point)) {
                        ret = TOUCH_TRUE;
                    }
                }
            }
            if (ret != TOUCH_FALSE) {
                newTouchTarget = touchLayer;
                    if (_swipeLayer && ret == TOUCH_INTERCEPT && _menuSwipe->isScrollTargeted()) {
                        _swipeLayer->cancelTouchEvent(_swipeLayer, _motionEvent.getTouch(0), &cocos2d::Vec2::ZERO, &_motionEvent);
                    }
                break;
            }
        }
        
        
        if (!_sideMenu || _sideMenu->getState() != kSideMenuStateClose) {
            touchLayer = dynamic_cast<SMView*>(_director->getSharedLayer(cocos2d::Director::SharedLayer::LEFT_MENU));
            if (touchLayer && touchLayer->isVisible() && (action == MotionEvent::ACTION_DOWN || _touchMotionTarget == touchLayer)) {
                ret = touchLayer->dispatchTouchEvent(&_motionEvent, touchLayer, false);
                newTouchTarget = touchLayer;
            }
            if (ret != TOUCH_FALSE) {
                newTouchTarget = touchLayer;
                break;
            }
        }
        
    } while (0);
    
    if (action == MotionEvent::ACTION_DOWN && newTouchTarget) {
        _touchMotionTarget = newTouchTarget;
    } else if (action == MotionEvent::ACTION_UP) {
        _touchMotionTarget = nullptr;
    }
    
}

bool SMApplication::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{
    // 5개 넘으면 cocos2dx가 안받아들임. 오류임. 터치 캔슬
    if (touch->getID() >= cocos2d::EventTouch::MAX_TOUCHES-1) {
        auto touches = buildTouchs();
        if (touches.size() > 0) {
            handleTouchEvent(MotionEvent::ACTION_CANCEL, touches, getTouchIndex(touch));
        }
        clearTouchBit();
        return false;
    }
    
    // 2개 초과 터치 무시
    if (touch->getID() >= VALID_TOUCH_COUNT) {
        return false;
    }
    
    // 2개 이하면
	//CCLOG("[[[[[ set touch bit before");
    setTouchBit(touch);
	//CCLOG("[[[[[ set touch bit after");
    auto touches = buildTouchs();
	//CCLOG("[[[[[ buildTouch after >> touches count :%d", touches.size());
    
    if (touches.size() > 1) {
        if (_director->getOpenGLView()->getAllTouches().size() == 1) {
            // 1개면 나머지 클리어... 방어코드
            clearTouchBit();
            // 1개 받고
            setTouchBit(touch);
            touches = buildTouchs();
        }
    }
    
    if (getTouchCount() == 1) {
        handleTouchEvent(MotionEvent::ACTION_DOWN, touches, getTouchIndex(touch));
    } else {
        handleTouchEvent(MotionEvent::ACTION_POINTER_DOWN, touches, getTouchIndex(touch));
    }
    
    return true;
}

void SMApplication::onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event)
{
    // 2개 초과 무시.. began에서 걸리지기 때문에 안해도 되긴 하는데...
    if (touch->getID() >= VALID_TOUCH_COUNT) {
        return;
    }
    
    // 등록되지 않은 터치 무시
    if (!isTouchBitSetted(touch)) {
        return;
    }
    
    setTouchBit(touch);
    auto touches = buildTouchs();
    
    handleTouchEvent(MotionEvent::ACTION_MOVE, touches, getTouchIndex(touch));
}

void SMApplication::onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event)
{
    // 2개 초과 무시.. began, move에서 걸리지기 때문에 안해도 되긴 하는데...
    if (touch->getID() >= VALID_TOUCH_COUNT) {
        return;
    }
    
    // 등록되지 않은 터치 무시
    if (!isTouchBitSetted(touch)) {
        return;
    }
    
    setTouchBit(touch);
    auto touches = buildTouchs();
    
    if (getTouchCount() == 1) {
        handleTouchEvent(MotionEvent::ACTION_UP, touches, getTouchIndex(touch));
    } else {
        handleTouchEvent(MotionEvent::ACTION_POINTER_UP, touches, getTouchIndex(touch));
    }
    
    // 사용한 터치 초기화
    clearTouchBit(touch);
}

void SMApplication::onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event)
{
    // 2개 초과 무시.. began, move에서 걸리지기 때문에 안해도 되긴 하는데...
    if (touch->getID() >= VALID_TOUCH_COUNT) {
        return;
    }
    
    // 등록되지 않은 터치 무시
    if (!isTouchBitSetted(touch)) {
        return;
    }
    
    setTouchBit(touch);
    auto touches = buildTouchs();
    
    handleTouchEvent(MotionEvent::ACTION_CANCEL, touches, getTouchIndex(touch));
    
    // 사용한 터치 초기화
    clearTouchBit();
}

int SMApplication::getTouchCount() {
    int touchCount = 0;
    for (int id = 0; id < VALID_TOUCH_COUNT; id++) {
        if (_touchHolder[id] != nullptr) touchCount++;
    }
    return touchCount;
}

void SMApplication::setTouchBit(cocos2d::Touch* touch) {
    int id = touch->getID();
    
    if (_touchHolder[id]) {
        auto tmp = _touchHolder[id];
        
        _touchHolder[id] = touch;
        touch->retain();
        
        CC_SAFE_RELEASE(tmp);
    } else {
        _touchHolder[id] = touch;
        touch->retain();
    }
}

int SMApplication::getTouchIndex(cocos2d::Touch* touch) {
    for (size_t index = 0; index < _touchSender.size(); index++) {
        if (_touchSender[index] == touch)
            return index;
    }
    return 0;
}

bool SMApplication::isTouchBitSetted(cocos2d::Touch* touch) {
    return _touchHolder[touch->getID()] != nullptr;
}

void SMApplication::clearTouchBit(cocos2d::Touch* touch) {
    if (touch == nullptr) {
        // clear all
        for (int id = 0; id < VALID_TOUCH_COUNT; id++) {
            if (_touchHolder[id]) {
                CC_SAFE_RELEASE(_touchHolder[id]);
                _touchHolder[id] = nullptr;
            }
        }
    } else {
        if (_touchHolder[touch->getID()]) {
            CC_SAFE_RELEASE(_touchHolder[touch->getID()]);
            _touchHolder[touch->getID()] = nullptr;
        }
    }
}

std::vector<cocos2d::Touch*>& SMApplication::buildTouchs() {
	//CCLOG("[[[[[ buildTouchs 1");
    _touchSender.clear();
    for (int id = 0; id < VALID_TOUCH_COUNT; id++) {
        if (_touchHolder[id]) {
			//CCLOG("[[[[[ buildTouchs 1");
            //_touchSender.emplace_back(_touchHolder[id]);
			_touchSender.push_back(_touchHolder[id]);
//			CCLOG("[[[[[ buildTouchs 2");
        }
    }
    return _touchSender;
}
