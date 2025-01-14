#pragma once

#include <cstdint>
#include <map>
#include <iostream>
#include <stdexcept>

class FieldMapBuffer {
    char buffer[4096];
    char *offset = buffer;
    int bytesAllocated = 0;
public:
    void *allocate(int n) {
        if(offset+n >= buffer+4096) {
            throw std::runtime_error("FieldMapBuffer exhausted");
        }
        auto addr = offset;
        offset += n;
        bytesAllocated += n;
        return addr; 
    }
    void deallocate(int n) {
        bytesAllocated -= n;
    }
    void reset() {
        offset = buffer;
        if(bytesAllocated>0) {
            std::cout << "FieldMapBuffer leak: " << bytesAllocated << "\n";
        }
    }
};

template <typename T>
class FieldMapAllocator {
    FieldMapBuffer& buffer;
public:
    FieldMapAllocator(FieldMapBuffer& _buffer) : buffer(_buffer){}
    FieldMapAllocator(const FieldMapAllocator<T>& alloc) : buffer(alloc.buffer){}
    operator FieldMapBuffer&() const {
        return buffer;
    }

    using value_type = T;

    T* allocate(std::size_t n) {
        return static_cast<T*>(buffer.allocate(n * sizeof(T))); 
    }
    void deallocate(T* p, std::size_t n) {
        buffer.deallocate(n * sizeof(T));
    }
};

struct Field {
    int tag;
    int offset;
    int length;
    bool isEmpty() {
        return tag==0;
    }
    constexpr Field() : tag(0),offset(0),length(0){}
    Field(int tag,int offset,int length) : tag(tag),offset(offset),length(length){}
    static const Field EMPTY;
};

struct Group;

typedef std::variant<Field,Group> FieldOrGroup;

struct FieldMap {
    FieldMapBuffer& buffer;
    using AllocatorType = FieldMapAllocator<std::pair<const uint32_t, FieldOrGroup>>;
    std::map<uint32_t, FieldOrGroup, std::less<uint32_t>, AllocatorType> map;
    FieldMap(FieldMapBuffer& buffer) : buffer(buffer), map(AllocatorType(buffer)){}
    void clear() { map.clear(); }
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

struct Group {
    int tag;
    int count;
    std::vector<FieldMap> groups;
    Group(int tag,int count) : tag(tag), count(count){
        groups.reserve(count);
    }
    constexpr Group() : tag(0), count(0){}
};
