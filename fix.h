#pragma once
#include <string>

enum class Tags {
    MSG_TYPE = 35,
    EOM = 10
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

static int value(Tags type) {
    return static_cast<int>(type);
}
