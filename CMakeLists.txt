cmake_minimum_required(VERSION 3.22)
project(KNM_SYNTHESIZER)

option(KNM_SYNTHESIZER_BUILD_DOCS "Generate the documentation (default=ON)" ON)
option(KNM_SYNTHESIZER_RUN_TESTS  "Run the tests during build (default=ON)" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(examples)
add_subdirectory(tests)

if (KNM_SYNTHESIZER_BUILD_DOCS)
    add_subdirectory(docs)
endif()
