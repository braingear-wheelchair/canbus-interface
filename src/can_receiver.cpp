#include "CANBus.h"
#include "CANMessage.h"
#include <iostream>
#include <iomanip>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <interface> [-F filter_id]\n";
        return 1;
    }

    std::string interface = argv[1];
    int filter_id = -1;

    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "-F" && i + 1 < argc) {
            filter_id = std::stoi(argv[++i], nullptr, 16);
        }
    }

    try {
        canbus::CANBus bus(interface, true);
        bus.open();

        while (true) {
            canbus::CANMessage msg = bus.receive();

            if (filter_id >= 0 && msg.get_id() != static_cast<uint32_t>(filter_id)) {
                continue;
            }

            auto ts = msg.get_timestamp();
            std::time_t time_sec = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::seconds>(ts));
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count() % 1000;

            std::tm* tm = std::localtime(&time_sec);

            std::cout << "[" << std::put_time(tm, "%F %T") << "." << std::setfill('0') << std::setw(3) << ms << "] ";

            std::cout << "ID: 0x" << std::hex << std::setw(3) << std::setfill('0') << msg.get_id()
                      << "  DLC: " << std::dec << static_cast<int>(msg.get_dlc()) << "  Data: ";

            for (size_t i = 0; i < msg.get_dlc(); ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(msg.get_data()[i]) << " ";
            }

            std::cout << std::dec << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}