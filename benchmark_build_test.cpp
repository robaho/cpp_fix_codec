#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <strstream>
#include <thread>

#include "fix.h"
#include "fix_builder.h"
#include "samples.h"

static const int N_MSGS = 10000000;

void buildMessages(bool cacheTime) {
  FixBuilder builder;

  struct timeval time;
  gettimeofday(&time, nullptr);

 auto cachedTime = builder.cacheTime(time);

  auto start = std::chrono::system_clock::now();
  long length = 0;
  for (int i = 0; i < N_MSGS; i++) {
    if(!cacheTime) {
        cachedTime = builder.cacheTime(time);
    }
    // build new order single
    builder.addField(8, "FIX.4.2");
    builder.addField(9, "000");
    builder.addField(35, "D");
    builder.addField(34, 4);
    builder.addField(49, "ABC_DEFG01");
    builder.addField(52, cachedTime);
    builder.addField(56, "CCG");
    builder.addField(115, "XYZ");
    builder.addField(11, "NF 0542/03232009");
    builder.addField(54, 1);
    builder.addField(38, 100);
    builder.addField(55, "CVS");
    builder.addField(40, 1);
    builder.addField(59, 0);
    builder.addField(47, "A");
    builder.addField(60, cachedTime);
    builder.addField(21, 1);
    builder.addField(207, "N");
    length += builder.messageView().length();
    builder.reset();
  }
  auto end = std::chrono::system_clock::now();
  if (length <= 0) {
    std::cout << "invalid length\n";
  }
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  if(cacheTime) {
    std::cout << "with cached time, ";
  }

  std::cout << "built " << N_MSGS << " messages, usec per msg "
            << (duration.count() / (double)(N_MSGS)) << ", msgs per sec "
            << (int)(((N_MSGS) / (duration.count() / 1000000.0))) << "\n";
}

int main(int argc, char **argv) { 
    buildMessages(false);
    buildMessages(true);
}