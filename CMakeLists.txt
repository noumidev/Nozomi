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
include_directories(${PROJECT_SOURCE_DIR}/include/android)
include_directories(${PROJECT_SOURCE_DIR}/include/common)
include_directories(${PROJECT_SOURCE_DIR}/include/hle)
include_directories(${PROJECT_SOURCE_DIR}/include/hle/service)
include_directories(${PROJECT_SOURCE_DIR}/include/loader)
include_directories(${PROJECT_SOURCE_DIR}/include/nvidia)
include_directories(${PROJECT_SOURCE_DIR}/include/sys)
include_directories(${PROJECT_SOURCE_DIR}/external/dynarmic)
include_directories(${PROJECT_SOURCE_DIR}/external/plog)

add_subdirectory(${PROJECT_SOURCE_DIR}/external/dynarmic)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/plog)

set(DYNARMIC_TESTS OFF)
set(DYNARMIC_FRONTENDS "A64")

# Set source files
set(SOURCES
    src/main.cpp
    src/android/buffer_queue.cpp
    src/android/parcel.cpp
    src/hle/handle_table.cpp
    src/hle/ipc_manager.cpp
    src/hle/kernel.cpp
    src/hle/object.cpp
    src/hle/svc.cpp
    src/hle/service/apm.cpp
    src/hle/service/applet_oe.cpp
    src/hle/service/fsp_srv.cpp
    src/hle/service/hid.cpp
    src/hle/service/nvdrv.cpp
    src/hle/service/set_sys.cpp
    src/hle/service/sm.cpp
    src/hle/service/time.cpp
    src/hle/service/vi.cpp
    src/loader/loader.cpp
    src/loader/nro.cpp
    src/nvidia/nvfile.cpp
    src/nvidia/nvflinger.cpp
    src/nvidia/nvhost_ctrl.cpp
    src/nvidia/nvmap.cpp
    src/sys/cpu.cpp
    src/sys/emulator.cpp
    src/sys/memory.cpp
)

# Set header files
set(HEADERS
    include/android/buffer_queue.hpp
    include/android/parcel.hpp
    include/common/types.hpp
    include/hle/handle.hpp
    include/hle/handle_table.hpp
    include/hle/ipc.hpp
    include/hle/ipc_manager.hpp
    include/hle/kernel.hpp
    include/hle/object.hpp
    include/hle/result.hpp
    include/hle/svc.hpp
    include/hle/service/apm.hpp
    include/hle/service/applet_oe.hpp
    include/hle/service/fsp_srv.hpp
    include/hle/service/hid.hpp
    include/hle/service/nvdrv.hpp
    include/hle/service/set_sys.hpp
    include/hle/service/sm.hpp
    include/hle/service/time.hpp
    include/hle/service/vi.hpp
    include/loader/loader.hpp
    include/loader/nro.hpp
    include/nvidia/nvfence.hpp
    include/nvidia/nvfile.hpp
    include/nvidia/nvflinger.hpp
    include/nvidia/nvhost_ctrl.hpp
    include/nvidia/nvmap.hpp
    include/sys/cpu.hpp
    include/sys/emulator.hpp
    include/sys/memory.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE dynarmic plog)
