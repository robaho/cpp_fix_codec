#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <array>

#include "fix.h"
#include "fix_parser.h"
#include "samples.h"

static const int N_ITRS = 10000000;

void accessFields(const char *text,GroupDefs &defs) {
    FixMessage msg;
    FixMessage::parse(text,msg,defs);

    auto start = std::chrono::system_clock::now();

    Fixed total(0);

    for(int i=0;i<N_ITRS;i++) {
        auto sec = msg.getString(tagValue(Tags::SYMBOL));
        auto price = msg.getFixed(tagValue(Tags::PRICE));
        auto qty = msg.getFixed(tagValue(Tags::ORDER_QTY));
        if(sec=="AMZN") {
            total = total + price + qty;
        }
    }
    int fieldCount = N_ITRS*3;

    if(total<0) {
        std::cout << "invalid total\n";
    }

    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

    std::cout << "accessed "<<fieldCount<<" fields, usec per op " << (duration.count()/(double)(fieldCount)) << ", fields per sec " << (int)(((fieldCount)/(duration.count()/1000000.0))) << "\n";
}

int main(int argc,char **argv) {
    char buffer[4096];
    GroupDefs defs;
    std::cout << "New Order Single Limit: ";
    decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE_LIMIT_ORDER,buffer);
    accessFields(buffer,defs);
}
