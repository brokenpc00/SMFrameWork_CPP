//
//  CalendarDayCell.cpp
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#include "CalendarDayCell.h"
#include <string>
#include <sstream>
#include "../Base/SMButton.h"
#include "../Const/SMFontColor.h"
#include "../Util/ViewUtil.h"

std::string convertInt(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

CalendarDayCell::CalendarDayCell() : _year(0), _month(0), _day(0), _listener(nullptr)
{
    
}

CalendarDayCell::~CalendarDayCell()
{
    
}

CalendarDayCell * CalendarDayCell::create(unsigned int year, unsigned int month, unsigned int day)
{
    CalendarDayCell * cell = new (std::nothrow)CalendarDayCell;
    if (cell && cell->init()) {
        cell->_year = year;
        cell->_month = month;
        cell->_day = day;
        cell->autorelease();
    } else {
        CC_SAFE_DELETE(cell);
    }
    
    return cell;
}

bool CalendarDayCell::init()
{
    if (!SMView::init()) {
        return false;
    }
    
    setContentSize(cocos2d::Size(CELL_SIZE, CELL_SIZE));
    
    return true;
}

void CalendarDayCell::onEnter()
{
    auto button = SMButton::create(0, SMButton::Style::SOLID_RECT, 0, 0, CELL_SIZE, CELL_SIZE);
    button->setTextTTF(convertInt(_day), SMFontConst::NotoSansLight, CELL_FONT_SIZE);
    button->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    button->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
    button->setShapeLineWidth(2.0f);
    button->setShapeCornerRadius(CELL_SIZE/2);
    if (_isToday) {
        button->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xdbdcdf, 1.0f));
        button->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
    } else {
        button->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xeeeff1, 1.0f));
        button->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xadafb3, 1.0f));
    }
    
    button->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    button->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x666666, 1.0f));
    button->setOnClickListener(this);
    addChild(button);
    SMView::onEnter();
}

int CalendarDayCell::getWeekDays()
{
    struct tm * fmt = new tm;
    
    fmt->tm_year = _year - 1900;
    fmt->tm_mon = _month;
    fmt->tm_mday = _day;
    fmt->tm_hour = 1;
    fmt->tm_min = 1;
    fmt->tm_sec = 1;
    mktime(fmt);
    int weekday = fmt->tm_wday;
    delete fmt;
    return weekday;
}

void CalendarDayCell::onClick(SMView *view)
{
    if (_listener) {
        _listener->onCalenderCellClick(_year, _month+1, _day);
    }
}
