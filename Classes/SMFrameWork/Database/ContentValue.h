//
//  ContentValue.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
// DB Access를 위한 데이터 타입 class

#ifndef ContentValue_h
#define ContentValue_h

#include <memory>
#include <string>
#include <list>

class ContentValue;

typedef std::unique_ptr<ContentValue> ContentValuePtr;

class ContentValue
{
private:
    // 데이터 Type enum
    enum _T {
        _TYPE_CHAR,
        _TYPE_INT,
        _TYPE_LONG,
        _TYPE_INT64,
        _TYPE_FLOAT,
        _TYPE_DOUBLE,
        _TYPE_BOOL,
        _TYPE_STRING
    };
    
    // 데이터 value;
    union _V {
        char    charValue;
        int     intValue;
        long    longValue;
        int64_t    int64Value;
        float   floatValue;
        double  doubleValue;
        bool    boolValue;
        std::string* stringValue;
    };

    // 데이터 형식
    struct _Item {
        std::string key;    // 데이터 키
        _T t;   // 데이터 타입
        _V v;   // 테이터 값
    };
    
public:
    static ContentValuePtr create();
    
    void putChar(const std::string& key, const char value);
    
    void putInt(const std::string& key, const int value);
    
    void putLong(const std::string& key, const long value);
    
    void putInt64(const std::string& key, const int64_t value);
    
    void putFloat(const std::string& key, const float value);
    
    void putDouble(const std::string& key, const double value);
    
    void putBool(const std::string& key, const bool value);
    
    void putString(const std::string& key, const std::string& value);
    
    char getChar(const std::string& key, const char defaultValue = '\0');
    
    int getInt(const std::string& key, const int defaultValue = 0);
    
    long getLong(const std::string& key, const long defaultValue = 0);
    
    int64_t getInt64(const std::string& key, const int64_t defaultValue = 0);
    
    float getFloat(const std::string& key, const float defaultValue = 0);
    
    double getDouble(const std::string& key, const double defaultValue = 0);
    
    bool getBool(const std::string& key, const bool defaultValue = false);
    
    bool isString(std::list<_Item>::iterator it);
    
    std::string getKey(std::list<_Item>::iterator it);
    
    std::string asString(std::list<_Item>::iterator it);
    
    std::string& getString(const std::string& key);
    
    std::list<_Item>::iterator begin() { return _data.begin(); }
    
    std::list<_Item>::iterator end() { return _data.end(); }
    
    int size() { return (int)_data.size(); }
    
    void clear();
    
public:
    ContentValue();
    virtual ~ContentValue();
    
private:
    static const std::string EMPTY_STRING;
    
    std::list<_Item> _data;
    
    void insertItem(const std::string& key, _Item& item);
    
    _Item* findItem(const std::string& key, _T type);
    
    void releaseMemValue(const std::list<_Item>::iterator& it);
};


#endif /* ContentValue_h */
