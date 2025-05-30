#include "CANBus.h"
#include "CANMessage.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <interface> <id>#<hexdata> [-f frequency]\n";
        return 1;
    }

    std::string interface;
    std::string message_str;
    int frequency = 0; // 0 = invio singolo

    interface = argv[1];
    message_str = argv[2];

    // Optional frequency argument
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "-f" && i + 1 < argc) {
            frequency = std::stoi(argv[++i]);
        }
    }

    size_t sep = message_str.find('#');
    if (sep == std::string::npos) {
        std::cerr << "Invalid message format. Use ID#DATA (e.g., 123#AABBCC)\n";
        return 1;
    }

    uint32_t id = std::stoul(message_str.substr(0, sep), nullptr, 16);
    std::string data_str = message_str.substr(sep + 1);
    std::array<uint8_t, 8> data{};
    size_t dlc = data_str.length() / 2;

    for (size_t i = 0; i < dlc && i < 8; ++i) {
        data[i] = static_cast<uint8_t>(std::stoul(data_str.substr(2 * i, 2), nullptr, 16));
    }

    canbus::CANMessage msg(id, data, dlc, id > 0x7FF);

    try {
        canbus::CANBus bus(interface, true, true);
        bus.open();

        if (frequency > 0) {
            while (true) {
                bus.send(msg);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / frequency));
            }
        } else {
            bus.send(msg); // Send once
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}