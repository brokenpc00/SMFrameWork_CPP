//
//  Intent.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "Intent.h"
#include <string>
#include <sstream>
#include "../Util/StringUtil.h"


const std::string Intent::EMPTY_STRING("");

Intent* Intent::create() {
    Intent* intent = new (std::nothrow) Intent();
    if (intent != nullptr) {
        intent->autorelease();
    }
    return intent;
}

void Intent::putChar(const std::string& key, const char value) {
    _Item item;
    item.t = _TYPE_CHAR;
    item.v.charValue = value;
    
    insertItem(key, item);
}

void Intent::putInt(const std::string& key, const int value) {
    _Item item;
    item.t = _TYPE_INT;
    item.v.intValue = value;
    
    insertItem(key, item);
}

void Intent::putLong(const std::string& key, const long value) {
    _Item item;
    item.t = _TYPE_LONG;
    item.v.longValue = value;
    
    insertItem(key, item);
}

void Intent::putInt64(const std::string& key, const int64_t value) {
    _Item item;
    item.t = _TYPE_INT64;
    item.v.int64Value = value;
    
    insertItem(key, item);
}

void Intent::putFloat(const std::string& key, const float value) {
    _Item item;
    item.t = _TYPE_FLOAT;
    item.v.floatValue = value;
    
    insertItem(key, item);
}

void Intent::putDouble(const std::string& key, const double value) {
    _Item item;
    item.t = _TYPE_DOUBLE;
    item.v.doubleValue = value;
    
    insertItem(key, item);
}

void Intent::putBool(const std::string& key, const bool value) {
    _Item item;
    item.t = _TYPE_BOOL;
    item.v.boolValue = value;
    
    insertItem(key, item);
}

void Intent::putRef(const std::string& key, const cocos2d::Ref* value) {
    if (value == nullptr)
        return;
    
    _Item item;
    item.t = _TYPE_REF;
    item.v.refValue = (cocos2d::Ref*)value;
    item.v.refValue->retain();
    
    insertItem(key, item);
}

void Intent::putString(const std::string& key, const std::string& value) {
    _Item item;
    item.t = _TYPE_STRING;
    item.v.stringValue = new std::string(value);
    
    insertItem(key, item);
}

char Intent::getChar(const std::string& key, const char defaultValue/* = '\0'*/) {
    _Item* item = findItem(key, _TYPE_CHAR);
    if (item != nullptr) {
        return item->v.charValue;
    }
    return defaultValue;
}

int Intent::getInt(const std::string& key, const int defaultValue/* = 0*/) {
    _Item* item = findItem(key, _TYPE_INT);
    if (item != nullptr) {
        return item->v.intValue;
    }
    return defaultValue;
}

long Intent::getLong(const std::string& key, const long defaultValue/* = 0*/) {
    _Item* item = findItem(key, _TYPE_LONG);
    if (item != nullptr) {
        return item->v.longValue;
    }
    return defaultValue;
}

int64_t Intent::getInt64(const std::string& key, const int64_t defaultValue/* = 0*/) {
    _Item* item = findItem(key, _TYPE_INT64);
    if (item != nullptr) {
        return item->v.int64Value;
    }
    return defaultValue;
}

float Intent::getFloat(const std::string& key, const float defaultValue/* = 0*/) {
    _Item* item = findItem(key, _TYPE_FLOAT);
    if (item != nullptr) {
        return item->v.floatValue;
    }
    return defaultValue;
}

double Intent::getDouble(const std::string& key, const double defaultValue/* = 0*/) {
    _Item* item = findItem(key, _TYPE_DOUBLE);
    if (item != nullptr) {
        return item->v.doubleValue;
    }
    return defaultValue;
}

bool Intent::getBool(const std::string& key, const bool defaultValue/* = false*/) {
    _Item* item = findItem(key, _TYPE_BOOL);
    if (item != nullptr) {
        return item->v.boolValue;
    }
    return defaultValue;
}
cocos2d::Ref* Intent::getRef(const std::string& key) {
    _Item* item = findItem(key, _TYPE_REF);
    if (item != nullptr) {
        return item->v.refValue;
    }
    return nullptr;
}

std::string& Intent::getString(const std::string& key, const std::string defaultValue) {
    _Item* item = findItem(key, _TYPE_STRING);
    if (item != nullptr) {
        return *item->v.stringValue;
    }
    return (std::string&)defaultValue;
}

std::string Intent::asString(std::map<std::string, _Item>::iterator it) {
    if (it != _data->end()) {
        switch (it->second.t) {
            case _TYPE_STRING:
                return *it->second.v.stringValue;
            case _TYPE_CHAR:
            {
                char ret[2] = { it->second.v.charValue, 0 };
                return std::string(ret);
            }
            case _TYPE_INT:
                return patch::to_string(it->second.v.intValue);
            case _TYPE_LONG:
                return patch::to_string(it->second.v.longValue);
            case _TYPE_INT64:
                return patch::to_string(it->second.v.int64Value);
            case _TYPE_FLOAT:
                return patch::to_string(it->second.v.floatValue);
            case _TYPE_DOUBLE:
                return patch::to_string(it->second.v.doubleValue);
            case _TYPE_BOOL:
                return it->second.v.boolValue?"true":"false";
            default:
                break;
        }
    }
    
    return EMPTY_STRING;
}

std::string Intent::asString(const std::string& key) {
    return asString(_data->find(key));
}

Intent::Intent() : _data(nullptr)
{
}

Intent::~Intent() {
    if (_data != nullptr) {
        
        for (std::map<std::string, _Item>::iterator it = _data->begin(); it != _data->end(); ++it) {
            releaseRefValue(it);
        }
        
        delete _data;
    }
}

void Intent::insertItem(const std::string& key, const _Item& item) {
    if (_data == nullptr) {
        _data = new std::map<std::string, _Item>();
    } else {
        std::map<std::string, _Item>::iterator it = _data->find(key);
        if (it != _data->end()) {
            releaseRefValue(it);
            _data->erase(it);
        }
    }
    
    _data->insert(std::pair<std::string, _Item>(key, item));
}

Intent::_Item* Intent::findItem(const std::string& key, _T type) {
    if (_data==NULL) {
        return nullptr;
    }
    std::map<std::string, _Item>::iterator it = _data->find(key);
    if (it != _data->end()) {
        if (it->second.t == type) {
            return &it->second;
        }
    }
    return nullptr;
}

void Intent::releaseRefValue(const std::map<std::string, _Item>::iterator& it) {
    _Item item = it->second;
    if (item.t == _TYPE_REF) {
        item.v.refValue->release();
    } else if (item.t == _TYPE_STRING) {
        delete item.v.stringValue;
    }
}
