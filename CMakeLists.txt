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

macro(ADD_PCH_RULE  _header_filename _pch_variable)
    set(_gch_filename "${_header_filename}.gch")
    list(APPEND ${_pch_variable} ${_gch_filename})
    set(_args ${CMAKE_CXX_FLAGS})
    list(APPEND _args -c ${_header_filename} -o ${_gch_filename})
    get_directory_property(DIRINC INCLUDE_DIRECTORIES)
    foreach(_inc ${DIRINC})
        list(APPEND _args "-I" ${_inc})
    endforeach(_inc ${DIRINC})
    separate_arguments(_args)
    add_custom_command(OUTPUT ${_gch_filename}
                       COMMAND rm -f ${_gch_filename}
                       COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
                       DEPENDS ${_header_filename})
endmacro()

else()
  message(FATAL_ERROR "Your compiler is unsupported. For the steps to add a new compiler, ask Branan in IRC.")
endif()
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

find_package(Threads REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/asio")
include_directories("${CMAKE_SOURCE_DIR}")
add_pch_rule("${CMAKE_SOURCE_DIR}/asio/asio.hpp" ASIO_PCH)
add_executable(wondruss lobby/lobby.cpp lobby/auth_slave.cpp lobby.cpp ${ASIO_PCH})
add_executable(wondruss_auth auth/authsrv.cpp auth.cpp ${ASIO_PCH})
target_link_libraries(wondruss_auth ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(wondruss ${CMAKE_THREAD_LIBS_INIT})