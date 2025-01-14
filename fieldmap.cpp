#include "fieldmap.h"
#include <stdexcept>
#include <variant>
#include <stdexcept>
#include <sstream>

const Field Field::EMPTY = Field();

FieldMap& FieldMap::addGroup(uint32_t tag) {
    auto itr = map.find(tag);
    if(itr==map.end() || !std::holds_alternative<Group>(itr->second)) {
        auto& tmp = map[tag] = Group(tag,1);
        std::get<Group>(tmp).groups.push_back(FieldMap(buffer));
        auto& grp = std::get<Group>(tmp);
        return grp.groups.at(0);
    }
    auto& grp = std::get<Group>(itr->second);
    grp.groups.push_back(FieldMap(buffer));
    return grp.groups.back();
}
FieldMap& FieldMap::addGroup(uint32_t tag,int count) {
    auto itr = map.find(tag);
    if(itr==map.end() || !std::holds_alternative<Group>(itr->second)) {
        auto& tmp = map[tag] = Group(tag,count);
        std::get<Group>(tmp).groups.push_back(FieldMap(buffer));
        auto& grp = std::get<Group>(tmp);
        return grp.groups.at(0);
    }
    auto& grp = std::get<Group>(itr->second);
    grp.groups.push_back(FieldMap(buffer));
    return grp.groups.back();
}
void FieldMap::set(uint32_t tag, Field field){
    map[tag] = field;
}
Field FieldMap::get(uint32_t tag) const {
    auto tmp = map.at(tag);
    if(std::holds_alternative<Field>(tmp)) {
        return std::get<Field>(tmp);
    } else {
        return Field::EMPTY;
    }
}
const FieldMap& FieldMap::getGroup(uint32_t tag,int index) const {
    auto itr = map.find(tag);
    if(itr==map.end() || !std::holds_alternative<Group>(itr->second)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return std::get<Group>(itr->second).groups.at(index);
}
int FieldMap::getGroupCount(uint32_t tag) const {
    auto& tmp = map.at(tag);
    if(!std::holds_alternative<Group>(tmp)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return std::get<Group>(tmp).count;
}
int FieldMap::getGroupSize(uint32_t tag) const {
    auto& tmp = map.at(tag);
    if(!std::holds_alternative<Group>(tmp)) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return std::get<Group>(tmp).groups.size();
}
