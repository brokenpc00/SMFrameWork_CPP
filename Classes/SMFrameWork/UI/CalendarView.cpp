//
//  CalendarView.cpp
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#include "CalendarView.h"
#include "../Const/SMFontColor.h"
#include "../Util/ViewUtil.h"

CalendarView::CalendarView() : _calendar(nullptr), _listener(nullptr), _currentYear(0), _currentMonth(0), _currentDay(0), _leftButton(nullptr), _rightButton(nullptr)
{
    _selectDayCallback = nullptr;
}

CalendarView::~CalendarView()
{
    
}

CalendarView * CalendarView::createWithDate(unsigned int year, unsigned int month, unsigned int day, OnSelectDateCallback callback, std::string title)
{
    CalendarView * view = new (std::nothrow)CalendarView;
    if (view) {
        cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
        view->setContentSize(s);
        view->setPosition(cocos2d::Vec2::ZERO);
        view->setAnchorPoint(cocos2d::Vec2::ZERO);
        view->_currentYear = year;
        view->_currentMonth = month;
        view->_currentDay = day;
        view->_selectDayCallback = callback;
        view->_title = title;
        if (view->init()) {
            view->autorelease();
        } else {
            CC_SAFE_RELEASE(view);
        }
    } else {
        CC_SAFE_DELETE(view);
    }
    
    return view;
}

bool CalendarView::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    
    
    // 배경뷰는 전체 뷰
    // calendar view는 가로 CELL_SIZE *7, 세로는 CELL_SIZE*5 + 날짜 및 좌우 버튼... 대충 CELL_SIZE*8로 잡자...
    // 나타나는건 화면 정 중앙...
    float calendarWidth = CELL_SIZE * 8;
    float calendarHeight = CELL_SIZE * 9;
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    
    _bgView = SMView::create(0, 0, s.width, s.height);
    _bgView->setBackgroundColor4F(MAKE_COLOR4F(0x000000, 0.8f));
    _bgView->setOnLongClickCallback([&](SMView * view){});
    addChild(_bgView);
    
    _contentView = SMView::create(0, s.width/2-calendarWidth/2, s.height/2-calendarHeight/2+(CELL_SIZE*3)/2, calendarWidth, calendarHeight);
    _contentView->setBackgroundColor4F(MAKE_COLOR4F(0xdbdcdf, 1.0f));
    addChild(_contentView);
    
    if (_title=="") {
        _title = "CALENDAR";
    }
    _titleLabel = cocos2d::Label::createWithTTF(_title, SMFontConst::NotoSansLight, 30);
    _titleLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
    _titleLabel->setPosition(cocos2d::Vec2(calendarWidth/2-_titleLabel->getContentSize().width/2, calendarHeight-70));
    _titleLabel->setTextColor(cocos2d::Color4B::BLACK);
    _contentView->addChild(_titleLabel);


    _leftButton = SMButton::create(0, SMButton::Style::DEFAULT, 50, calendarHeight-180, CELL_SIZE, CELL_SIZE);
    _leftButton->setIcon(SMButton::State::NORMAL, "images/leftarrow.png");
    _leftButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
    _leftButton->setIconColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
    _leftButton->setOnClickListener(this);
    _rightButton = SMButton::create(1, SMButton::Style::DEFAULT, calendarWidth-50-CELL_SIZE, calendarHeight-180, CELL_SIZE, CELL_SIZE);
    _rightButton->setIcon(SMButton::State::NORMAL, "images/rightarrow.png");
    _rightButton->setIconColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
    _rightButton->setIconColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
    _rightButton->setOnClickListener(this);
    
    _contentView->addChild(_leftButton);
    _contentView->addChild(_rightButton);

    _calendar = Calendar::create(_currentYear, _currentMonth, _currentDay);
    _calendar->setPosition(cocos2d::Vec2(CELL_SIZE/2, CELL_SIZE));
    _contentView->addChild(_calendar);
    _calendar->setCalendarClickListener(this);
    
    _currentYear = _calendar->getYear();
    _currentMonth = _calendar->getMonth();

    std::string dateString = cocos2d::StringUtils::format("%s년 %s월", convertInt(_currentYear).c_str(), convertInt(_currentMonth+1).c_str());
    _dateLabel = cocos2d::Label::createWithTTF(dateString, SMFontConst::NotoSansLight, 30);
    _dateLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
    _dateLabel->setPosition(cocos2d::Vec2(calendarWidth/2-_dateLabel->getContentSize().width/2, calendarHeight-150));
    _dateLabel->setTextColor(cocos2d::Color4B::BLACK);
    _contentView->addChild(_dateLabel);
    
    
    return true;
}

void CalendarView::setTitle(std::string title)
{
    _title = title;
    _titleLabel->setString(_title);
    cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();
    float calendarWidth = CELL_SIZE * 8;
    _titleLabel->setPosition(cocos2d::Vec2(calendarWidth/2 - _titleLabel->getContentSize().width/2, _titleLabel->getPosition().y));
}

void CalendarView::onClick(SMView *view)
{
    if (view==_leftButton) {
        // prevMonth
        CCLOG("[[[[[ Left button");
        if (_currentMonth==0) {
            _currentYear--;
            _currentMonth = 11;
        } else {
            _currentMonth--;
        }
    } else if (view==_rightButton) {
        // nextMonth
        CCLOG("[[[[[ right button");
        if (_currentMonth==11) {
            _currentYear++;
            _currentMonth = 0;
        } else {
            _currentMonth++;
        }
    }
    _currentDay = 0;
    std::string dateString = cocos2d::StringUtils::format("%s년 %s월", convertInt(_currentYear).c_str(), convertInt(_currentMonth+1).c_str());
    _dateLabel->setString(dateString);
    
    _calendar->reset(_currentYear, _currentMonth, _currentDay);
}

int CalendarView::dispatchTouchEvent(const int action, const cocos2d::Touch *touch, const cocos2d::Vec2 *point, MotionEvent *event)
{
    int ret = SMView::dispatchTouchEvent(action, touch, point, event);
  
    const SMView * target = getMotionTarget();
    
    if (target==_bgView) {
        return TOUCH_TRUE;
    }
    
    return ret;
    
}

void CalendarView::onSelectDay(int year, int month, int day)
{
    _currentYear = year;
    _currentMonth = month;
    _currentDay = day;
    if (_selectDayCallback) {
        _selectDayCallback(this, year, month, day);
    }
    if (_listener) {
        _listener->onLastSelectDay(this, year, month, day);
    }
}


