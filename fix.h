#pragma once
#include <string>

enum class Tags {
    MSG_TYPE = 35,
    SEQ_NUM = 34,
    SENDER_COMP_ID = 49,
    TARGET_COMP_ID = 56,
    SENDING_TIME = 52,
    CLORDID = 11,
    ORDERID = 37,
    EXECID = 17,
    SYMBOL = 55,
    SIDE = 54,
    ORDER_QTY = 38,
    PRICE = 44,
    ORD_TYPE = 40,
    TIME_IN_FORCE = 59,
    TRANSACT_TIME = 60,
    HANDL_INST = 21,
    SECURITY_ID = 48,
    SECURITY_ID_SOURCE = 22,
    LAST_QTY = 32,
    LAST_PX = 31,
    LEAVES_QTY = 151,
    CUM_QTY = 14,
    AVG_PX = 6,
    TEXT = 58,
    BODY_LENGTH = 9,
    CHECK_SUM = 10
};

const static std::string NEW_ORDER_SINGLE = "D";
const static std::string EXECUTION_REPORT = "8";
const static std::string ORDER_CANCEL_REJECT = "9";
const static std::string ORDER_CANCEL_REPLACE_REQUEST = "G";
const static std::string ORDER_CANCEL_REQUEST = "F";
const static std::string ORDER_STATUS_REQUEST = "H";
const static std::string LOGON = "A";
const static std::string LOGOUT = "5";
const static std::string HEARTBEAT = "0";
const static std::string TEST_REQUEST = "1";
const static std::string RESEND_REQUEST = "2";
const static std::string REJECT = "3";
const static std::string SEQUENCE_RESET = "4";
const static std::string NEW_ORDER_LIST = "E";
const static std::string LIST_STATUS = "N";
const static std::string LIST_EXECUTE = "L";
const static std::string LIST_CANCEL_REQUEST = "K";
const static std::string LIST_STATUS_REQUEST = "M";
const static std::string MASS_QUOTE = "i";
const static std::string MASS_QUOTE_ACKNOWLEDGEMENT = "b";

static int tagValue(Tags type) {
    return static_cast<int>(type);
}
