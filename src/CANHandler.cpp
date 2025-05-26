#include "../include/CANHandler.hpp"
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <fcntl.h>    // For fcntl, F_GETFL, F_SETFL, O_NONBLOCK

CANHandler::CANHandler() : socketFd_(-1) {
    // Default constructor - initialize with bus 0
    if(openSocket(0)){
        std::cout << "CAN interface initialized successfully" << std::endl;
    }
    else{
        std::cerr << "Failed to initialize CAN interface" << std::endl;
    }
}

CANHandler::CANHandler(int busNum) : socketFd_(-1) {
    if(openSocket(busNum)){
        std::cout << "CAN interface initialized successfully" << std::endl;
    }
    else{
        std::cerr << "Failed to initialize CAN interface" << std::endl;
    }
}

CANHandler::~CANHandler() {
    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
}

//Opens a socket to the CAN interface and uses a virtual Can if regular CAN fails
bool CANHandler::openSocket(int canNum) {
    // Close existing socket if open
    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }

    // Create CAN socket
    socketFd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socketFd_ < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return false;
    }
    int buffer_size = 1024 * 1024; // Increase buffer size to 500 KB
    setsockopt(socketFd_, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));

    // Specify CAN interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    std::string canInterface = "can" + std::to_string(canNum);
    strcpy(ifr.ifr_name, canInterface.c_str());
    
    // Retrieve the interface index 
    if (ioctl(socketFd_, SIOCGIFINDEX, &ifr) < 0) {
        // Try virtual CAN if regular CAN fails, used for testing
        std::string vcanInterface = "vcan" + std::to_string(canNum);
        strcpy(ifr.ifr_name, vcanInterface.c_str());
        if (ioctl(socketFd_, SIOCGIFINDEX, &ifr) < 0) {
            std::cerr << "Failed to open " << canInterface << " and " << vcanInterface 
                      << ": " << strerror(errno) << std::endl;
            close(socketFd_);
            socketFd_ = -1;
            return false;
        }
        std::cout << "Connected to " << vcanInterface << std::endl;
    } else {
        std::cout << "Connected to " << canInterface << std::endl;
    }

    // Bind the socket to the CAN interface
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socketFd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    std::cout << "CAN socket opened successfully" << std::endl;
    return true;
} 

//Builds a CAN frame from a string representation (see "CANFormat" for the format)
struct can_frame CANHandler::buildFrame(const std::string& canStr) {
    struct can_frame frame = {0};  
    
    // Check for '#' delimiter
    size_t delimiter = canStr.find('#');
    if (delimiter == std::string::npos) {
        std::cerr << "buildFrame: missing #" << std::endl;
        return frame;
    }

    // Split ID and data 
    std::string id_str = canStr.substr(0, delimiter);
    std::string data_str = canStr.substr(delimiter + 1);
    
    // Parse ID
    try {
        // Convert hex string ID to integer
        unsigned int id = std::stoul(id_str, nullptr, 16);
        
        // Set CAN ID based on length (3 for standard, 8 for extended)
        if (id_str.length() == 3) {
            frame.can_id = id;
        } 
        else if (id_str.length() == 8) {
            frame.can_id = id | CAN_EFF_FLAG;  // Set extended frame flag
        } 
        else {
            std::cerr << "Invalid ID length" << std::endl;
            return frame;
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
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing CAN frame: " << e.what() << std::endl;
        frame = {0};
    }

    return frame;
}

//Dissects a CAN frame and returns a string representation
std::string CANHandler::dissectFrame(const can_frame& frame) {

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

bool CANHandler::sendFrame(const std::string& frameStr) {
    if (socketFd_ < 0) {
        std::cerr << "Cannot send frame: socket not initialized" << std::endl;
        
        // Try to reopen the socket
        if (!openSocket(0)) {
            std::cerr << "Failed to reopen CAN socket" << std::endl;
            return false;
        }
        std::cout << "Successfully reopened CAN socket" << std::endl;
    }
    
    try {
        // Build the CAN frame from the string
        can_frame frame = buildFrame(frameStr);
        
        // Send the frame
        ssize_t nbytes = write(socketFd_, &frame, sizeof(struct can_frame));
        
        if (nbytes != sizeof(struct can_frame)) {
            std::cerr << "Error sending CAN frame " << frameStr << ": " << strerror(errno) << std::endl;
            flushCANBuffer();
            close(socketFd_);
            socketFd_ = -1;
            
            
            // If the socket is bad, close it and try to reopen
            
            if (!openSocket(0)) {
                std::cerr << "Failed to reopen CAN socket after bad file descriptor" << std::endl;
                return false;
            }
            std::cout << "Successfully reopened CAN socket after bad file descriptor" << std::endl;
            
            // Try sending again with the new socket
            nbytes = write(socketFd_, &frame, sizeof(struct can_frame));
            if (nbytes != sizeof(struct can_frame)) {
                std::cerr << "Error sending CAN frame after socket reopen: " << strerror(errno) << std::endl;
                return false;
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while sending CAN frame: " << e.what() << std::endl;
        return false;
    }
}

void CANHandler::flushCANBuffer() {
    struct can_frame frame;
    int nbytes;

    // Set the socket to non-blocking mode temporarily
    int flags = fcntl(socketFd_, F_GETFL, 0);
    fcntl(socketFd_, F_SETFL, flags | O_NONBLOCK);

    // Read and discard frames until the buffer is empty
    while ((nbytes = read(socketFd_, &frame, sizeof(struct can_frame))) > 0) {
        // Discard the frame
    }

    // Restore original socket flags
    fcntl(socketFd_, F_SETFL, flags);

    if (nbytes < 0 && errno != EAGAIN) {
        std::cerr << "Error flushing CAN buffer: " << strerror(errno) << std::endl;
    } else {
        std::cout << "CAN buffer flushed" << std::endl;
    }
}
