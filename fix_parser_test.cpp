#define BOOST_TEST_MODULE fix
#include <boost/test/included/unit_test.hpp>

#include "fix.h"
#include "fixed.h"
#include "fix_parser.h"
#include "samples.h"

BOOST_AUTO_TEST_CASE( basic_parsing ) {
    GroupDefs defs;
    char buffer[4096];
    FixMessage msg;
    sampleToBuffer(SAMPLE_NEW_ORDER_SINGLE,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getString(49)=="ABC_DEFG01");
    sampleToBuffer(SAMPLE_CANCEL_REPLACE_QTY_CHANGE,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getFixed(44)=="25.47");
}

BOOST_AUTO_TEST_CASE( group_parsing ) {
    GroupDefs defs;
    defs.add(NEW_ORDER_SINGLE,{100,54});

    char buffer[4096];
    FixMessage msg;
    sampleToBuffer(SAMPLE_NEW_ORDER_SINGLE_WITH_GROUP,buffer);
    FixMessage::parse(buffer,msg,defs);
    BOOST_TEST(msg.getString(value(Tags::MSG_TYPE))==NEW_ORDER_SINGLE);
    BOOST_TEST(msg.getString(100,0,55)=="AAPL");
    BOOST_TEST(msg.getString(100,1,55)=="MSFT");
}
