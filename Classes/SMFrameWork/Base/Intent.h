//
//  Intent.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#ifndef Intent_h
#define Intent_h

#include <base/CCRef.h>
#include <string>
#include <map>

class Intent : public cocos2d::Ref {
    
private:
    enum _T {
        _TYPE_CHAR,
        _TYPE_INT,
        _TYPE_LONG,
        _TYPE_INT64,
        _TYPE_FLOAT,
        _TYPE_DOUBLE,
        _TYPE_BOOL,
        _TYPE_REF,
        _TYPE_STRING
    };
    
    union _V {
        char    charValue;
        int     intValue;
        long    longValue;
        int64_t    int64Value;
        float   floatValue;
        double  doubleValue;
        bool    boolValue;
        cocos2d::Ref* refValue;
        std::string* stringValue;
    };
    
    struct _Item {
        _T t;
        _V v;
    };
    
public:
    static Intent* create();
    
    void putChar(const std::string& key, const char value);
    
    void putInt(const std::string& key, const int value);
    
    void putLong(const std::string& key, const long value);
    
    void putInt64(const std::string& key, const int64_t value);
    
    void putFloat(const std::string& key, const float value);
    
    void putDouble(const std::string& key, const double value);
    
    void putBool(const std::string& key, const bool value);
    
    void putRef(const std::string& key, const cocos2d::Ref* value);
    
    void putString(const std::string& key, const std::string& value);
    
    char getChar(const std::string& key, const char defaultValue = '\0');
    
    int getInt(const std::string& key, const int defaultValue = 0);
    
    long getLong(const std::string& key, const long defaultValue = 0);
    
    int64_t getInt64(const std::string& key, const int64_t defaultValue = 0);
    
    float getFloat(const std::string& key, const float defaultValue = 0);
    
    double getDouble(const std::string& key, const double defaultValue = 0);
    
    bool getBool(const std::string& key, const bool defaultValue = false);
    
    std::string asString(const std::string& key);
    
    std::string asString(std::map<std::string, _Item>::iterator iter);
    
    cocos2d::Ref* getRef(const std::string& key);
    
    std::string& getString(const std::string& key, const std::string defaultValue = EMPTY_STRING);
    
    std::map<std::string, _Item>::iterator begin() { return _data->begin(); }
    
    std::map<std::string, _Item>::iterator end() { return _data->end(); }
    
protected:
    Intent();
    virtual ~Intent();
    
private:
    CC_DISALLOW_COPY_AND_ASSIGN(Intent);
    
    static const std::string EMPTY_STRING;
    
    std::map<std::string, _Item>* _data;
    
    void insertItem(const std::string& key, const _Item& item);
    
    _Item* findItem(const std::string& key, _T type);
    
    void releaseRefValue(const std::map<std::string, _Item>::iterator& it);
    
};



#endif /* Intent_h */
