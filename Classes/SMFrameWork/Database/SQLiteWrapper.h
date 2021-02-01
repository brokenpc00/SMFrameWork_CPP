//
//  SQLiteWrapper.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
// SQLite3 Wrapper.... Realm도 해야하나....

#ifndef SQLiteWrapper_h
#define SQLiteWrapper_h

#include "ContentValue.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>

struct sqlite3;
struct sqlite3_stmt;

class DBCursor; // DB Access 관리자
typedef std::unique_ptr<DBCursor> CursorPtr;

class SQLiteWrapper
{
public:
    static const char* ROWID;
    static const char* ALL_COLUMNS[1];
    static const char* NO_COLUMNS[1];
    
    struct Format {
        static const char* INTEGER_UNIQUE_PK_AUTOINCREMENT;
        static const char* INTEGER_UNIQUE_PK;
        static const char* INTEGER_UNIQUE_INDEX;
        static const char* INTEGER_INDEX;
        static const char* INTEGER;
        static const char* TEXT;
        static const char* REAL;
        static const char* BLOB;
        static const char* NUMERIC;
    };

    struct Column {
        const std::string name;
        const char* format;
        
        Column(const std::string& name, const char* format);
    };

    
    typedef std::vector<Column> SCHEMA;
    
    
public:
    bool beginTransaction();
    
    bool commit();
    
    void registerScheme(const std::string& tableName, const SCHEMA* scheme);
    
    SCHEMA* getScheme(const std::string& tableName);
    
    bool createTable(const std::string& tableName);
    
    bool dropTable(const std::string& tableName);
    
    void deleteAllTables();
    
    void displayTable(const std::string& tableName);
    
    bool dumpTable(const std::string& tableName);
    
    bool executeSQLFile(const std::string& filePath);

    
public:
    /**
     *  "SELECT columns,... FROM tableName WHERE whereClause.."
     */
    CursorPtr sqlSelect(const std::string& tableName, const char** columns=nullptr, const std::string& whereClause="", bool countRows=false);
    
    /**
     *  "INSERT INTO tableName (columns..) VALUES (values...)"
     */
    int64_t sqlInsert(const std::string& tableName, ContentValuePtr& contentValue);
    
    /**
     *  "INSERT OR REPLACE INTO tableName (columns..) VALUES (values...)"
     */
    int64_t sqlInsertOrReplace(const std::string& tableName, ContentValuePtr& contentValue);
    
    /**
     *  "INSERT OR IGNORE INTO tableName (columns..) VALUES (values...)"
     */
    int64_t sqlInsertOrIgnore(const std::string& tableName, ContentValuePtr& contentValue);
    
    /**
     *  "DELETE FROM tableName WHERE whereClause.."
     */
    int sqlDelete(const std::string& tableName, const std::string& whereClause="");
    
    /**
     *  "UPDATE tableName SET column=value.. WHERE whereClause"
     */
    int sqlUpdate(const std::string& tableName, ContentValuePtr& contentValue, const std::string& whereClause="");
    
    /**
     * Query sql statement : sqlite3_exec(sqlString...)
     */
    int sqlRawQuery(const std::string& sqlString);
    
    /**
     *  "SELECT count(*) FROM tableName WHERE whereClause.."
     */
    int sqlCountRows(const std::string& tableName, const std::string& whereClause="");
    
    /**
     *  "SELECT A.columns..,B.columns.. FROM tableNameA A INNER JOIN tableNameB B ON joinClause.. WHERE whereClause..";
     *  칼럼 이름 조심할 것... 같은 이름인 경우 B 참조.. 값이 다르다면 원하는 결과가 아닐 수도 있음.
     */
    CursorPtr sqlInnerJoin(const std::string& tableNameA, const char** columnsA, const std::string& tableNameB, const char** columnsB,
                           const std::string& joinClause, const std::string& whereClause="");
    
    
    static std::string assign(const std::string& key, const std::string& value);
    
    static std::string assign(const std::string& key, const int& value);
    
    static std::string assign(const std::string& key, const int64_t& value);
    
    static std::string assign(const std::string& key, const float& value);
    
    static std::string assign(const std::string& key, const double& value);
    
    static std::string assignIn(const std::string& key, const std::string& values);
    
    static std::string escapeString(const std::string& str);
    
    static std::string printableString(const unsigned char* str);
    
protected:
    SQLiteWrapper();
    
    virtual ~SQLiteWrapper();
    
    virtual bool initDatabase(const std::string& databaseName, const int version);
    
    virtual void release();
    
    sqlite3* getDatabase() { return _database; }
    
    virtual void onCreate() = 0;
    
    virtual void onUpgrade(int oldVersion, int newVersion);
    
    virtual void onDowngrade(int oldVersion, int newVersion);
    
private:
    int64_t sqlInsertInternal(const std::string& sqlCommand, const std::string& tableName, ContentValuePtr& contentValue);
    
private:
    static const SCHEMA sDBVersion;
    
    sqlite3* _database;
    
    int numRows;
    
    std::map<std::string, SCHEMA*> _schemes;
    
    std::mutex _mutex;
    
    int _transactionCount;
    
};




// DB Cursor : DB Access 관리자

class DBCursor {
    
public:
    bool step();
    
    void close();
    
    bool hasColumn(const std::string& columnName);
    
    /**
     * Returns true if the value in the indicated column is null.
     */
    bool isNull(const std::string& columnName);
    
    /**
     * Returns the value of the requested column as an string.
     */
    std::string getString(const std::string& columnName, const std::string& defaultValue = "");
    
    /**
     * Returns the value of the requested column as an int.
     */
    int getInt(const std::string& columnName, const int defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an long.
     */
    long getLong(const std::string& columnName, const long defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an int64_t.
     */
    int64_t getInt64(const std::string& columnName, const int64_t defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an float.
     */
    int getFloat(const std::string& columnName, const float defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an double.
     */
    double getDouble(const std::string& columnName, const double defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an string.
     */
    std::string getString(int columnIndex, const std::string& defaultValue = "");
    
    /**
     * Returns the value of the requested column as an int.
     */
    int getInt(int columnIndex, const int defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an long.
     */
    long getLong(int columnIndex, const long defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an int64_t.
     */
    int64_t getInt64(int columnIndex, const int64_t defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an float.
     */
    int getFloat(int columnIndex, const float defaultValue = 0);
    
    /**
     * Returns the value of the requested column as an double.
     */
    double getDouble(int columnIndex, const double defaultValue = 0);
    
public:
    DBCursor(sqlite3_stmt* stmt, std::map<std::string, int>& columns);
    
    virtual ~DBCursor();
    
protected:
    static CursorPtr create(sqlite3_stmt* stmt, std::map<std::string, int>& columns);
    
private:
    
    sqlite3_stmt* _stmt;
    
    std::map<std::string, int> _columMap;
    
    friend class SQLiteWrapper;
};


#endif /* SQLiteWrapper_h */
