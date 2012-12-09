project(wondruss)
cmake_minimum_required(VERSION 2.6)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if(GCC_VERSION VERSION_LESS 4.6)
        message(FATAL_ERROR "GCC version >= 4.6 required")
    endif()

    set(WARN_FLAGS "-Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_C_FLAGS "${WARN_FLAGS} ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-std=c++0x ${WARN_FLAGS} ${CMAKE_CXX_FLAGS}")
else()
  message(FATAL_ERROR "Your compiler is unsupported. For the steps to add a new compiler, ask Branan in IRC.")
endif()
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options)
find_package(Protobuf)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS proto/db.proto)

set(Wondruss_DEPS mongoclient ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${PROTOBUF_LIBRARIES} log4cplus)
set(COMMON_SRCS common/logger.cpp ${PROTO_SRCS} ${PROTO_HDRS})


add_executable(wondruss lobby/lobby.cpp lobby/auth_slave.cpp lobby/db_slave.cpp lobby/slave.cpp lobby/service_slave.cpp lobby.cpp ${COMMON_SRCS})
target_link_libraries(wondruss ${Wondruss_DEPS})

add_executable(wondruss_auth auth/authsrv.cpp auth.cpp ${COMMON_SRCS})
target_link_libraries(wondruss_auth ${Wondruss_DEPS})

add_executable(wondruss_db db/dbsrv.cpp db.cpp ${COMMON_SRCS})
target_link_libraries(wondruss_db ${Wondruss_DEPS})
