cmake_minimum_required(VERSION 3.13)
project(CPRg)

find_package(PkgConfig)

pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

get_filename_component(hw_proto "protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/helloworld.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND protoc
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="/usr/bin/grpc_cpp_plugin"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})

target_link_libraries(hw_grpc_proto PUBLIC ${GRPC_LIBRARIES} ${PROTOBUF_LIBRARIES})
target_include_directories(hw_grpc_proto PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" ${GRPC_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})

add_executable(client client.cpp)
target_link_libraries(client hw_grpc_proto)

add_executable(server server.cpp)
target_link_libraries(server hw_grpc_proto)
