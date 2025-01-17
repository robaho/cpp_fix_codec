#include "fix_builder.h"
#include <stdexcept>
#include <unistd.h>

void FixBuilder::writeTo(int fd) {
    int toWrite = cp-message;
    int length = write(fd,message,toWrite);
    if(length!=toWrite) throw std::runtime_error("failed to write to fd");
    reset();
}