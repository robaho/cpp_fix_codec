#pragma once

#include <unistd.h>
#include <cstdint>
#include <variant>
#include <vector>
#include <map>
#include <string_view>
#include <charconv>
#include <istream>
#include "fix.h"
#include "fixed.h"
#include "fieldmap.h"

struct GroupDef {
    // the tag that holds the "start of group" count
    uint32_t groupCountTag;
    // the tag that ends the group
    uint32_t groupEndTag;
};

class GroupDefs {
    std::map<const std::string_view,std::vector<GroupDef>> _defs;
public:
    void add(const std::string_view msgType, const GroupDef def) {
        auto vec = {def};
        _defs[msgType] = vec;
    } 
    void add(const std::string_view msgType, std::vector<GroupDef> defs) {
        _defs[msgType] = defs;
    } 
    const std::vector<GroupDef>* defs(const std::string_view& msgType) const {
        auto itr = _defs.find(msgType);
        return itr==_defs.end() ? nullptr : &itr->second;
    }
};


class FixMessage : public FieldAccessor {
private:
    FixBuffer buf;
    const char *msgBytes;
    FieldMap *map;
    void reset(const char * msg) {
        msgBytes = msg;
        map = new ((FieldMap*)buf.allocate(sizeof(FieldMap))) FieldMap(buf,msgBytes);
        FieldAccessor::reset(map);
    }
public:
    FixMessage() : FixMessage(2048) {}
    FixMessage(int maxMessageSize) : buf(maxMessageSize*4), msgBytes((char*)buf.allocate(maxMessageSize)) {
        map = new ((FieldMap*)buf.allocate(sizeof(FieldMap))) FieldMap(buf,msgBytes);
        FieldAccessor::reset(map);
    }
    // parse next message from istream, any string_view from previous parse are invalid
    static void parse(std::istream &in,FixMessage &msg, const GroupDefs &defs);
    // convience method for testing to parse parse a message from a buffer, delegates to parse using istream
    static void parse(const char* in,FixMessage &msg, const GroupDefs &defs);
    // convenience method to get the message type
    std::string_view msgType() const { return getString(tagValue(Tags::MSG_TYPE)); };
};