//
//  JsonWrapper.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 12. 7..
//
// external/Json/RapidJson.h를 wrapping한 클래스 통신 때문에 만듬...

#ifndef JsonWrapper_h
#define JsonWrapper_h

#include <cocos2d.h>
#include <json/rapidjson.h>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <network/HttpResponse.h>
#include <vector>
#include <stack>
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <platform/android/jni/JniHelper.h>
#include <jni.h>
#include <android/log.h>
#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

class JsonParseResult
{
public:
    // Return Code
    static const int RET_SUCCESS;   // 성공 : 0
    static const int RET_RESPONSE_FAILED; // 응답 없음. : -1
    static const int RET_PARSE_ERROR; // 파싱에러 : -2
    static const int RET_RCODE_ERROR; // 이거는 통신 에러.. ret가 200이 아닐때 : -3
    static const int RET_NO_RCODE;   // rect code가 없음 : -4
    static const int RET_NO_DATA;   // data가 없음. : -5
    static const int RET_NO_LIST;   // list가 없음... : -6
    static const int RET_ITEM_ERROR; // item 항목을 파싱하다가 에러 : -7
    static const int RET_VALUE_IS_NULL;    // value가 Null일때 : -8
    static const int RET_VALUE_NOT_ARRAY; // data를 array로 변환 했을때 실패... array가 아니다. : -9
    static const int RET_INPUT_STRING_FAILED; // data가 string이 아니거나 비어있을때... :: -10
};

template <class T>
class JsonWrapper
{
public:
    
    JsonWrapper() : _error(0), _cause(0), _index(0), _size(0), _rcode(""), _rmsg(""), _rcodeInt(-1), _value(nullptr), _isHttpResponse(false)
    {
    
    }

//    virtual ~JsonWrapper();
    
    // 사용법
    /*
     JsonWrapper<데이터_클래스> parser;
     
     데이터_클래스 * result = new 데이터_클래스(); 
     또는 
     std::vector<데이터_클래스> * result = new std::vector<데이터_클래스>;
     
     int retCode = parser.parseResponse(*result, response, 파싱함수, 하위레벨1, 하위레벨2);  -> 하위레벨은 없으면 생략.. 기본적으로 "data"라는 이름으로 찾는다.
     통신에서는 retCode로 체크 (통신에서는 response에 retCode를 setting 해준다.
     */
    typedef std::function<bool(JsonWrapper<T>&, T&)> ParseFunc;
    
    
    // json method
    // HttpResponse에서 data 항목 하위를 파싱
    int parseResponse(T& result, cocos2d::network::HttpResponse* response, ParseFunc parseFunc, const std::string& dataHeader = "data", const std::string& itemHeader = "")
    {
        if (response == nullptr || !response->isSucceed()) {
            setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
            return getError();
        }
    
        std::vector<char>* data = response->getResponseData();
        if (data == nullptr) {
            setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
            return getError();
        }
        
        std::string json(data->begin(), data->end());
        
        _isHttpResponse = true;
        return parseResponseString(result, json, parseFunc, dataHeader, itemHeader);
    };

    
    // HttpResponse에서 data 하위 list를 파싱
    int parseResponseToList(std::vector<T>& result, cocos2d::network::HttpResponse* response, ParseFunc parseFunc, const std::string& dataHeader = "data", const std::string& listHeader = "list")
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
//        LOGD("[[[[[ parseResponseToList 1");
#endif

        if (response == nullptr || !response->isSucceed()) {
            setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
            return getError();
        }
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
//        LOGD("[[[[[ parseResponseToList 2");
#endif

        std::vector<char>* data = response->getResponseData();
        if (data == nullptr) {
            setError(JsonParseResult::RET_RESPONSE_FAILED); // response failed
            return getError();
        }
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
//        LOGD("[[[[[ parseResponseToList 3");
#endif

        std::string jsonString(data->begin(), data->end());
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
//        LOGD("[[[[[ parseResponseToList 4");
#endif

        _isHttpResponse = true;
        return parseResponseStringToList(result, jsonString, parseFunc, dataHeader, listHeader);
    }



    // 입력된 response string을 파싱
    int parseResponseString(T& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader="", const std::string& itemHeader="")
    {
        if (jsonString.empty()) {
            setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
            return getError();
        }

        _isHttpResponse = true;
        return processParseData(result, jsonString.c_str(), parseFunc, dataHeader, itemHeader);
    }
    // parseResponseString와 동일 (이름만 다름... reponse string이 아니라 json string을 파싱하는 느낌...)
    int parseJsonString(T& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader = "", const std::string& itemHeader = "")
    {
        if (jsonString.empty()) {
            setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
            return getError();
        }

        _isHttpResponse = false;
        return processParseData(result, jsonString.c_str(), parseFunc, dataHeader, itemHeader);
    }


    // 입력된 response string의 하위 리스트를 파싱
    int parseResponseStringToList(std::vector<T>& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader = "", const std::string& listHeader = "")
    {
        if (jsonString.empty()) {
            setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
            return getError();
        }

        _isHttpResponse = true;
        return processParseListData(result, jsonString.c_str(), parseFunc, dataHeader, listHeader);
    }

    //  parseResponseStringToList와 동일 (이름만 다름... reponse string이 아니라 json string을 파싱하는 느낌...)
    int parseJsonStringToList(std::vector<T>& result, const std::string& jsonString, ParseFunc parseFunc, const std::string& dataHeader = "", const std::string& listHeader = "")
    {
        if (jsonString.empty()) {
            setError(JsonParseResult::RET_INPUT_STRING_FAILED); // input string failed
            return getError();
        }

        _isHttpResponse = false;
        return processParseListData(result, jsonString.c_str(), parseFunc, dataHeader, listHeader);
    }



    // value array를 파싱
    int parseJsonArray(std::vector<T>& result, rapidjson::Value* jsonArrayValue, ParseFunc parseFunc)
    {
        do {
            if (jsonArrayValue == nullptr) {
                setError(JsonParseResult::RET_VALUE_IS_NULL); // value is null
                break;
            }

            if (!jsonArrayValue->IsNull()) {
                if (!jsonArrayValue->IsArray()) {
                    setError(JsonParseResult::RET_VALUE_NOT_ARRAY); // array아님
                    break;
                }

                bool success = true;

                _size = jsonArrayValue->Size();
                for (_index = 0; _index < _size; _index++) {
                    T item;

                    clearValueStack();
                    _value = &(*jsonArrayValue)[_index];
                    _stack.push(_value);

                    if (parseFunc(*this, item)) {
                        result.push_back(item);
                    } else {
                        setError(JsonParseResult::RET_ITEM_ERROR); // 아이템 파싱 에러
                        success = false;
                        break;
                    }

                    _stack.pop();
                }
                if (!success) break;
            } else {
            }

            setError(JsonParseResult::RET_SUCCESS);
        } while (false);

        return getError();
    }
    // value를 파싱
    int parseJsonValue(T& result, rapidjson::Value* jsonValue, ParseFunc parseFunc)
    {
        do {
            if (jsonValue == nullptr) {
                setError(JsonParseResult::RET_VALUE_IS_NULL); // value is null
                break;
            }

            if (!jsonValue->IsNull()) {
                clearValueStack();
                _value = jsonValue;
                _stack.push(_value);

                if (!parseFunc(*this, result)) {
                    setError(JsonParseResult::RET_ITEM_ERROR); // 아이템 파싱 에러
                    break;
                }

                _stack.pop();
                _size = 1;
            } else {
            }
            setError(JsonParseResult::RET_SUCCESS);
        } while (false);

        return getError();
    }


    // 리스트 사이즈
    int size() {return (int)_size;}
    // 리스트 인덱스
    int index() {return (int)_index;}
    // 파싱중인 value (For DEBUG)
    rapidjson::Value* getValue() {return _value;}
    // 에러코드
    int getError() {return _error;}

    // 파싱 결과 값들...
    std::string getRcode() {return _rcode;}
    int getRcodeInt() {return _rcodeInt;}
    std::string getRmsg() {return _rmsg;}
    void setCause(int cause) {_cause = cause;}
    // 파싱중 에러가 있을 때 cause를 세팅하고 false를 리턴하면 RET_ITEM_ERROR가 오는데 이때 cause를 읽어본다.
    int getCause() {return _cause;}

    rapidjson::Value* getJsonArray(const std::string& member)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value && value->IsArray()) {
            return value;
        }

        return nullptr;
    }
    rapidjson::Value* getJsonValue(const std::string& member)
    {
        return getMemberValue(_value, member.c_str());
    }

    bool push(const std::string& member)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            _stack.push(value);
            _value = value;
            return true;
        }
        return false;
    }

    void pop()
    {
        CCASSERT(_stack.size() > 0, "[JSON PARSER] stack is empty");
        _stack.pop();
        _value = _stack.top();
    }
    bool hasMember(const std::string& member)
    {
        if (_value->IsObject() && _value->HasMember(member.c_str())) {
            return true;
        }
        return false;
    }

    // get method
    std::string getString(const std::string& member, const std::string& defaultValue = "")
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsString()) {
                RAPIDJSON_ASSERT(value->IsString());
//                rapidjson::SizeType size = value->GetStringLength();
                return value->GetString();
            }
        }

        return defaultValue;
    }

    std::string getJsonString(const std::string& member, const std::string& defaultValue = "")
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
    
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            value->Accept(writer);
    
            return buffer.GetString();
        }
    
        return defaultValue;
    }

    int getInt(const std::string& member, const int defaultValue = 0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsInt()) return value->GetInt();
        }
        
        return defaultValue;
    }

    unsigned getUint(const std::string& member, const int defaultValue = 0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsUint()) return value->GetUint();
        }
        
        return defaultValue;
    }

    int64_t getInt64(const std::string& member, const int64_t defaultValue = 0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsInt64()) return value->GetInt64();
        }
        
        return defaultValue;
    }

    uint64_t getUint64(const std::string& member, const uint64_t defaultValue = 0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsUint64()) return value->GetUint64();
        }
        
        return defaultValue;
    }

    double getDouble(const std::string& member, const double defaultValue = 0.0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsDouble()) return value->GetDouble();
        }
    
        return defaultValue;
    }

    bool getBool(const std::string& member, const bool defaultValue = false)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsBool()) return value->GetBool();
        }
        
        return defaultValue;
    }
    
    float getFloatFromString(const std::string& member, const float defaultValue = 0.0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsString()) return atof(value->GetString());
        }
        
        return defaultValue;
    }
    

    
    // to method
    std::string toString(const std::string& defaultValue = "")
    {
        if (_value->IsString()) {
            return _value->GetString();
        }
        
        return defaultValue;
    }

    int toInt(const int defaultValue = 0)
    {
        if (_value->IsInt()) {
            return _value->GetInt();
        }
        
        return defaultValue;
    }

    unsigned toUint(const int defaultValue = 0)
    {
        if (_value->IsUint()) {
            return _value->GetUint();
        }
        
        return defaultValue;
    }

    int64_t toInt64(const int64_t defaultValue = 0)
    {
        if (_value->IsInt64()) {
            return _value->GetInt64();
        }
        
        return defaultValue;
    }

    uint64_t toUint64(const uint64_t defaultValue = 0)
    {
        if (_value->IsUint64()) {
            return _value->GetUint64();
        }
        
        return defaultValue;
    }

    double toDouble(const std::string& member, const double defaultValue = 0.0)
    {
        if (_value->IsDouble()) {
            return _value->GetDouble();
        }
        
        return defaultValue;
    }
    

    bool toBool(const std::string& member, const bool defaultValue = false)
    {
        if (_value->IsBool()) {
            return _value->GetBool();
        }
        
        return defaultValue;
    }

    float toFloatFromString(const float defaultValue = 0.0)
    {
        if (_value->IsString()) {
            return atof(_value->GetString());
        }
        
        return defaultValue;
    }

    float getFlexFloat(const std::string& member, const float defaultValue = 0.0)
    {
        auto value = getMemberValue(_value, member.c_str());
        if (value) {
            if (value->IsNumber()) {
                if (value->IsDouble()) {
                    return (float)value->GetDouble();
                } else if (value->IsInt()) {
                    return (float)value->GetInt();
                } else if (value->IsInt64()) {
                    return (float)value->GetInt64();
                } else if (value->IsUint()) {
                    return (float)value->GetUint();
                } else if (value->IsUint64()) {
                    return (float)value->GetUint64();
                }
            } else {
                if (value->IsString()) return atof(value->GetString());
            }
        }
        
        return defaultValue;
    }

    
private:
    int processParseListData(std::vector<T>& result, const char* json, ParseFunc parseFunc, const std::string& dataHeader, const std::string& listHeader)
    {
        do {
            rapidjson::Document document;
            if (document.Parse<0>(json).HasParseError()) {
                setError(JsonParseResult::RET_PARSE_ERROR); // parse error
                break;
            }
    
//            if (_isHttpResponse) {
//                if (!document.HasMember("rcode") || document["rcode"].IsNull()) {
//                    if (document.HasMember("error")) {
//                        std::string errorString = document["error" ].GetString();
//                        //                        if (errorString=="invalid_token") {
//                        //                            setError(JsonParseResult::RET_INVALID_ACCESS_TOKEN); // no rcode
//                        //                            break;
//                        //                        }
//                    }
//                    setError(JsonParseResult::RET_NO_RCODE); // no rcode
//                    break;
//                }
//
//                _rcode = document["rcode"].GetString();
//                _rcodeInt = decodeRCode(_rcode);
//
//                if (_rcodeInt != 0) {
//                    setError(JsonParseResult::RET_RCODE_ERROR); // rcode error
//                    if (document.HasMember("rmsg") && !document["rmsg"].IsNull() && document["rmsg"].IsString()) {
//                        _rmsg = document["rmsg"].GetString();
//                    }
//                    break;
//                }
//            }
    
            rapidjson::Value* data = nullptr;
            if (dataHeader.empty()) {
                data = &document;
            } else {
                data = getMemberValue(&document, dataHeader.c_str());
                if (data == nullptr) {
                    setError(JsonParseResult::RET_NO_DATA); // no data
                    break;
                }
            }
    
            rapidjson::Value* list = nullptr;
            if (listHeader.empty()) {
                list = data;
            } else {
                list = getMemberValue(data, listHeader.c_str());
    
                if (list == nullptr) {
                    setError(JsonParseResult::RET_NO_LIST); // no list
                    break;
                }
            }
            parseJsonArray(result, list, parseFunc);
        } while (false);
        return getError();
    }
    
    int processParseData(T& result, const char* json, ParseFunc parseFunc, const std::string& dataHeader, const std::string& itemHeader="")
    {
        do {
            rapidjson::Document document;
            if (document.Parse<0>(json).HasParseError()) {
                setError(JsonParseResult::RET_PARSE_ERROR); // parse error
                break;
            }
    
//            if (_isHttpResponse) {
//                if (!document.HasMember("rcode") || document["rcode"].IsNull()) {
//                    setError(JsonParseResult::RET_NO_RCODE); // no rcode
//                    break;
//                }
//
//                _rcode = document["rcode"].GetString();
//                _rcodeInt = decodeRCode(_rcode);
//
//                if (_rcodeInt != 0) {
//                    setError(JsonParseResult::RET_RCODE_ERROR); // rcode error
//                    break;
//                }
//            }
    
            rapidjson::Value* data = nullptr;
            if (dataHeader.empty()) {
                data = &document;
            } else {
                data = getMemberValue(&document, dataHeader.c_str());
    
                if (data == nullptr) {
                    setError(JsonParseResult::RET_NO_DATA); // no data
                    break;
                }
    
                if (!itemHeader.empty()) {
                    auto data2 = getMemberValue(data, itemHeader.c_str());
                    if (data2) {
                        data = data2;
                    }
                }
            }
            
            parseJsonValue(result, data, parseFunc);
            
        } while (false);
    
        return getError();
    }

    
    rapidjson::Value* getMemberValue(rapidjson::Value* value, const char* m)
    {
        if (value->IsObject() && value->HasMember(m)) {
            rapidjson::Value* v = &(*value)[m];
            if (!v->IsNull()) {
                return v;
            }
            return v;
        }
        return nullptr;
    }

    
//    int decodeRCode(const std::string& rcode)
//    {
//        if (rcode.empty() || rcode.length() != 7 || rcode.find("RET") != 0)
//            return -1;
//
//        std::string number(rcode.substr(3, 6));
//        if (!std::all_of(number.begin(), number.end(), ::isdigit))
//            return -1;
//
//        return atoi(number.c_str());
//    }
    
    void setError(int error)
    {
        _rcodeInt = error;
        
        _error = error;
        std::string msg = cocos2d::StringUtils::format("[JSON PARSER] error (%d)", _error);;
        if (_error!=0) {
        CCLOG("%s", msg.c_str());
        }
        //        CCASSERT(error == 0, msg.c_str());
    }
    
    void clearValueStack()
    {
        while (!_stack.empty()) {
            _stack.pop();
        }
    }

    
private:
    rapidjson::Value* _value;
    std::stack<rapidjson::Value*> _stack;
    rapidjson::SizeType _size;
    rapidjson::SizeType _index;
    std::string _rcode;
    std::string _rmsg;
    int _rcodeInt;
    int _error;
    int _cause;
    bool _isHttpResponse;
};


#endif /* JsonWrapper_h */
