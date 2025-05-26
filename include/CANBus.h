#ifndef CANBUS_H_
#define CANBUS_H_

#include <string>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>    // For fcntl, F_GETFL, F_SETFL, O_NONBLOCK

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cerrno>
#include <cstdint>
#include <mutex>

namespace canbus {


class CANBus {
    public:
        CANBus(int canbus = DEFAULT_CAN_BUS, int buffer_size = DEFAULT_BUFFER_SIZE);

        ~CANBus(void);


        bool open(void);
        void flush(void);
        bool send(const std::string& msg);
        bool send(const can_frame& frame);
        struct can_frame receive(int timeout = -1);

        int get_socket(void);
        bool is_open(void);
        

        struct can_frame toframe(const std::string& msg) const;
        std::string tostring(const can_frame& frame) const;

    private:
        int socket_;
        int canbus_;
        int buffer_size_;

        static constexpr int DEFAULT_CAN_BUS     = 0;
        static constexpr int DEFAULT_BUFFER_SIZE = 1048576;

        mutable std::mutex mutex_;

};


}

#endif