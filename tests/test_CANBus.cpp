#include <gtest/gtest.h>
#include "CANBus.h"

TEST(CANBusTest, SendAndReceive) {
    canbus::CANBus bus("vcan0", true);
    ASSERT_TRUE(bus.open());

    std::array<uint8_t, 8> data = {0x01, 0x02, 0x03};
    canbus::CANMessage msg_out(0x123, data, 3, false);
    ASSERT_TRUE(bus.send(msg_out));

    // Ricevi il messaggio con timeout di 1 secondo
    canbus::CANMessage msg_in = bus.receive(1000);

    EXPECT_EQ(msg_in.get_id(), msg_out.get_id());
    EXPECT_EQ(msg_in.get_dlc(), msg_out.get_dlc());
    EXPECT_EQ(msg_in.get_data()[0], msg_out.get_data()[0]);
    EXPECT_EQ(msg_in.get_data()[1], msg_out.get_data()[1]);
    EXPECT_EQ(msg_in.get_data()[2], msg_out.get_data()[2]);

    bus.close();
}