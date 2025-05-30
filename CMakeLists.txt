cmake_minimum_required(VERSION 3.10)
project(canbus)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

# Libreria dinamica
add_library(${PROJECT_NAME} SHARED
    src/CANBus.cpp
    src/CANMessage.cpp
)

# Header file
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC -pthread)

# Installazione
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# CAN sender executable
add_executable(can_sender src/can_sender.cpp)
target_include_directories(can_sender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(can_sender ${PROJECT_NAME})

# CAN receiver executable
add_executable(can_receiver src/can_receiver.cpp)
target_include_directories(can_receiver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(can_receiver ${PROJECT_NAME})

# Installazione
install(TARGETS can_sender can_receiver
    RUNTIME DESTINATION bin
)

# -------------------------
#         TEST
# -------------------------

# Set policy per evitare warning (CMP0135)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# GoogleTest (con timestamp extraction abilitato)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Aggiungi test
add_executable(test_CANMessage tests/test_CANMessage.cpp)
target_link_libraries(test_CANMessage PRIVATE ${PROJECT_NAME} gtest_main)

add_executable(test_CANBus tests/test_CANBus.cpp)
target_link_libraries(test_CANBus PRIVATE ${PROJECT_NAME} gtest_main)

add_executable(test_CANBus_multithread tests/test_CANBus_multithread.cpp)
target_link_libraries(test_CANBus_multithread PRIVATE ${PROJECT_NAME} gtest_main)

include(GoogleTest)
gtest_discover_tests(test_CANMessage)
gtest_discover_tests(test_CANBus)
gtest_discover_tests(test_CANBus_multithread)

