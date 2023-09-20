# Require CMake 3.16
cmake_minimum_required(VERSION 3.16)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror -Wextra)

# Default to debug build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

project(Nozomi CXX)

# Set include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/loader)
include_directories(${PROJECT_SOURCE_DIR}/include/sys)
include_directories(${PROJECT_SOURCE_DIR}/external/dynarmic)
include_directories(${PROJECT_SOURCE_DIR}/external/plog)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/plog)

# Set source files
set(SOURCES
    src/main.cpp
    src/loader/loader.cpp
    src/loader/nro.cpp
    src/sys/emulator.cpp
)

# Set header files
set(HEADERS
    include/loader/loader.hpp
    include/loader/nro.hpp
    include/sys/emulator.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE plog)
