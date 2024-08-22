cmake_minimum_required(VERSION 3.29)
project(protobuf_minimal)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

set(DEPLOY_DIR ${CMAKE_SOURCE_DIR}/deploy)

message("CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# Proto files
file(GLOB PROTO_FILES ./*.proto)

# Add Library target with protobuf sources
add_library(GrpcService ${PROTO_FILES})

target_link_libraries(GrpcService
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

target_include_directories(GrpcService PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "GrpcService CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")

# Compile protobuf and grpc files in GrpcService target to cpp
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET GrpcService LANGUAGE cpp)
protobuf_generate(TARGET GrpcService LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

add_subdirectory(utility)
add_subdirectory(tls_keys)

add_executable(server)
target_sources(server PRIVATE server.cpp)
target_link_libraries(server GrpcService utility unofficial::sqlite3::sqlite3)
add_dependencies(server tls_keys)

if(DEFINED DEPLOY_DIR)
    add_custom_command(TARGET server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:server> ${DEPLOY_DIR})
endif()

add_executable(client)
target_sources(client PRIVATE client.cpp)
target_link_libraries(client GrpcService utility)

add_dependencies(client tls_keys)

if(DEFINED DEPLOY_DIR)
add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:client> ${DEPLOY_DIR})
endif()

add_custom_target(project ALL DEPENDS server client)