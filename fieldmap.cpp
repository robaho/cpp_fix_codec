#include "fieldmap.h"
#include <stdexcept>
#include <variant>
#include <stdexcept>
#include <sstream>

const Field Field::EMPTY = Field();

// Group::~Group() {
//     for(auto itr=groups.begin();itr!=groups.end();itr++) {
//         delete *itr;
//         // auto grp = *itr;
//         // grp->buffer.deallocate(sizeof(FieldMap));
//     }
// }

FieldMap& FieldMap::addGroup(uint32_t tag) {
    auto itr = map.find(tag);
    if(itr==nullptr || !std::holds_alternative<Group>(*itr)) {
        auto& tmp = map[tag] = Group(tag,1);
        // FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        std::get<Group>(tmp).groups.push_back(new FieldMap(buffer));
        auto& grp = std::get<Group>(tmp);
        return *grp.groups.at(0);
    }
    auto& grp = std::get<Group>(*itr);
    // FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    grp.groups.push_back(new FieldMap(buffer));
    return *grp.groups.back();
}
FieldMap& FieldMap::addGroup(uint32_t tag,int count) {
    auto itr = map.find(tag);
    if(itr==nullptr || !std::holds_alternative<Group>(*itr)) {
        auto& tmp = map[tag] = Group(tag,count);
        // FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        std::get<Group>(tmp).groups.push_back(new FieldMap(buffer));
        auto& grp = std::get<Group>(tmp);
        return *grp.groups.at(0);
    }
    auto& grp = std::get<Group>(*itr);
    // FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    grp.groups.push_back(new FieldMap(buffer));
    return *grp.groups.back();
}
void FieldMap::set(uint32_t tag, Field field){
    map.put(tag,field);
}
Field FieldMap::get(uint32_t tag) const {
    auto tmp = map.find(tag);
    if(tmp!=nullptr && std::holds_alternative<Field>(*tmp)) {
        return std::get<Field>(*tmp);
    } else {
        return Field::EMPTY;
    }
}
const FieldMap& FieldMap::getGroup(uint32_t tag,int index) const {
    auto itr = map.find(tag);
    if(itr==nullptr || !std::holds_alternative<Group>(*itr)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return *std::get<Group>(*itr).groups.at(index);
}
int FieldMap::getGroupCount(uint32_t tag) const {
    auto tmp = map.find(tag);
    if(tmp==nullptr || !std::holds_alternative<Group>(*tmp)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return std::get<Group>(*tmp).count;
}
int FieldMap::getGroupSize(uint32_t tag) const {
    auto tmp = map.find(tag);
    if(tmp==nullptr || !std::holds_alternative<Group>(*tmp)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return std::get<Group>(*tmp).groups.size();
}
