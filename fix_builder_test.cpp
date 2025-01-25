#define BOOST_TEST_MODULE fix_builder
#include <boost/test/included/unit_test.hpp>
#include <string_view>

#include "fix_builder.h"
#include "samples.h"

BOOST_AUTO_TEST_CASE( add_tag ) {
    FixBuilder builder;

    builder.addField(8,"FIX.4.4");
    BOOST_TEST(encodeFix(builder.messageView())=="8=FIX.4.4^A10=033^A");
}

BOOST_AUTO_TEST_CASE( add_int_tag ) {
    FixBuilder builder;

    builder.addField(9,1234);
    BOOST_TEST(encodeFix(builder.messageView())=="9=1234^A10=065^A");
}
BOOST_AUTO_TEST_CASE( add_char_tag ) {
    FixBuilder builder;

    builder.addField(9,1234);
    builder.addField(20,char('I'));
    BOOST_TEST(encodeFix(builder.messageView())=="9=1234^A20=I^A10=042^A");
}

BOOST_AUTO_TEST_CASE( body_length ) {
    FixBuilder builder;

    builder.addField(8,"FIX 4.4");
    builder.addField(9,"0");
    builder.addField(35,"0");

    BOOST_TEST(encodeFix(builder.messageView())=="8=FIX 4.4^A9=5^A35=0^A10=149^A");
    builder.reset();

    builder.addField(8,"FIX.4.4");
    builder.addField(9,"0000");
    builder.addField(35,"D");
    builder.addField(34,1234);
    BOOST_TEST(encodeFix(builder.messageView())=="8=FIX.4.4^A9=0013^A35=D^A34=1234^A10=181^A");
}

BOOST_AUTO_TEST_CASE( check_sum_logon ) {
    FixBuilder builder;

    builder.addField(8,"FIX.4.4");
    builder.addField(9,"00");
    builder.addField(35,"A");
    builder.addField(34,1092);
    builder.addField(49,"TESTBUY1");
    builder.addField(52,"20180920-18:24:59.643");
    builder.addField(56,"TESTSELL1");
    builder.addField(98,0);
    builder.addField(108,60);

    BOOST_TEST(encodeFix(builder.messageView())==SAMPLE_LOGON);
}

BOOST_AUTO_TEST_CASE( check_sum_logout ) {
    FixBuilder builder;

    builder.addField(8,"FIX.4.4");
    builder.addField(9,"00");
    builder.addField(35,"5");
    builder.addField(34,1091);
    builder.addField(49,"TESTBUY1");
    builder.addField(52,"20180920-18:24:58.675");
    builder.addField(56,"TESTSELL1");

    BOOST_TEST(encodeFix(builder.messageView())==SAMPLE_LOGOUT);
}

BOOST_AUTO_TEST_CASE( check_sum2 ) {
    FixBuilder builder;

    builder.addField(8,"FIX.4.2");
    builder.addField(9,"000");
    builder.addField(35,"D");
    builder.addField(34,4);
    builder.addField(49,"ABC_DEFG01");
    builder.addField(52,"20090323-15:40:29");
    builder.addField(56,"CCG");
    builder.addField(115,"XYZ");
    builder.addField(11,"NF 0542/03232009");
    builder.addField(54,1);
    builder.addField(38,100);
    builder.addField(55,"CVS");
    builder.addField(40,1);
    builder.addField(59,0);
    builder.addField(47,"A");
    builder.addField(60,"20090323-15:40:29");
    builder.addField(21,1);
    builder.addField(207,"N");

    BOOST_TEST(encodeFix(builder.messageView())==SAMPLE_NEW_ORDER_SINGLE);
}

BOOST_AUTO_TEST_CASE(time_field) {
    FixBuilder builder;

    const std::time_t epoch_plus_11h = 60 * 60 * 11;
    const int local_time = localtime(&epoch_plus_11h)->tm_hour;
    const int gm_time = gmtime(&epoch_plus_11h)->tm_hour;
    const int tz_diff = local_time - gm_time;

    struct tm tm_time = {};
    tm_time.tm_year = 2023 - 1900; // Year - 1900
    tm_time.tm_mon = 11; // 0-based index, so December is 11
    tm_time.tm_mday = 25;
    tm_time.tm_hour = 13+tz_diff;
    tm_time.tm_min = 25;
    tm_time.tm_sec = 59;

    time_t time_in_seconds = mktime(&tm_time); 
    timeval tv_time;

    // Convert time_t to timeval
    tv_time.tv_sec = time_in_seconds;
    tv_time.tv_usec = 567*1000;

    builder.addTime(60,tv_time);
    auto msg = builder.messageView();

    auto expected = "20231225-13:25:59.567^A";

    BOOST_TEST(encodeFix(msg).find(expected)!=std::string::npos,encodeFix(msg));
}

BOOST_AUTO_TEST_CASE(fixed_field) {
    FixBuilder builder;
    builder.addField(44,Fixed("12345.6789"));

    auto expected = "44=12345.6789^A";
    auto msg = builder.messageView();

    BOOST_TEST(encodeFix(msg).find(expected)!=std::string::npos,encodeFix(msg));
}
