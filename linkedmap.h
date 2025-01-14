#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "fieldmap_alloc.h"

template <class K,class V>
class LinkedMap {
private:
    using MapAllocatorType = FieldMapAllocator<std::pair<const K, int>>;
    using VecAllocatorType = FieldMapAllocator<V>;

    FieldMapBuffer& buffer;
    std::unordered_map<K,int,std::hash<uint32_t>,std::equal_to<uint32_t>,MapAllocatorType> map;
    std::vector<V,VecAllocatorType> list;
public:
    LinkedMap(FieldMapBuffer& buffer): buffer(buffer), map(MapAllocatorType(buffer)), list(VecAllocatorType(buffer)) {}
    LinkedMap& operator=(const LinkedMap& other) {
        if (this == &other) return *this;
        buffer = other.buffer;
        map = other.map;
        list = other.list;
        return *this;
    }
    V& operator[](const K key) {
        return get(key);
    }
    void put(const K& key,V value) {
        if(map.contains(key)) {
            int index = map[key];
            list[index] = value;
        } else {
            int size = list.size();
            list.push_back(value);
            map[key] = size;
        }
    }
    V& get(const K& key) {
        if(!map.contains(key)) {
            int size = list.size();
            list.push_back(V());
            map[key] = size;
        }
        int index = map.at(key);
        return list.at(index);
    }
    bool contains(const K& key) const {
        return map.contains(key);
    }
    V* find(const K& key) const {
        if(!map.contains(key)) return nullptr;
        int index = map.at(key);
        return &(const_cast<V&>(list.at(index)));
    }
};