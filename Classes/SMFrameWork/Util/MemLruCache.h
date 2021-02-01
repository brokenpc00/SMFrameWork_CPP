//
//  MemoryCache.h
//  BaseProject
//
//  Created by KimSteve on 2016. 11. 15..
//
// Memory LRU Cache

#ifndef MemoryCache_h
#define MemoryCache_h

#include "LRUCache.h"
#include <memory>

#define INIT_ALLOC_SIZE (ssize_t)(50*1024)

class MemoryCacheEntry {
public:
    static std::shared_ptr<MemoryCacheEntry> createEntry(uint8_t* data=nullptr, ssize_t size =0) {
        CCASSERT((data==nullptr && size==0) || (data!=nullptr && size>0), "invalid data & size");
        
        std::shared_ptr<MemoryCacheEntry> entry = std::make_shared<MemoryCacheEntry>();
        entry->_data = data;
        entry->_size = size;
        entry->_capacity = size;
        
        return entry;
    }
    
    uint8_t* getData() {return _data;}
    ssize_t size() {return _size;}
    
    void appendData(uint8_t* data, ssize_t size) {
        if (data==nullptr || size==0) {
            CCLOG("Invalid data & size");
            return;
        }
        
        if (_data==nullptr) {
            ssize_t newCapacity = std::max(_size+size, INIT_ALLOC_SIZE);
            _capacity = newCapacity;
            _data = (uint8_t*)malloc(_capacity);
        } else {
            ssize_t newCapacity = _capacity;
            if (_size+size>newCapacity) {
                newCapacity = std::max(_size+size, (ssize_t)(_capacity*1.65));
            }
            
            if (newCapacity>_capacity) {
                _capacity = newCapacity;
                _data = (uint8_t*)realloc(_data, _capacity);
            }
        }
        
        CCASSERT(_data!=nullptr, "Memory allocatation error!");

        memcpy(_data+_size, data, size);
        _size += size;
    }
    
    void shrinkToFit() {
//        if(_v.capacity() != _v.size())
//        {
//            std::vector<T>(_v).swap(_v);
//        }
//        if(_index.capacity() != _index.size())
//        {
//            std::vector<size_type>(_index).swap(_index);
//        }
        if (_size!=_capacity && _data) {
            if (_size>0) {
                _data = (uint8_t*)realloc(_data, _size);
            } else {
                free(_data);
            }
            _capacity = _size;
        }
    }
    
public:
    MemoryCacheEntry() :
    _data(nullptr),
    _size(0),
    _capacity(0)
    {
    }
    
    ~MemoryCacheEntry() {
        if (_data) {
            free(_data);
            _data = nullptr;
        }
    }
    
private:
    uint8_t* _data;
    ssize_t _size;
    ssize_t _capacity;
};

class MemoryLRUCache : public LRUCache<std::string, std::shared_ptr<MemoryCacheEntry>> {
public:
    MemoryLRUCache(ssize_t maxCapacity){
        setMaxCapacity(maxCapacity);
    }

protected:
    virtual ssize_t sizeOf(const std::string& key, std::shared_ptr<MemoryCacheEntry>& entry) override {
        return entry->size();
    }

};

#endif /* MemoryCache_h */
