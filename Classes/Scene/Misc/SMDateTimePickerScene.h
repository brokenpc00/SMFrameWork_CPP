//
//  SMDateTimePickerScene.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 15..
//

#ifndef SMDateTimePickerScene_h
#define SMDateTimePickerScene_h

#include "../../SMFrameWork/Base/SMScene.h"
#include "../../SMFrameWork/Picker/SMWheelPicker.h"

enum InputPickerType {
    InputPickerTypeDate,
    InputPickerTypeTime,
    InputPickerTypeDateTime,
};

enum InputPickerTag {
    InputPickerTagYear = 10,
    InputPickerTagMonth = 20,
    InputPickerTagDay = 30,
    InputPickerTagAmPm = 40,
    InputPickerTagHour = 50,
    InputPickerTagMinute = 60
};

class SMButton;

class OnDateTimePickerListener {
public:
    virtual void onDateTimeSelected(InputPickerType type, std::string str1="", std::string str2="", std::string str3="", std::string str4="", std::string str5="", std::string str6="") = 0;
};

class SMDateTimePickerScene : public SMScene, public OnClickListener, public OnPickerListener
{
public:
    CREATE_SCENE(SMDateTimePickerScene);
    
    void setOnDateTimePickerListener(OnDateTimePickerListener* l) {_listener =  l;};
    
protected:
    SMDateTimePickerScene();
    virtual ~SMDateTimePickerScene();
    
    virtual bool init() override;
    virtual void onClick(SMView * view) override;
    virtual void onPickerSelected(int tag, int index) override;
    
    void openPickerView(InputPickerTag tag, std::vector<std::string> items, int curIndex);
    
private:
    std::vector<std::string> _yearArray;
    std::vector<std::string> _monthArray;
    std::vector<std::string> _dayArray;
    std::vector<std::string> _ampmArray;
    std::vector<std::string> _hourArray;
    std::vector<std::string> _minuteArray;

    
    SMView * _contentView;
    SMButton * _yearButton;
    SMButton * _monthButton;
    SMButton * _dayButton;
    SMButton * _ampmButton;
    SMButton * _hourButton;
    SMButton * _minuteButton;
  
    InputPickerType _type;
    OnDateTimePickerListener* _listener;
    
};



#endif /* SMDateTimePickerScene_h */
