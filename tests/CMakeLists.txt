#------CMAKELIST FOR TESTS-----
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

add_executable(laser_proto_tests
    test_laser_proto.cpp
    ${CMAKE_SOURCE_DIR}/shared/laser_proto/laser_proto.c
)

target_include_directories(laser_proto_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/shared/laser_proto
)

target_link_libraries(laser_proto_tests PRIVATE GTest::gtest_main)

# registering CTest
include(GoogleTest)
gtest_discover_tests(laser_proto_tests)

add_executable(ring_buffer_tests
    test_ring_buffer.cpp
    ${CMAKE_SOURCE_DIR}/shared/ring_buffer/ring_buffer.c
)

target_include_directories(ring_buffer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/shared/ring_buffer
)

target_link_libraries(ring_buffer_tests PRIVATE GTest::gtest_main)

gtest_discover_tests(ring_buffer_tests)
