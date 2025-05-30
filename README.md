# CANBus C++ Library

A lightweight C++ library for interacting with Linux SocketCAN interfaces, including utilities for sending, receiving, and manipulating CAN frames. This project includes a shared library and command-line tools for testing CAN communication.

## Features

* Object-oriented wrapper over Linux SocketCAN (`AF_CAN`, `SOCK_RAW`, `CAN_RAW`)
* Support for standard and extended CAN frames
* Timestamped message representation
* Optional loopback for test environments (e.g., `vcan0`)
* CLI tools for sending/receiving CAN messages
* GoogleTest-based unit and multithreaded integration tests
* Configurable constructor with auto-open
* Non-blocking and timeout-based receive support
* Utility method to check if a CAN message is empty

## Directory Structure

```
.
├── include/
│   ├── CANBus.h
│   └── CANMessage.h
├── src/
│   ├── CANBus.cpp
│   ├── CANMessage.cpp
│   ├── can_sender.cpp
│   └── can_receiver.cpp
├── tests/
│   ├── test_CANBus.cpp
│   ├── test_CANMessage.cpp
│   └── test_CANBus_multithread.cpp
├── CMakeLists.txt
└── README.md
```

## Requirements

* Linux system with SocketCAN support
* CMake ≥ 3.10
* GCC or Clang with C++17 support
* Optional: `vcan` interface for testing
* GoogleTest (automatically fetched by CMake)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage (CLI)

### CAN Sender

```bash
./can_sender <interface> <id>#<hexdata> [-f frequency]
```

* Example (single send): `./can_sender vcan0 123#DEADBEEF`

* Example (send repeatedly at 10Hz): `./can_sender vcan0 123#DEADBEEF -f 10`

### CAN Receiver

```bash
./can_receiver <interface> [-F filter_id]
```

* Example (receive all): `./can_receiver vcan0`

* Example (filter by ID): `./can_receiver vcan0 -F 123`

## Usage Example (C++ API)

```cpp
#include "CANBus.h"
#include "CANMessage.h"
#include <array>
#include <iostream>

int main() {
    using namespace canbus;

    try {
        CANBus bus("vcan0", true, true, true);  // auto_open = true, loopback = true, receive_own_msgs = true

        std::array<uint8_t, 8> data = {0xDE, 0xAD, 0xBE, 0xEF};
        CANMessage message(0x123, data, 4, false);  // Standard ID

        if (bus.send(message)) {
            std::cout << "Message sent: " << message.to_string() << std::endl;
        }

        // Non-blocking receive
        CANMessage received = bus.receive(0);
        if (!received.empty()) {
            std::cout << "Message received: " << received.to_string() << std::endl;
        } else {
            std::cout << "No message available.\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "CAN error: " << e.what() << std::endl;
    }

    return 0;
}
```

## Testing

Run all unit and integration tests:

```bash
cd build
ctest
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Author

Developed by \[Luca Tonin, UNIPD], 2025.
