//
//  JsonWriter.h
//  SMFrameWork
//
//  Created by SteveMac on 2018. 7. 6..
//

#ifndef JsonWriter_h
#define JsonWriter_h

#include <json/rapidjson.h>
#include <json/document.h>
#include <json/stringbuffer.h>
#include <json/writer.h>
#include <vector>
#include <stack>
#include <unordered_map>
#include <cocos2d.h>


class JsonWriter
{
public:
    static JsonWriter * getInstance() {
        static JsonWriter * instance = nullptr;
        if (instance==nullptr) {
            instance = new JsonWriter();
            instance->initJson();
        }
        
        return instance;
    };

    JsonWriter(){};

    
    void initJson() {
        _doc.SetObject();
        _allocator = &_doc.GetAllocator();
        _value = &_doc;
    }
    
    rapidjson::Value* getCurrentValue() {return _value;}
    
    void clear() {
        _doc.RemoveAllMembers();
    }
    
    void addInt(std::string key, int value)
    {
        rapidjson::Value addValue;
        addValue.SetInt(value);
        addJsonValue(key, addValue);
    }
    void addUint(std::string key, uint32_t value)
    {
        rapidjson::Value addValue;
        addValue.SetUint(value);
        addJsonValue(key, addValue);
    }
    void addInt64(std::string key, int64_t value)
    {
        rapidjson::Value addValue;
        addValue.SetInt64(value);
        addJsonValue(key, addValue);
    }
    void addUint64(std::string key, uint64_t value)
    {
        rapidjson::Value addValue;
        addValue.SetUint64(value);
        addJsonValue(key, addValue);
    }
    void addDouble(std::string key, double value)
    {
        rapidjson::Value addValue;
        addValue.SetDouble(value);
        addJsonValue(key, addValue);
    }
    void addFloat(std::string key, float value)
    {
        rapidjson::Value addValue;
        addValue.SetFloat(value);
        addJsonValue(key, addValue);
    }
    void addString(std::string key, std::string value)
    {
        rapidjson::Value addValue(value.c_str(), *_allocator);
        addJsonValue(key, addValue);
    }
    
    void addJsonValue(std::string key, rapidjson::Value& addValue)
    {
        rapidjson::Value addKey(key.c_str(), *_allocator);
        _doc.AddMember(addKey, addValue, *_allocator);
        _value = &_doc;
    }
    
    void addJsonValue(rapidjson::Value * value, std::string key, rapidjson::Value& addValue)
    {
        rapidjson::Value addKey(key.c_str(), *_allocator);
        value->AddMember(addKey, addValue, *_allocator);
    }
    
    void pushJsonValueToArray(rapidjson::Value * arrayValue, rapidjson::Value& addValue)
    {
        arrayValue->PushBack(addValue, *_allocator);
    }
    
    void getJsonValueFromStringMap(rapidjson::Value * mapValue, std::unordered_map<std::string, std::string> map)
    {
        for (auto kv : map) {
            std::string keyString = kv.first;
            std::string valueString = kv.second;
            rapidjson::Value addKey(keyString.c_str(), *_allocator);
            rapidjson::Value addValue(valueString.c_str(), *_allocator);
            
            mapValue->AddMember(addKey, addValue, *_allocator);
        }
    }
    
    void getJsonArrayFromKeyValueMapArray(rapidjson::Value* arrayValue, std::vector<std::unordered_map<std::string, std::string>> array)
    {
//        rapidjson::Value arrayValue(rapidjson::kArrayType);
        std::vector<std::unordered_map<std::string, std::string>>::iterator iter = array.begin();
        std::vector<std::unordered_map<std::string, std::string>>::iterator eiter = array.end();
        for (; iter != eiter; ++iter) {
            std::unordered_map<std::string, std::string> map = *iter;
            
            rapidjson::Value mapValue;
            mapValue.SetObject();
            
            
            for (auto kv : map) {
                std::string keyString = kv.first;
                std::string valueString = kv.second;
                rapidjson::Value addKey(keyString.c_str(), *_allocator);
                rapidjson::Value addValue(valueString.c_str(), *_allocator);
                
                mapValue.AddMember(addKey, addValue, *_allocator);
            }
            
            arrayValue->PushBack(mapValue, *_allocator);
        }
    }
    
    std::string arrayToJsonString(std::vector<std::unordered_map<std::string, std::string>> array)
    {
        rapidjson::Value arrayValue(rapidjson::kArrayType);
        
        std::vector<std::unordered_map<std::string, std::string>>::iterator iter = array.begin();
        std::vector<std::unordered_map<std::string, std::string>>::iterator eiter = array.end();
        for (; iter != eiter; ++iter) {
            std::unordered_map<std::string, std::string> map = *iter;
            
            rapidjson::Value mapValue;
            mapValue.SetObject();
            std::string keyString;
            std::string valueString;
            for (auto kv : map) {
                keyString = kv.first;
                valueString = kv.second;
                rapidjson::Value addKey(keyString.c_str(), *_allocator);
                rapidjson::Value addValue(valueString.c_str(), *_allocator);
                
                mapValue.AddMember(addKey, addValue, *_allocator);
            }
            
            arrayValue.PushBack(mapValue, *_allocator);
        }

        return getJsonStringFromValue(arrayValue);
    }
    
    void addArrayFromKeyValueMapArray(std::string key, std::vector<std::unordered_map<std::string, std::string>> array)
    {
        rapidjson::Value arrayValue(rapidjson::kArrayType);
        
        std::vector<std::unordered_map<std::string, std::string>>::iterator iter = array.begin();
        std::vector<std::unordered_map<std::string, std::string>>::iterator eiter = array.end();
        for (; iter != eiter; ++iter) {
            std::unordered_map<std::string, std::string> map = *iter;
            
            rapidjson::Value mapValue;
            mapValue.SetObject();
            std::string keyString;
            std::string valueString;
            for (auto kv : map) {
                keyString = kv.first;
                valueString = kv.second;
                rapidjson::Value addKey(keyString.c_str(), *_allocator);
                rapidjson::Value addValue(valueString.c_str(), *_allocator);

                mapValue.AddMember(addKey, addValue, *_allocator);
            }
            
            arrayValue.PushBack(mapValue, *_allocator);
        }
        
        rapidjson::Value addKey(key.c_str(), *_allocator);
        _doc.AddMember(addKey, arrayValue, *_allocator);
        _value = &_doc;
    }
    
    void addArray(std::string key, std::vector<rapidjson::Value*> values)
    {
        rapidjson::Value addKey(key.c_str(), *_allocator);
        
        rapidjson::Value arrayValue(rapidjson::kArrayType);
        
        std::vector<rapidjson::Value*>::iterator iter = values.begin();
        std::vector<rapidjson::Value*>::iterator eiter = values.end();
        for (; iter != eiter; ++iter) {
            rapidjson::Value * value = *iter;
            arrayValue.PushBack(*value, *_allocator);
        }
        _doc.AddMember(addKey, arrayValue, *_allocator);
        _value = &_doc;
    }
    
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
    std::string getMemberJsonString(std::string member)
    {
        auto value = getMemberValue(&_doc, member.c_str());
        if (value) {
            
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            value->Accept(writer);
            
            return buffer.GetString();
        }
        
        return "";
    }
    
    std::string getJsonStringFromValue(rapidjson::Value& value)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        
        std::string jsonString(buffer.GetString(), buffer.GetSize());
        
        return jsonString;
    }
    
    std::string getCurrentJsonString()
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
       _doc.Accept(writer);
        
        std::string jsonString(buffer.GetString(), buffer.GetSize());
        
        return jsonString;
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

    
    virtual ~JsonWriter(){};
    
private:
    rapidjson::Document _doc;
    std::stack<rapidjson::Value*> _stack;
    rapidjson::Document::AllocatorType * _allocator;
    rapidjson::Value* _value;
};



#endif /* JsonWriter_h */
