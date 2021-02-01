//
//  SQLiteWrapper.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "SQLiteWrapper.h"
#include "../Util/FileManager.h"
#include "../Util/StringUtil.h"
#include <platform/CCFileUtils.h>
#include <cocos2d.h>
#include "sqlite3.h"
//#include <sqlite3/include/sqlite3.h>

const char* SQLiteWrapper::ROWID = "ROWID";
const char* SQLiteWrapper::ALL_COLUMNS[]   = { "*" };
const char* SQLiteWrapper::NO_COLUMNS[]    = { "" };


static const char* VERSION_TABLE_NAME = "VERSION";
static const char* VERSION_COLUMN_NAME = "code";


const char* SQLiteWrapper::Format::INTEGER_UNIQUE_PK_AUTOINCREMENT = "INTEGER PRIMARY KEY AUTOINCREMENT";
const char* SQLiteWrapper::Format::INTEGER_UNIQUE_PK     = "INTEGER PRIMARY KEY";
const char* SQLiteWrapper::Format::INTEGER_UNIQUE_INDEX  = "INTEGER NOT NULL UNIQUE";
const char* SQLiteWrapper::Format::INTEGER_INDEX = "INTEGER NOT NULL";
const char* SQLiteWrapper::Format::INTEGER = "INTEGER";
const char* SQLiteWrapper::Format::TEXT    = "TEXT";
const char* SQLiteWrapper::Format::REAL    = "REAL";
const char* SQLiteWrapper::Format::BLOB    = "BLOB";
const char* SQLiteWrapper::Format::NUMERIC = "NUMERIC";


const SQLiteWrapper::SCHEMA SQLiteWrapper::sDBVersion = {
    Column(VERSION_COLUMN_NAME, Format::INTEGER),
};

SQLiteWrapper::Column::Column(const std::string& _name, const char* _format) : name(_name), format(_format)
{
}

const SQLiteWrapper::SCHEMA sDBVersion = {
    SQLiteWrapper::Column(VERSION_COLUMN_NAME, SQLiteWrapper::Format::INTEGER),
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// SQLiteWrapper class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
SQLiteWrapper::SQLiteWrapper() : _transactionCount(0)
{
    registerScheme(VERSION_TABLE_NAME, &sDBVersion);
}

SQLiteWrapper::~SQLiteWrapper()
{
    release();
}

bool SQLiteWrapper::initDatabase(const std::string& databaseName, const int version)
{
    auto fileUtils = FileManager::getInstance();
    
    auto dbPath = fileUtils->getFullFilePath(FileManager::FileType::DB, databaseName);
    
    
    // android는 rb로 열어서 다시
//#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    FILE * pFile = fopen(dbPath.c_str(), "r");
    if (pFile==nullptr) {
        cocos2d::Data data = cocos2d::FileUtils::getInstance()->getDataFromFile(dbPath);
        pFile = fopen(dbPath.c_str(), "wb");
        fwrite(data.getBytes(), data.getSize(), 1, pFile);
        data.clear();
    }
    fclose(pFile);
//#endif
    
    // Open database
    if (SQLITE_OK != sqlite3_open(dbPath.c_str(), &_database)) {
        return false;
    }
    
    // 버전 체크
    auto cursor = sqlSelect(VERSION_TABLE_NAME);
    if (cursor == nullptr || !cursor->step()) {
        // 처음 만들어짐.
        onCreate();
        
        // 버전 저장
        createTable(VERSION_TABLE_NAME);
        
        auto contentValue = ContentValue::create();
        contentValue->putInt(VERSION_COLUMN_NAME, version);
        sqlInsert(VERSION_TABLE_NAME, contentValue);
    } else {
        // 현재 DB 버전
        int currentVersion = cursor->getInt(VERSION_COLUMN_NAME);
        if (version != currentVersion) {
            // 저장되어 있는 버전과 다름
            if (version > currentVersion) {
                // 업그레이드
                onUpgrade(currentVersion, version);
            } else if (version < currentVersion) {
                // 다운그레이드
                onDowngrade(currentVersion, version);
            }
            
            // 버전 업데이트
            auto contentValue = ContentValue::create();
            contentValue->putInt(VERSION_COLUMN_NAME, version);
            sqlUpdate(VERSION_COLUMN_NAME, contentValue);
        }
    }
    
    return true;
}

void SQLiteWrapper::release() {
    if (_database) {
        sqlite3_close(_database);
        _database = nullptr;
    }
}

bool SQLiteWrapper::executeSQLFile(const std::string& filePath)
{
    
    auto fileUtils = cocos2d::FileUtils::getInstance();
    auto path = fileUtils->fullPathForFilename(filePath);
    
    auto query = fileUtils->getStringFromFile(path);
    
    int ret = sqlite3_exec(_database, query.c_str(), nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
//        CCLOG("[DATABASE] execute SQL file \"%s\" FAILED : %s", filePath.c_str(), sqlite3_errmsg(_database));
        CC_ASSERT(0);
        return false;
    }
    
    return true;
}

bool SQLiteWrapper::beginTransaction()
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    if (_transactionCount == 0) {
        int ret = sqlite3_exec(_database, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        if (ret == SQLITE_OK) {
            _transactionCount++;
        } else {
            return false;
        }
    } else {
        _transactionCount++;
    }
    
    return true;
}

bool SQLiteWrapper::commit()
{
    std::lock_guard<std::mutex> guard(_mutex);
    
//    CCASSERT(_transactionCount > 0, "[DATABASE] beginTransaction() first");
    
    _transactionCount--;
    
    if (_transactionCount == 0) {
        int ret = sqlite3_exec(_database, "COMMIT;", nullptr, nullptr, nullptr);
        if (ret != SQLITE_OK) {
//            CCLOG("[DATABASE] COMMIT FAILED");
        }
    }
    
    return true;
}

void SQLiteWrapper::registerScheme(const std::string& tableName, const SCHEMA* scheme)
{
    _schemes.insert(std::pair<std::string, SCHEMA*>(tableName, (SCHEMA*)scheme));
}

SQLiteWrapper::SCHEMA* SQLiteWrapper::getScheme(const std::string& tableName)
{
    auto iter = _schemes.find(tableName);
    if (iter != _schemes.end()) {
        return iter->second;
    }
    return nullptr;
}

bool SQLiteWrapper::createTable(const std::string& tableName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    auto scheme = getScheme(tableName);
//    CCASSERT(scheme, "[DATABASE] registerScheme(" + tableName + ") first");
    
    std::string query = "CREATE TABLE \"" + tableName + "\" (";
    int pkIndex = -1;
    
    int numColumn = (int)scheme->size();
    for (int i = 0; i < numColumn; i++) {
        auto& column = scheme->at(i);
        if (i > 0) {
            query += ",";
        }
        query += "`" + column.name + "`" + " " + *column.format;
        
        if (column.format == Format::INTEGER_UNIQUE_PK || column.format == Format::INTEGER_UNIQUE_PK_AUTOINCREMENT) {
//            CCASSERT(pkIndex < 0, "[DATABASE] DUPLICATE PRIMARY KEY");
            pkIndex = i;
        }
    }
    query += ");";
    
    int result = sqlRawQuery(query);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] CREATE TABLE \"%s\" FAILED : %s", tableName.c_str(), sqlite3_errmsg(_database));
        CC_ASSERT(0);
        return false;
    }
    
    // create index
    for (int i = 0; i < numColumn; i++) {
        auto& column = scheme->at(i);
        
        if (column.format == Format::INTEGER_UNIQUE_INDEX) {
            query = "CREATE UNIQUE INDEX " + tableName + "_" + column.name + " ON " + tableName + "(" + column.name + ")";
            result = sqlRawQuery(query);
            if (SQLITE_OK != result) {
//                CCLOG("[DATABASE] CREATE UNIQUE INDEX \"%s_%s\" FAILED : %s", tableName.c_str(), column.name.c_str(), sqlite3_errmsg(_database));
                CC_ASSERT(0);
                return false;
            }
        } else if (column.format == Format::INTEGER_INDEX) {
            query = "CREATE INDEX " + tableName + "_" + column.name + " ON " + tableName + "(" + column.name + ")";
            result = sqlRawQuery(query);
            if (SQLITE_OK != result) {
//                CCLOG("[DATABASE] CREATE INDEX \"%s_%s\" FAILED : %s", tableName.c_str(), column.name.c_str(), sqlite3_errmsg(_database));
                CC_ASSERT(0);
                return false;
            }
        }
    }
    
//    CCLOG("[DATABASE] CREATE TABLE \"%s\" SUCCESS", tableName.c_str());
    
    return true;
}

bool SQLiteWrapper::dropTable(const std::string& tableName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query = "DROP TABLE IF EXISTS " + tableName + ";";
    
    int result = sqlite3_exec(_database, query.c_str(), nullptr, nullptr, nullptr);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] DROP TABLE \"%s\" FAILED : %s", tableName.c_str(), sqlite3_errmsg(_database));
        CC_ASSERT(0);
        return false;
    }
    
//    CCLOG("[DATABASE] DROP TABLE \"%s\" SUCCESS", tableName.c_str());
    
    return true;
}

void SQLiteWrapper::deleteAllTables()
{
    for (auto iter = _schemes.begin(); iter != _schemes.end(); ++iter) {
        dropTable(iter->first);
    }
}

void SQLiteWrapper::displayTable(const std::string &tableName)
{
    std::string query = "SELECT * from `" + tableName + "`;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, nullptr);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] DUMP TABLE \"%s\" FAILED : %s", tableName.c_str(), sqlite3_errmsg(_database));
//        CC_ASSERT(0);
        return;
    }
    
    auto scheme = _schemes[tableName];
    int numColumn = (int)scheme->size();

    while (SQLITE_ROW == sqlite3_step(stmt)) {
        std::string statement = "";
        
        for (int i = 0; i < numColumn; i++) {
            if (scheme->at(i).format == Format::TEXT) {
                statement += "'" + printableString(sqlite3_column_text(stmt, i)) + "'";
            } else if (scheme->at(i).format == Format::INTEGER_UNIQUE_PK_AUTOINCREMENT) {
                statement += StringUtil::format("%lld", sqlite3_column_int(stmt, i));
            } else if (scheme->at(i).format == Format::INTEGER_UNIQUE_PK) {
                statement += StringUtil::format("%lld", sqlite3_column_int(stmt, i));
            } else if (scheme->at(i).format == Format::INTEGER_UNIQUE_INDEX) {
                statement += StringUtil::format("%d", sqlite3_column_int(stmt, i));
            } else if (scheme->at(i).format == Format::BLOB) {
                statement += StringUtil::format("%d", sqlite3_column_int(stmt, i));
            } else if (scheme->at(i).format == Format::REAL) {
                statement += StringUtil::format("%f", sqlite3_column_double(stmt, i));
            } else {
                statement += StringUtil::format("%s", sqlite3_column_text(stmt, i));
            }
            if (i < numColumn-1) {
                statement += " | ";
            }
        }
        
        CCLOG("%s", statement.c_str());
    }
}

bool SQLiteWrapper::dumpTable(const std::string& tableName)
{
    std::string query = "SELECT * from `" + tableName + "`;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, nullptr);
    if (SQLITE_OK != result) {
        CCLOG("[DATABASE] DUMP TABLE \"%s\" FAILED : %s", tableName.c_str(), sqlite3_errmsg(_database));
        CC_ASSERT(0);
        return false;
    }
    
    auto scheme = _schemes[tableName];
    int numColumn = (int)scheme->size();
    
    CCLOG("//----------------------------------------------------------------");
    CCLOG("// dump '%s' table", tableName.c_str());
    CCLOG("//=========================");
    int lines = 0;
    while (SQLITE_ROW == sqlite3_step(stmt)) {
        std::string statement = "INSERT INTO `"+tableName+"` VALUES (";
        
        for (int i = 0; i < numColumn; i++) {
            if (scheme->at(i).format == Format::TEXT) {
                statement += "'" + printableString(sqlite3_column_text(stmt, i)) + "'";
            } else {
                statement += StringUtil::format("%s", sqlite3_column_text(stmt, i));
            }
            if (i < numColumn-1) {
                statement += ",";
            }
        }
        statement += ");";
        
        CCLOG("%s", statement.c_str());
        lines++;
    }
    CCLOG("//=========================");
    CCLOG("// done (%d lines)", lines);
    CCLOG("");
    
    return true;
}

/**
 *  "SELECT columns,... FROM tableName WHERE whereClause.."
 */
CursorPtr SQLiteWrapper::sqlSelect(const std::string& tableName, const char** columns, const std::string& whereClause, bool countRows)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query;
    if (countRows) {
        query = "SELECT count(";
    } else {
        query = "SELECT ";
    }
    std::map<std::string, int> columnMap;
    
    if (columns == nullptr || columns == ALL_COLUMNS) {
        query += "*";
        auto scheme = _schemes[tableName];
        int numColumn = (int)scheme->size();
        for (int i = 0; i < numColumn; i++) {
            auto& column = scheme->at(i);
            columnMap.insert(std::pair<std::string, int>(column.name, i));
        }
    } else {
        int i = 0;
        while (columns[i] == nullptr) {
            query += columns[i];
            
            columnMap.insert(std::pair<std::string, int>(columns[i], i));
            i++;
            if (columns[i] != nullptr) {
                query += ",";
            }
        }
    }
    if (countRows) {
        query += ")";
    }
    
    query += " FROM `" + tableName + "`";
    if (!whereClause.empty()) {
        query += " WHERE " + whereClause;
    }
    query += ";";
    
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, nullptr);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] SELECT FAILED : %s", query.c_str());
//        CCLOG("ERROR : %s", sqlite3_errmsg(_database));
        //        CC_ASSERT(0);
        return nullptr;
    }
    
    //    return std::move(DBCursor::create(stmt, columnMap));
    return DBCursor::create(stmt, columnMap);
}

/**
 *  "SELECT A.columns..,B.columns.. FROM tableNameA A INNER JOIN tableNameB B ON joinClause.. WHERE whereClause..";
 */
CursorPtr SQLiteWrapper::sqlInnerJoin(const std::string& tableNameA, const char** columnsA,
                                     const std::string& tableNameB, const char** columnsB,
                                      const std::string& joinClause, const std::string& whereClause)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query = "SELECT ";
    
    std::map<std::string, int> columnMap;
    int indexCount = 0;
    
    // left table
    if (columnsA == nullptr || columnsA == ALL_COLUMNS) {
        query += "A.*";
        auto scheme = _schemes[tableNameA];
        
        int numColumns = (int)scheme->size();
        for (int i = 0; i < numColumns; i++) {
            auto& column = scheme->at(i);
            columnMap.insert(std::pair<std::string, int>(column.name, i));
        }
        indexCount = numColumns;
    } else if (columnsA != NO_COLUMNS) {
        int i = 0;
        while (columnsA[i] == nullptr) {
            query += "A.";
            query += columnsA[i];
            
            columnMap.insert(std::pair<std::string, int>(columnsA[i], i));
            i++;
            indexCount++;
            if (columnsA[i] != nullptr) {
                query += ",";
            }
        }
    }
    
    // right table
    if (columnsB == nullptr || columnsB == ALL_COLUMNS) {
        if (columnsA != NO_COLUMNS) {
            query += ",";
        }
        
        query += "B.*";
        auto scheme = _schemes[tableNameB];
        
        int numColumns = (int)scheme->size();
        for (int i = 0; i < numColumns; i++) {
            auto& column = scheme->at(i);
            columnMap.insert(std::pair<std::string, int>(column.name, indexCount+i));
        }
    } else if (columnsB != NO_COLUMNS) {
        if (columnsA != NO_COLUMNS) {
            query += ",";
        }
        
        int i = 0;
        while (columnsB[i] == nullptr) {
            query += "B.";
            query += columnsB[i];
            
            columnMap.insert(std::pair<std::string, int>(columnsB[i], indexCount+i));
            i++;
            if (columnsB[i] != nullptr) {
                query += ",";
            }
        }
    }
    
    query += " FROM `" + tableNameA + "` A INNER JOIN `" + tableNameB + "` B ON " + joinClause;
    if (!whereClause.empty()) {
        query += " WHERE " + whereClause;
    }
    query += ";";
    
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, nullptr);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] SELECT FAILED : %s", query.c_str());
//        CCLOG("ERROR : %s", sqlite3_errmsg(_database));
        return nullptr;
    }
    
    //    return std::move(DBCursor::create(stmt, columnMap));
    return DBCursor::create(stmt, columnMap);
}

/**
 *  "SELECT count(*) FROM tableName WHERE whereClause.."
 */
int SQLiteWrapper::sqlCountRows(const std::string& tableName, const std::string& whereClause) {
    auto cursor = sqlSelect(tableName, nullptr, whereClause, true);
    if (cursor && cursor->step()) {
        return cursor->getInt(0);
    }
    
    return 0;
}


int64_t SQLiteWrapper::sqlInsertInternal(const std::string& sqlCommand, const std::string& tableName, ContentValuePtr& contentValue)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query = sqlCommand + " INTO `" + tableName + "`(";
    
    // column
    auto iter = contentValue->begin();
    int index = 0;
    while (iter != contentValue->end()) {
        if (index > 0) {
            query += ",";
        }
        query += contentValue->getKey(iter);
        ++iter;
        ++index;
    }
    query += ") VALUES (";
    
    // value
    iter = contentValue->begin();
    index = 0;
    while (iter != contentValue->end()) {
        if (contentValue->isString(iter)) {
            query += "'"+contentValue->asString(iter)+"'";
        } else {
            query += contentValue->asString(iter);
        }
        ++iter;
        ++index;
        if (iter != contentValue->end()) {
            query += ",";
        }
    }
    query += ");";
    CCLOG("[[[[[ insert query : %s", query.c_str());
    int result = sqlRawQuery(query);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] %s FAILED : %s", sqlCommand.c_str(), query.c_str());
//        CCLOG("ERROR : %s", sqlite3_errmsg(_database));
        CC_ASSERT(0);
        return -1;
    }
    
    int64_t lastRowId =  sqlite3_last_insert_rowid(_database);
//    CCLOG("[DATABASE] %s INTO \"%s\" SUCCESS (rowId : %lld)", sqlCommand.c_str(), tableName.c_str(), lastRowId);
    
    displayTable(tableName);
    return lastRowId;
}

/**
 *  "INSERT INTO tableName (columns..) VALUES (values...)"
 */
int64_t SQLiteWrapper::sqlInsert(const std::string& tableName, ContentValuePtr& contentValue)
{
    return sqlInsertInternal("INSERT", tableName, contentValue);
}

/**
 *  "INSERT OR REPLACE INTO tableName (columns..) VALUES (values...)"
 */
int64_t SQLiteWrapper::sqlInsertOrReplace(const std::string& tableName, ContentValuePtr& contentValue)
{
    return sqlInsertInternal("INSERT OR REPLACE", tableName, contentValue);
}

/**
 *  "INSERT OR IGNORE INTO tableName (columns..) VALUES (values...)"
 */
int64_t SQLiteWrapper::sqlInsertOrIgnore(const std::string& tableName, ContentValuePtr& contentValue)
{
    return sqlInsertInternal("INSERT OR IGNORE", tableName, contentValue);
}

/**
 *  "DELETE FROM tableName WHERE whereClause.."
 */
int SQLiteWrapper::sqlDelete(const std::string& tableName, const std::string& whereClause)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query = "DELETE FROM `" + tableName + "`";
    if (!whereClause.empty()) {
        query += " WHERE " + whereClause;
    }
    query += ";";
    
    int result = sqlRawQuery(query);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] DELETE FAILED : %s", query.c_str());
//        CCLOG("ERROR : %s", sqlite3_errmsg(_database));
        //        CC_ASSERT(0);
        return -1;
    }
    
    int changes = sqlite3_changes(_database);
    
//    CCLOG("[DATABASE] DELETE FROM \"%s\" SUCCESS (%d rows affected)", tableName.c_str(), changes);
    
    return changes;
}

/**
 *  "UPDATA tableName SET column=value.. WHERE whereClause.."
 */
int SQLiteWrapper::sqlUpdate(const std::string& tableName, ContentValuePtr& contentValue, const std::string& whereClause)
{
    std::lock_guard<std::mutex> guard(_mutex);
    
    std::string query = "UPDATE `" + tableName + "` SET ";
    
    // column
    auto iter = contentValue->begin();
    int index = 0;
    while (iter != contentValue->end()) {
        query += contentValue->getKey(iter) + "=";
        if (contentValue->isString(iter)) {
            query += "'"+contentValue->asString(iter)+"'";
        } else {
            query += contentValue->asString(iter);
        }
        ++iter;
        ++index;
        if (iter != contentValue->end()) {
            query += ",";
        }
    }
    
    if (!whereClause.empty()) {
        query += " WHERE " + whereClause;
    }
    query += ";";
    
    int result = sqlRawQuery(query);
    if (SQLITE_OK != result) {
//        CCLOG("[DATABASE] UPDATE FAILED : %s", query.c_str());
//        CCLOG("ERROR : %s", sqlite3_errmsg(_database));
        //        CC_ASSERT(0);
        return -1;
    }
    
    int changes = sqlite3_changes(_database);
    
//    CCLOG("[DATABASE] UPDATE \"%s\" SUCCESS (%d rows affected)", tableName.c_str(), changes);
    
    return changes;
}

/**
 * Execute sql statement
 */
int SQLiteWrapper::sqlRawQuery(const std::string& sqlString)
{
    
    int result = sqlite3_exec(_database, sqlString.c_str(), nullptr, nullptr, nullptr);
    
    return result;
}

void SQLiteWrapper::onUpgrade(int oldVersion, int newVersion)
{
}

void SQLiteWrapper::onDowngrade(int oldVersion, int newVersion)
{
}


std::string SQLiteWrapper::assign(const std::string& key, const std::string& value)
{
    return key + "='" + value +"'";
}

std::string SQLiteWrapper::assign(const std::string& key, const int& value)
{
    return cocos2d::StringUtils::format("%s=%d", key.c_str(), value);
//    return key + "=" + std::to_string(value);
}

std::string SQLiteWrapper::assign(const std::string& key, const int64_t& value)
{
    return cocos2d::StringUtils::format("%s=%lld", key.c_str(), value);
//    return key + "=" + std::to_string(value);
}

std::string SQLiteWrapper::assign(const std::string& key, const float& value)
{
    return cocos2d::StringUtils::format("%s=%ff", key.c_str(), value);
//    return key + "=" + std::to_string(value);
}

std::string SQLiteWrapper::assign(const std::string& key, const double& value)
{
    return cocos2d::StringUtils::format("%s=%f", key.c_str(), value);
//    return key + "=" + std::to_string(value);
}

std::string SQLiteWrapper::assignIn(const std::string& key, const std::string& values)
{
    return key + " IN (" + values + ")";
}

std::string SQLiteWrapper::escapeString(const std::string& str)
{
    return StringUtil::replaceAll(str, "'", "''");
}

std::string SQLiteWrapper::printableString(const unsigned char* str)
{
    std::string s(reinterpret_cast<const char*>(str));
    
    if (!s.empty()) {
        s = StringUtil::replaceAll(s, "'", "''");
        s = StringUtil::replaceAll(s, "\n", "\\n");
    }
    
    return s;
}


//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// DBCursor class
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
CursorPtr DBCursor::create(sqlite3_stmt* stmt, std::map<std::string, int>& columns)
{
    //    CursorPtr cursor(new DBCursor(stmt, columns));
    //    return std::move(cursor);
    return CursorPtr(new DBCursor(stmt, columns));
}

DBCursor::DBCursor(sqlite3_stmt* stmt, std::map<std::string, int>& columns)
{
    _stmt = stmt;
    _columMap = columns;
}

DBCursor::~DBCursor()
{
    close();
}

void DBCursor::close()
{
    if (_stmt) {
        _columMap.clear();
        sqlite3_reset(_stmt);
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
    }
}

bool DBCursor::step()
{
    int result = sqlite3_step(_stmt);
    if (result == SQLITE_ROW) {
        return true;
    }
    
    return false;
}

bool DBCursor::hasColumn(const std::string& columnName)
{
    if (_stmt == nullptr)
        return true;
    
    return (_columMap.find(columnName) != _columMap.end());
}

bool DBCursor::isNull(const std::string& columnName)
{
    if (_stmt == nullptr)
        return true;
    
    auto iter = _columMap.find(columnName);
    if (iter != _columMap.end()) {
        int columnIndex = iter->second;
        sqlite3_value* value = sqlite3_column_value(_stmt, columnIndex);
        if (sqlite3_value_type(value) != SQLITE_NULL) {
            return false;
        }
    }
    
    return true;
}

std::string DBCursor::getString(const std::string& columnName, const std::string& defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    auto iter = _columMap.find(columnName);
    if (iter != _columMap.end()) {
        int columnIndex = iter->second;
        return std::string(reinterpret_cast<const char*>(sqlite3_column_text(_stmt, columnIndex)));
    }
    
//    CCASSERT(0, "[DATABASE] Invalid column name : " + columnName);
    
    return defaultValue;
}

int DBCursor::getInt(const std::string& columnName, const int defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    auto iter = _columMap.find(columnName);
    if (iter != _columMap.end()) {
        int columnIndex = iter->second;
        return sqlite3_column_int(_stmt, columnIndex);
    }
    
//    CCASSERT(0, "[DATABASE] Invalid column name : " + columnName);
    
    return defaultValue;
}

int64_t DBCursor::getInt64(const std::string& columnName, const int64_t defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    auto iter = _columMap.find(columnName);
    if (iter != _columMap.end()) {
        int columnIndex = iter->second;
        return (int64_t)sqlite3_column_int64(_stmt, columnIndex);
    }
    
//    CCASSERT(0, "[DATABASE] Invalid column name : " + columnName);
    
    return defaultValue;
}

double DBCursor::getDouble(const std::string& columnName, const double defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    auto iter = _columMap.find(columnName);
    if (iter != _columMap.end()) {
        int columnIndex = iter->second;
        return sqlite3_column_double(_stmt, columnIndex);
    }
    
//    CCASSERT(0, "[DATABASE] Invalid column name : " + columnName);
    
    return defaultValue;
}

int DBCursor::getFloat(const std::string& columnName, const float defaultValue)
{
    return (float)getDouble(columnName, (float)defaultValue);
}

long DBCursor::getLong(const std::string& columnName, const long defaultValue)
{
    return (long)getInt(columnName, (int)defaultValue);
}


std::string DBCursor::getString(int columnIndex, const std::string& defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(_stmt, columnIndex)));
}

int DBCursor::getInt(int columnIndex, const int defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    return sqlite3_column_int(_stmt, columnIndex);
}

int64_t DBCursor::getInt64(int columnIndex, const int64_t defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    return (int64_t)sqlite3_column_int64(_stmt, columnIndex);
}

double DBCursor::getDouble(int columnIndex, const double defaultValue)
{
    if (_stmt == nullptr)
        return defaultValue;
    
    return sqlite3_column_double(_stmt, columnIndex);
}

int DBCursor::getFloat(int columnIndex, const float defaultValue)
{
    return (float)getDouble(columnIndex, (float)defaultValue);
}

long DBCursor::getLong(int columnIndex, const long defaultValue)
{
    return (long)getInt(columnIndex, (int)defaultValue);
}

