#include "CANBus.h"

namespace canbus {


CANBus::CANBus(int canbus, int buffer_size) : socket_(-1), canbus_(canbus), buffer_size_(buffer_size) {}

CANBus::~CANBus(void) {
    if(this->is_open())
        close(this->socket_);

    this->socket_ = -1;
}

bool CANBus::is_open(void) {

    return this->socket_ >=0 ? true : false;
}

bool CANBus::open(void) {

    if(this->is_open())
        close(this->socket_);
    
    this->socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if(this->socket_ < 0)
        throw std::runtime_error(std::string("Failed to create the socket: ") + strerror(errno));

    setsockopt(this->socket_, SOL_SOCKET, SO_SNDBUF, &(this->buffer_size_), sizeof(this->buffer_size_));


    // Specify CAN interface

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    std::string caniface = "can" + std::to_string(this->canbus_);
    strcpy(ifr.ifr_name, caniface.c_str());
    
    // Retrieve the interface index 
    if (ioctl(this->socket_, SIOCGIFINDEX, &ifr) < 0) {
        // Try virtual CAN if regular CAN fails, used for testing
        std::string vcaniface = "vcan" + std::to_string(this->canbus_);
        strcpy(ifr.ifr_name, vcaniface.c_str());
        if (ioctl(this->socket_, SIOCGIFINDEX, &ifr) < 0) {
            close(this->socket_);
            throw std::runtime_error(std::string("Failed to open ") + caniface + std::string(" and ") + vcaniface + std::string(": ") + strerror(errno)); 
        }
        std::cout << "Connected to " << vcaniface << std::endl;
    } else {
        std::cout << "Connected to " << caniface << std::endl;
    }

    
    // Bind the socket to the CAN interface
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(this->socket_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(this->socket_);
        throw std::runtime_error(std::string("Error binding socket: ") + strerror(errno));
    }

    std::cout << "CAN socket opened successfully" << std::endl;
    return true;
}

struct can_frame CANBus::toframe(const std::string& msg) const {

    struct can_frame frame = {0};  
    
    // Check for '#' delimiter
    size_t delimiter = msg.find('#');
    if (delimiter == std::string::npos) 
        throw std::runtime_error("Missing # character in the message");

    // Split ID and data 
    std::string id_str   = msg.substr(0, delimiter);
    std::string data_str = msg.substr(delimiter + 1);
    
    // Parse ID
    try {
        // Convert hex string ID to integer
        unsigned int id = std::stoul(id_str, nullptr, 16);
        
        // Set CAN ID based on length (3 for standard, 8 for extended)
        if (id_str.length() == 3) {
            frame.can_id = id;
        } else if (id_str.length() == 8) {
            frame.can_id = id | CAN_EFF_FLAG;  // Set extended frame flag
        } else {
            throw std::runtime_error("Invalid ID length");
        }

        // Check for Remote Transmission Request
        if (data_str.find('R') != std::string::npos) {
            frame.can_id |= CAN_RTR_FLAG;
            frame.can_dlc = 0;
            return frame;
        }

        // Parse data
        frame.can_dlc = data_str.length() / 2;  // Two hex chars per byte
        for (size_t i = 0; i < frame.can_dlc && i < 8; i++) {
            frame.data[i] = std::stoul(data_str.substr(i*2, 2), nullptr, 16);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error parsing CAN frame: ") + e.what());
        frame = {0};
    }

    return frame;
}

std::string CANBus::tostring(const can_frame& frame) const {

    //Retrieve information about the frame
    uint32_t can_id = frame.can_id & CAN_EFF_MASK;  
    bool is_extended = frame.can_id & CAN_EFF_FLAG;
    bool is_rtr = frame.can_id & CAN_RTR_FLAG;
    uint8_t dlc = frame.can_dlc;

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    // Format ID with appropriate width (3 or 8 chars)
    if (is_extended) {
        ss << std::setw(8) << can_id;
    } else {
        ss << std::setw(3) << can_id;
    }
    
    ss << "#";
    
    for (int i = 0; i < dlc; i++) {
        ss << std::setw(2) << static_cast<int>(frame.data[i]);
    }
    
    if (is_rtr) {
        ss << "R";
    }

    return ss.str();

}

bool CANBus::send(const std::string& msg) {

    return this->send(this->toframe(msg));

}

bool CANBus::send(const can_frame& frame) {

    if(this->is_open() == false) {
        std::cerr << "Cannot send frame: socket not initialized" << std::endl;
        
        // Try to reopen the socket
        if (!this->open()) {
            std::cerr << "Failed to reopen CAN socket" << std::endl;
            return false;
        }
        std::cout << "Successfully reopened CAN socket" << std::endl;
    }

    try {
        // Send the frame
        ssize_t nbytes = write(this->socket_, &frame, sizeof(struct can_frame));
        
        if (nbytes != sizeof(struct can_frame)) {
            std::cerr << "Error sending CAN frame " << this->tostring(frame) << ": " << strerror(errno) << std::endl;
            this->flush();
            close(this->socket_);
            this->socket_ = -1;
            
            
            // If the socket is bad, close it and try to reopen
            if (!this->open()) {
                std::cerr << "Failed to reopen CAN socket after bad file descriptor" << std::endl;
                return false;
            }
            std::cout << "Successfully reopened CAN socket after bad file descriptor" << std::endl;
            
            // Try sending again with the new socket
            nbytes = write(this->socket_, &frame, sizeof(struct can_frame));
            if (nbytes != sizeof(struct can_frame)) {
                std::cerr << "Error sending CAN frame after socket reopen: " << strerror(errno) << std::endl;
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while sending CAN frame: " << e.what() << std::endl;
        return false;
    }

}

struct can_frame CANBus::receive(int timeout_ms) {
    struct can_frame frame = {0};

    if (!this->is_open()) {
        throw std::runtime_error("Cannot receive: CAN socket is not open");
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(this->socket_, &read_fds);

    struct timeval tv;
    struct timeval* tv_ptr = nullptr;

    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tv_ptr = &tv;
    }

    int ret = select(this->socket_ + 1, &read_fds, nullptr, nullptr, tv_ptr);

    if (ret < 0) {
        throw std::runtime_error(std::string("select() error: ") + strerror(errno));
    } else if (ret == 0) {
        throw std::runtime_error("CAN receive timeout");
    }

    ssize_t nbytes = read(this->socket_, &frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        throw std::runtime_error(std::string("read() failed: ") + strerror(errno));
    } else if (nbytes != sizeof(struct can_frame)) {
        throw std::runtime_error("Incomplete CAN frame received");
    }

    return frame;
}



void CANBus::flush() {
    struct can_frame frame;
    int nbytes;

    if (!this->is_open()) return;

    // Set the socket to non-blocking mode temporarily
    int flags = fcntl(this->socket_, F_GETFL, 0);
    fcntl(this->socket_, F_SETFL, flags | O_NONBLOCK);

    // Read and discard frames until the buffer is empty
    while ((nbytes = read(this->socket_, &frame, sizeof(struct can_frame))) > 0) {
        // Discard the frame
    }

    // Restore original socket flags
    fcntl(this->socket_, F_SETFL, flags);

    if (nbytes < 0 && errno != EAGAIN) {
        std::cerr << "Error flushing CAN buffer: " << strerror(errno) << std::endl;
    } else {
        std::cout << "CAN buffer flushed" << std::endl;
    }
}

int CANBus::get_socket(void) {
    return this->socket_;
}

}