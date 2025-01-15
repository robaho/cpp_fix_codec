#include "fieldmap.h"
#include <stdexcept>
#include <variant>
#include <stdexcept>
#include <sstream>

const Field Field::EMPTY = Field();
const Group Group::EMPTY = Group();

// Group::~Group() {
//     for(auto itr=groups.begin();itr!=groups.end();itr++) {
//         delete *itr;
//         // auto grp = *itr;
//         // grp->buffer.deallocate(sizeof(FieldMap));
//     }
// }

FieldMap& FieldMap::addGroup(uint32_t tag) {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        auto& grp = map.put(Group(tag, 1)).group();
        FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        grp.groups->push_back(new (obj) FieldMap(buffer));
        return *grp.groups->at(0);
    }
    auto& grp = itr->group();
    // FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    grp.groups->push_back(new FieldMap(buffer));
    return *grp.groups->back();
}
FieldMap& FieldMap::addGroup(uint32_t tag,int count) {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        auto& grp = map.put(Group(tag,count)).group();
        FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        grp.groups->push_back(new (obj) FieldMap(buffer));
        return *grp.groups->at(0);
    }
    auto& grp = itr->group();
    FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    grp.groups->push_back(new (obj) FieldMap(buffer));
    return *grp.groups->back();
}
void FieldMap::set(uint32_t tag, const Field& field){
    map.put(field);
}
const Field& FieldMap::get(uint32_t tag) const {
    auto itr = map.find(tag);
    if(itr!=nullptr && itr->isField()) {
        return itr->field();
    } else {
        return Field::EMPTY;
    }
}
const FieldMap& FieldMap::getGroup(uint32_t tag,int index) const {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return *itr->group().groups->at(index);
}
int FieldMap::getGroupCount(uint32_t tag) const {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return itr->group().count;
}
int FieldMap::getGroupSize(uint32_t tag) const {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        std::ostringstream oss;
        oss << "tag " << tag << " is not a group";
        throw std::runtime_error(oss.str());
    }
    return itr->group().groups->size();
}
