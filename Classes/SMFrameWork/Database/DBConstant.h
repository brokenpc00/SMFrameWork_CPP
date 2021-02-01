//
//  DBConstant.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//  Database (for SQLite3 or Realm) 를 위한 table 및 column 구조

#ifndef DBConstant_h
#define DBConstant_h


#include <string>


namespace dbconst {
    
    // table name을 포함한 DB 정보
    struct Database {
        static const char* NAME;    // database name : 보통 프로젝트명으로 DB명을 만들지....
        static const int VERSION;   // db version... 이 버전으로 테이블을 alter하거나 drop 하거나... 등등..
        
        static const char* TABLENAME_1;  // 테이블 명
        static const char* TABLENAME_2;  // 테이블 명
        static const char* TABLENAME_3;  // 테이블 명
        static const char* TABLENAME_4;  // 테이블 명
        static const char* TABLENAME_5;  // 테이블 명... 이런 식으로 만든다...
    };
    
    // TABLENAME_1과 매칭되는 이름
    struct TABLENAME1 {
        static const char* COLUMN1;  // column1명
        static const char* COLUMN2;  // column1명
        static const char* COLUMN3;  // column1명
        static const char* COLUMN4;  // column1명
        static const char* COLUMN5;  // column1명... 이딴 식으로 만든다.
    };
    
    // TABLENAME_2와 매칭되는 이름
    struct TABLENAME2 {
        static const char* COLUMN1;  // column1명
        static const char* COLUMN2;  // column1명
        static const char* COLUMN3;  // column1명
        static const char* COLUMN4;  // column1명
        static const char* COLUMN5;  // column1명... 이딴 식으로 만든다.
    };
    
    // TABLENAME_3과 매칭되는 이름
    struct TABLENAME3 {
        static const char* COLUMN1;  // column1명
        static const char* COLUMN2;  // column1명
        static const char* COLUMN3;  // column1명
        static const char* COLUMN4;  // column1명
        static const char* COLUMN5;  // column1명... 이딴 식으로 만든다.
    };
    
    // TABLENAME_4와 매칭되는 이름
    struct TABLENAME4 {
        static const char* COLUMN1;  // column1명
        static const char* COLUMN2;  // column1명
        static const char* COLUMN3;  // column1명
        static const char* COLUMN4;  // column1명
        static const char* COLUMN5;  // column1명... 이딴 식으로 만든다.
    };
    
    // TABLENAME_5와 매칭되는 이름
    struct TABLENAME5 {
        static const char* COLUMN1;  // column1명
        static const char* COLUMN2;  // column1명
        static const char* COLUMN3;  // column1명
        static const char* COLUMN4;  // column1명
        static const char* COLUMN5;  // column1명... 이딴 식으로 만든다.
    };
        
}


#endif /* DBConstant_h */
