cmake_minimum_required(VERSION 3.16)
project(rover_host_demo C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)
add_executable(rover_host_demo
  firmware/app/src/rover_state.c
  firmware/app/src/rover_safety.c
  firmware/platform/host/rover_hal_host.c
  firmware/platform/host/main.c)
target_include_directories(rover_host_demo PRIVATE
  firmware/app/include firmware/platform/include firmware/drivers/include)
enable_testing()
add_executable(test_safety tests/host/test_safety.c firmware/app/src/rover_safety.c)
target_include_directories(test_safety PRIVATE firmware/app/include)
add_test(NAME safety_logic COMMAND test_safety)
