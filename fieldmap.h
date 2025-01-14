#pragma once

#include <cstdint>
#include <map>
#include <iostream>
#include <stdexcept>
#include "linkedmap.h"

struct Field {
    int tag;
    int offset;
    int length;
    bool isEmpty() {
        return tag==0;
    }
    constexpr Field() : tag(0),offset(0),length(0){}
    Field(int tag,int offset,int length) : tag(tag),offset(offset),length(length){}
    Field(const Field& other) {
        tag = other.tag;
        offset = other.offset;
        length = other.length;
    }
    static const Field EMPTY;
};

class FieldMap;

struct Group {
    int tag;
    int count;
    std::vector<FieldMap*> groups;
    Group(int tag,int count) : tag(tag), count(count){
        groups.reserve(count);
    }
    // ~Group();
    constexpr Group() : tag(0), count(0){}
};

typedef std::variant<Field,Group> FieldOrGroup;


class FieldMap {
friend struct Group;
    FieldMapBuffer& buffer;
    LinkedMap<uint32_t,FieldOrGroup> map;
public:
    FieldMap(FieldMapBuffer& buffer) : buffer(buffer), map(buffer){}
    FieldMap& operator=(const FieldMap& other) {
        map = other.map;
        buffer = other.buffer;
        return *this;
    }
    /**
     * @brief add a group to the FieldMap
     * 
     * @param tag 
     * @return a mutable FieldMap to which fields and groups can be added
     */
    FieldMap& addGroup(uint32_t tag);
    /**
     * @brief add a group to the FieldMap
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

