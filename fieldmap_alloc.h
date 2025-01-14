#pragma once

#include <memory>
#include <iostream>

class FieldMapBuffer {
    char buffer[8192];
    char *offset = buffer;
    int bytesAllocated = 0;
public:
    void *allocate(int n) {
        if(offset+n >= buffer+sizeof(buffer)) {
            throw std::runtime_error("FieldMapBuffer exhausted");
        }
        auto addr = offset;
        offset += n;
        bytesAllocated += n;
        memset(addr,0,n);
        // std::cout << "allocated at " << static_cast<void *>(addr) << ", bytes " << n << "\n";
        return addr; 
    }
    void deallocate(int n) {
        bytesAllocated -= n;
    }
    void reset() {
        offset = buffer;
        // std::cout << "reset buffer \n";
        // if(bytesAllocated>0) {
        //     std::cout << "FieldMapBuffer leak: " << bytesAllocated << "\n";
        // }
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
