//
//  DBHelper.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "DBHelper.h"
#include "DBConstant.h"
#include <cocos2d.h>

using namespace dbconst;


DBHelper* DBHelper::sInstance = nullptr;

DBHelper* DBHelper::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new (std::nothrow) DBHelper();
        // 지정된 DB 이름으로 초기화 한다...
        // android는 "rb"로 열고 "wb"로 다시 한번 써야한다.
        sInstance->initDatabase(Database::NAME, Database::VERSION);
    }
    
    return sInstance;
}


DBHelper::DBHelper()
{
    // 테이블 등록... 없으면 만들고 있으면 넘어간다.
    registerScheme(Database::TABLENAME_1, &sTable1);    // "Table1"이라는 이름의 테이블
    registerScheme(Database::TABLENAME_2, &sTable2);    // "Table2"이라는 이름의 테이블
    registerScheme(Database::TABLENAME_3, &sTable3);    // "Table3"이라는 이름의 테이블
    registerScheme(Database::TABLENAME_4, &sTable4);    // "Table4"이라는 이름의 테이블
    registerScheme(Database::TABLENAME_5, &sTable5);    // "Table5"이라는 이름의 테이블
}

// "Table1"이라는 이름의 테이블
const SQLiteWrapper::SCHEMA DBHelper::sTable1 = {
    Column(TABLENAME1::COLUMN1,  Format::TEXT),     // "TEXT" 데이터 타입으로 "Column1"라는 컬럼을 만든다.
    Column(TABLENAME1::COLUMN2,  Format::INTEGER),   // "INTEGER" 데이터 타입으로 "Column2"라는 컬럼을 만든다.
    Column(TABLENAME1::COLUMN3,  Format::REAL),  // "REAL" 데이터 타입으로 "Column3"라는 컬럼을 만든다.
    Column(TABLENAME1::COLUMN4,  Format::TEXT),  // "TEXT" 데이터 타입으로 "Column4"라는 컬럼을 만든다.  sqlite3에 bool 값 넣는게 없어서 텍스트로 대체.. "Y" 또는  "N"이 될거임
    Column(TABLENAME1::COLUMN5,  Format::NUMERIC),   // "NUMERIC" 데이터 타입으로 "Column5"라는 컬럼을 만든다.
};

// "Table2"이라는 이름의 테이블
const SQLiteWrapper::SCHEMA DBHelper::sTable2 = {
    Column(TABLENAME2::COLUMN1,  Format::TEXT),     // "TEXT" 데이터 타입으로 "Column1"라는 컬럼을 만든다.
    Column(TABLENAME2::COLUMN2,  Format::INTEGER),   // "INTEGER" 데이터 타입으로 "Column2"라는 컬럼을 만든다.
    Column(TABLENAME2::COLUMN3,  Format::REAL),  // "REAL" 데이터 타입으로 "Column3"라는 컬럼을 만든다.
    Column(TABLENAME2::COLUMN4,  Format::TEXT),  // "TEXT" 데이터 타입으로 "Column4"라는 컬럼을 만든다.  sqlite3에 bool 값 넣는게 없어서 텍스트로 대체.. "Y" 또는  "N"이 될거임
    Column(TABLENAME2::COLUMN5,  Format::NUMERIC),   // "NUMERIC" 데이터 타입으로 "Column5"라는 컬럼을 만든다.
};

// "Table3"이라는 이름의 테이블
const SQLiteWrapper::SCHEMA DBHelper::sTable3 = {
    Column(TABLENAME3::COLUMN1,  Format::TEXT),     // "TEXT" 데이터 타입으로 "Column1"라는 컬럼을 만든다.
    Column(TABLENAME3::COLUMN2,  Format::INTEGER),   // "INTEGER" 데이터 타입으로 "Column2"라는 컬럼을 만든다.
    Column(TABLENAME3::COLUMN3,  Format::REAL),  // "REAL" 데이터 타입으로 "Column3"라는 컬럼을 만든다.
    Column(TABLENAME3::COLUMN4,  Format::TEXT),  // "TEXT" 데이터 타입으로 "Column4"라는 컬럼을 만든다.  sqlite3에 bool 값 넣는게 없어서 텍스트로 대체.. "Y" 또는  "N"이 될거임
    Column(TABLENAME3::COLUMN5,  Format::NUMERIC),   // "NUMERIC" 데이터 타입으로 "Column5"라는 컬럼을 만든다.
};

// "Table4"이라는 이름의 테이블
const SQLiteWrapper::SCHEMA DBHelper::sTable4 = {
    Column(TABLENAME4::COLUMN1,  Format::TEXT),     // "TEXT" 데이터 타입으로 "Column1"라는 컬럼을 만든다.
    Column(TABLENAME4::COLUMN2,  Format::INTEGER),   // "INTEGER" 데이터 타입으로 "Column2"라는 컬럼을 만든다.
    Column(TABLENAME4::COLUMN3,  Format::REAL),  // "REAL" 데이터 타입으로 "Column3"라는 컬럼을 만든다.
    Column(TABLENAME4::COLUMN4,  Format::TEXT),  // "TEXT" 데이터 타입으로 "Column4"라는 컬럼을 만든다.  sqlite3에 bool 값 넣는게 없어서 텍스트로 대체.. "Y" 또는  "N"이 될거임
    Column(TABLENAME4::COLUMN5,  Format::NUMERIC),   // "NUMERIC" 데이터 타입으로 "Column5"라는 컬럼을 만든다.
};

// "Table5"이라는 이름의 테이블
const SQLiteWrapper::SCHEMA DBHelper::sTable5 = {
    Column(TABLENAME5::COLUMN1,  Format::TEXT),     // "TEXT" 데이터 타입으로 "Column1"라는 컬럼을 만든다.
    Column(TABLENAME5::COLUMN2,  Format::INTEGER),   // "INTEGER" 데이터 타입으로 "Column2"라는 컬럼을 만든다.
    Column(TABLENAME5::COLUMN3,  Format::REAL),  // "REAL" 데이터 타입으로 "Column3"라는 컬럼을 만든다.
    Column(TABLENAME5::COLUMN4,  Format::TEXT),  // "TEXT" 데이터 타입으로 "Column4"라는 컬럼을 만든다.  sqlite3에 bool 값 넣는게 없어서 텍스트로 대체.. "Y" 또는  "N"이 될거임
    Column(TABLENAME5::COLUMN5,  Format::NUMERIC),   // "NUMERIC" 데이터 타입으로 "Column5"라는 컬럼을 만든다.
};

// init Database
bool DBHelper::initDatabase(const std::string &databaseName, const int version)
{
    if (!SQLiteWrapper::initDatabase(databaseName, version)) {
        CCASSERT(0, "DATABASE INIT FAILED");
//        CCLOG("[[[[[ Database Init Failed");
        return false;
    }

 
    // 여기다가 앱 실행시 해야될 추가 DB 작업 해줌... 없음 말고
    
    return true;
}

void DBHelper::onCreate()
{
    // 처음 만들어질때 SQLiteWrapper를 통해 호출됨.
    // 트랜잭션 시작
    beginTransaction();
    {
        // 테이블을 만든다.
        createTable(Database::TABLENAME_1);
        createTable(Database::TABLENAME_2);
        createTable(Database::TABLENAME_3);
        createTable(Database::TABLENAME_4);
        createTable(Database::TABLENAME_5);
        
        
        // 초기 데이터 세팅
        auto contentValue = ContentValue::create();
        contentValue->putString(TABLENAME1::COLUMN1, "테스트테이블1_텍스트값");       // 텍스트 타입이니까 텍스트를 넣어보자
        contentValue->putInt(TABLENAME1::COLUMN2, 1);   // interger 타입이니까 숫자를 넣어보자
        contentValue->putFloat(TABLENAME1::COLUMN3, 1.0);   // float 타입이니까 소수를 넣어보자
        contentValue->putBool(TABLENAME1::COLUMN4, true);   // boolean 타입이니까 true를 넣어보자
        contentValue->putInt64(TABLENAME1::COLUMN5, 99999999);  // numeric 타입이니까 큰 숫자를 넣어보자
        sqlInsert(Database::TABLENAME_1, contentValue); // "Table1"이라는 이름의 테이블에 레코드 추가...
        contentValue->clear();
        
        
        // 할게 너무 많다면 sql로 만들어서 넣어도 된다.
//        executeSQLFile("초기실행할.sql");
    }
    commit();
    // 트랜잭션 끝
}
