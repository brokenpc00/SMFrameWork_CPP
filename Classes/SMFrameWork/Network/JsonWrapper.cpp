//
//  JsonWrapper.c
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
//

#include "JsonWrapper.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cocos2d.h>

// json parse result code

const int JsonParseResult::RET_SUCCESS = 0;
const int JsonParseResult::RET_RESPONSE_FAILED = -1;
const int JsonParseResult::RET_PARSE_ERROR = -2;
const int JsonParseResult::RET_RCODE_ERROR = -3;
const int JsonParseResult::RET_NO_RCODE = -4;
const int JsonParseResult::RET_NO_DATA = -5;
const int JsonParseResult::RET_NO_LIST = -6;
const int JsonParseResult::RET_ITEM_ERROR = -7;
const int JsonParseResult::RET_VALUE_IS_NULL = -8;
const int JsonParseResult::RET_VALUE_NOT_ARRAY = -9;
const int JsonParseResult::RET_INPUT_STRING_FAILED = -10;

//template <class T> JsonWrapper<T>::JsonWrapper()  : _error(0), _cause(0), _index(0), _size(0), _rcode(""), _rmsg(""), _rcodeInt(-1), _value(nullptr), _isHttpResponse(false)
//{
//    
//}

//template <class T> int JsonWrapper<T>::parseResponse(T& result, cocos2d::network::HttpResponse* response, ParseFunc parseFunc, const std::string& dataHeader, const std::string& itemHeader)
//{
//    if (response == nullptr || !response->isSucceed()) {
//        setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
//        return getError();
//    }
//    
//    std::vector<char>* data = response->getResponseData();
//    if (data == nullptr) {
//        setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
//        return getError();
//    }
//    
//    std::string json(data->begin(), data->end());
//    
//    _isHttpResponse = true;
//    return parseResponseString(result, json, parseFunc, dataHeader, itemHeader);
//};


//template <class T> int JsonWrapper<T>::parseResponseToList(std::vector<T>& result, cocos2d::network::HttpResponse* response, ParseFunc parseFunc, const std::string& dataHeader, const std::string& listHeader)
//{
//    if (response == nullptr || !response->isSucceed()) {
//        setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
//        return getError();
//    }
//    
//    std::vector<char>* data = response->getResponseData();
//    if (data == nullptr) {
//        setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
//        return getError();
//    }
//    
//    std::string jsonString(data->begin(), data->end());
//    
//    _isHttpResponse = true;
//    return parseResponseStringToList(result, jsonString, parseFunc, dataHeader, listHeader);
//}


//template <class T> int JsonWrapper<T>::parseResponseString(T& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader, const std::string& itemHeader)
//{
//    if (jsonString.empty()) {
//        setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
//        return getError();
//    }
//    
//    _isHttpResponse = true;
//    return processParseData(result, jsonString.c_str(), parseFunc, dataHeader, itemHeader);
//}


//template <class T> int JsonWrapper<T>::parseResponseStringToList(std::vector<T>& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader, const std::string& listHeader)
//{
//    if (jsonString.empty()) {
//        setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
//        return getError();
//    }
//
//    _isHttpResponse = true;
//    return processParseListData(result, jsonString.c_str(), parseFunc, dataHeader, listHeader);
//}


//template <class T> int JsonWrapper<T>::parseJsonString(T& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader, const std::string& itemHeader)
//{
//    if (jsonString.empty()) {
//        setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
//        return getError();
//    }
//    
//    _isHttpResponse = false;
//    return processParseData(result, jsonString.c_str(), parseFunc, dataHeader, itemHeader);
//}


//template <class T>int JsonWrapper<T>::parseJsonStringToList(std::vector<T>& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader, const std::string& listHeader)
//{
//    if (jsonString.empty()) {
//        setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
//        return getError();
//    }
//    
//    _isHttpResponse = false;
//    return processParseListData(result, jsonString.c_str(), parseFunc, dataHeader, listHeader);
//}


//template <class T> int JsonWrapper<T>::parseJsonArray(std::vector<T>& result, rapidjson::Value* jsonArrayValue, ParseFunc parseFunc)
//{
//    do {
//        if (jsonArrayValue == nullptr) {
//            setError(JsonParseResult::RET_VALUE_IS_NULL); // value is null
//            break;
//        }
//        
//        if (!jsonArrayValue->IsNull()) {
//            if (!jsonArrayValue->IsArray()) {
//                setError(JsonParseResult::RET_VALUE_NOT_ARRAY); // array아님
//                break;
//            }
//            
//            bool success = true;
//            
//            _size = jsonArrayValue->Size();
//            for (_index = 0; _index < _size; _index++) {
//                T item;
//                
//                clearValueStack();
//                _value = &(*jsonArrayValue)[_index];
//                _stack.push(_value);
//                
//                if (parseFunc(*this, item)) {
//                    result.push_back(item);
//                } else {
//                    setError(JsonParseResult::RET_ITEM_ERROR); // 아이템 파싱 에러
//                    success = false;
//                    break;
//                }
//                
//                _stack.pop();
//            }
//            if (!success) break;
//        } else {
//        }
//        
//        setError(JsonParseResult::RET_SUCCESS);
//    } while (false);
//    
//    return getError();
//}


//template <class T> int JsonWrapper<T>::parseJsonValue(T& result, rapidjson::Value* jsonValue, ParseFunc parseFunc)
//{
//    do {
//        if (jsonValue == nullptr) {
//            setError(JsonParseResult::RET_VALUE_IS_NULL); // value is null
//            break;
//        }
//        
//        if (!jsonValue->IsNull()) {
//            clearValueStack();
//            _value = jsonValue;
//            _stack.push(_value);
//            
//            if (!parseFunc(*this, result)) {
//                setError(JsonParseResult::RET_ITEM_ERROR); // 아이템 파싱 에러
//                break;
//            }
//            
//            _stack.pop();
//            _size = 1;
//        } else {
//        }
//        setError(JsonParseResult::RET_SUCCESS);
//    } while (false);
//    
//    return getError();
//}


//template <class T> rapidjson::Value* JsonWrapper<T>::getJsonArray(const std::string& member) {
//    auto value = getMemberValue(_value, member.c_str());
//    if (value && value->IsArray()) {
//        return value;
//    }
//
//    return nullptr;
//}


//template <class T> rapidjson::Value* JsonWrapper<T>::getJsonValue(const std::string& member)
//{
//    return getMemberValue(_value, member.c_str());
//}


//template <class T> bool JsonWrapper<T>::push(const std::string& member)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        _stack.push(value);
//        _value = value;
//        return true;
//    }
//    return false;
//}


//template <class T> void JsonWrapper<T>::pop()
//{
//    CCASSERT(_stack.size() > 0, "[JSON PARSER] stack is empty");
//    _stack.pop();
//    _value = _stack.top();
//}


//template <class T> bool JsonWrapper<T>::hasMember(const std::string& member)
//{
//    if (_value->IsObject() && _value->HasMember(member.c_str())) {
//        return true;
//    }
//    return false;
//}


//template <class T> std::string JsonWrapper<T>::getString(const std::string& member, const std::string& defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsString()) return value->GetString();
//    }
//    
//    return defaultValue;
//}


//template <class T> std::string JsonWrapper<T>::getJsonString(const std::string& member, const std::string& defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        
//        rapidjson::StringBuffer buffer;
//        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
//        value->Accept(writer);
//        
//        return buffer.GetString();
//    }
//    
//    return defaultValue;
//}


//template <class T> int JsonWrapper<T>::getInt(const std::string& member, const int defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsInt()) return value->GetInt();
//    }
//    
//    return defaultValue;
//}


//template <class T> unsigned JsonWrapper<T>::getUint(const std::string& member, const int defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsUint()) return value->GetUint();
//    }
//    
//    return defaultValue;
//}


//template <class T> int64_t JsonWrapper<T>::getInt64(const std::string& member, const int64_t defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsInt64()) return value->GetInt64();
//    }
//    
//    return defaultValue;
//}


//template <class T> uint64_t JsonWrapper<T>::getUint64(const std::string& member, const uint64_t defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsUint64()) return value->GetUint64();
//    }
//    
//    return defaultValue;
//}


//template <class T> double JsonWrapper<T>::getDouble(const std::string& member, const double defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsDouble()) return value->GetDouble();
//    }
//    
//    return defaultValue;
//}


//template <class T> bool JsonWrapper<T>::getBool(const std::string& member, const bool defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsBool()) return value->GetBool();
//    }
//    
//    return defaultValue;
//}


//template <class T> float JsonWrapper<T>::getFloatFromString(const std::string& member, const float defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsString()) return atof(value->GetString());
//    }
//    
//    return defaultValue;
//}
//

//template <class T> std::string JsonWrapper<T>::toString(const std::string& defaultValue)
//{
//    if (_value->IsString()) {
//        return _value->GetString();
//    }
//    
//    return defaultValue;
//}


//template <class T> int JsonWrapper<T>::toInt(const int defaultValue)
//{
//    if (_value->IsInt()) {
//        return _value->GetInt();
//    }
//    
//    return defaultValue;
//}


//template <class T> unsigned JsonWrapper<T>::toUint(const int defaultValue)
//{
//    if (_value->IsUint()) {
//        return _value->GetUint();
//    }
//    
//    return defaultValue;
//}


//template <class T> int64_t JsonWrapper<T>::toInt64(const int64_t defaultValue)
//{
//    if (_value->IsInt64()) {
//        return _value->GetInt64();
//    }
//    
//    return defaultValue;
//}


//template <class T> uint64_t JsonWrapper<T>::toUint64(const uint64_t defaultValue)
//{
//    if (_value->IsUint64()) {
//        return _value->GetUint64();
//    }
//    
//    return defaultValue;
//}


//template <class T> double JsonWrapper<T>::toDouble(const std::string& member, const double defaultValue)
//{
//    if (_value->IsDouble()) {
//        return _value->GetDouble();
//    }
//    
//    return defaultValue;
//}
//

//template <class T> bool JsonWrapper<T>::toBool(const std::string& member, const bool defaultValue)
//{
//    if (_value->IsBool()) {
//        return _value->GetBool();
//    }
//    
//    return defaultValue;
//}


//template <class T> float JsonWrapper<T>::toFloatFromString(const float defaultValue)
//{
//    if (_value->IsString()) {
//        return atof(_value->GetString());
//    }
//    
//    return defaultValue;
//}

//template <class T> float JsonWrapper<T>::getFlexFloat(const std::string &member, const float defaultValue)
//{
//    auto value = getMemberValue(_value, member.c_str());
//    if (value) {
//        if (value->IsNumber()) {
//            if (value->IsDouble()) {
//                return (float)value->GetDouble();
//            } else if (value->IsInt()) {
//                return (float)value->GetInt();
//            } else if (value->IsInt64()) {
//                return (float)value->GetInt64();
//            } else if (value->IsUint()) {
//                return (float)value->GetUint();
//            } else if (value->IsUint64()) {
//                return (float)value->GetUint64();
//            }
//        } else {
//            if (value->IsString()) return atof(value->GetString());
//        }
//    }
//    
//    return defaultValue;
//}

//template <class T> int JsonWrapper<T>::processParseListData(std::vector<T>& result, const char* json, ParseFunc parseFunc, const std::string& dataHeader, const std::string& listHeader)
//{
//    do {
//        rapidjson::Document document;
//        if (document.Parse<0>(json).HasParseError()) {
//            setError(JsonParseResult::RET_PARSE_ERROR); // parse error
//            break;
//        }
//        
//        if (_isHttpResponse) {
//            if (!document.HasMember("rcode") || document["rcode"].IsNull()) {
//                if (document.HasMember("error")) {
//                    std::string errorString = document["error" ].GetString();
//                    //                        if (errorString=="invalid_token") {
//                    //                            setError(JsonParseResult::RET_INVALID_ACCESS_TOKEN); // no rcode
//                    //                            break;
//                    //                        }
//                }
//                setError(JsonParseResult::RET_NO_RCODE); // no rcode
//                break;
//            }
//            
//            _rcode = document["rcode"].GetString();
//            _rcodeInt = decodeRCode(_rcode);
//            
//            if (_rcodeInt != 0) {
//                setError(JsonParseResult::RET_RCODE_ERROR); // rcode error
//                if (document.HasMember("rmsg") && !document["rmsg"].IsNull() && document["rmsg"].IsString()) {
//                    _rmsg = document["rmsg"].GetString();
//                }
//                break;
//            }
//        }
//        
//        rapidjson::Value* data = nullptr;
//        if (dataHeader.empty()) {
//            data = &document;
//        } else {
//            data = getMemberValue(&document, dataHeader.c_str());
//            if (data == nullptr) {
//                setError(JsonParseResult::RET_NO_DATA); // no data
//                break;
//            }
//        }
//        
//        rapidjson::Value* list = nullptr;
//        if (listHeader.empty()) {
//            list = data;
//        } else {
//            list = getMemberValue(data, listHeader.c_str());
//            
//            if (list == nullptr) {
//                setError(JsonParseResult::RET_NO_LIST); // no list
//                break;
//            }
//        }
//        parseJsonArray(result, list, parseFunc);
//    } while (false);
//    return getError();
//}


//template <class T> int JsonWrapper<T>::processParseData(T& result, const char* json, ParseFunc parseFunc, const std::string& dataHeader, const std::string& itemHeader)
//{
//    do {
//        rapidjson::Document document;
//        if (document.Parse<0>(json).HasParseError()) {
//            setError(JsonParseResult::RET_PARSE_ERROR); // parse error
//            break;
//        }
//        
//        if (_isHttpResponse) {
//            if (!document.HasMember("rcode") || document["rcode"].IsNull()) {
//                setError(JsonParseResult::RET_NO_RCODE); // no rcode
//                break;
//            }
//            
//            _rcode = document["rcode"].GetString();
//            _rcodeInt = decodeRCode(_rcode);
//            
//            if (_rcodeInt != 0) {
//                setError(JsonParseResult::RET_RCODE_ERROR); // rcode error
//                break;
//            }
//        }
//        
//        rapidjson::Value* data = nullptr;
//        if (dataHeader.empty()) {
//            data = &document;
//        } else {
//            data = getMemberValue(&document, dataHeader.c_str());
//            
//            if (data == nullptr) {
//                setError(JsonParseResult::RET_NO_DATA); // no data
//                break;
//            }
//            
//            if (!itemHeader.empty()) {
//                auto data2 = getMemberValue(data, itemHeader.c_str());
//                if (data2) {
//                    data = data2;
//                }
//            }
//        }
//        
//        parseJsonValue(result, data, parseFunc);
//        
//    } while (false);
//    
//    return getError();
//}


//template <class T> rapidjson::Value* JsonWrapper<T>::getMemberValue(rapidjson::Value* value, const char* m)
//{
//    if (value->IsObject() && value->HasMember(m)) {
//        rapidjson::Value* v = &(*value)[m];
//        if (!v->IsNull()) {
//            return v;
//        }
//        return v;
//    }
//    return nullptr;
//}


//template <class T> int JsonWrapper<T>::decodeRCode(const std::string& rcode)
//{
//    if (rcode.empty() || rcode.length() != 7 || rcode.find("RET") != 0)
//        return -1;
//    
//    std::string number(rcode.substr(3, 6));
//    if (!std::all_of(number.begin(), number.end(), ::isdigit))
//        return -1;
//    
//    return atoi(number.c_str());
//}

//template <class T> void JsonWrapper<T>::setError(int error)
//{
//    _error = error;
//    std::string msg = cocos2d::StringUtils::format("[JSON PARSER] error (%f)", _error);;
//    CCLOG("%s", msg.c_str());
//    //        CCASSERT(error == 0, msg.c_str());
//}

//template <class T> void JsonWrapper<T>::clearValueStack()
//{
//    while (!_stack.empty()) {
//        _stack.pop();
//    }
//}

