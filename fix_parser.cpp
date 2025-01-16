#include <iostream>
#include <sstream>
#include <stdexcept>
#include <strstream>

#include "fix.h"
#include "fix_parser.h"

void FixMessage::parse(const char* in, FixMessage &msg, const GroupDefs &defs) {
    std::istrstream is(in);
    return parse(is,msg,defs);
}
void FixMessage::parse(std::istream& in, FixMessage &msg, const GroupDefs &defs) {
    msg.clear();

    char* buffer = msg.buffer = (char*)msg.buf.allocate(msg.buf.length/4);
    char* start = buffer;
    char *end = buffer;
    char* cp = buffer;

    const std::vector<GroupDef>* msgGroups = nullptr;
    std::vector<FieldMap*> stack;

    FieldMap *map = msg.map;

    // read the mandatory portion, at a minimum must be 8=FIX.4.4^A9=nn^A49=xx^A56=xx^A10=x^A = 37 characters
    if(!in.read(end,37)) return;
    end+=37;

    while(true) {
        if(cp==end) {
            if(!in.get(*end)) return;
            end++;
        }
        if(*cp!='=') {
            cp++; continue;
        }
        int tag = parseInt(start,cp);
        cp++;
        start=cp;
        if(cp==end) {
            if(!in.get(*end)) return;
            end++;
        }
        while(*cp!='\x01') {
            cp++;
            if(cp==end) {
                if(!in.get(*end)) return;
                end++;
            }
        }
        int offset = start-buffer;
        int length = cp-start;

        if(tag==value(Tags::CHECK_SUM)) {
            if(end-cp!=1) {
                throw std::runtime_error("invalid state, buffered data not read");
            }
            return;
        }

        map->set(tag,Field(tag,offset,length));

        if(tag==value(Tags::BODY_LENGTH)) {
            // the body length does not include the 10=x^A portion which must be included
            auto length = msg.getInt(tag);
            int toread = (length+5) - (end-cp); // remove any data already buffered
            if(!in.read(end, toread)) return;
            end+=toread;
        }
        if(tag==value(Tags::MSG_TYPE)) {
            auto msgType = msg.getString(tag);
            msgGroups = defs.defs(msgType);
        }

        if(msgGroups==nullptr) goto next;

        for(auto itr = msgGroups->begin(); itr!=msgGroups->end(); itr++) {
            if(itr->groupCountTag == tag) {
                stack.push_back(map);
                auto& tmp = map->addGroup(tag,msg.getInt(tag));
                map = &tmp;
            }
            if(itr->groupEndTag == tag) {
                if(stack.empty()) continue;
                auto parent = stack.back();
                if(parent->getGroupCount(itr->groupCountTag) == parent->getGroupSize(itr->groupCountTag)) {
                    // received all expected groups, so go back a level
                    stack.pop_back();
                    map = parent;
                } else {
                    // more groups expected
                    map = &parent->addGroup(itr->groupCountTag);
                }
            }
        }
next:        
        cp++;
        start = cp;
    }
}
