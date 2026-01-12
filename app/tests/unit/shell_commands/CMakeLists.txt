cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smart_feeder_unit_shell_commands)

target_sources(app PRIVATE
  src/test_shell_commands.c
  ../../../src/shell_commands.c
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/../../../include
)

target_compile_definitions(app PRIVATE SMART_FEEDER_UNIT_TEST=1)
