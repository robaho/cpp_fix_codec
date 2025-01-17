#include <string>
#include <string_view>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include "fix.h"
#include "fix_alloc.h"
#include "fixed.h"

class FixBuilder {
private:
  const int maxMessageSize;
  FixBuffer buffer;
  // used for converting int to string
  char tmp[32];
  char tmpTime[22];
  char *message;
  char *cp;

  char *itoa(int64_t val) { return itoa(val, tmp, sizeof(tmp)); }

  // encode int to chars without ending null
  static char *itoa(int64_t val, char *tmp, int len) {
    bool neg = val < 0;

    if (neg) {
      val = val * -1;
    }

    int i = len - 1;

    for (; val >= 10;) {
      tmp[i] = val % 10 + '0';
      i--;
      val /= 10;
    }
    tmp[i] = val + '0';
    if (neg) {
      i--;
      tmp[i] = '-';
    }
    return tmp + i;
  }

  char *bodyLenStart;
  char *bodyLenEnd;
  int bodyLength = 0;

  void addCheckSum() {
    unsigned int checksum = 0;
    for (auto cpp = message; cpp != cp; cpp++) {
      checksum += (unsigned int)*cpp;
    }
    memcpy(cp, "10=000\x01", 7);
    itoa(checksum % 256, cp + 3, 3);
    cp += 7;
  }

  // encode time as FIX UTC with millis. dst must be at least 22 bytes.
  void encodeTime(timeval &time, char *dst) {
    struct tm gmTime;
    time_t secs = time.tv_sec;
    gmtime_r(&secs, &gmTime);
    memcpy(dst, "YYYY0000-00:00:00.000\x01", 22);
    itoa(gmTime.tm_year + 1900, dst, 4);
    dst += 4;
    itoa(gmTime.tm_mon + 1, dst, 2);
    dst += 2;
    itoa(gmTime.tm_mday, dst, 2);
    dst += 3;
    itoa(gmTime.tm_hour, dst, 2);
    dst += 3;
    itoa(gmTime.tm_min, dst, 2);
    dst += 3;
    itoa(gmTime.tm_sec, dst, 2);
    dst += 3;
    itoa(time.tv_usec / 1000, dst, 3);
    dst += 4;
  }

public:
  FixBuilder() : FixBuilder(8192) {}
  FixBuilder(int maxMessageSize)
      : maxMessageSize(maxMessageSize), buffer(maxMessageSize * 2) {
    reset();
  }
  inline void addField(uint32_t tag, const std::string_view &value) {
    bool isBodyLen = tag == tagValue(Tags::BODY_LENGTH);
    char *cpBegin = cp;
    auto start = itoa(tag);
    auto len = sizeof(tmp) - (start - tmp);
    memcpy(cp, start, len);
    cp += len;
    *cp++ = '=';
    if (isBodyLen)
      bodyLenStart = cp;
    memcpy(cp, value.data(), value.size());
    cp += value.size();
    if (isBodyLen)
      bodyLenEnd = cp;
    *cp++ = '\x01';
    if (!isBodyLen && bodyLenStart != nullptr) {
      bodyLength += (cp - cpBegin);
    }
  }
  inline void addField(uint32_t tag, const int value) {
    char *cpBegin = cp;
    auto start = itoa(tag);
    auto len = sizeof(tmp) - (start - tmp);
    memcpy(cp, start, len);
    cp += len;
    *cp++ = '=';
    start = itoa(value);
    len = sizeof(tmp) - (start - tmp);
    memcpy(cp, start, len);
    cp += len;
    *cp++ = '\x01';
    if (bodyLenStart != nullptr) {
      bodyLength += (cp - cpBegin);
    }
  }
  template <int nPlaces = 7>
  inline void addField(uint32_t tag, Fixed<nPlaces> fixed) {
    char *cpBegin = cp;
    auto start = itoa(tag);
    auto len = sizeof(tmp) - (start - tmp);
    memcpy(cp, start, len);
    cp += len;
    *cp++ = '=';
    fixed.str(cp);
    len = strlen(cp);
    cp += len;
    *cp++ = '\x01';
    if (bodyLenStart != nullptr) {
      bodyLength += (cp - cpBegin);
    }
  }
  // only a single cached time per message is supported
  inline const std::string_view cacheTime(timeval &time) {
    encodeTime(time, tmpTime);
    return std::string_view(tmpTime, 21);
  }
  inline void addTimeNow(uint32_t tag) {
    struct timeval time;
    gettimeofday(&time, nullptr);
    addTime(tag, time);
  }
  inline void addTime(uint32_t tag, timeval &time) {
    char *cpBegin = cp;
    auto start = itoa(tag);
    auto len = sizeof(tmp) - (start - tmp);
    memcpy(cp, start, len);
    cp += len;
    *cp++ = '=';
    encodeTime(time, cp);
    // include the SOH character
    cp += 22;
    if (bodyLenStart != nullptr) {
      bodyLength += (cp - cpBegin);
    }
  }
  void addBuilder(const FixBuilder &builder);
  // write message to fd and reset
  void writeTo(int fd);
  // write message to ostream and reset
  void writeTo(std::ostream out);
  // returns a view into the message only valid until reset() is called
  const std::string_view messageView() {
    if (bodyLenStart) {
      // write the body length into the already reserved space, must use
      // addTag(9,"00000"); to reserve
      itoa(bodyLength, bodyLenStart, bodyLenEnd - bodyLenStart);
    }
    addCheckSum();
    return std::string_view(message, cp);
  }
  // resets internals for next message
  void reset() {
    buffer.reset();
    message = (char *)buffer.allocate(maxMessageSize);
    cp = message;
    bodyLenStart = nullptr;
    bodyLenEnd = nullptr;
    bodyLength = 0;
  }
};