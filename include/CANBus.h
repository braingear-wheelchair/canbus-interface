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
#include <fcntl.h>
#include "CANMessage.h"

namespace canbus {

class CANBus {
    public:
        explicit CANBus(const std::string& interface_name, bool auto_open = false, bool enable_loopback = true, bool receive_own_msgs = true);
        ~CANBus();

        bool open(void);
        void close(void);
        bool is_open(void) const;
        bool send(const CANMessage& message);
        CANMessage receive(int timeout_ms = -1); // -1: blocking; 0: non-blocking; >0: with timeout 
        
        std::string get_interface(void);
        void set_interface(const std::string& interface_name);

    private:
        std::string interface_name_;
        bool auto_open_;
        int socket_fd_;
        bool is_open_;
        bool has_loopback_;
        bool receive_own_msgs_;

        mutable std::mutex mutex_;
};

}

#endif