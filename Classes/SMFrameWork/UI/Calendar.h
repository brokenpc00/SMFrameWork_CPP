//
//  Calendar.h
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#ifndef Calendar_h
#define Calendar_h

#include "../Base/SMView.h"
#include "CalendarDayCell.h"

class SMButton;

class CalendarClickListener
{
public:
    virtual void onSelectDay(int year, int month, int day) = 0;
};

class Calendar : public SMView, public CalendarDayCellListener
{
public:
    Calendar();
    virtual ~Calendar();
    
    static Calendar * create(unsigned int year, unsigned int month, unsigned int day);
    void reset(unsigned int year, unsigned int month, unsigned int day);
    
    std::vector<CalendarDayCell*> getGridCellList() {return _gridCellList;}
    void setGridCellList(std::vector<CalendarDayCell*> list) {_gridCellList = list;}
    
    unsigned int getYear() {return _year;}
    unsigned int getMonth() {return _month;}
    
    float getGapX() {return _gapX;}
    float getGapY() {return _gapY;}
    
    void addGridCell(CalendarDayCell* cell);
    int getColumnCount();
    int getRowCount();
    cocos2d::Vec2 getLastAvailableCellPosition();
    void setCalendarClickListener(CalendarClickListener * l) {_listener = l;}
    
    int getMonthDays();
    
    virtual void onCalenderCellClick(int year, int month, int day) override;
    
protected:
    virtual bool init() override;
    
    unsigned int _firstDayOfTheWeekThisMonth;
    
    void makeCellbutton();
    
private:
    CalendarClickListener * _listener;
    std::vector<CalendarDayCell*> _gridCellList;
    SMButton * _calButton;
    unsigned int _year;
    unsigned int _month;
    unsigned int _day;
    float _gapX;
    float _gapY;
};

#endif /* Calendar_h */
