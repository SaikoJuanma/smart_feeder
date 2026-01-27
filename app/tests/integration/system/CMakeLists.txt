cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smart_feeder_integration_system)

target_sources(app PRIVATE
  src/test_system.c
  ../../../src/shell_commands.c
  ../../../src/init.c
  ../../../src/configuration.c
  ../../../src/motor_control.c
  ../../../src/check_health.c
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/../../../include
)

target_compile_definitions(app PRIVATE SMART_FEEDER_UNIT_TEST=1)
