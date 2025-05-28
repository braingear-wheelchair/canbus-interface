#include "CANBus.h"
#include "CANMessage.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace canbus;

void sender_thread(CANBus& writer, const CANMessage& message) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bool result = writer.send(message);
    if (!result) {
        std::cerr << "[Sender] Send failed!" << std::endl;
    } else {
        std::cout << "[Sender] Message sent." << std::endl;
    }
}

TEST(CANBusThreadSafetyTest, ConcurrentSendReceiveSeparateInstances) {
    CANBus reader("vcan0", true);
    CANBus writer("vcan0", true);

    ASSERT_TRUE(reader.open());
    ASSERT_TRUE(writer.open());

    std::array<uint8_t, 8> data = {0xDE, 0xAD, 0xBE, 0xEF};
    CANMessage msg(0x1AB, data, 4, false);

    std::thread sender(sender_thread, std::ref(writer), msg);

    try {
        CANMessage received = reader.receive(1000);
        EXPECT_EQ(received.get_id(), msg.get_id());
        EXPECT_EQ(received.get_dlc(), msg.get_dlc());
        for (int i = 0; i < msg.get_dlc(); ++i) {
            EXPECT_EQ(received.get_data()[i], msg.get_data()[i]);
        }
    } catch (const std::exception& e) {
        FAIL() << "Receive failed: " << e.what();
    }

    sender.join();
    reader.close();
    writer.close();
}