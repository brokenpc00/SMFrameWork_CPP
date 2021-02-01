//
//  CalendarView.h
//  IMKSupply
//
//  Created by KimSteve on 2017. 9. 7..
//
//

#ifndef CalendarView_h
#define CalendarView_h

#include "../../SMFrameWork/Base/SMView.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "Calendar.h"

class CalendarViewListener
{
public:
    virtual void onPrevMonth(SMView * view) = 0;
    virtual void onNextMonth(SMView * view) = 0;
    virtual void onLastSelectDay(SMView * view, unsigned int year, unsigned int month, unsigned day) = 0;
};

typedef std::function<void(SMView * view, unsigned int year, unsigned int month, unsigned int day)> OnSelectDateCallback;

class CalendarView : public SMView, public OnClickListener, public CalendarClickListener
{
public:
    CalendarView();
    virtual ~CalendarView();
    static CalendarView * createWithDate(unsigned int year=0, unsigned int month=0, unsigned int day=0, OnSelectDateCallback callback=nullptr, std::string title="");
    
    void setCalendarViewListener(CalendarViewListener * l) {_listener = l;}
    
    virtual int dispatchTouchEvent(const int action, const cocos2d::Touch * touch, const cocos2d::Vec2 * point, MotionEvent * event) override;
    void setTitle(std::string title);
protected:
    virtual void onClick(SMView * view) override;
    virtual bool init() override;
    
    virtual void onSelectDay(int year, int month, int day) override;
    
    
    
private:
    SMView * _bgView;
    SMView * _contentView;
    std::string _title;
    cocos2d::Label * _titleLabel;
    SMButton * _leftButton;
    SMButton * _rightButton;
    cocos2d::Label * _dateLabel;
    Calendar * _calendar;
    CalendarViewListener * _listener;
    unsigned int _currentYear;
    unsigned int _currentMonth;
    unsigned int _currentDay;
    OnSelectDateCallback _selectDayCallback;
};

#endif /* CalendarView_h */
