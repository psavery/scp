project(SCP)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${SCP_SOURCE_DIR}/cmake/modules)

set(SCP_SRCS
    src/main.c
    src/scp.c
    src/passwordPrompt.c
    src/connectSSH.c
    src/loadBar.c
    src/sshUtils.c
    src/fileSystemUtils.c)

include_directories(${SCP_SOURCE_DIR}/include)

find_package(LibSSH REQUIRED)
if(NOT LIBSSH_FOUND)
  message(FATAL_ERROR "libssh not found!")
endif()

# Set -fPIC on x86_64
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC"  )
endif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")

add_executable(scp ${SCP_SRCS})
target_link_libraries(scp ${LIBSSH_LIBRARIES})


