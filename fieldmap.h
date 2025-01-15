#pragma once

#include <_types/_uint32_t.h>
#include <cstdint>
#include <map>
#include <iostream>
#include <stdexcept>
#include <variant>
#include <vector>

#include "fieldmap_alloc.h"

struct Field {
    uint32_t tag;
    int offset;
    int length;
    bool isEmpty() {
        return tag==0;
    }
    constexpr Field() : tag(0),offset(0),length(0){}
    Field(uint32_t tag,int offset,int length) : tag(tag),offset(offset),length(length){}
    Field(const Field& other) {
        tag = other.tag;
        offset = other.offset;
        length = other.length;
    }
    static const Field EMPTY;
};

class FieldMap;

struct Group {
    uint32_t tag;
    int count;
    std::vector<FieldMap*> *groups;
    Group(uint32_t tag,int count) : tag(tag), count(count){
        groups = new std::vector<FieldMap*>();
        groups->reserve(count);
    }
    // ~Group();
    constexpr Group() : tag(0), count(0), groups(nullptr){}
    static const Group EMPTY;
};

struct FieldOrGroup {
private:
    bool _group = false;
    union FieldType {
        FieldType() {}
        Field field;
        Group group;
    } fg;
public:
    uint32_t tag;
    bool isGroup() {
        return _group;
    }
    bool isField() {
        return !_group;
    }
    const Group& group() {
        return fg.group;
    }
    const Field& field() {
        return fg.field;
    }
    FieldOrGroup(const Group& group) : _group(true), tag(group.tag) {
        new(&fg.group) Group(group);
    }
    FieldOrGroup(const Field& field) : _group(false), tag(field.tag) {
        new(&fg.group) Field(field);
    }
    FieldOrGroup(const FieldOrGroup& other) : _group(other._group), tag(other.tag) {
        if (_group) {
            new(&fg.group) Group(other.fg.group);
        } else {
            new(&fg.field) Field(other.fg.field);
        }
    }
    FieldOrGroup(FieldOrGroup&& other) noexcept : _group(other._group), tag(other.tag) {
        if (_group) {
            new(&fg.group) Group(std::move(other.fg.group));
        } else {
            new(&fg.field) Field(std::move(other.fg.field));
        }
        other._group = false;
        other.tag = 0;
    }
    FieldOrGroup& operator=(const FieldOrGroup& other) {
        if (this != &other) {
            if (_group) {
                fg.group.~Group();
            } else {
                fg.field.~Field();
            }
            _group = other._group;
            tag = other.tag;
            if (_group) {
                new(&fg.group) Group(other.fg.group);
            } else {
                new(&fg.field) Field(other.fg.field);
            }
        }
        return *this;
    }
};

// a linear search actually performs better than a binary search for both
// parsing and access since the locality is near perfect, and the message sizes
// are often small

#define LINEAR_SEARCH

#ifdef LINEAR_SEARCH
class FieldList {
    // using linear search
  private:
    using VecAllocatorType = FieldMapAllocator<FieldOrGroup>;

    std::vector<FieldOrGroup, VecAllocatorType> list;

  public:
    FieldList(FieldMapBuffer &buffer) : list(VecAllocatorType(buffer)) {
        list.reserve(16);
    }

    // add or replace a field/group in the list
    FieldOrGroup &put(const FieldOrGroup &fg) {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == fg.tag) {
              *itr = std::move(fg);
              return *itr;
            }
        }
        list.push_back(std::move(fg));
        return list.back();
    }

    // get a field/group in the list by tag, or throw exception if it does not exist
    FieldOrGroup &get(const uint32_t tag) {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == tag) {
              return *itr;
            }
        }
        throw std::runtime_error("tag does not exist");
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
    FieldOrGroup *find(const uint32_t tag) const {
        for (auto itr = list.begin(); itr != list.end(); itr++) {
            if (itr->tag == tag) {
              return const_cast<FieldOrGroup *>(&(*itr));
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
    using AllocatorType = FieldMapAllocator<std::pair<const uint32_t,FieldOrGroup>>;
    std::map<uint32_t,FieldOrGroup,std::less<uint32_t>,AllocatorType> list;
  public:
    FieldList(FieldMapBuffer &buffer) : list(AllocatorType(buffer)) {
    }

    // add or replace a field/group in the list
    FieldOrGroup &put(const FieldOrGroup &fg) {
        auto pair = list.insert_or_assign(fg.tag,fg);
        return pair.first->second;
    }

    // get a field/group in the list by tag, or throw exception if it does not exist
    FieldOrGroup &get(const uint32_t tag) {
        return list.at(tag);
    }
    // determine if a field/group is in the list
    bool contains(const uint32_t tag) const {
        return list.contains(tag);
    }
    // find a field/group is in the list
    FieldOrGroup *find(const uint32_t tag) const {
        auto itr = list.find(tag);
        if(itr==list.end()) return nullptr;
        return const_cast<FieldOrGroup*>(&itr->second);
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

class FieldMap {
friend struct Group;
    FieldMapBuffer& buffer;
    FieldList map;
public:
    FieldMap(FieldMapBuffer& buffer) : buffer(buffer), map(buffer){}
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
    // returns the number of groups expected 
    int getGroupCount(uint32_t tag) const;
    // returns the number of groups added 
    int getGroupSize(uint32_t tag) const;
    std::vector<uint32_t> tags() const {
        return map.tags();
    }
};

