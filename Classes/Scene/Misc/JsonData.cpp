//
//  JsonData.cpp
//  SMFrameWork
//
//  Created by SteveMac on 2018. 5. 21..
//

#include "JsonData.h"

const char * CheckVersionInfoString::APP_ID = "APP_ID";
const char * CheckVersionInfoString::ResultMessage = "ResultMessage";
const char * CheckVersionInfoString::VER_NUM = "VER_NUM";
const char * CheckVersionInfoString::Result = "Result";

bool CheckVersion::parseFunc(JsonWrapper<CheckVersion> &p, CheckVersion &item)
{

    item.APP_ID = p.getString(CheckVersionInfoString::APP_ID);
    item.ResultMessage = p.getString(CheckVersionInfoString::ResultMessage);
    item.VER_NUM = p.getString(CheckVersionInfoString::VER_NUM);
    item.Result = p.getString(CheckVersionInfoString::Result);

    return true;
}

const char * SummaryInfoString::SEND_UNCHECK = "SEND_UNCHECK";
const char * SummaryInfoString::ORDER_UNCHECK = "ORDER_UNCHECK";
const char * SummaryInfoString::ORDER_REPORT_MONTH = "ORDER_REPORT_MONTH";
const char * SummaryInfoString::SEND_MONTH = "SEND_MONTH";
const char * SummaryInfoString::SEND_TOTAL = "SEND_TOTAL";
const char * SummaryInfoString::RECEIVE_UNCHECK = "RECEIVE_UNCHECK";
const char * SummaryInfoString::ORDER_DECIDE_WAIT = "ORDER_DECIDE_WAIT";
const char * SummaryInfoString::RECEIVE_MONTH = "RECEIVE_MONTH";
const char * SummaryInfoString::ORDER_REPORT_UNCHECK = "ORDER_REPORT_UNCHECK";
const char * SummaryInfoString::ORDER_MONTH = "ORDER_MONTH";
const char * SummaryInfoString::BOARD_NEW_COUNT = "BOARD_NEW_COUNT";
const char * SummaryInfoString::RECEIVE_TOTAL = "RECEIVE_TOTAL";

bool SummaryInfo::parseFunc(JsonWrapper<SummaryInfo> &p, SummaryInfo &item)
{
    item.SEND_UNCHECK = p.getString(SummaryInfoString::SEND_UNCHECK);
    item.ORDER_UNCHECK = p.getString(SummaryInfoString::ORDER_UNCHECK);
    item.ORDER_REPORT_MONTH = p.getString(SummaryInfoString::ORDER_REPORT_MONTH);
    item.SEND_MONTH = p.getString(SummaryInfoString::SEND_MONTH);
    item.SEND_TOTAL = p.getString(SummaryInfoString::SEND_TOTAL);
    item.RECEIVE_UNCHECK = p.getString(SummaryInfoString::RECEIVE_UNCHECK);
    item.ORDER_DECIDE_WAIT = p.getString(SummaryInfoString::ORDER_DECIDE_WAIT);
    item.RECEIVE_MONTH = p.getString(SummaryInfoString::RECEIVE_MONTH);
    item.ORDER_REPORT_UNCHECK = p.getString(SummaryInfoString::ORDER_REPORT_UNCHECK);
    item.ORDER_MONTH = p.getString(SummaryInfoString::ORDER_MONTH);
    item.BOARD_NEW_COUNT = p.getString(SummaryInfoString::BOARD_NEW_COUNT);
    item.RECEIVE_TOTAL = p.getString(SummaryInfoString::RECEIVE_TOTAL);

    return true;
}

const char * NewCheckInfoString::REPORT_NEW_CNT = "REPORT_NEW_CNT";
const char * NewCheckInfoString::NOTICE_NEW_CNT = "NOTICE_NEW_CNT";
const char * NewCheckInfoString::BOARD_NEW_COUNT = "BOARD_NEW_COUNT";
const char * NewCheckInfoString::ORDER_NEW_CNT = "ORDER_NEW_CNT";

bool NewCheckInfo::parseFunc(JsonWrapper<NewCheckInfo> &p, NewCheckInfo &item)
{
    item.REPORT_NEW_CNT = p.getString(NewCheckInfoString::REPORT_NEW_CNT);
    item.NOTICE_NEW_CNT = p.getString(NewCheckInfoString::NOTICE_NEW_CNT);
    item.BOARD_NEW_COUNT = p.getString(NewCheckInfoString::BOARD_NEW_COUNT);
    item.ORDER_NEW_CNT = p.getString(NewCheckInfoString::ORDER_NEW_CNT);
    return true;
}

