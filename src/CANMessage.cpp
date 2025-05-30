#include "CANMessage.h"

namespace canbus {

CANMessage::CANMessage() = default;

CANMessage::CANMessage(uint32_t id, const std::array<uint8_t, 8>& data, uint8_t dlc, bool ext)
    : id_(id), data_(data), dlc_(dlc), extended_(ext), timestamp_(std::chrono::system_clock::now()) {}

CANMessage::CANMessage(const struct can_frame& frame)
    : id_(frame.can_id & CAN_EFF_MASK),
      dlc_(frame.can_dlc),
      extended_(frame.can_id & CAN_EFF_FLAG),
      timestamp_(std::chrono::system_clock::now()) {
    std::copy(std::begin(frame.data), std::begin(frame.data) + this->dlc_, this->data_.begin());
}

uint32_t CANMessage::get_id() const { 
    return this->id_; 
}

void CANMessage::set_id(uint32_t value) { 
    this->id_ = value; 
}

uint8_t CANMessage::get_dlc() const { 
    return this->dlc_; 
}

void CANMessage::set_dlc(uint8_t value) { 
    this->dlc_ = std::min(value, static_cast<uint8_t>(8)); 
}

const std::array<uint8_t, 8>& CANMessage::get_data() const { 
    return this->data_; 
}

void CANMessage::set_data(const std::array<uint8_t, 8>& d) { 
    this->data_ = d; 
}

bool CANMessage::is_extended_id() const { 
    return this->extended_; 
}

void CANMessage::set_extended(bool ext) { 
    this->extended_ = ext; 
}

std::chrono::system_clock::time_point CANMessage::get_timestamp() const { 
    return this->timestamp_; 
}

void CANMessage::set_timestamp(std::chrono::system_clock::time_point ts) { 
    this->timestamp_ = ts; 
}

struct can_frame CANMessage::to_can_frame() const {
    struct can_frame frame{};
    frame.can_id = this->extended_ ? (this->id_ | CAN_EFF_FLAG) : (this->id_ & CAN_SFF_MASK);
    frame.can_dlc = this->dlc_;
    std::copy(this->data_.begin(), this->data_.begin() + this->dlc_, frame.data);
    return frame;
}

std::string CANMessage::to_string(bool show_timestamp) const {
    std::ostringstream oss;

    if (show_timestamp) {
        auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                          this->timestamp_.time_since_epoch()).count();
        oss << "[" << now_us << "] ";
    }

    oss << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << this->id_ << "#";

    for (size_t i = 0; i < this->dlc_; ++i) {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(this->data_[i]) << " ";
    }

    std::string result = oss.str();
    if (!result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}

CANMessage CANMessage::from_string(const std::string& str) {
    CANMessage msg;
    std::istringstream iss(str);
    std::string id_str;

    if (str[0] == '[') {
        std::getline(iss, id_str, ']');
        iss.get();
    }

    if (!std::getline(iss, id_str, '#')) {
        throw std::runtime_error("Invalid CAN frame: missing ID or #");
    }

    msg.id_ = std::stoul(id_str, nullptr, 16);
    msg.extended_ = (msg.id_ > 0x7FF);

    std::string byte_str;
    while (iss >> std::hex >> byte_str) {
        if (msg.dlc_ >= 8) break;
        uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        msg.data_[msg.dlc_++] = byte;
    }

    msg.timestamp_ = std::chrono::system_clock::now();
    return msg;
}

bool CANMessage::empty(void) const {
    return this->dlc_ == 0 && this->id_ == 0;
}

}