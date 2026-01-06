#include <catch2/catch_test_macros.hpp>

#include "JkBmsFrames.h"
#include "CppWrappers.h"

TEST_CASE("checkFrameStart", "[JkBmsFrames]") {
    uint8_t correctBuffer[] = {0x55, 0xaa, 0xeb, 0x90, 0x00};
    JkBmsCpp::JkBmsDataBuffer buffer1(correctBuffer, 5);    
    REQUIRE(JkBmsCpp::checkFrameStart(buffer1) == true);

    uint8_t incorrectBuffer[] = {0x22, 0xaa, 0xeb, 0x90, 0x00};
    JkBmsCpp::JkBmsDataBuffer buffer2(incorrectBuffer, 5);
    REQUIRE(JkBmsCpp::checkFrameStart(buffer2) == false);
}


