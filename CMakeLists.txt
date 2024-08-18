cmake_minimum_required(VERSION 3.15)
project(bot_discord VERSION 1.0.0)

option(REPLIT "run this code on replit" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DPP_CORO ON)

# Vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND EXISTS $ENV{VCPKG_ROOT})
    message("Found vcpkg at $ENV{VCPKG_ROOT}")
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Define a function to handle FetchContent for a library
function(fetch_content_if_not_found LIB_NAME GIT_REPO TARGET_NAME)
    if(NOT ${LIB_NAME}_FOUND)
        include(FetchContent)
        set(${LIB_NAME}_SOURCE_DIR "${CMAKE_BINARY_DIR}/3rdlib/${LIB_NAME}")
        set(${LIB_NAME}_BINARY_DIR "${CMAKE_BINARY_DIR}/build/${LIB_NAME}")
        if(NOT EXISTS ${${LIB_NAME}_SOURCE_DIR})
            message(STATUS "${LIB_NAME} dir is not found, starting install and build from repository...")
            FetchContent_Declare(
                ${LIB_NAME}
                GIT_REPOSITORY ${GIT_REPO}
                SOURCE_DIR ${${LIB_NAME}_SOURCE_DIR}
                BINARY_DIR ${${LIB_NAME}_BINARY_DIR}
            )
            FetchContent_MakeAvailable(${LIB_NAME})
        else()
            message(STATUS "${LIB_NAME} is found and not installed, installing from source...")
            add_subdirectory(${${LIB_NAME}_SOURCE_DIR} ${${LIB_NAME}_BINARY_DIR})
        endif()
        target_link_libraries(bot_discord PRIVATE ${TARGET_NAME})
    else()
        target_link_libraries(bot_discord PRIVATE ${LIB_NAME}::${TARGET_NAME})
    endif()
endfunction()

# Find required packages
find_package(OpenSSL CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-libmysql CONFIG REQUIRED)
find_package(Threads REQUIRED)

# Define the executable target
add_executable(bot_discord bot.cpp)

# Add include directories
target_include_directories(bot_discord PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add compile definitions
target_compile_definitions(bot_discord PRIVATE ASIO_STANDALONE)

# Fetch and link required libraries if not found
fetch_content_if_not_found(dpp "https://github.com/brainboxdotcc/DPP.git" dpp)
fetch_content_if_not_found(CURL "https://github.com/curl/curl.git" libcurl)
fetch_content_if_not_found(OpenSSL "https://github.com/openssl/openssl.git" OpenSSL)
fetch_content_if_not_found(fmt "https://github.com/fmtlib/fmt.git" fmt-header-only)
fetch_content_if_not_found(unofficial-libmysql "https://github.com/mysql/mysql-server.git" libmysql)

# Link threads library
target_link_libraries(bot_discord PRIVATE ${CMAKE_THREAD_LIBS_INIT})