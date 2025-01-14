#include "fix.h"
#include "fix_codec.h"

void FixMessage::parse(const char *buffer, FixMessage &msg, GroupDefs &defs) {
    msg.buffer = buffer;
    msg.clear();
    
    const char* start = buffer;
    const char* cp = buffer;

    std::vector<GroupDef> msgGroups;
    std::vector<FieldMap*> stack;

    FieldMap *map = msg.map;

    while(true) {
        if(*cp!='=') {
            cp++; continue;
        }
        int tag;
        std::from_chars(start,cp,tag);
        cp++;
        start=cp;
        while(*cp!='\x01') cp++;
        int offset = start-buffer;
        int length = cp-start;

        map->set(tag,Field(tag,offset,length));

        if(tag==value(Tags::CHECK_SUM)) return;
        if(tag==value(Tags::MSG_TYPE)) {
            auto msgType = msg.getString(tag);
            msgGroups = defs.defs(msgType);
        }

        for(auto itr = msgGroups.begin(); itr!=msgGroups.end(); itr++) {
            if(itr->groupCountTag == tag) {
                stack.push_back(map);
                auto& tmp = map->addGroup(tag,msg.getInt(tag));
                map = &tmp;
            }
            if(itr->groupEndTag == tag) {
                auto parent = stack.back();
                if(parent->getGroupCount(itr->groupCountTag) == parent->getGroupSize(itr->groupCountTag)) {
                    stack.pop_back();
                    map = parent;
                } else {
                    // more groups expected
                    map = &parent->addGroup(itr->groupCountTag);
                }
            }
        }
        
        cp++;
        start = cp;
    }
}
