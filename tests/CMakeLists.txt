cmake_minimum_required(VERSION 3.10)
project(TaskSchedulerTests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/tests)

# Gather source files
file(GLOB TASKSCHEDULER_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

file(GLOB TEST_SOURCES 
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

# Check if we have test files
list(LENGTH TEST_SOURCES TEST_COUNT)
if(TEST_COUNT EQUAL 0)
    message(WARNING "No test files found in tests/ directory")
endif()

# Create the test executable only if we have test sources
if(TEST_COUNT GREATER 0)
    add_executable(taskscheduler_tests
        ${TEST_SOURCES}
        ${TASKSCHEDULER_SOURCES}
    )

    # Link libraries
    target_link_libraries(taskscheduler_tests
        gtest
        gtest_main
        pthread
    )

    # Compiler definitions for Arduino compatibility
    target_compile_definitions(taskscheduler_tests PRIVATE
        ARDUINO=200
        _TASK_MICRO_RES=1
        _TASK_STD_FUNCTION=0
        _TASK_TIMECRITICAL=1
        _TASK_STATUS_REQUEST=1
        _TASK_WDT_IDS=1
        _TASK_LTS_POINTER=1
        _TASK_PRIORITY=1
        _TASK_TIMEOUT=1
        _TASK_OO_CALLBACKS=1
        _TASK_DEFINE_MILLIS=0
        _TASK_INLINE=0
        _TASK_THREAD_SAFE=0
        _TASK_SLEEP_ON_IDLE_RUN=0
    )

    # Compiler flags
    target_compile_options(taskscheduler_tests PRIVATE
        -Wall
        -Wextra
        -O2
    )

    # Enable testing
    enable_testing()
    add_test(NAME TaskSchedulerUnitTests COMMAND taskscheduler_tests)

    # Print summary
    message(STATUS "TaskScheduler Test Build Configuration:")
    message(STATUS "  Test files found: ${TEST_COUNT}")
    message(STATUS "  TaskScheduler sources: ${TASKSCHEDULER_SOURCES}")
    message(STATUS "  Test sources: ${TEST_SOURCES}")
else()
    message(STATUS "No tests to build - tests/ directory is empty or missing .cpp files")
endif()