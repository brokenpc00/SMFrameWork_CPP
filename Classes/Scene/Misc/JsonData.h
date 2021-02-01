//
//  JsonData.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 21..
//

#ifndef JsonData_h
#define JsonData_h

#include "../../SMFrameWork/Network/JsonWrapper.h"
#include <string>
#include <vector>


// 여기에 통신용 json data의 structure를 구현


// version info
struct CheckVersionInfoString {
    static const char * APP_ID;
    static const char * ResultMessage;
    static const char * VER_NUM;
    static const char * Result;
};

struct CheckVersion {
    std::string APP_ID;
    std::string ResultMessage;
    std::string VER_NUM;
    std::string Result;

public:
    CheckVersion() {
        
    }
    
    static bool parseFunc(JsonWrapper<CheckVersion>& p, CheckVersion& item);
    
};

// summary
struct SummaryInfoString {
    static const char * SEND_UNCHECK;
    static const char * ORDER_UNCHECK;
    static const char * ORDER_REPORT_MONTH;
    static const char * SEND_MONTH;
    static const char * SEND_TOTAL;
    static const char * RECEIVE_UNCHECK;
    static const char * ORDER_DECIDE_WAIT;
    static const char * RECEIVE_MONTH;
    static const char * ORDER_REPORT_UNCHECK;
    static const char * ORDER_MONTH;
    static const char * BOARD_NEW_COUNT;
    static const char * RECEIVE_TOTAL;
};

struct SummaryInfo {
    std::string SEND_UNCHECK;
    std::string ORDER_UNCHECK;
    std::string ORDER_REPORT_MONTH;
    std::string SEND_MONTH;
    std::string SEND_TOTAL;
    std::string RECEIVE_UNCHECK;
    std::string ORDER_DECIDE_WAIT;
    std::string RECEIVE_MONTH;
    std::string ORDER_REPORT_UNCHECK;
    std::string ORDER_MONTH;
    std::string BOARD_NEW_COUNT;
    std::string RECEIVE_TOTAL;
    
public:
    SummaryInfo() {
        
    }
    
    static bool parseFunc(JsonWrapper<SummaryInfo>& p, SummaryInfo& item);
};



// new count check
struct NewCheckInfoString {
    static const char * REPORT_NEW_CNT;
    static const char * NOTICE_NEW_CNT;
    static const char * BOARD_NEW_COUNT;
    static const char * ORDER_NEW_CNT;
};

struct NewCheckInfo {
    std::string REPORT_NEW_CNT;
    std::string NOTICE_NEW_CNT;
    std::string BOARD_NEW_COUNT;
    std::string ORDER_NEW_CNT;
public:
    NewCheckInfo(){
        
    }
    
    static bool parseFunc(JsonWrapper<NewCheckInfo>& p, NewCheckInfo& item);
    
    
};

#endif /* JsonData_h */
