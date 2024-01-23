cmake_minimum_required (VERSION 3.8)
project(ProxyClient)

set(CMAKE_CXX_STANDARD 17)

if(NOT ${CMAKE_SYSTEM} MATCHES Windows)
	message(FATAL_ERROR "Usupported system.")
endif()

# Libraries
add_subdirectory(libs/argparse)
add_subdirectory(libs/minhook)
add_subdirectory(libs/spdlog)

# Projects
add_subdirectory(common)
add_subdirectory(winpipe)
add_subdirectory(redirector)
add_subdirectory(client)
