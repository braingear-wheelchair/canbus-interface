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

CANBus::CANBus(const std::string& interface_name, bool auto_open, bool enable_loopback, bool receive_own_msgs)
    : interface_name_(interface_name), auto_open_(auto_open_), socket_fd_(-1), is_open_(false), has_loopback_(enable_loopback),
      receive_own_msgs_(receive_own_msgs) {

    if (this->auto_open_) {
        this->open();
    }
}

CANBus::~CANBus(void) {
    close();
}

bool CANBus::open(void) {
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

    // Configura loopback
    int loopback = has_loopback_ ? 1 : 0;
    setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

    // Configura ricezione dei messaggi propri
    int recv_own = receive_own_msgs_ ? 1 : 0;
    setsockopt(socket_fd_, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own, sizeof(recv_own));

    // Forza modalità bloccante
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(socket_fd_, F_SETFL, flags);

    is_open_ = true;
    return true; 
}

void CANBus::close(void) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (is_open_ && socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
        is_open_ = false;
    }
}

bool CANBus::is_open(void) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_open_;
}

bool CANBus::send(const CANMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    struct can_frame frame = message.to_can_frame();
    ssize_t nbytes = write(socket_fd_, &frame, sizeof(struct can_frame));
    return nbytes == sizeof(struct can_frame);
}

CANMessage CANBus::receive(int timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);

    fd_set read_fds;
    struct timeval tv;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd_, &read_fds);

    struct timeval* tv_ptr = nullptr;
    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tv_ptr = &tv;
    }

    int retval = select(socket_fd_ + 1, &read_fds, nullptr, nullptr, tv_ptr);
    if (retval == -1) {
        throw std::runtime_error("select() failed: " + std::string(strerror(errno)));
    } else if (retval == 0) {
        return CANMessage();  // Messaggio vuoto per timeout/non-bloccante
    }

    struct can_frame frame {};
    ssize_t nbytes = read(socket_fd_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error("Failed to receive CAN frame: " + std::string(strerror(errno)));
    }

    return CANMessage(frame); 
}

std::string CANBus::get_interface(void) {
    std::lock_guard<std::mutex> lock(mutex_);

    return this->interface_name_;
}

void CANBus::set_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    this->interface_name_ = interface_name;
    if(this->is_open()) {
        this->close();
        this->open();
    }
}


} 
