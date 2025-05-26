#ifndef CANHANDLER_HPP
#define CANHANDLER_HPP

#include <string>
#include <cstdint>
#include <iostream>
#include <linux/can.h>
/**
 * @class CANHandler
 * @brief A class to handle CAN communication.
 *
 * This class provides methods to send and receive CAN frames,
 * manage the CAN socket, and flush the CAN buffer.
 */
class CANHandler {
public:
    /**
     * @brief Default constructor - uses bus 0.
     */
    CANHandler();
    
    /**
     * @brief Constructor with specified bus number.
     * @param busNum The bus number to be used for CAN communication.
     */
    CANHandler(int busNum);
    
    /**
     * @brief Destructor.
     */
    ~CANHandler();

    /**
     * @brief Flushes the CAN buffer by reading and discarding frames.
     */
    void flushCANBuffer();

    /**
     * @brief Opens a socket for the specified CAN bus.
     * @param canNum The CAN bus number to open.
     * @return True if the socket was successfully opened, false otherwise.
     */
    bool openSocket(int canNum);

    /**
     * @brief Builds a CAN frame from a string representation.
     * @param canStr The string representation of the CAN frame.
     * @return The constructed can_frame structure.
     */
    struct can_frame buildFrame(const std::string& canStr);

    /**
     * @brief Sends a CAN frame.
     * @param frameStr The string representation of the CAN frame to send.
     * @return True if the frame was successfully sent, false otherwise.
     */
    bool sendFrame(const std::string& frameStr);

    /**
     * @brief Dissects a CAN frame into a string representation.
     * @param frame The CAN frame to dissect.
     * @return The string representation of the CAN frame.
     */
    std::string dissectFrame(const can_frame& frame);

private:
    int socketFd_; ///< File descriptor for the CAN socket.
};

#endif // CANHANDLER_HPP 
