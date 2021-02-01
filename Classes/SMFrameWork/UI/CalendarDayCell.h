//
//  CalendarDayCell.h
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#ifndef CalendarDayCell_h
#define CalendarDayCell_h

#include "../Base/SMView.h"

#define CELL_SIZE 80
#define CELL_FONT_SIZE 30.0f

std::string convertInt(int number);

class CalendarDayCellListener
{
public:
    virtual void onCalenderCellClick(int year, int month, int day) = 0;
};

class CalendarDayCell : public SMView, public OnClickListener
{
public:
    CalendarDayCell();
    virtual ~CalendarDayCell();
    static CalendarDayCell * create(unsigned int year, unsigned int month, unsigned int day);
    int getWeekDays();
    
    void setCalendarDayCellListener(CalendarDayCellListener* l) {_listener = l;}
    void setToday(bool bToday) {_isToday = bToday;}
    bool getToday() {return _isToday;}
    
protected:
    virtual bool init() override;
    virtual void onClick(SMView * view) override;
    virtual void onEnter() override;
    
private:
    CalendarDayCellListener * _listener;
    bool _isToday;
    unsigned int _year;
    unsigned int _month;
    unsigned int _day;
};


#endif /* CalendarDayCell_h */
