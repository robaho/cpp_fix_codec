#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <strstream>

#include "fix.h"
#include "fix_parser.h"
#include "samples.h"

static const int N_MSGS = 1000000;

void parseMessages(const char *text, GroupDefs &defs) {
  FixMessage msg;

  std::istrstream in(text);

  auto start = std::chrono::system_clock::now();
  for (int i = 0; i < N_MSGS; i++) {
    FixMessage::parse(in, msg, defs);
    in.seekg(0);
  }
  auto end = std::chrono::system_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "parsed " << N_MSGS << " messages, usec per order "
            << (duration.count() / (double)(N_MSGS)) << ", orders per sec "
            << (int)(((N_MSGS) / (duration.count() / 1000000.0))) << "\n";
}

int main(int argc, char **argv) {
  char buffer[4096];
  GroupDefs defs;
  std::cout << "New Order Single: ";
  decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE, buffer);
  parseMessages(buffer, defs);

  defs.add(NEW_ORDER_SINGLE, {100, 54});

  std::cout << "New Order Single w/Groups: ";
  decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE_WITH_GROUP, buffer);
  parseMessages(buffer, defs);
}