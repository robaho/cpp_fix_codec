#include <_types/_uint32_t.h>
#include <cstdint>
#include <variant>
#include <vector>
#include <map>
#include <string_view>
#include <charconv>
#include <istream>
#include "fixed.h"
#include "fieldmap.h"

const int MAP_SIZE=1000;

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


class FixMessage {
private:
    char *buffer;
    FixBuffer buf;
    FieldMap *map;
public:
    FixMessage() : buf(8192) {
        map = new ((FieldMap*)buf.allocate(sizeof(FieldMap))) FieldMap(buf);
    }
    FixMessage(int bufferSize) : buf(bufferSize) {
        map = new ((FieldMap*)buf.allocate(sizeof(FieldMap))) FieldMap(buf);
    }
    void clear() {
        buf.reset();
        map = new ((FieldMap*)buf.allocate(sizeof(FieldMap))) FieldMap(buf);
    }
    // parse next message from istream, any string_view from previous parse are invalid
    static void parse(std::istream &in,FixMessage &msg, const GroupDefs &defs);
    // convience method for testing to parse parse a message from a buffer, delegates to parse using istream
    static void parse(const char* in,FixMessage &msg, const GroupDefs &defs);
    inline int getInt(uint32_t tag) const {
        auto field = map->get(tag);
        if(field.isEmpty()) return -999999999;
        int value=parseInt(buffer+field.offset,buffer+field.offset+field.length);
        return value;
    }
    inline std::string_view getString(uint32_t tag) const {
        auto field = map->get(tag);
        if(field.isEmpty()) return "";
        auto start = buffer+field.offset;
        return std::string_view(start,field.length);
    }
    template<int nPlaces=7> inline Fixed<nPlaces> getFixed(uint32_t tag) const {
        auto field = map->get(tag);
        if(field.isEmpty()) return Fixed<nPlaces>::NaN();
        auto start = buffer+field.offset;
        return Fixed(std::string_view(start,field.length));
    }
    inline int getInt(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return -999999999;
        int value=parseInt(buffer+field.offset,buffer+field.offset+field.length);
        return value;
    }
    inline std::string_view getString(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return "";
        auto start = buffer+field.offset;
        return std::string_view(start,field.length);
    }
    template<int nPlaces=7> inline Fixed<nPlaces> getFixed(uint32_t groupTag, uint32_t index, uint32_t tag) const {
        auto& grp = map->getGroup(groupTag,index);
        auto field = grp.get(tag);
        if(field.isEmpty()) return Fixed<nPlaces>::NaN();
        auto start = buffer+field.offset;
        return Fixed(std::string_view(start,field.length));
    }
    std::vector<uint32_t> getTags() const {
        return map->tags();
    }
    std::vector<uint32_t> getTags(uint32_t groupTag, int index) const {
        auto& grp = map->getGroup(groupTag,index);
        return grp.tags();
    }
};