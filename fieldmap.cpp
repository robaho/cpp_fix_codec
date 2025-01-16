#include "fieldmap.h"
#include <stdexcept>
#include <variant>
#include <stdexcept>
#include <sstream>

const Field Field::EMPTY = Field();
const Group Group::EMPTY = Group();

FieldMap& FieldMap::addGroup(uint32_t tag) {
    return addGroup(tag,1);
}
FieldMap& FieldMap::addGroup(uint32_t tag,int count) {
    auto itr = map.find(tag);
    if(itr==nullptr || !itr->isGroup()) {
        FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        auto fm = new (obj) FieldMap(buffer);
        auto& grp = map.put(Group(tag,count,fm)).group();
        return *const_cast<FieldMap*>(grp.groups);
    }
    auto& grp = itr->group();
    FieldMap* tail = grp.groups;
    while(tail->next!=nullptr) tail = tail->next;
    FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    tail->next = new (obj) FieldMap(buffer);
    grp.size++;
    return *tail->next;
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
    return *itr->group().group(index);
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
    return itr->group().size;
}

FieldMap* Group::group(int n) const {
    auto ptr = groups;
    while(n>0) { n--; ptr = ptr->next; }
    return ptr;
}
