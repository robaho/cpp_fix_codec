#pragma once

#include <_types/_uint32_t.h>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "fix_alloc.h"
#include "fixed.h"

class FieldMap;

struct Field {
    uint32_t tag;
    int offset;
    int length;
    // groups is != nullptr when this is a Group tag
    FieldMap* groups = nullptr;
    bool isEmpty() const {
        return tag==0;
    }
    bool isGroup() const {
        return groups!=nullptr;
    }
    constexpr Field() : tag(0),offset(0),length(0){}
    Field(uint32_t tag,int offset,int length) : tag(tag),offset(offset),length(length){}
    Field(uint32_t tag,int offset,int length,FieldMap* group) : tag(tag),offset(offset),length(length),groups(group){}
    Field(const Field& other) {
        tag = other.tag;
        offset = other.offset;
        length = other.length;
    }
    Field(Field&& other) {
        tag = other.tag;
        offset = other.offset;
        length = other.length;
        groups = other.groups;
        other.groups = nullptr;
    }
    Field& operator=(const Field& other) {
        tag = other.tag;
        offset = other.offset;
        length = other.length;
        return *this;
    }
    const static Field EMPTY;
    FieldMap* group(int n) const;
    int groupCount() const;
};

class FieldMap;

// a linear search actually performs better than a binary search for both
// parsing and access since the locality is near perfect, and the message sizes
// are often small

#define LINEAR_SEARCH

#ifdef LINEAR_SEARCH
class FieldList {
    // using linear search
  private:
    using VecAllocatorType = FixAllocator<Field>;

    std::vector<Field, VecAllocatorType> list;

  public:
    FieldList(FixBuffer &buffer) : list(VecAllocatorType(buffer)) {
        list.reserve(16);
    }

    // add or replace a field/group in the list
    Field &put(const Field &fg) {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == fg.tag) {
              *itr = fg;
              return *itr;
            }
        }
        list.push_back(fg);
        return list.back();
    }
    // get a field/group in the list by tag, or throw exception if it does not exist
    Field &get(const uint32_t tag) {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == tag) {
              return *itr;
            }
        }
        throw std::runtime_error(std::string("tag does not exist ")+std::to_string(tag));
    }
    // determine if a field/group is in the list
    bool contains(const uint32_t tag) const {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == tag) {
              return true;
            }
        }
        return false;
    }
    // find a field/group is in the list
    Field *find(const uint32_t tag) const {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == tag) {
              return const_cast<Field *>(&(*itr));
            }
        }
        return nullptr;
    }
    std::vector<uint32_t> tags() const {
        std::vector<uint32_t> tags;
        tags.reserve(list.size());
        for(auto field : list) {
            tags.push_back(field.tag);
        }
        return tags;
    }
};

#else
class FieldList {
    // using sorted map
  private:
    using AllocatorType = FixAllocator<std::pair<const uint32_t,Field>>;
    std::map<uint32_t,Field,std::less<uint32_t>,AllocatorType> list;
  public:
    FieldList(FixBuffer &buffer) : list(AllocatorType(buffer)) {
    }

    // add or replace a field/group in the list
    Field &put(const Field &fg) {
        auto pair = list.insert_or_assign(fg.tag,fg);
        return pair.first->second;
    }

    // get a field/group in the list by tag, or throw exception if it does not exist
    Field &get(const uint32_t tag) {
        return list.at(tag);
    }
    // determine if a field/group is in the list
    bool contains(const uint32_t tag) const {
        return list.contains(tag);
    }
    // find a field/group is in the list
    Field *find(const uint32_t tag) const {
        auto itr = list.find(tag);
        if(itr==list.end()) return nullptr;
        return const_cast<Field*>(&itr->second);
    }
    std::vector<uint32_t> tags() const {
        std::vector<uint32_t> tags;
        tags.reserve(list.size());
        for(auto field : list) {
            tags.push_back(field.first);
        }
        return tags;
    }
};
#endif

static inline int parseInt(const char *start,const char* endExclusive) {
    int value=0;
    bool negate=false;
    if(*start=='-') { negate=true; start++; }
    while(start!=endExclusive) {
        value *= 10;
        value += *start - '0';
        start++;
    }
    return negate ? value * -1 : value;
}

static inline long parseLong(const char *start,const char* endExclusive) {
    long value=0;
    bool negate=false;
    if(*start=='-') { negate=true; start++; }
    while(start!=endExclusive) {
        value *= 10;
        value += *start - '0';
        start++;
    }
    return negate ? value * -1 : value;
}

class FieldMap {
friend struct Field;
friend class FieldAccessor;

    FixBuffer& buffer;
    // ptr back to raw message data
    const char* msgBytes;
    FieldList map;
    // next allows for efficient linking in a Group
    FieldMap *next = nullptr;
public:
    FieldMap(FixBuffer& buffer,const char* msgBytes) : buffer(buffer), msgBytes(msgBytes), map(buffer){}
    /**
     * @brief add a group to the FieldMap
     * 
     * @param tag 
     * @return a mutable FieldMap to which fields and groups can be added
     */
    FieldMap& addGroup(uint32_t tag);
    /**
     * @brief add a group to the FieldMap with count
     * 
     * @param tag 
     * @param count the expected number of groups
     * @return a mutable FieldMap to which fields and groups can be added
     */
    FieldMap& addGroup(uint32_t tag,int count);
    void set(uint32_t tag, const Field& field);
    const Field& get(uint32_t tag) const;
    const FieldMap& getGroup(uint32_t tag,int index) const;
    std::vector<uint32_t> tags() const {
        return map.tags();
    }
    int getInt(uint32_t tag) const {
        auto field = get(tag);
        if(field.isEmpty()) throw std::runtime_error(std::string("tag does not exist ")+std::to_string(tag));
        int value=parseInt(msgBytes+field.offset,msgBytes+field.offset+field.length);
        return value;
    }
    char getChar(uint32_t tag) const {
        auto field = get(tag);
        if(field.isEmpty()) throw std::runtime_error(std::string("tag does not exist ")+std::to_string(tag));
        return *(msgBytes+field.offset);
    }
    long getLong(uint32_t tag) const {
        auto field = get(tag);
        if(field.isEmpty()) throw std::runtime_error(std::string("tag does not exist ")+std::to_string(tag));
        int value=parseLong(msgBytes+field.offset,msgBytes+field.offset+field.length);
        return value;
    }
    int getInt(const Field& field) const {
        if(field.isEmpty()) throw std::runtime_error(std::string("tag does not exist ")+std::to_string(field.tag));
        int value=parseInt(msgBytes+field.offset,msgBytes+field.offset+field.length);
        return value;
    }
};

// all FieldAccessor methods are only valid for the lifetime of the source message
class FieldAccessor {
friend class FixMessage;
private:
    const FieldMap * map;
    const char *msgBytes;
    FieldAccessor(const FieldMap *map) : map(map), msgBytes(map->msgBytes){}
    FieldAccessor(){}
    void reset(const FieldMap* map) {
        this->map = map;
        this->msgBytes = map->msgBytes;
    }
public:
    inline int getInt(uint32_t tag) const {
        return map->getInt(tag);
    }
    inline char getChar(uint32_t tag) const {
        return map->getChar(tag);
    }
    inline long getLong(uint32_t tag) const {
        return map->getLong(tag);
    }
    inline std::string_view getString(uint32_t tag) const {
        auto field = map->get(tag);
        if(field.isEmpty()) return "";
        auto start = msgBytes+field.offset;
        return std::string_view(start,field.length);
    }
    template<int nPlaces=7> inline Fixed<nPlaces> getFixed(uint32_t tag) const {
        auto field = map->get(tag);
        if(field.isEmpty()) return Fixed<nPlaces>::NaN();
        auto start = msgBytes+field.offset;
        return Fixed(std::string_view(start,field.length));
    }
    std::vector<uint32_t> tags() const {
        return map->tags();
    }
    inline int getInt(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return -999999999;
        int value=parseInt(msgBytes+field.offset,msgBytes+field.offset+field.length);
        return value;
    }
    inline std::string_view getString(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return "";
        auto start = msgBytes+field.offset;
        return std::string_view(start,field.length);
    }
    inline const FieldAccessor getGroup(uint32_t groupTag,uint32_t index) {
        auto& grp = map->getGroup(groupTag,index);
        return FieldAccessor(&grp);
    }
    template<int nPlaces=7> inline Fixed<nPlaces> getFixed(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return Fixed<nPlaces>::NaN();
        auto start = msgBytes+field.offset;
        return Fixed(std::string_view(start,field.length));
    }
    std::vector<uint32_t> tags(uint32_t groupTag, int index) const {
        auto& grp = map->getGroup(groupTag,index);
        return grp.tags();
    }
};
