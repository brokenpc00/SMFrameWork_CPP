//
//  Table1.h
//  iShop
//
//  Created by KimSteve on 2016. 12. 7..
//
//  테스트용 DB 테이블 struct... "table1"이라는 테이블을 관리한다.

#ifndef Table1_h
#define Table1_h

#include <string>
#include <vector>

class DBCursor;
class ContentValue;

struct Table1 {
public:
    std::string _column1;    // 이거를 키로 하자...
    int _column2;
    float _column3;
    bool _column4;
    int64_t _column5;

    Table1() {memset(this, 0, sizeof(Table1));}
    
    static bool readFromDB(std::string column1, Table1& item);    // 지정된 키(첫번째 컬럼)로 DB에서 레코드 하나를 읽은 다음 구조체에 값을 넣는다.
    static bool updateToDBColumn2(Table1& item);   // column2만 업데이트 해본다.
    static bool saveToDB(Table1& item); // 구조체를 DB에 저장 (없으면 Insert, 있으면 Update)
    static std::vector<Table1> readArrayFromDB(); // 이 테이블 전체를 배열로 읽어온다... 테이블 성격에 따라 다르지만 데이터 양이 많은 테이블은 조건을 줘야 할 듯... 메모리 및 속도를 보고 조절하자.
    
private:
    static bool readFromCursor(DBCursor * cursor, Table1& item);    // 쿼리 결과 리스트중에 하나를 뽑기위하여 커서를 이용해서 데이터를 반환하는 함수
    static void setContentValue(ContentValue* cv, Table1& item);    // 구조체의 값을 DB와 연동하기 위하여 ContentValue로 변환하는 함수
};

// 이 구조체를 기준으로 DB 접근 파일을 추가하면 된다.


#endif /* Table1_h */
