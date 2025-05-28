cmake_minimum_required(VERSION 3.10)
project(libcanbus)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

# Libreria dinamica
add_library(canbus SHARED
    src/CANBus.cpp
    src/CANMessage.cpp
)

# Header file
target_include_directories(canbus PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(canbus PUBLIC -pthread)

# Installazione
install(TARGETS canbus
    EXPORT canbusTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT canbusTargets
    FILE canbusConfig.cmake
    NAMESPACE canbus::
    DESTINATION lib/cmake/canbus
)

# -------------------------
#      SEZIONE TEST
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
target_link_libraries(test_CANMessage PRIVATE canbus gtest_main)

add_executable(test_CANBus tests/test_CANBus.cpp)
target_link_libraries(test_CANBus PRIVATE canbus gtest_main)

add_executable(test_CANBus_multithread tests/test_CANBus_multithread.cpp)
target_link_libraries(test_CANBus_multithread PRIVATE canbus gtest_main)

include(GoogleTest)
gtest_discover_tests(test_CANMessage)
gtest_discover_tests(test_CANBus)
gtest_discover_tests(test_CANBus_multithread)

