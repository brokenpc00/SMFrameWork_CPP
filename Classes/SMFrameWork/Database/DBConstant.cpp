//
//  DBConstant.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "DBConstant.h"


using namespace dbconst;


const char* Database::NAME    = "iShop.db";       /// DB 파일 이름... 보통 프로젝트명으로 만들지... 당근 미리 만들어져 있어야겠지...
const int   Database::VERSION = 1;  // DB 버전... 일단 1

// 테이블 이름들...
const char* Database::TABLENAME_1 = "Table1";   // 테이블 명
const char* Database::TABLENAME_2 = "Table2";   // 테이블 명
const char* Database::TABLENAME_3 = "Table3";   // 테이블 명
const char* Database::TABLENAME_4 = "Table4";   // 테이블 명
const char* Database::TABLENAME_5 = "Table5";   // 테이블 명





// Table1의 컬럼들
const char* TABLENAME1::COLUMN1 = "Column1";    // 컬럼 명
const char* TABLENAME1::COLUMN2 = "Column2";    // 컬럼 명
const char* TABLENAME1::COLUMN3 = "Column3";    // 컬럼 명
const char* TABLENAME1::COLUMN4 = "Column4";    // 컬럼 명
const char* TABLENAME1::COLUMN5 = "Column5";    // 컬럼 명

// Table2의 컬럼들
const char* TABLENAME2::COLUMN1 = "Column1";    // 컬럼 명
const char* TABLENAME2::COLUMN2 = "Column2";    // 컬럼 명
const char* TABLENAME2::COLUMN3 = "Column3";    // 컬럼 명
const char* TABLENAME2::COLUMN4 = "Column4";    // 컬럼 명
const char* TABLENAME2::COLUMN5 = "Column5";    // 컬럼 명

// Table3의 컬럼들
const char* TABLENAME3::COLUMN1 = "Column1";    // 컬럼 명
const char* TABLENAME3::COLUMN2 = "Column2";    // 컬럼 명
const char* TABLENAME3::COLUMN3 = "Column3";    // 컬럼 명
const char* TABLENAME3::COLUMN4 = "Column4";    // 컬럼 명
const char* TABLENAME3::COLUMN5 = "Column5";    // 컬럼 명

// Table4의 컬럼들
const char* TABLENAME4::COLUMN1 = "Column1";    // 컬럼 명
const char* TABLENAME4::COLUMN2 = "Column2";    // 컬럼 명
const char* TABLENAME4::COLUMN3 = "Column3";    // 컬럼 명
const char* TABLENAME4::COLUMN4 = "Column4";    // 컬럼 명
const char* TABLENAME4::COLUMN5 = "Column5";    // 컬럼 명

// Table5의 컬럼들
const char* TABLENAME5::COLUMN1 = "Column1";    // 컬럼 명
const char* TABLENAME5::COLUMN2 = "Column2";    // 컬럼 명
const char* TABLENAME5::COLUMN3 = "Column3";    // 컬럼 명
const char* TABLENAME5::COLUMN4 = "Column4";    // 컬럼 명
const char* TABLENAME5::COLUMN5 = "Column5";    // 컬럼 명


// 대충 이런 식으로 만든다...
// 여기서 만든 string table로 SQLite or Realm를 이용하여 실제 DB를 만들고 테이블을 만들고... DB Access를 한다.
