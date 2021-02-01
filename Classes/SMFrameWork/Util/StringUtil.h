//
//  StringUtil.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef StringUtil_h
#define StringUtil_h

#include <stdio.h>
#include <string>
#include <vector>
#include <cstdarg>
#include <memory>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <map>
#include <locale>
#include <iomanip>
#include <regex>
#include <base/ccUTF8.h>
#include <algorithm>
#include <functional>
#include <cstring>
#include <cctype>

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#include "winHelp.h"
#endif
#endif


namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}


class StringUtil {
public:
    
    static std::vector<std::string> split(const std::string& str, const std::string& delim);
    
    static void split(const std::string& str, const std::string& delim, std::vector<std::string>& elems);
    
    static uint32_t hexToColor(const std::string& str);
    
    static std::string toUpper(const std::string& str);
    
    static std::string toLower(const std::string& str);
    
    static int toInt(const std::string& str);
    
    static long toLong(const std::string& str);
    
    static int64_t toInt64(const std::string& str);
    
    static bool isDigits(const std::string &str);
    
    
    static bool compareIgnoreCase(const std::string& src, const std::string& dst);
    
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    static bool startsWithIgnoreCase(const std::string& str, const std::string& prefix);
    
    static std::string& ltrim(std::string& str);
    
    static std::string& rtrim(std::string& str);
    
    static std::string& trim(std::string& str);
    
    static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);
    
    static double atof(const char *nptr);
    
    //http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf?rq=1
    template<typename ... Args>
    static std::string format(const std::string& format, Args ... args) {
        size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args ...);
        
        return std::string(buf.get(), buf.get() + size - 1);
    }
    
    template<typename T>
    static std::string makeCommaString(std::vector<T> elems) {
        int numElem = (int)elems.size();
        std::string result;
        for (int i = 0; i < numElem; i++) {
            result += patch::to_string(elems[i]);
            if (i < numElem-1) {
                result += ",";
            }
        }
        return result;
    }
    
    static bool checkEmailIsValid(const std::string& str);
    static std::string getCurrencyStringFromNumeric(int64_t decimal);
    static std::vector<std::string> separateStringByString(const std::string str, const char *delimiter);
    
    static std::string getYYYYMMDDString(time_t date);

    static std::string getDateStringYear(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%Y", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }
    static std::string getDateStringMonth(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%m", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }
    static std::string getDateStringDay(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%d", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }
    static std::string getTimeStringAMPM(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%p", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }
    static std::string getTimeStringHour(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%l", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }
    static std::string getTimeStringMinute(time_t date)
    {
        char timeStamp[256];
        
        strftime(timeStamp, BUFSIZ, "%M", localtime(&date));
        
        std::string ts(timeStamp);
        
        return ts;
    }

    
    static std::string getTodayStringYear()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getDateStringYear(now);
    }
    static std::string getTodayStringMonth()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getDateStringMonth(now);
    }
    static std::string getTodayStringDay()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getDateStringDay(now);
    }
    
    static std::string getCurrentTimeStringAMPM()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getTimeStringAMPM(now);
    }
    static std::string getCurrentTimeStringHour()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getTimeStringHour(now);
    }
    static std::string getCurrentTimeStringMinute()
    {
        time_t now;
        time (&now);
        
        return StringUtil::getTimeStringMinute(now);
    }
    
//    static std::string format(const char* format, ...)
//    {
//#define CC_MAX_STRING_LENGTH (1024*100)
//
//        std::string ret;
//
//        va_list ap;
//        va_start(ap, format);
//
//        char* buf = (char*)malloc(CC_MAX_STRING_LENGTH);
//        if (buf != nullptr)
//        {
//            vsnprintf(buf, CC_MAX_STRING_LENGTH, format, ap);
//            ret = buf;
//            free(buf);
//        }
//        va_end(ap);
//
//        return ret;
//    }
    
    static std::string getNowString()
    {
        time_t now;
        time (&now);
        char timeStamp[256];
        strftime(timeStamp, sizeof timeStamp, "%Y%m%d%H%M%S", gmtime(&now));
        std::string ts(timeStamp);
        return ts;
    }
    
    static time_t getDateTimeFromDateTimeString(std::string dateTimeString)
    {
        struct tm when = {0};
        
        strptime(dateTimeString.c_str(), "%Y%m%d%H%M%S", &when);
//        strptime(dateTimeString.c_str(), "%F %T", &when);
        
        return mktime(&when);
    }

    static time_t getDateTimeFromDateString(std::string dateTimeString)
    {
        struct tm when = {0};
        
        strptime(dateTimeString.c_str(), "%Y%m%d", &when);
        
        return mktime(&when);
    }
    
    static std::string getReportDateTime(std::string dateTimeString)
    {
        struct tm when = {0};
        
        strptime(dateTimeString.c_str(), "%Y%m%d%H%M%S", &when);
        
        time_t datetime = mktime(&when);
//
        std::vector<std::string> week;
		week.push_back("Sun");
		week.push_back("Mon");
		week.push_back("Tue");
		week.push_back("Wen");
		week.push_back("Thu");
		week.push_back("Fri");
		week.push_back("Sat");
		//
        struct tm *ts = localtime( &datetime);

        std::string dayString = cocos2d::StringUtils::format("%04d.%02d.%02d (%s) %02d:%02d:%02d", ts->tm_year +1900, ts->tm_mon+1, ts->tm_mday, week[ts->tm_wday].c_str(), ts->tm_hour, ts->tm_min, ts->tm_sec);

        return dayString;
    }
    
    static std::string getOrderReceiveDateTime(std::string dateTimeString)
    {
        struct tm when = {0};
        
        strptime(dateTimeString.c_str(), "%Y%m%d%H%M%S", &when);
        
        time_t datetime = mktime(&when);
        //
        std::vector<std::string> week;
		week.push_back("Sun");
		week.push_back("Mon");
		week.push_back("Tue");
		week.push_back("Wen");
		week.push_back("Thu");
		week.push_back("Fri");
		week.push_back("Sat");
        //
        struct tm *ts = localtime( &datetime);
        
        std::string dayString = cocos2d::StringUtils::format("%02d.%02d (%s)", ts->tm_mon+1, ts->tm_mday, week[ts->tm_wday].c_str());
        
        return dayString;
    }

    static std::string getMDayStringFromDateTimeString(std::string dateTimeString)
    {
        return StringUtil::getMDayString(StringUtil::getDateTimeFromDateTimeString(dateTimeString));
    }
    
    static std::string getDateString(std::string yearStr, std::string monthStr, std::string dayStr)
    {
        int nYear = atoi(yearStr.c_str());
        int nMonth = atoi(monthStr.c_str());
        int nDay = atoi(dayStr.c_str());
//        struct tm when = {0};
        
        std::string dateTimeString = cocos2d::StringUtils::format("%04d%02d%02d", nYear, nMonth, nDay);
//        CCLOG("[[[[[ dateTime string : %s", dateTimeString.c_str());
        
        return dateTimeString;

//        when.tm_year = nYear;
//        when.tm_mon = nMonth;
//        when.tm_mday = nDay;
//        time_t datetime = mktime(&when);
//
//        return StringUtil::getYYYYMMDDString(datetime);
    }

    static std::string getDateTimeString(std::string yearStr, std::string monthStr, std::string dayStr, std::string ampm, std::string hourStr, std::string minStr)
    {
    
        int nYear = atoi(yearStr.c_str());
        int nMonth = atoi(monthStr.c_str());
        int nDay = atoi(dayStr.c_str());
        
        int nHour = atoi(hourStr.c_str());
        nHour = ampm == "PM" ? nHour+12 : nHour;
        if (nHour>=24) {
            nHour -= 24;
        }
        int nMin = atoi(minStr.c_str());

        std::string dateTimeString = cocos2d::StringUtils::format("%04d%02d%02d%02d%02d00", nYear, nMonth, nDay, nHour, nMin);
//        CCLOG("[[[[[ dateTime string : %s", dateTimeString.c_str());

        return dateTimeString;
        
//        struct tm when = {0};
//
//        strptime(dateTimeString.c_str(), "%Y%m%d%H%M", &when);
//
//        time_t datetime = mktime(&when);
//
//        std::string dateString = StringUtil::getYYYYMMDDString(datetime);
//        std::string timeString = StringUtil::getTimeString(datetime);
//
//        return StringUtil::format("%s %s", dateString.c_str(), timeString.c_str());
    }
    
    static std::string getMDayString(time_t dateTime);
    
    static std::string getTodayDateString();
    static std::string getTimeString(time_t dateTime);
    static std::string getCurrentTimeString();

    static std::string getDateString(time_t date);
    static std::string getDateCompareString(time_t date);
    
    static std::string getDeviceCurrentUinqueID();
    static std::string getDeviceForeverUniqueID();
    
    static std::string getTimeStampString();
  
    
    static bool isValidURL(std::string url);
    
    static std::string getOpenWeatherMapStringFromID(std::string weatherID);
    
    static std::string url_encode(const std::string &s)
    {
        const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
        
        std::string escaped="";
        for(size_t i=0; i<s.length(); i++)
        {
            if (unreserved.find_first_of(s[i]) != std::string::npos)
            {
                escaped.push_back(s[i]);
            }
            else
            {
                escaped.append("%");
                char buf[3];
                sprintf(buf, "%.2X", (unsigned char)s[i]);
                escaped.append(buf);
            }
        }
        return escaped;
    }
    
    static std::string url_decode(std::string &SRC) {
        std::string ret;
        char ch;
        int i, ii;
        for (i=0; i<SRC.length(); i++) {
            if (int(SRC[i])==37) {
                sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
                ch=static_cast<char>(ii);
                ret+=ch;
                i=i+2;
            } else {
                ret+=SRC[i];
            }
        }
        return (ret);
    }
};


static char easytolower(char in) {
    if(in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

#include <stddef.h>

extern size_t decode_html_entities_utf8(char *dest, const char *src);


#endif /* StringUtil_h */
