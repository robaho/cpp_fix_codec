#include <string>
#include <regex>

const std::string SAMPLE_NEW_ORDER_SINGLE = "8=FIX.4.2^A9=146^A35=D^A34=4^A49=ABC_DEFG01^A52=20090323-15:40:29^A56=CCG^A115=XYZ^A11=NF 0542/03232009^A54=1^A38=100^A55=CVS^A40=1^A59=0^A47=A^A60=20090323-15:40:29^A21=1^A207=N^A10=195^A";
const std::string SAMPLE_CANCEL_REPLACE_QTY_CHANGE = "8=FIX.4.2^A9=197^A35=G^A34=118^A49=ABC_DEFG01^A52=20090325-15:14:47^A56=CCG^A115=XYZ^A11=NF 0574/03252009^A37=NF 0573/03252009^A41=NF 0573/03252009^A54=1^A38=2000^A55=CVS^A40=2^A44=25.47^A59=0^A47=A^A60=20090325-15:14:47^A21=1^A207=N^A10=185^A";
const std::string SAMPLE_NEW_ORDER_SINGLE_WITH_GROUP = "8=FIX.4.4^A9=128^A35=D^A49=BROKER1^A52=20230815-10:00:00.000^A56=CLIENTA^A100=2^A55=AAPL^A60=20230815-10:00:00.000^A54=1^A55=MSFT^A60=20230815-10:00:00.000^A54=1^A10=123^A";
const std::string SAMPLE_NEW_ORDER_SINGLE_LIMIT_ORDER = "8=FIX.4.2^A9=152^A35=D^A34=7332^A49=TEST1^A52=20160208-14:41:33.643^A56=DWFIX01^A1=DPQP000013^A11=982A298766020822123456797^A21=1^A38=170^A40=1^A44=0.01^A54=1^A55=AMZN^A60=20160208-14:41:33.643^A10=243^A";
const std::string SAMPLE_LOGON = "8=FIX.4.4^A9=75^A35=A^A34=1092^A49=TESTBUY1^A52=20180920-18:24:59.643^A56=TESTSELL1^A98=0^A108=60^A10=178^A";
const std::string SAMPLE_LOGOUT = "8=FIX.4.4^A9=63^A35=5^A34=1091^A49=TESTBUY1^A52=20180920-18:24:58.675^A56=TESTSELL1^A10=138^A";

const std::regex decodeReg("\\^A\\s*");
const std::regex encodeReg("\\\x01\\s*");

inline void decodeFixToBuffer(const std::string& sample,char *buffer) {
    auto s = std::regex_replace(sample,decodeReg,"\x01");
    strcpy(buffer,s.c_str());
}
inline std::string decodeFix(const std::string& sample) {
    return std::regex_replace(sample,decodeReg,"\x01");
}
inline std::string encodeFix(const std::string_view& str) {
    return std::regex_replace(std::string(str),encodeReg,"^A");
}