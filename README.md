# CANBus Interface Library

A modern C++17 library for interfacing with the Linux CAN bus subsystem using raw sockets. This library provides a clean, high-level abstraction for sending and receiving CAN frames, with support for virtual CAN (vcan), timeout-based reception, and formatted string/frame conversion.

---

## 🔧 Features

- Simple, object-oriented interface to CAN sockets
- Automatic fallback to virtual CAN (`vcan`) if hardware interface fails
- Frame encoding/decoding to and from string formats like `123#DEADBEEF`
- Timeout support on receive operations
- Automatic socket recovery on send failure
- Optional non-blocking receive
- Flush support for draining CAN receive buffer
- Unit tests with Google Test (GTest)
- Installable via CMake

---

## 📁 Project Structure

```
.
├── include/
│   └── CANBus.h              # Public interface
├── src/
│   └── CANBus.cpp            # Implementation
├── tests/
│   └── test_CANBus.cpp       # Unit tests
├── CMakeLists.txt            # Build & install configuration
└── README.md
```

---

## 🛠 Build & Install

```bash
# Clone the repository
git clone <your-repo-url>
cd canbus-interface

# Create build directory
mkdir build && cd build

# Configure & build
cmake ..
make

# Run tests
ctest

# Optionally install system-wide
sudo make install
```

---

## 🧪 Running Unit Tests

The project uses **GoogleTest** via CMake's `FetchContent`:

```bash
./test_CANBus
```

Tests include:
- Socket opening
- String <-> Frame conversions
- Message transmission

---

## 🧵 Example Usage

```cpp
#include "CANBus.h"
#include <iostream>

int main() {
    canbus::CANBus can(0); // use can0 or fallback to vcan0

    try {
        can.open();

        // Send a frame
        can.send("123#DEADBEEF");

        // Receive with timeout (1000ms)
        auto frame = can.receive(1000);
        std::cout << "Received: " << can.tostring(frame) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "CAN Error: " << e.what() << std::endl;
    }

    return 0;
}
```

---

## 📚 API Overview

### Class: `canbus::CANBus`

| Method | Description |
|--------|-------------|
| `bool open()` | Opens the CAN interface socket |
| `bool send(std::string)` | Sends frame from string |
| `bool send(can_frame)` | Sends raw CAN frame |
| `can_frame receive(int timeout_ms = -1)` | Receives a CAN frame, with optional timeout |
| `std::string tostring(const can_frame&)` | Converts a frame to string |
| `can_frame toframe(const std::string&)` | Converts a string to CAN frame |
| `void flush()` | Empties the CAN buffer |

---

## 📦 Installation Paths

If installed via `make install`, the files will be placed in:

- Headers: `/usr/include/`
- Library: `/usr/lib/libcanbus.so`
- CMake config: `/usr/lib/cmake/canbus/canbusConfig.cmake`

You can then link the library in another project:

```cmake
find_package(canbus REQUIRED)
target_link_libraries(my_app PRIVATE canbus::canbus)
```

---

## 🔧 Dependencies

- Linux OS with SocketCAN support
- C++17 compatible compiler
- CMake 3.10+
- GoogleTest (downloaded automatically)

---

## 📄 License

MIT License — © 2025 Your Name
