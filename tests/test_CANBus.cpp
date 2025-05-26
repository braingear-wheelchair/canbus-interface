#include <gtest/gtest.h>
#include "CANBus.h"

TEST(CANBusTest, OpenSocket) {
    canbus::CANBus can;
    EXPECT_NO_THROW(can.open());
}

TEST(CANBusTest, SendMessage) {
    canbus::CANBus can;
    can.open();
    EXPECT_TRUE(can.send("123#DEADBEEF"));
}

TEST(CANBusTest, ToFrameAndToString) {
    canbus::CANBus can;
    std::string msg = "123#DEADBEEF";
    struct can_frame frame = can.toframe(msg);
    std::string result = can.tostring(frame);
    EXPECT_EQ(msg, result);
}
