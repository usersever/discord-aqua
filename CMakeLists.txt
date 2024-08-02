cmake_minimum_required(VERSION 3.15)
project(bot_discord)

option(REPLIT "run this code on replit" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DPP_CORO ON)

# Vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS $ENV{VCPKG_ROOT})
  set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Find required packages
find_package(OpenSSL QUIET)
find_package(CURL QUIET)
find_package(dpp QUIET)
find_package(fmt QUIET)
find_package(unofficial-libmysql QUIET)
find_package(Threads REQUIRED)

# Define the executable target
add_executable(bot_discord bot.cpp)

# Add include directories
target_include_directories(bot_discord PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add compile definitions
target_compile_definitions(bot_discord PRIVATE ASIO_STANDALONE)

if(NOT dpp_FOUND)
	include(FetchContent)
	set(DPP_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/DPP")
    set(DPP_BINARY_DIR "${CMAKE_BINARY_DIR}/build/DPP")
	if(NOT EXISTS ${DPP_SOURCE_DIR})
		message(STATUS "DPP dir is not found, starting install and build from repository...")
		FetchContent_Declare(
			dpp
			GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git 
			SOURCE_DIR ${DPP_SOURCE_DIR} 
			BINARY_DIR ${DPP_BINARY_DIR})
		FetchContent_MakeAvailable(dpp)
	else()
		message(STATUS "DPP is found and not installed, installing from source...")	
		add_subdirectory(${DPP_SOURCE_DIR} ${DPP_BINARY_DIR})
	endif()
	target_link_libraries(bot_discord PRIVATE dpp)
else()
	target_link_libraries(bot_discord PRIVATE dpp::dpp)
endif()

if(NOT CURL_FOUND)
	include(FetchContent)
	set(CURL_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/CURL")
    set(CURL_BINARY_DIR "${CMAKE_BINARY_DIR}/build/CURL")
	if(NOT EXISTS ${CURL_SOURCE_DIR})
		message(STATUS "CURL dir is not found, starting install and build from repository...")
		FetchContent_Declare(
			CURL
			GIT_REPOSITORY https://github.com/curl/curl.git
			SOURCE_DIR ${CURL_SOURCE_DIR} 
			BINARY_DIR ${CURL_BINARY_DIR})
		FetchContent_MakeAvailable(CURL)
	else()
		message(STATUS "CURL is found and not installed, installing from source...")	
		add_subdirectory(${CURL_SOURCE_DIR} ${CURL_BINARY_DIR})
	endif()
	target_link_libraries(bot_discord PRIVATE CURL)
else()
	target_link_libraries(bot_discord PRIVATE CURL::libcurl)
endif()

if(NOT OpenSSL_FOUND)
	include(FetchContent)
	set(OpenSSL_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/OpenSSL")
    set(OpenSSL_BINARY_DIR "${CMAKE_BINARY_DIR}/build/OpenSSL")
	if(NOT EXISTS ${OpenSSL_SOURCE_DIR})
		message(STATUS "OpenSSL dir is not found, starting install and build from repository...")
		FetchContent_Declare(
			OpenSSL
			GIT_REPOSITORY https://github.com/openssl/openssl.git
			SOURCE_DIR ${OpenSSL_SOURCE_DIR} 
			BINARY_DIR ${OpenSSL_BINARY_DIR})
		FetchContent_MakeAvailable(OpenSSL)
	else()
		message(STATUS "OpenSSL is found and not installed, installing from source...")	
		add_subdirectory(${OpenSSL_SOURCE_DIR} ${OpenSSL_BINARY_DIR})
	endif()
	target_link_libraries(bot_discord PRIVATE OpenSSL)
else()
	target_link_libraries(bot_discord PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(NOT fmt_FOUND)
	include(FetchContent)
	set(fmt_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/fmt")
    set(fmt_BINARY_DIR "${CMAKE_BINARY_DIR}/build/fmt")
	if(NOT EXISTS ${fmt_SOURCE_DIR})
		message(STATUS "fmt dir is not found, starting install and build from repository...")
		FetchContent_Declare(
			fmt
			GIT_REPOSITORY https://github.com/fmtlib/fmt.git
			SOURCE_DIR ${fmt_SOURCE_DIR} 
			BINARY_DIR ${fmt_BINARY_DIR})
		FetchContent_MakeAvailable(fmt)
	else()
		message(STATUS "fmt is found and not installed, installing from source...")	
		add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
	endif()
	target_link_libraries(bot_discord PRIVATE fmt)
else()
	target_link_libraries(bot_discord PRIVATE fmt::fmt-header-only)
endif()

if(NOT unofficial-libmysql_FOUND)
	include(FetchContent)
	set(mysql_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/mysql")
    set(mysql_BINARY_DIR "${CMAKE_BINARY_DIR}/build/mysql")
	if(NOT EXISTS ${mysql_SOURCE_DIR})
		message(STATUS "mysql dir is not found, starting install and build from repository...")
		FetchContent_Declare(
			mysql
			GIT_REPOSITORY https://github.com/mysql/mysql-server.git
			SOURCE_DIR ${mysql_SOURCE_DIR} 
			BINARY_DIR ${mysql_BINARY_DIR})
		FetchContent_MakeAvailable(mysql)
	else()
		message(STATUS "mysql is found and not installed, installing from source...")	
		add_subdirectory(${mysql_SOURCE_DIR} ${mysql_BINARY_DIR})
	endif()
	target_link_libraries(bot_discord PRIVATE mysql)
else()
	target_link_libraries(bot_discord PRIVATE unofficial::libmysql::libmysql)
endif()

target_link_libraries(bot_discord PRIVATE ${CMAKE_THREAD_LIBS_INIT})
