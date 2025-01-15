#pragma once

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
    Field field;
    Group group;
    uint32_t tag;
    bool isGroup() {
        return group.tag==tag;
    }
    bool isField() {
        return field.tag==tag;
    }
};

class FieldList {
private:
    using VecAllocatorType = FieldMapAllocator<FieldOrGroup>;

    std::vector<FieldOrGroup,VecAllocatorType> list;
public:
    FieldList(FieldMapBuffer& buffer): list(VecAllocatorType(buffer)) {
        list.reserve(16);
    }
    FieldOrGroup& put(const FieldOrGroup& fg) {
        for(auto itr=list.begin();itr!=list.end();itr++) {
            if(itr->tag==fg.tag) {
                *itr=std::move(fg);
                return *itr;
            }
        }
        list.push_back(std::move(fg));
        return list.back();
    }

    FieldOrGroup& get(const uint32_t tag) {
        for(auto itr=list.begin();itr!=list.end();itr++) {
            if(itr->tag==tag) {
                return *itr;
            }
        }
        throw std::runtime_error("tag does not exist");
    }
    bool contains(const uint32_t tag) const {
        for(auto itr=list.begin();itr!=list.end();itr++) {
            if(itr->tag==tag) {
                return true;
            }
        }
        return false;
    }
    FieldOrGroup* find(const uint32_t tag) const {
        for(auto itr=list.begin();itr!=list.end();itr++) {
            if(itr->tag==tag) {
                return const_cast<FieldOrGroup*>(&(*itr));
            }
        }
        return nullptr;
    }
};

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
    void set(uint32_t tag, Field field);
    Field get(uint32_t tag) const;
    const FieldMap& getGroup(uint32_t tag,int index) const;
    // returns the number of groups expected 
    int getGroupCount(uint32_t tag) const;
    // returns the number of groups added 
    int getGroupSize(uint32_t tag) const;
};

