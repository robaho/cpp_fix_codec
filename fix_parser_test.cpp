#define BOOST_TEST_MODULE fix
#include <boost/test/included/unit_test.hpp>
#include <sstream>

#include "fix.h"
#include "fix_parser.h"
#include "samples.h"

BOOST_AUTO_TEST_CASE( basic_parsing ) {
    GroupDefs defs;
    char buffer[4096];
    FixMessage msg;
    decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getString(49)=="ABC_DEFG01");
    decodeFixToBuffer(SAMPLE_CANCEL_REPLACE_QTY_CHANGE,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getFixed(44)=="25.47");
    decodeFixToBuffer(SAMPLE_LIMIT_ORDER,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getFixed(Tag::PRICE)=="0.01");
    BOOST_TEST(msg.getFixed(Tag::ORDER_QTY)==170);
    BOOST_TEST(msg.getChar(40)=='1');
}

// test parsing a message where the group_end_tag but it is not in a group because the group count was not seen
BOOST_AUTO_TEST_CASE( group_end_tag ) {
    GroupDefs defs;
    defs.add(NEW_ORDER_SINGLE,{100,54});

    char buffer[4096];
    FixMessage msg;
    decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE,buffer);
    FixMessage::parse(buffer,msg, defs);
    BOOST_TEST(msg.getString(Tag::MSG_TYPE)==NEW_ORDER_SINGLE);
}

BOOST_AUTO_TEST_CASE( group_parsing ) {
    GroupDefs defs;
    defs.add(NEW_ORDER_SINGLE,{100,54});

    char buffer[4096];
    FixMessage msg;
    decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE_WITH_GROUP,buffer);
    FixMessage::parse(buffer,msg,defs);
    BOOST_TEST(msg.getString(Tag::MSG_TYPE)==NEW_ORDER_SINGLE);
    BOOST_TEST(msg.getString(100,0,55)=="AAPL");
    BOOST_TEST(msg.getString(100,1,55)=="MSFT");
}

// test parsing of a stream of messages simulating a fix tcp connection
BOOST_AUTO_TEST_CASE( stream_parsing ) {
    GroupDefs defs;
    defs.add(NEW_ORDER_SINGLE,{100,54});

    char buffer[4096];
    decodeFixToBuffer(SAMPLE_NEW_ORDER_SINGLE+SAMPLE_CANCEL_REPLACE_QTY_CHANGE+SAMPLE_NEW_ORDER_SINGLE,buffer);
    std::istringstream ss(buffer);

    FixMessage msg;
    FixMessage::parse(ss,msg,defs);
    BOOST_TEST(msg.getString(Tag::MSG_TYPE)==NEW_ORDER_SINGLE);
    FixMessage::parse(ss,msg,defs);
    BOOST_TEST(msg.getString(Tag::MSG_TYPE)==ORDER_CANCEL_REPLACE_REQUEST);
    FixMessage::parse(ss,msg,defs);
    BOOST_TEST(msg.getString(Tag::MSG_TYPE)==NEW_ORDER_SINGLE);
}

