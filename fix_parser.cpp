#include <iostream>
#include <stdexcept>
#include <strstream>

#include "fix.h"
#include "fix_parser.h"

void FixMessage::parse(const char* in, FixMessage &msg, const GroupDefs &defs) {
    std::istrstream is(in);
    return parse(is,msg,defs);
}
void FixMessage::parse(std::istream& in, FixMessage &msg, const GroupDefs &defs) {
    msg.buf.reset();

    // msgBytes is filled in as the data is read from the stream
    char* msgBytes = (char*)msg.buf.allocate(msg.buf.length/4);
    msg.reset(msgBytes);

    char* start = msgBytes;
    char *end = msgBytes;
    char* cp = msgBytes;

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
        int offset = start-msgBytes;
        int length = cp-start;

        if(tag==tagValue(Tags::CHECK_SUM)) {
            if(end-cp!=1) {
                throw std::runtime_error("invalid state, buffered data not read");
            }
            return;
        }

        map->set(tag,Field(tag,offset,length));

        if(tag==tagValue(Tags::BODY_LENGTH)) {
            // the body length does not include the 10=NNN^A portion which must be included
            auto bodyLength = msg.getInt(tag);
            int toread = (bodyLength+7) - (end-cp); // remove any data already buffered
            if(!in.read(end, toread)) return;
            end+=toread;
        }
        if(tag==tagValue(Tags::MSG_TYPE)) {
            auto msgType = msg.getString(tag);
            msgGroups = defs.defs(msgType);
        }

        // skip end of field marker
        cp++;
        start = cp;

        if(msgGroups==nullptr) continue;

        for(auto itr = msgGroups->begin(); itr!=msgGroups->end(); itr++) {
            if(itr->groupCountTag == tag) {
                stack.push_back(map);
                auto& tmp = map->addGroup(tag,msg.getInt(tag));
                map = &tmp;
            }
            if(itr->groupEndTag == tag) {
                if(stack.empty()) continue;
                auto parent = stack.back();
                auto field = parent->get(itr->groupCountTag);
                if(field.isEmpty()) continue;
                if(parent->getInt(field) == field.groupCount()) {
                    // received all expected groups, so go back a level
                    stack.pop_back();
                    map = parent;
                } else {
                    // more groups expected
                    map = &parent->addGroup(itr->groupCountTag);
                }
            }
        }
    }
}
