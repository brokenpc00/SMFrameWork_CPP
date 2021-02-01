//
//  ContentValue.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "ContentValue.h"
#include <iostream>
#include <sstream>
#include <cocos2d.h>

const std::string ContentValue::EMPTY_STRING("");

std::unique_ptr<ContentValue> ContentValue::create()
{
    auto contentValue = std::unique_ptr<ContentValue>(new ContentValue());
    return contentValue;
}

void ContentValue::putChar(const std::string& key, const char value)
{
    _Item item;
    item.t = _TYPE_CHAR;
    item.v.charValue = value;
    
    insertItem(key, item);
}

void ContentValue::putInt(const std::string& key, const int value)
{
    _Item item;
    item.t = _TYPE_INT;
    item.v.intValue = value;
    
    insertItem(key, item);
}

void ContentValue::putLong(const std::string& key, const long value)
{
    _Item item;
    item.t = _TYPE_LONG;
    item.v.longValue = value;
    
    insertItem(key, item);
}

void ContentValue::putInt64(const std::string& key, const int64_t value)
{
    _Item item;
    item.t = _TYPE_INT64;
    item.v.int64Value = value;
    
    insertItem(key, item);
}

void ContentValue::putFloat(const std::string& key, const float value)
{
    _Item item;
    item.t = _TYPE_FLOAT;
    item.v.floatValue = value;
    
    insertItem(key, item);
}

void ContentValue::putDouble(const std::string& key, const double value)
{
    _Item item;
    item.t = _TYPE_DOUBLE;
    item.v.doubleValue = value;
    
    insertItem(key, item);
}

void ContentValue::putBool(const std::string& key, const bool value)
{
    _Item item;
    item.t = _TYPE_BOOL;
    item.v.boolValue = value;
    
    insertItem(key, item);
}

void ContentValue::putString(const std::string& key, const std::string& value)
{
    _Item item;
    item.t = _TYPE_STRING;
    item.v.stringValue = new std::string(value);
    
    insertItem(key, item);
}

char ContentValue::getChar(const std::string& key, const char defaultValue/* = '\0'*/)
{
    _Item* item = findItem(key, _TYPE_CHAR);
    if (item != nullptr) {
        return item->v.charValue;
    }
    return defaultValue;
}

int ContentValue::getInt(const std::string& key, const int defaultValue/* = 0*/)
{
    _Item* item = findItem(key, _TYPE_INT);
    if (item != nullptr) {
        return item->v.intValue;
    }
    return defaultValue;
}

long ContentValue::getLong(const std::string& key, const long defaultValue/* = 0*/)
{
    _Item* item = findItem(key, _TYPE_LONG);
    if (item != nullptr) {
        return item->v.longValue;
    }
    return defaultValue;
}

int64_t ContentValue::getInt64(const std::string& key, const int64_t defaultValue/* = 0*/)
{
    _Item* item = findItem(key, _TYPE_INT64);
    if (item != nullptr) {
        return item->v.int64Value;
    }
    return defaultValue;
}

float ContentValue::getFloat(const std::string& key, const float defaultValue/* = 0*/)
{
    _Item* item = findItem(key, _TYPE_FLOAT);
    if (item != nullptr) {
        return item->v.floatValue;
    }
    return defaultValue;
}

double ContentValue::getDouble(const std::string& key, const double defaultValue/* = 0*/)
{
    _Item* item = findItem(key, _TYPE_DOUBLE);
    if (item != nullptr) {
        return item->v.doubleValue;
    }
    return defaultValue;
}

bool ContentValue::getBool(const std::string& key, const bool defaultValue/* = false*/)
{
    _Item* item = findItem(key, _TYPE_BOOL);
    if (item != nullptr) {
        return item->v.boolValue;
    }
    return defaultValue;
}

std::string& ContentValue::getString(const std::string& key)
{
    _Item* item = findItem(key, _TYPE_STRING);
    if (item != nullptr) {
        return *item->v.stringValue;
    }
    return (std::string&)EMPTY_STRING;
}

bool ContentValue::isString(std::list<_Item>::iterator it)
{
    if (it != _data.end() && it->t == _TYPE_STRING) {
        return true;
    }
    
    return false;
}

std::string ContentValue::getKey(std::list<_Item>::iterator it)
{
    return it->key;
}

std::string ContentValue::asString(std::list<_Item>::iterator it)
{
    if (it != _data.end()) {
        switch (it->t) {
            case _TYPE_STRING:
                return *it->v.stringValue;
            case _TYPE_CHAR:
            {
                char ret[2] = { it->v.charValue, 0 };
                return std::string(ret);
            }
            case _TYPE_INT:
//                return std::to_string(it->v.intValue);
                return cocos2d::StringUtils::format("%d", it->v.intValue);
            case _TYPE_LONG:
//                return std::to_string(it->v.longValue);
                return cocos2d::StringUtils::format("%ld", it->v.longValue);
            case _TYPE_INT64:
//                return std::to_string(it->v.int64Value);
                return cocos2d::StringUtils::format("%lld", it->v.int64Value);
            case _TYPE_FLOAT:
//                return std::to_string(it->v.floatValue);
                return cocos2d::StringUtils::format("%f", it->v.floatValue);
            case _TYPE_DOUBLE:
//                return std::to_string(it->v.doubleValue);
                return cocos2d::StringUtils::format("%f", it->v.doubleValue);
            case _TYPE_BOOL:
                return it->v.boolValue?"true":"false";
            default:
                break;
        }
    }
    
    return EMPTY_STRING;
}

ContentValue::ContentValue()
{
}

ContentValue::~ContentValue()
{
    clear();
}

void ContentValue::clear()
{
    for (auto it = _data.begin(); it != _data.end(); ++it) {
        releaseMemValue(it);
    }
    _data.clear();
}


void ContentValue::insertItem(const std::string& key, _Item& item)
{
    item.key = key;
    _data.push_back(item);
}

ContentValue::_Item* ContentValue::findItem(const std::string& key, _T type)
{
    for (auto& item : _data) {
        if (item.key == key && item.t == type) {
            return &item;
        }
    }
    return nullptr;
}

void ContentValue::releaseMemValue(const std::list<_Item>::iterator& it)
{
    if (it->t == _TYPE_STRING) {
        delete it->v.stringValue;
    }
}


