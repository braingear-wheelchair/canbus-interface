#include "CANBus.h"
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can/raw.h>
#include <linux/can.h>
#include <sys/select.h>

namespace canbus {

CANBus::CANBus(const std::string& interface_name, bool enable_loopback)
    : interface_name_(interface_name), socket_fd_(-1), is_open_(false), has_loopback_(enable_loopback) {}

CANBus::~CANBus() {
    close();
}

bool CANBus::open() {
    std::lock_guard<std::mutex> lock(mutex_);

    socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket_fd_ < 0) {
        throw std::runtime_error("Failed to create CAN socket: " + std::string(strerror(errno)));
    }

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ);
    if (ioctl(socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
        throw std::runtime_error("Failed to get interface index: " + std::string(strerror(errno)));
    }

    struct sockaddr_can addr {};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
        throw std::runtime_error("Failed to bind CAN socket: " + std::string(strerror(errno)));
    }

    if (has_loopback_) {
        int loopback = 1;
        setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback, sizeof(loopback));
    }

    is_open_ = true;
    return true;
}

void CANBus::close() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (is_open_ && socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
        is_open_ = false;
    }
}

bool CANBus::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_open_;
}

bool CANBus::send(const CANMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    struct can_frame frame = message.to_can_frame();
    ssize_t nbytes = write(socket_fd_, &frame, sizeof(struct can_frame));
    return nbytes == sizeof(struct can_frame);
}

CANMessage CANBus::receive() {
    std::lock_guard<std::mutex> lock(mutex_);
    struct can_frame frame {};
    ssize_t nbytes = read(socket_fd_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error("Failed to receive CAN frame: " + std::string(strerror(errno)));
    }
    return CANMessage(frame);
}

CANMessage CANBus::receive(int timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);

    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd_, &read_fds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int retval = select(socket_fd_ + 1, &read_fds, nullptr, nullptr, &tv);
    if (retval == -1) {
        throw std::runtime_error("select() failed: " + std::string(strerror(errno)));
    } else if (retval == 0) {
        throw std::runtime_error("CAN receive timeout after " + std::to_string(timeout_ms) + " ms");
    }

    struct can_frame frame {};
    ssize_t nbytes = read(socket_fd_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error("Failed to receive CAN frame: " + std::string(strerror(errno)));
    }

    return CANMessage(frame);
}

} 


/*#include "CANBus.h"

namespace canbus {

CANBus::CANBus(const std::string& interface_name, bool enable_loopback)
    : interface_name_(interface_name), socket_fd_(-1), is_open_(false), has_loopback_(enable_loopback) {}

CANBus::~CANBus() {
    close();
}

bool CANBus::open() {
    this->socket_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (this->socket_fd_ < 0) {
        throw std::runtime_error("Failed to create CAN socket: " + std::string(strerror(errno)));
    }

    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ);
    if (ioctl(this->socket_fd_, SIOCGIFINDEX, &ifr) < 0) {
        ::close(this->socket_fd_);
        this->socket_fd_ = -1;
        throw std::runtime_error("Failed to get interface index: " + std::string(strerror(errno)));
    }

    struct sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(this->socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(this->socket_fd_);
        this->socket_fd_ = -1;
        throw std::runtime_error("Failed to bind CAN socket: " + std::string(strerror(errno)));
    }

    if (this->has_loopback_) {
        int loopback = 1;
        setsockopt(this->socket_fd_, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback, sizeof(loopback));
    }


    this->is_open_ = true;
    return true;
}

void CANBus::close() {
    if (this->is_open_ && this->socket_fd_ >= 0) {
        ::close(this->socket_fd_);
        this->socket_fd_ = -1;
        this->is_open_ = false;
    }
}

bool CANBus::isOpen() const {
    return this->is_open_;
}

bool CANBus::send(const CANMessage& message) {
    struct can_frame frame = message.to_can_frame();
    ssize_t nbytes = write(this->socket_fd_, &frame, sizeof(struct can_frame));
    return nbytes == sizeof(struct can_frame);
}

CANMessage CANBus::receive() {
    struct can_frame frame{};
    ssize_t nbytes = read(this->socket_fd_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error("Failed to receive CAN frame: " + std::string(strerror(errno)));
    }
    return CANMessage(frame);
}

CANMessage CANBus::receive(int timeout_ms) {
    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(this->socket_fd_, &read_fds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int retval = select(this->socket_fd_ + 1, &read_fds, nullptr, nullptr, &tv);
    if (retval == -1) {
        throw std::runtime_error("select() failed: " + std::string(strerror(errno)));
    } else if (retval == 0) {
        throw std::runtime_error("CAN receive timeout after " + std::to_string(timeout_ms) + " ms");
    }

    struct can_frame frame{};
    ssize_t nbytes = read(this->socket_fd_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error("Failed to receive CAN frame: " + std::string(strerror(errno)));
    }

    return CANMessage(frame);
}

}*/
