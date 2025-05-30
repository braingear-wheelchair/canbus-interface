#ifndef CAN_MESSAGE_H_
#define CAN_MESSAGE_H_

#include <array>
#include <cstdint>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <linux/can.h>

namespace canbus {

class CANMessage {

    public:
        CANMessage(void);
        CANMessage(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc, bool ext = false);
        explicit CANMessage(const struct can_frame& frame);

        bool empty(void) const;
        
        uint32_t get_id() const;
        void set_id(uint32_t value);

        uint8_t get_dlc() const;
        void set_dlc(uint8_t value);

        const std::array<uint8_t, 8>& get_data() const;
        void set_data(const std::array<uint8_t, 8>& d);

        bool is_extended_id() const;
        void set_extended(bool ext);

        std::chrono::system_clock::time_point get_timestamp() const;
        void set_timestamp(std::chrono::system_clock::time_point ts);

        struct can_frame to_can_frame() const;
        std::string to_string(bool show_timestamp = true) const;
        static CANMessage from_string(const std::string& str);

    private:
        uint32_t id_ = 0;
        std::array<uint8_t, 8> data_{};
        uint8_t dlc_   = 0;
        bool extended_ = false;
        std::chrono::system_clock::time_point timestamp_;
};

}

#endif