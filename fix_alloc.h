#pragma once

#include <memory>
#include <iostream>

class FixBuffer {
    char* buffer;
    char *offset = buffer;
    int bytesAllocated = 0;
public:
    const int length;
    FixBuffer(int length) : buffer((char*)malloc(length)), length(length){}
    void *allocate(int n) {
        if(offset+n >= buffer+length) {
            throw std::runtime_error("FieldMapBuffer exhausted");
        }
        auto addr = offset;
        offset += n;
        bytesAllocated += n;
        // memset(addr,0,n);
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
class FixAllocator {
    FixBuffer& buffer;
public:
    FixAllocator(FixBuffer& _buffer) : buffer(_buffer){}
    FixAllocator(const FixAllocator<T>& alloc) : buffer(alloc.buffer){}
    operator FixBuffer&() const {
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
