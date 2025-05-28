#include <gtest/gtest.h>
#include "CANMessage.h"

TEST(CANMessageTest, SerializationDeserialization) {
    std::array<uint8_t, 8> data = {0xDE, 0xAD, 0xBE, 0xEF};
    canbus::CANMessage msg(0x1ABCDE, data, 4, true);

    std::string serialized = msg.to_string(false);
    canbus::CANMessage parsed = canbus::CANMessage::from_string(serialized);

    EXPECT_EQ(parsed.get_id(), msg.get_id());
    EXPECT_EQ(parsed.get_dlc(), msg.get_dlc());
    EXPECT_EQ(parsed.is_extended_id(), msg.is_extended_id());
    EXPECT_EQ(parsed.get_data()[0], msg.get_data()[0]);
    EXPECT_EQ(parsed.get_data()[1], msg.get_data()[1]);
    EXPECT_EQ(parsed.get_data()[2], msg.get_data()[2]);
    EXPECT_EQ(parsed.get_data()[3], msg.get_data()[3]);
}