//
//  Table1.cpp
//  iShop
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "Table1.h"
#include "../Database/DBHelper.h"
#include "../Database/DBConstant.h"
#include "../Util/StringUtil.h"
#include <cocos2d.h>

using namespace dbconst;


bool Table1::readFromCursor(DBCursor *cursor, Table1 &item)
{
    // 커서가 없으면 아무것도 안된다.
    if (cursor==nullptr) {
        return false;
    }
    
    item._column1 = cursor->getString(TABLENAME1::COLUMN1);
    item._column2 = cursor->getInt(TABLENAME1::COLUMN2);
    item._column3 = cursor->getFloat(TABLENAME1::COLUMN3);
    // bool 값은 SQLitee에서 sqlite3_column_bool이라는 함수가 없다.
    // 그래서 sqlite3_column_int를 가지고 0을 판단하거나
    // sqlite3_column_text를 가지고 "Y" or "N"을 판단해서 bool을 반환한다. 그래서 getBool은 안만들었다. 여기서는 text가지고 한다.
    item._column4 = cursor->getString(TABLENAME5::COLUMN4)=="Y";
    item._column5  = cursor->getInt64(TABLENAME1::COLUMN5);
    // 이런 식으로 레코드 값을 구조체에 대입한다.
    // 그러면 DB에서 하나를 읽어온게 되는거임.
    return true;
}

void Table1::setContentValue(ContentValue *cv, Table1 &item)
{
    // 구조체 값을 ContentValue에 넣고
    // 이것을 가지고 DBHelper를 통해 DB 저장 할거임.
    cv->putString(TABLENAME1::COLUMN1, item._column1);
    cv->putInt(TABLENAME1::COLUMN2, item._column2);
    cv->putFloat(TABLENAME1::COLUMN3, item._column3);
    std::string blobString = item._column4 ? "Y" : "N";
    cv->putString(TABLENAME1::COLUMN4, blobString);
    cv->putInt64(TABLENAME1::COLUMN5, item._column5);
}

bool Table1::readFromDB(std::string column1, Table1 &item)
{
    // DB를 가져와서
    auto db = DBHelper::getInstance();
    
    // 쿼리를 통해 커서를 읽어온다.
    // "Table1"이라는 테이블에서 첫번째 컬럼을 키로하여 파라미터 column1을 찾는다. 두번째 인자는 필요한 컬럼명을 문자열로 넘기면 된다. 여기서는 "Column1, Column2, Column3, Column4, Column5"와 같은 효과가 되겠다...
    auto cursor = db->sqlSelect(Database::TABLENAME_1, nullptr, DBHelper::assign(TABLENAME1::COLUMN1, column1));
    if (cursor) {
        if (cursor->step()) {
            // 첫번째 넘이 있으면 그넘을 넘겨준다.
            if (readFromCursor(cursor.get(), item)) {
                return true;
            }
        }
    }
    
    // 커서가 없으면 "그런넘 없다" 이다..
    return false;
}

std::vector<Table1> Table1::readArrayFromDB()
{
    std::vector<Table1> items;
    
    auto db = DBHelper::getInstance();
    
    // "Table1"이라는 테이블에서 모든 데이터를 가져온다.
    auto cursor = db->sqlSelect(Database::TABLENAME_1);
    if (cursor) {
        while (cursor->step()) {
            Table1 item;
            // 레코드 하나를 읽어와서
            if (readFromCursor(cursor.get(), item)) {
                // vector에 push
                items.push_back(item);
            }
        }
    }
    // 만들어진 array를 반환
    return items;
}

bool Table1::updateToDBColumn2(Table1 &item)
{
    auto db = DBHelper::getInstance();
    auto cv = ContentValue::create();
    // column2만 업뎃하는 함수이므로 column2만 셋팅
    cv->putInt(TABLENAME1::COLUMN2, item._column2);

    // columm1이 여기에서는 키이므로 column1으로 업데이트 함
    db->sqlUpdate(Database::TABLENAME_1, cv, DBHelper::assign(TABLENAME1::COLUMN1, item._column1));
    
    return true;
}

bool Table1::saveToDB(Table1 &item)
{
    auto db = DBHelper::getInstance();
    auto cv = ContentValue::create();
    
    // item의 값을 전부 cv에 셋팅
    setContentValue(cv.get(), item);
    
    // 트랜잭션 시작
    db->beginTransaction();
    {
        // 키 값이 있으면 DB에 있는지 확인 해 본다.
        if (item._column1!="") {
            // 키 값인 column1으로 조회하여 카운트를 가져온다.
            int rows = db->sqlCountRows(Database::TABLENAME_1, DBHelper::assign(TABLENAME1::COLUMN1, item._column1));
            if (rows==0) {
                // DB에 없는거임.. 그럼 추가해야지
                item._column1 = "";
            }
        }
        
        if (item._column1=="") {
            // 추가 할거냐?
            int64_t key = db->sqlInsert(Database::TABLENAME_1, cv);
            // 추가 됐으면 받아온 키를 넣는다.
            item._column1 = cocos2d::StringUtils::format("%lld", key);
        } else {
            // 업뎃 할거냐?
            // column1을 키로 하여 업뎃한다.
            db->sqlUpdate(Database::TABLENAME_1, cv, DBHelper::assign(TABLENAME1::COLUMN1, item._column1));
        }
    }
    db->commit();
    // 트랜잭션 끝
    
    return true;
}
