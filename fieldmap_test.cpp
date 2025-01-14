#define BOOST_TEST_MODULE fieldmap
#include <boost/test/included/unit_test.hpp>

#include "fieldmap.h"

BOOST_AUTO_TEST_CASE( basic_test ) {
    FieldMapBuffer buffer;
    FieldMap fm(buffer);

    fm.set(100,Field(100,101,102));
    fm.set(200,Field(200,201,202));

    BOOST_TEST(fm.get(100).offset == 101);
    BOOST_TEST(fm.get(100).length == 102);
}


BOOST_AUTO_TEST_CASE( basic_groups ) {
    FieldMapBuffer buffer;
    FieldMap fm(buffer);

    fm.set(100,Field(100,101,102));
    fm.set(200,Field(200,201,202));
    
    auto& gm = fm.addGroup(299);
    gm.set(300,Field(300,301,302));

    BOOST_TEST(fm.get(100).offset == 101);
    BOOST_TEST(fm.get(100).length == 102);
    BOOST_TEST(fm.getGroup(299,0).get(300).offset==301);
}

