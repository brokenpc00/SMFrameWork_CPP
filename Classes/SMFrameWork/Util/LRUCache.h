//
//  LRUCache.h
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 15..
//
// Least Recently Used Cache

#ifndef LRUCache_h
#define LRUCache_h

#define LOG_DOWNLOAD_STAT

#include <list>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <cocos2d.h>

template <class _K, class _V>
class LRUCache {
    typedef std::list<_K> __key_list;
    typedef typename __key_list::iterator __key_list_iterator;
    typedef std::unordered_map<_K, std::pair<_V, __key_list_iterator>> __data_map;
    typedef typename __data_map::iterator __data_map_iterator;
    
public:
    void removeAll() {
        std::lock_guard<std::mutex> guard(_mutex);
        
        trimToSize(-1);
        
        _data.clear();
        _keys.clear();
        
        _capacity = 0;
    }
    
    ssize_t size() const {
        return _keys.size();
    }
    
    ssize_t getCapacity() const {
        return _capacity;
    }
    
    ssize_t getMaxCapacity() const {
        return _maxCapacity;
    }
    
    void setMaxCapacity(const ssize_t maxCapacity) {
        CCASSERT(maxCapacity>0, "maxCapacity must bigger than zero");
        
        std::lock_guard<std::mutex> guard(_mutex);
        
        if (maxCapacity<_maxCapacity) {
            trimToSize(maxCapacity);
        }
        _maxCapacity = maxCapacity;
    }
    
    _V* get(const _K& key) {
        std::lock_guard<std::mutex> guard(_mutex);
        
        auto iter = _data.find(key);
        
        if (iter != _data.end()) {
            // find
            // 사용한 카운트 증가
            _stat_hit_count++;
            
            reorder(iter);
            
            return &iter->second.first;
        } else {
            // 못찾은 카운트 증가
            _stat_miss_count++;
        }
        return nullptr;
    }
    
    _V& insert(const _K& key, _V& value) {
        CCASSERT(_maxCapacity>0, "masCapacity must bigger than zero");
        
        std::lock_guard<std::mutex> guard(_mutex);
        
        // 추가한 카운트 증가
        _stat_insert_count++;
        
        auto iter = _data.find(key);
        
        if (iter!=_data.end()) {
            // exist data
            // 이미 있는거면 내용만 교체한다.
            auto oldValue = iter->second.first;
            
            iter->second.first = value;
            reorder(iter);
            
            _stat_remove_count++;
            
            entryRemoved(false, key, &oldValue, &value);
        } else {
            ssize_t size = safeSizeOf(key, value);
            
            if (size>0 && size<=_maxCapacity) {
                if (_capacity + size > _maxCapacity) {
                    trimToSize(_maxCapacity-size);
                }
                
                _capacity += size;
                
                auto end = _keys.insert(_keys.end(), key);
                auto newElement = _data.insert(std::make_pair(key, std::make_pair(value, end)));
                
                iter = newElement.first;
            }
        }
        
        return iter->second.first;
    }
    
    std::unique_ptr<_V> remove(const _K& key) {
        std::lock_guard<std::mutex> guard(_mutex);
        
        std::unique_ptr<_V> mapValue = nullptr;
        auto iter = _data.find(key);
        if (iter!=_data.end()) {
            mapValue = std::unique_ptr<_V> (new _V(iter->second.first));
            
            auto value = iter->second.first;
            
            _capacity -= safeSizeOf(key, value);
            
            _keys.erase(iter->second.second);
            _data.erase(iter);
            
            _stat_remove_count++;
            
            entryRemoved(true, key, &value, nullptr);
        }
        
        return mapValue;
    }


    LRUCache() :
    _maxCapacity(0),
    _capacity(0),
    _stat_hit_count(0),
    _stat_remove_count(0),
    _stat_miss_count(0),
    _stat_insert_count(0)
    {
    }
    
    LRUCache(ssize_t maxCapacity) {
        setMaxCapacity(maxCapacity);
    }
    
    virtual ~LRUCache() {
        removeAll();
    }
    
protected:
    virtual ssize_t sizeOf(const _K& key, _V& value) {
        return 1;
    }
    
    virtual void entryRemoved(bool bRemove, const _K& key, _V* oldValue, _V* newValue) {}
    
private:
    void trimToSize(const ssize_t capacity) {
        while (_capacity > capacity &&  _keys.size() >0) {
            auto iter = _data.find(_keys.front());
            _capacity -= safeSizeOf(iter->first, iter->second.first);
            
            auto key = iter->first;
            auto value = iter->second.first;
            
            _data.erase(iter);
            _keys.pop_front();
            
            _stat_remove_count++;
            
            entryRemoved(true, key, &value, nullptr);
        }
    }
    
    ssize_t safeSizeOf(const _K& key, _V& value) {
        ssize_t size = sizeOf(key, value);
        CCASSERT(size>=0, "size must not less than 0");
        // 1
        return size;
    }
    
    void reorder(const __data_map_iterator& iter) {
        _keys.splice(_keys.end(), _keys, iter->second.second);
    }
    
public:
    // utility
    std::string formatBytes(ssize_t bytes) {
        if (bytes==0) {
            return "0 Byte";
        }
        
        const char* size[] = {"Byes", "KB", "MB", "GB", "TB"};
        
        int i = std::floor(std::log(bytes)) / std::log(1024);
        
        if (i==0) {
            return cocos2d::StringUtils::format("%lu%s", bytes, size[i]);
        }
        
        float f = bytes / std::pow(1024, i);
        
        return cocos2d::StringUtils::format("%.1f%s", f, size[i]);
    }
    
    void printfDownloadStat(std::string tag) {
#ifdef LOG_DOWNLOAD_STAT
//        CCLOG("[%s] %s (%.2f%% used) - %lu entries", tag.c_str(), formatBytes(_capacity).c_str(), 100.0*_capacity/_maxCapacity, _keys.size());
#endif
    }
    
    void printStat() {
#ifdef LOG_DOWNLOAD_STAT
//        CCLOG("----------------- LRU CACHE STAT----------------");
//        CCLOG("  + capacity : %ld/%ld (%.2f)%%", _capacity, _maxCapacity, 100.0*_capacity/_maxCapacity);
//        CCLOG("  + key : %ld", _keys.size());
//        CCLOG("  + map : %ld", _data.size());
//        CCLOG("  - hit count : %d", _stat_hit_count);
//        CCLOG("  - miss count : %d", _stat_miss_count);
//        CCLOG("  - insert count : %d", _stat_insert_count);
//        CCLOG("  - evict count : %d", _stat_remove_count);
//        CCLOG("------------------------------------------------");
#endif
    }

    virtual void printEntry() {
#ifdef LOG_DOWNLOAD_STAT
//        CCLOG("------------------- ENTRY LIST -----------------");
        int order = 0;
        std::for_each(_keys.begin(), _keys.end(), [&](std::string& key){
//            CCLOG("%04d : %s", order, key.c_str());
            order++;
            
        });
//        CCLOG("------------------------------------------------");
#endif
    }
    
    
private:
    __key_list _keys;
    __data_map _data;
    
    ssize_t _maxCapacity;
    ssize_t _capacity;
    
    std::mutex _mutex;
    
    // count stat
    int _stat_insert_count;
    int _stat_remove_count;
    int _stat_hit_count;
    int _stat_miss_count;
};

#endif /* LRUCache_h */
