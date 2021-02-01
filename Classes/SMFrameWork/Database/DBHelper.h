//
//  DBHelper.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
// SQLiteWrapper 또는 RealmWrapper를 상속받아 테이블을 정의하고 초기 데이터를 세팅하는 상속 클래스

#ifndef DBHelper_h
#define DBHelper_h

#include "SQLiteWrapper.h"

class DBHelper : public SQLiteWrapper
{
public:
    static DBHelper* getInstance();
    
protected:
    DBHelper();
    
    virtual bool initDatabase(const std::string& databaseName, const int version) override;
    
    virtual void onCreate() override;
    
    // test용 테이블
    static const SCHEMA sTable1;
    static const SCHEMA sTable2;
    static const SCHEMA sTable3;
    static const SCHEMA sTable4;
    static const SCHEMA sTable5;
    
private:
    static DBHelper* sInstance;    
};

#endif /* DBHelper_h */
