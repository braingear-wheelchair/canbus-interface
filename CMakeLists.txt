cmake_minimum_required(VERSION 3.10)
project(canbus VERSION 1.0.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Libreria dinamica
add_library(${PROJECT_NAME} SHARED
    src/CANBus.cpp
    src/CANMessage.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME canbus)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC -pthread)

# Eseguibili
add_executable(can_sender src/can_sender.cpp)
target_include_directories(can_sender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(can_sender ${PROJECT_NAME})

add_executable(can_receiver src/can_receiver.cpp)
target_include_directories(can_receiver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(can_receiver ${PROJECT_NAME})

# -------------------------
#         TEST
# -------------------------
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
  #DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

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

# -------------------------
#       INSTALLATION
# -------------------------

include(CMakePackageConfigHelpers)

# .pc file per pkg-config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/canbus.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/canbus.pc
    @ONLY
)

# Installa la libreria e i target
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Installa header
install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# Installa eseguibili
install(TARGETS can_sender can_receiver
    RUNTIME DESTINATION bin
)

# Installa .pc file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/canbus.pc
    DESTINATION lib/pkgconfig
)

# Configurazione CMake find_package
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# -------------------------
#      UNINSTALL TARGET
# -------------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
