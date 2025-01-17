#include "fieldmap.h"
#include <stdexcept>
#include <variant>
#include <stdexcept>
#include <sstream>

const Field Field::EMPTY = Field();

FieldMap& FieldMap::addGroup(uint32_t tag) {
    return addGroup(tag,1);
}
FieldMap& FieldMap::addGroup(uint32_t tag,int count) {
    auto itr = map.find(tag);
    if(itr==nullptr) throw std::runtime_error("group field tag does not exist");
    if(!itr->isGroup()) {
        FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
        auto fm = new (obj) FieldMap(buffer,msgBytes);
        itr->groups = fm;
        return *const_cast<FieldMap*>(itr->groups);
    }
    FieldMap* tail = itr->groups;
    while(tail->next!=nullptr) tail = tail->next;
    FieldMap *obj = (FieldMap*)buffer.allocate(sizeof(FieldMap));
    tail->next = new (obj) FieldMap(buffer,msgBytes);
    return *tail->next;
}
void FieldMap::set(uint32_t tag, const Field& field){
    map.put(field);
}
const Field& FieldMap::get(uint32_t tag) const {
    auto itr = map.find(tag);
    if(itr!=nullptr) {
        return *itr;
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
    return *itr->group(index);
}
FieldMap* Field::group(int n) const {
    auto ptr = groups;
    while(n-->0) { ptr = ptr->next; }
    return ptr;
}
int Field::groupCount() const {
    auto ptr = groups;
    int count = 0;
    while(ptr!=nullptr) { count++; ptr = ptr->next; }
    return count;
}
