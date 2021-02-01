//
//  TimerUtil.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 16..
//
//

#ifndef TimerUtil_h
#define TimerUtil_h

#include <base/CCConsole.h>
#include <platform/CCPlatformMacros.h>
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

// for timezone
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

// gettimeofday in windows
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		// system time�� ���ϱ�
		GetSystemTimeAsFileTime(&ft);

		// unsigned 64 bit�� �����
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// 100nano�� 1micro�� ��ȯ�ϱ�
		tmpres /= 10;

		// epoch time���� ��ȯ�ϱ�
		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		// sec�� micorsec���� ���߱�
		tv->tv_sec = (tmpres / 1000000UL);
		tv->tv_usec = (tmpres % 1000000UL);
	}

	// timezone ó��
	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif
#include <string>

class TimeDuration {
public:
    
private:
    static TimeDuration& getInstance() {
        static TimeDuration instance;
        return instance;
    }

};

class TimerUtil {
public:
    static void start() {
        timeval tv;
        gettimeofday(&tv, 0);
        
        getInstance()._startTime = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
    }
    
    static void stop(const std::string& tag) {
        timeval tv;
        gettimeofday(&tv, 0);
        
        double endTime = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
        double elapsedTime = endTime - getInstance()._startTime;
        CCLOG("-------------------------------------------------");
        CCLOG("[TIMER UTIL] %s : %.04f(s)", tag.c_str(), (float)(elapsedTime));
        CCLOG("-------------------------------------------------");
    }
    
    static void stop() {
        timeval tv;
        gettimeofday(&tv, 0);
        
        double endTime = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
        double elapsedTime = endTime - getInstance()._startTime;
        
        CCLOG("-------------------------------------------------");
        CCLOG("[TIMER UTIL] : %.04f(s)", (float)(elapsedTime));
        CCLOG("-------------------------------------------------");
    }
	/*
    static time_t dateTimeTAdd(const tm* const dateTime, const int& months, const int& days, const int& hours, const int& mins, const int& secs)
    {
        tm* newTime = new tm;
        memcpy(newTime, dateTime, sizeof(tm));
        
        newTime->tm_mon += months;
        newTime->tm_mday += days;
        newTime->tm_hour += hours;
        newTime->tm_min += mins;
        newTime->tm_sec += secs;
        
        time_t nt_seconds = mktime(newTime) - timezone;
        delete newTime;
        
        return nt_seconds;
    }
    
    static struct tm* dateTimeTMAdd(const tm* const dateTime, const int& months, const int& days, const int& hours, const int& mins, const int& secs)
    {
        time_t nt_seconds = TimerUtil::dateTimeTAdd(dateTime, months, days, hours, mins, secs);

        return gmtime(&nt_seconds);
    }
	*/
    
private:
    double _startTime;
    
    static TimerUtil& getInstance() {
        static TimerUtil instance;
        return instance;
    }
};

#endif /* TimerUtil_h */
