//
//  SMDateTimePickerScene.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 6. 15..
//

#include "SMDateTimePickerScene.h"
#include "../../SMFrameWork/Base/SMButton.h"
#include "../../SMFrameWork/Util/ViewUtil.h"
#include "../../SMFrameWork/Const/SMFontColor.h"
#include "../../SMFrameWork/Base/ShaderNode.h"
#include "../../SMFrameWork/Util/StringUtil.h"
#include <cocos2d.h>

#define BUTTON_HEIGHT  (140.0)

#define FOUR_SIZE 250
#define TWO_SIZE 160

#define FONT_SIZE 55

enum PickBtnTag {
    PickBtnTagYear = 0,
    PickBtnTagMonth,
    PickBtnTagDay,
    PickBtnTagAmPm,
    PickBtnTagHour,
    PickBtnTagMinute
};


SMDateTimePickerScene::SMDateTimePickerScene() : _listener(nullptr)
, _contentView(nullptr)
, _yearButton(nullptr)
, _monthButton(nullptr)
, _dayButton(nullptr)
, _ampmButton(nullptr)
, _hourButton(nullptr)
, _minuteButton(nullptr)
, _type(InputPickerTypeDate)
{
    _yearArray.clear();
    for (int i=1930; i<2051; i++) {
        std::string str = cocos2d::StringUtils::format("%04d", i);
        _yearArray.push_back(str);
    }
    
    _monthArray.clear();
    for (int i=1; i<13; i++) {
        std::string str = cocos2d::StringUtils::format("%d", i);
        _monthArray.push_back(str);
    }
    
    _ampmArray.clear();
    _ampmArray.push_back("AM");
    _ampmArray.push_back("PM");
    
    _hourArray.clear();
    for (int i=1; i<13; i++) {
        std::string str = cocos2d::StringUtils::format("%d", i);
        _hourArray.push_back(str);
    }
    
    _minuteArray.clear();
    for (int i=1; i<60; i++) {
        std::string str = cocos2d::StringUtils::format("%d", i);
        _minuteArray.push_back(str);
    }
}

SMDateTimePickerScene::~SMDateTimePickerScene()
{
    
}

static int LastDay (int iMonth, int iYear)
{
    struct tm when;
    time_t lastday;
    
    // Set up current month
    when.tm_hour = 0;
    when.tm_min = 0;
    when.tm_sec = 0;
    when.tm_mday = 1;
    
    // Next month 0=Jan
    if (iMonth == 12)
    {
        when.tm_mon = 0;
        when.tm_year = iYear - 1900 + 1;
    }
    else
    {
        when.tm_mon = iMonth;
        when.tm_year = iYear - 1900;
    }
    // Get the first day of the next month
    lastday = mktime (&when);
    
    // Subtract 1 day
    lastday -= 86400;
    
    // Convert back to date and time
    when = *localtime (&lastday);
    
    return when.tm_mday;
}

bool SMDateTimePickerScene::init()
{
    if (!SMScene::init()) {
        return false;
    }
    
    auto param = getSceneParam();
    
    std::string dateY = "";
    std::string dateM = "";
    std::string dateD = "";
    std::string timeAMPM = "";
    std::string timeH = "";
    std::string timeM = "";

    if (param) {
        _type = (InputPickerType)param->getInt("PICKER_TYPE");
        if (_type==InputPickerTypeDate) {
            dateY = param->getString("DATE_YEAR", StringUtil::getTodayStringYear());
            dateM = param->getString("DATE_MONTH", StringUtil::getTodayStringMonth());
            dateD = param->getString("DATE_DAY", StringUtil::getTodayStringDay());
            dateM = cocos2d::StringUtils::format("%d", atoi(dateM.c_str()));
            dateD = cocos2d::StringUtils::format("%d", atoi(dateD.c_str()));
            
            int yearInt = atoi(dateY.c_str());
            int monthInt = atoi(dateM.c_str());
            int dayInt = LastDay(monthInt, yearInt);
            
            
            _dayArray.clear();
            // dayInt가 0 base이므로 2를 더한거보다 작게 돌린다.
            for (int i=1; i<dayInt+2; i++) {
                std::string str = cocos2d::StringUtils::format("%d", i);
                _dayArray.push_back(str);
            }
        } else if (_type==InputPickerTypeTime) {
            timeAMPM = param->getString("TIME_AMPM", StringUtil::getCurrentTimeStringAMPM());
            timeH = param->getString("TIME_HOUR", StringUtil::getCurrentTimeStringHour());
            timeM = param->getString("TIME_MINUTE", StringUtil::getCurrentTimeStringMinute());
            
            timeH = cocos2d::StringUtils::format("%d", atoi(timeH.c_str()));
            timeM = cocos2d::StringUtils::format("%d", atoi(timeM.c_str()));
        } else { // datetime
            dateY = param->getString("DATE_YEAR", StringUtil::getTodayStringYear());
            dateM = param->getString("DATE_MONTH", StringUtil::getTodayStringMonth());
            dateD = param->getString("DATE_DAY", StringUtil::getTodayStringDay());
            dateM = cocos2d::StringUtils::format("%d", atoi(dateM.c_str()));
            dateD = cocos2d::StringUtils::format("%d", atoi(dateD.c_str()));
            
            int yearInt = atoi(dateY.c_str());
            int monthInt = atoi(dateM.c_str());
            int dayInt = LastDay(monthInt, yearInt);
            
            
            _dayArray.clear();
            // dayInt가 0 base이므로 2를 더한거보다 작게 돌린다.
            for (int i=1; i<dayInt+2; i++) {
                std::string str = cocos2d::StringUtils::format("%d", i);
                _dayArray.push_back(str);
            }

            timeAMPM = param->getString("TIME_AMPM", StringUtil::getCurrentTimeStringAMPM());
            timeH = param->getString("TIME_HOUR", StringUtil::getCurrentTimeStringHour());
            timeM = param->getString("TIME_MINUTE", StringUtil::getCurrentTimeStringMinute());
            
            timeH = cocos2d::StringUtils::format("%d", atoi(timeH.c_str()));
            timeM = cocos2d::StringUtils::format("%d", atoi(timeM.c_str()));
        }
    }

    auto s = _director->getWinSize();
    
    _contentView = SMView::create(0, 0, 0, s.width, s.height);
    _contentView->setBackgroundColor4F(MAKE_COLOR4F(0xffffff, 1.0f));
    addChild(_contentView);
    
    auto applyButton = SMButton::create(0, SMButton::Style::SOLID_RECT, 0, 0, s.width, BUTTON_HEIGHT*1.5f);
    applyButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
    applyButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
    applyButton->setTextSystemFont("완료", SMFontConst::SystemFontRegular, FONT_SIZE);
    applyButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
    applyButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1ef, 1.0f));
    applyButton->setShapeLineWidth(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
    applyButton->setOutlineWidth(ShaderNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
    applyButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x222222, 1.0f));
    applyButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
    _contentView->addChild(applyButton);
    
    applyButton->setOnClickCallback([this](SMView * view){
        _director->getScheduler()->performFunctionInCocosThread([this]{
            if (_listener) {
                if (_type==InputPickerTypeDate) {
                    _listener->onDateTimeSelected(_type, _yearButton->getTextLabel()->getString(), _monthButton->getTextLabel()->getString(), _dayButton->getTextLabel()->getString());
                } else if (_type==InputPickerTypeTime) {
                    _listener->onDateTimeSelected(_type, _ampmButton->getTextLabel()->getString(), _hourButton->getTextLabel()->getString(), _minuteButton->getTextLabel()->getString());
                } else {
                    _listener->onDateTimeSelected(_type, _yearButton->getTextLabel()->getString(), _monthButton->getTextLabel()->getString(), _dayButton->getTextLabel()->getString(), _ampmButton->getTextLabel()->getString(), _hourButton->getTextLabel()->getString(), _minuteButton->getTextLabel()->getString());
                }
            }
            auto pScene = SceneTransition::SlideOutToBottom::create(SceneTransition::Time::NORMAL, cocos2d::Director::getInstance()->getPreviousScene());
            _director->popSceneWithTransition(pScene);
        });
    });

    auto bgView = SMView::create(0, 0, 0, 100, 220);
    _contentView->addChild(bgView);

    float totalWidth = 0;
    float posX = 40;

    if (_type==InputPickerTypeDate) {
        // 날짜
        _yearButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, FOUR_SIZE, 220);
        _yearButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _yearButton->setShapeCornerRadius(20);
        _yearButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _yearButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _yearButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setTextSystemFont(dateY, SMFontConst::SystemFontRegular, FONT_SIZE);
        _yearButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_yearButton);
        posX += _yearButton->getContentSize().width;
        posX += 40;
        auto yearLabel = cocos2d::Label::createWithSystemFont("년", SMFontConst::SystemFontRegular, FONT_SIZE);
        yearLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        yearLabel->setPosition(cocos2d::Vec2(posX, 110-yearLabel->getContentSize().height/2));
        yearLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(yearLabel);
        posX += yearLabel->getContentSize().width;
        posX += 40;
        
        _monthButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _monthButton->setShapeCornerRadius(20);
        _monthButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _monthButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _monthButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _monthButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setTextSystemFont(dateM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _monthButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_monthButton);
        posX += _monthButton->getContentSize().width;
        posX += 40;
        auto monthLabel = cocos2d::Label::createWithSystemFont("월", SMFontConst::SystemFontRegular, FONT_SIZE);
        monthLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        monthLabel->setPosition(cocos2d::Vec2(posX, 110-monthLabel->getContentSize().height/2));
        monthLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(monthLabel);
        posX += monthLabel->getContentSize().width;
        posX += 40;

        _dayButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _dayButton->setShapeCornerRadius(20);
        _dayButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _dayButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _dayButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _dayButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setTextSystemFont(dateD, SMFontConst::SystemFontRegular, FONT_SIZE);
        _dayButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_dayButton);
        posX += _dayButton->getContentSize().width;
        posX += 40;
        auto dayLabel = cocos2d::Label::createWithSystemFont("일", SMFontConst::SystemFontRegular, FONT_SIZE);
        dayLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        dayLabel->setPosition(cocos2d::Vec2(posX, 110-dayLabel->getContentSize().height/2));
        dayLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(dayLabel);
        posX += dayLabel->getContentSize().width;
        posX += 40;
        
        
        _yearButton->setOnClickCallback([&](SMView*view){
            _director->getScheduler()->performFunctionInCocosThread([&]{
                int idx = 0;
                for (auto compStr : _yearArray) {
                    std::string str = _yearButton->getTextLabel()->getString();
                    if (compStr==str) {
                        break;
                    }
                    idx++;
                }
                openPickerView(InputPickerTagYear, _yearArray, idx);
            });
        });
        
        _monthButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _monthArray) {
                std::string str = _monthButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagMonth, _monthArray, idx);
        });

        _dayButton->setOnClickCallback([&](SMView*view){
            
            // day를 누를때마다 계산하자
            int yearInt = atoi(_yearButton->getTextLabel()->getString().c_str());
            int monthInt = atoi(_monthButton->getTextLabel()->getString().c_str());
            int dayInt = LastDay(monthInt, yearInt);
            
            _dayArray.clear();
            // dayInt가 0 base이므로 2를 더한거보다 작게 돌린다.
            for (int i=1; i<dayInt+2; i++) {
                std::string str = cocos2d::StringUtils::format("%d", i);
                _dayArray.push_back(str);
            }

            
            int idx = 0;
            for (auto compStr : _dayArray) {
                std::string str = _dayButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagDay, _dayArray, idx);
        });
        
        totalWidth = posX;;
        
        bgView->setContentSize(cocos2d::Size(totalWidth, 220));
        bgView->setPosition(cocos2d::Vec2(s.width/2-bgView->getContentSize().width/2, s.height/2-bgView->getContentSize().height/2));

    } else if (_type==InputPickerTypeTime) {
        // 시간
        
        _ampmButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _ampmButton->setShapeCornerRadius(20);
        _ampmButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _ampmButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _ampmButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _ampmButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setTextSystemFont(timeAMPM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _ampmButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_ampmButton);
        posX += _ampmButton->getContentSize().width;
        posX += 40;
        
        _hourButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _hourButton->setShapeCornerRadius(20);
        _hourButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _hourButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _hourButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _hourButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setTextSystemFont(timeH, SMFontConst::SystemFontRegular, FONT_SIZE);
        _hourButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_hourButton);
        posX += _hourButton->getContentSize().width;
        posX += 40;
        
        auto hourLabel = cocos2d::Label::createWithSystemFont("시", SMFontConst::SystemFontRegular, FONT_SIZE);
        hourLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        hourLabel->setPosition(cocos2d::Vec2(posX, 110-hourLabel->getContentSize().height/2));
        hourLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(hourLabel);
        posX += hourLabel->getContentSize().width;
        posX += 40;

        _minuteButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _minuteButton->setShapeCornerRadius(20);
        _minuteButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _minuteButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _minuteButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _minuteButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setTextSystemFont(timeM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _minuteButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_minuteButton);
        posX += _minuteButton->getContentSize().width;
        posX += 40;
        
        auto minuteLabel = cocos2d::Label::createWithSystemFont("분", SMFontConst::SystemFontRegular, FONT_SIZE);
        minuteLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        minuteLabel->setPosition(cocos2d::Vec2(posX, 110-minuteLabel->getContentSize().height/2));
        minuteLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(minuteLabel);
        posX += minuteLabel->getContentSize().width;
        posX += 40;
        
        _ampmButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _ampmArray) {
                std::string str = _ampmButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagAmPm, _ampmArray, idx);
        });
        
        _hourButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _hourArray) {
                std::string str = _hourButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagHour, _hourArray, idx);
        });
        
        _minuteButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _minuteArray) {
                std::string str = _minuteButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagMinute, _minuteArray, idx);
        });
        
    totalWidth = posX;;
        
    bgView->setContentSize(cocos2d::Size(totalWidth, 220));
    bgView->setPosition(cocos2d::Vec2(s.width/2-bgView->getContentSize().width/2, s.height/2-bgView->getContentSize().height/2));

    } else {
        
        // 날짜
        _yearButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, FOUR_SIZE, 220);
        _yearButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _yearButton->setShapeCornerRadius(20);
        _yearButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _yearButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _yearButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setTextSystemFont(dateY, SMFontConst::SystemFontRegular, FONT_SIZE);
        _yearButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _yearButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_yearButton);
        posX += _yearButton->getContentSize().width;
        posX += 40;
        auto yearLabel = cocos2d::Label::createWithSystemFont("년", SMFontConst::SystemFontRegular, FONT_SIZE);
        yearLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        yearLabel->setPosition(cocos2d::Vec2(posX, 110-yearLabel->getContentSize().height/2));
        yearLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(yearLabel);
        posX += yearLabel->getContentSize().width;
        posX += 40;
        
        _monthButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _monthButton->setShapeCornerRadius(20);
        _monthButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _monthButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _monthButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _monthButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setTextSystemFont(dateM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _monthButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _monthButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_monthButton);
        posX += _monthButton->getContentSize().width;
        posX += 40;
        auto monthLabel = cocos2d::Label::createWithSystemFont("월", SMFontConst::SystemFontRegular, FONT_SIZE);
        monthLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        monthLabel->setPosition(cocos2d::Vec2(posX, 110-monthLabel->getContentSize().height/2));
        monthLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(monthLabel);
        posX += monthLabel->getContentSize().width;
        posX += 40;
        
        _dayButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX, 0, TWO_SIZE, 220);
        _dayButton->setShapeCornerRadius(20);
        _dayButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _dayButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _dayButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _dayButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setTextSystemFont(dateD, SMFontConst::SystemFontRegular, FONT_SIZE);
        _dayButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _dayButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView->addChild(_dayButton);
        posX += _dayButton->getContentSize().width;
        posX += 40;
        auto dayLabel = cocos2d::Label::createWithSystemFont("일", SMFontConst::SystemFontRegular, FONT_SIZE);
        dayLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        dayLabel->setPosition(cocos2d::Vec2(posX, 110-dayLabel->getContentSize().height/2));
        dayLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView->addChild(dayLabel);
        posX += dayLabel->getContentSize().width;
        posX += 40;
        
        
        _yearButton->setOnClickCallback([&](SMView*view){
            _director->getScheduler()->performFunctionInCocosThread([&]{
                int idx = 0;
                for (auto compStr : _yearArray) {
                    std::string str = _yearButton->getTextLabel()->getString();
                    if (compStr==str) {
                        break;
                    }
                    idx++;
                }
                openPickerView(InputPickerTagYear, _yearArray, idx);
            });
        });
        
        _monthButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _monthArray) {
                std::string str = _monthButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagMonth, _monthArray, idx);
        });
        
        _dayButton->setOnClickCallback([&](SMView*view){
            
            // day를 누를때마다 계산하자
            int yearInt = atoi(_yearButton->getTextLabel()->getString().c_str());
            int monthInt = atoi(_monthButton->getTextLabel()->getString().c_str());
            int dayInt = LastDay(monthInt, yearInt);
            
            _dayArray.clear();
            // dayInt가 0 base이므로 2를 더한거보다 작게 돌린다.
            for (int i=1; i<dayInt+2; i++) {
                std::string str = cocos2d::StringUtils::format("%d", i);
                _dayArray.push_back(str);
            }
            
            
            int idx = 0;
            for (auto compStr : _dayArray) {
                std::string str = _dayButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagDay, _dayArray, idx);
        });
        
        auto bgView2 = SMView::create(0, 0, 0, 100, 220);
        _contentView->addChild(bgView2);

        
        // 시간
        float posX2 = 40;
        _ampmButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX2, 0, TWO_SIZE, 220);
        _ampmButton->setShapeCornerRadius(20);
        _ampmButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _ampmButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _ampmButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _ampmButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setTextSystemFont(timeAMPM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _ampmButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _ampmButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView2->addChild(_ampmButton);
        posX2 += _ampmButton->getContentSize().width;
        posX2 += 40;
        
        _hourButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX2, 0, TWO_SIZE, 220);
        _hourButton->setShapeCornerRadius(20);
        _hourButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _hourButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _hourButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _hourButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setTextSystemFont(timeH, SMFontConst::SystemFontRegular, FONT_SIZE);
        _hourButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _hourButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView2->addChild(_hourButton);
        posX2 += _hourButton->getContentSize().width;
        posX2 += 40;
        
        auto hourLabel = cocos2d::Label::createWithSystemFont("시", SMFontConst::SystemFontRegular, FONT_SIZE);
        hourLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        hourLabel->setPosition(cocos2d::Vec2(posX2, 110-hourLabel->getContentSize().height/2));
        hourLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView2->addChild(hourLabel);
        posX2 += hourLabel->getContentSize().width;
        posX2 += 40;
        
        _minuteButton = SMButton::create(10, SMButton::Style::SOLID_ROUNDEDRECT, posX2, 0, TWO_SIZE, 220);
        _minuteButton->setShapeCornerRadius(20);
        _minuteButton->setOutlineWidth(ShapeNode::DEFAULT_ANTI_ALIAS_WIDTH*2);
        _minuteButton->setOutlineColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setOutlineColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x222222, 1.0f));
        _minuteButton->setButtonColor(SMButton::State::NORMAL, MAKE_COLOR4F(0xffffff, 1.0f));
        _minuteButton->setButtonColor(SMButton::State::PRESSED, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setTextSystemFont(timeM, SMFontConst::SystemFontRegular, FONT_SIZE);
        _minuteButton->setTextColor(SMButton::State::NORMAL, MAKE_COLOR4F(0x00a1e4, 1.0f));
        _minuteButton->setTextColor(SMButton::State::PRESSED, MAKE_COLOR4F(0xffffff, 1.0f));
        bgView2->addChild(_minuteButton);
        posX2 += _minuteButton->getContentSize().width;
        posX2 += 40;
        
        auto minuteLabel = cocos2d::Label::createWithSystemFont("분", SMFontConst::SystemFontRegular, FONT_SIZE);
        minuteLabel->setAnchorPoint(cocos2d::Vec2::ZERO);
        minuteLabel->setPosition(cocos2d::Vec2(posX2, 110-minuteLabel->getContentSize().height/2));
        minuteLabel->setTextColor(MAKE_COLOR4B(0xff222222));
        bgView2->addChild(minuteLabel);
        posX2 += minuteLabel->getContentSize().width;
        posX2 += 40;
        
        _ampmButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _ampmArray) {
                std::string str = _ampmButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagAmPm, _ampmArray, idx);
        });
        
        _hourButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _hourArray) {
                std::string str = _hourButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagHour, _hourArray, idx);
        });
        
        _minuteButton->setOnClickCallback([&](SMView*view){
            int idx = 0;
            for (auto compStr : _minuteArray) {
                std::string str = _minuteButton->getTextLabel()->getString();
                if (compStr==str) {
                    break;
                }
                idx++;
            }
            openPickerView(InputPickerTagMinute, _minuteArray, idx);
        });
        bgView->setContentSize(cocos2d::Size(posX, 220));
        bgView->setPosition(cocos2d::Vec2(s.width/2-bgView->getContentSize().width/2, s.height/2-bgView->getContentSize().height/2 + 160));

        bgView2->setContentSize(cocos2d::Size(posX2, 220));
        bgView2->setPosition(cocos2d::Vec2(s.width/2-bgView2->getContentSize().width/2, s.height/2-bgView2->getContentSize().height/2 - 160));

    }

    
    return true;
}

void SMDateTimePickerScene::onClick(SMView *view)
{
    
}

void SMDateTimePickerScene::openPickerView(InputPickerTag tag, std::vector<std::string> items, int curIndex)
{
    SMWheelPicker::open(tag, "", items, curIndex, this);
}

void SMDateTimePickerScene::onPickerSelected(int tag, int index)
{
    if (index==-1) {
        return;
    }
    
    std::string yearStr = "";
    std::string monthStr = "";
    std::string dayStr = "";
    std::string ampmStr = "";
    std::string hourStr = "";
    std::string minuteStr = "";
    
    switch (tag) {
        case InputPickerTagYear:
        {
            _yearButton->setTextSystemFont(_yearArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);
        }
            break;
        case InputPickerTagMonth:
        {
            _monthButton->setTextSystemFont(_monthArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);

            
            int yearInt = atoi(_yearButton->getTextLabel()->getString().c_str());
            int monthInt = atoi(_monthButton->getTextLabel()->getString().c_str());
            int dayInt = LastDay(monthInt, yearInt)+1;
            
            _dayArray.clear();
            // dayInt가 0 base이므로 1를 더한거보다 작게 돌린다.
            for (int i=1; i<dayInt+1; i++) {
                std::string str = cocos2d::StringUtils::format("%d", i);
                _dayArray.push_back(str);
            }

            int curDayInt = atoi(_dayButton->getTextLabel()->getString().c_str());
            if (dayInt<curDayInt) {
                // 새로 달을 선택했을 경우 현재 날짜가 max를 넘어가는지 확인...
                curDayInt = dayInt;
                std::string dayString = cocos2d::StringUtils::format("%d", curDayInt);
                _dayButton->setTextSystemFont(dayString, SMFontConst::SystemFontRegular, FONT_SIZE);
            }

        }
            break;
        case InputPickerTagDay:
        {
            _dayButton->setTextSystemFont(_dayArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);
        }
            break;
        case InputPickerTagAmPm:
        {
            _ampmButton->setTextSystemFont(_ampmArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);
        }
            break;
        case InputPickerTagHour:
        {
            _hourButton->setTextSystemFont(_hourArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);
        }
            break;
        case InputPickerTagMinute:
        {
            _minuteButton->setTextSystemFont(_minuteArray[index], SMFontConst::SystemFontRegular, FONT_SIZE);
        }
            break;
    }
    
    if (_yearButton!=nullptr) {
        yearStr = _yearButton->getTextLabel()->getString();
    }
    if (_monthButton!=nullptr) {
        monthStr = _monthButton->getTextLabel()->getString();
    }
    if (_dayButton!=nullptr) {
        dayStr = _dayButton->getTextLabel()->getString();
    }
    if (_ampmButton!=nullptr) {
        ampmStr = _ampmButton->getTextLabel()->getString();
    }
    if (_hourButton!=nullptr) {
        hourStr = _hourButton->getTextLabel()->getString();
    }
    if (_minuteButton!=nullptr) {
        minuteStr = _minuteButton->getTextLabel()->getString();
    }
    
    if (_listener) {
        if (_type==InputPickerTypeDate) {
            _listener->onDateTimeSelected(_type, yearStr, monthStr, dayStr);
        } else if (_type==InputPickerTypeTime) {
            _listener->onDateTimeSelected(_type, ampmStr, hourStr, minuteStr);
        } else {
            _listener->onDateTimeSelected(_type, yearStr, monthStr, dayStr, ampmStr, hourStr, minuteStr);
        }
    }
}


