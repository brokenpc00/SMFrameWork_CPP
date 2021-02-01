//
//  Calendar.cpp
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#include "Calendar.h"

Calendar::Calendar() : _gapX(0), _gapY(0), _year(0), _month(0), _listener(nullptr)
{
    _gridCellList.clear();
}

Calendar::~Calendar()
{
    _gridCellList.clear();
}

Calendar * Calendar::create(unsigned int year, unsigned int month, unsigned int day)
{
    Calendar * calendar = new (std::nothrow)Calendar;
    calendar->_year = year;
    calendar->_month = month;
    calendar->_day = day;
    calendar->setAnchorPoint(cocos2d::Vec2::ZERO);
    calendar->setContentSize(cocos2d::Size(CELL_SIZE*7, CELL_SIZE*5));
    if ( calendar && calendar->init()) {
        calendar->autorelease();
    } else {
        CC_SAFE_DELETE(calendar);
    }
    
    return calendar;
}

void Calendar::reset(unsigned int year, unsigned int month, unsigned int day)
{
    for (int i=0; i<_gridCellList.size(); i++) {
        CalendarDayCell * cell = _gridCellList[i];
        cell->removeFromParent();
    }
    _gridCellList.clear();
    
    _year = year;
    _month = month;
    _day = day;
    
    makeCellbutton();
}

void Calendar::makeCellbutton()
{
    if (_year==0 && _month==0) {
        time_t now;
        struct tm *fmt;
        time(&now);
        fmt = localtime(&now);
        _year = fmt->tm_year + 1900;
        _month = fmt->tm_mon;
    }
    
    struct tm *fmt = new tm;
    fmt->tm_year = _year - 1900;
    fmt->tm_mon = _month;
    fmt->tm_mday = 1;
    fmt->tm_hour = 1;
    fmt->tm_min = 1;
    fmt->tm_sec = 1;
    mktime(fmt);
    _firstDayOfTheWeekThisMonth = fmt->tm_wday;
    delete fmt;
    
    int daysInMonth =  getMonthDays();
    
    for (int i = 1; i <= daysInMonth; i ++)
    {
        CalendarDayCell * cell = CalendarDayCell::create(_year, _month, i);
        cell->setAnchorPoint(cocos2d::Vec2::ZERO);
        cell->setContentSize(cocos2d::Size(CELL_SIZE, CELL_SIZE));
        cell->setCalendarDayCellListener(this);
        cell->setToday(_day==i);
        addGridCell(cell);
    }
}

bool Calendar::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setBackgroundColor4F(cocos2d::Color4F(0, 1, 0, 0.6f));
    
    _gapX = CELL_SIZE;
    _gapY = CELL_SIZE;
    
    makeCellbutton();
    
    return true;
}

void Calendar::addGridCell(CalendarDayCell *cell)
{
//    cell->retain();
    cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
    if (_gridCellList.size()<=0) {
        // first cell
        pos = cocos2d::Vec2(cell->getContentSize().width*_firstDayOfTheWeekThisMonth, getContentSize().height-(cell->getContentSize().height));
    } else {
        pos = getLastAvailableCellPosition();
    }
    cell->setPosition(pos);
    
    _gridCellList.push_back(cell);
    addChild(cell);
}

int Calendar::getColumnCount()
{
    return 7;
}

int Calendar::getRowCount()
{
    return 5;
}

cocos2d::Vec2 Calendar::getLastAvailableCellPosition()
{
    if (_gridCellList.size()<=0) {
        return cocos2d::Vec2::ZERO;
    }
    
    CalendarDayCell * lastCell = _gridCellList[_gridCellList.size()-1];
    if (lastCell->getWeekDays()==6) {
        return cocos2d::Vec2(0, lastCell->getPosition().y-lastCell->getContentSize().height);
    } else {
        return cocos2d::Vec2(lastCell->getPosition().x+lastCell->getContentSize().width, lastCell->getPosition().y);
    }
}

int Calendar::getMonthDays()
{
    switch (_month) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 7:
        case 9:
        case 11:
            return 31;
            break;
        case 1:
        {
            // 2월
            if ((_year % 4 == 0 && _year % 100 != 0)|| _year % 400 == 0)
            {
                return 29;
            }
            else
            {
                return 28;
            }
            break;
        }
        case 3:
        case 5:
        case 8:
        case 10:
            return 30;
            break;
        default:
            return 0;
            break;
    }
}

void Calendar::onCalenderCellClick(int year, int month, int day)
{
    if (_listener) {
        _listener->onSelectDay(year, month, day);
    }
}
