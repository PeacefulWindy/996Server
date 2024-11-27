cmake_minimum_required(VERSION 3.22)

project(996Server)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options("/utf-8")
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    link_directories(${VCPKG_INSTALL_ROOT}/debug/lib)
else()
    link_directories(${VCPKG_INSTALL_ROOT}/lib)
endif()

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${VCPKG_INSTALL_ROOT}/include)

aux_source_directory(src srcs)
aux_source_directory(src/lua srcs)
aux_source_directory(src/json srcs)
aux_source_directory(src/log srcs)
aux_source_directory(src/sql/mysql srcs)
aux_source_directory(src/socket srcs)
aux_source_directory(src/worker srcs)
aux_source_directory(src/service srcs)
aux_source_directory(src/service/msg srcs)
aux_source_directory(src/http/server srcs)
aux_source_directory(src/http/client srcs)
aux_source_directory(src/websocket/server srcs)
aux_source_directory(src/socket/tcp/server srcs)
aux_source_directory(src/socket/tcp/client srcs)
aux_source_directory(src/socket/tcp/session srcs)

find_package(lua REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(cJSON CONFIG REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ixwebsocket CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${srcs} ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    fmt::fmt
    cryptopp::cryptopp 
    lua 
    cjson 
    unofficial::libmariadb 
    protobuf::libprotoc 
    protobuf::libprotobuf 
    protobuf::libprotobuf-lite 
    ixwebsocket::ixwebsocket
    bcrypt
    asio::asio
)