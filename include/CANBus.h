#ifndef CANBUS_H_
#define CANBUS_H_

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "CANMessage.h"

namespace canbus {

class CANBus {
    public:
        explicit CANBus(const std::string& interface_name, bool enable_loopback = false);
        ~CANBus();

        bool open();
        void close();
        bool isOpen() const;
        bool send(const CANMessage& message);
        CANMessage receive();
        CANMessage receive(int timeout_ms); 

    private:
        std::string interface_name_;
        int socket_fd_;
        bool is_open_;
        bool has_loopback_;

        mutable std::mutex mutex_;
};

}

#endif