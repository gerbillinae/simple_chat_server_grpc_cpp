cmake_minimum_required(VERSION 3.29)
project(protobuf_minimal)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

message("CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

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

add_executable(server)
target_sources(server PRIVATE server.cpp)
target_link_libraries(server GrpcService)

add_executable(client)
target_sources(client PRIVATE client.cpp)
target_link_libraries(client GrpcService)


