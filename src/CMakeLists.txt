cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

set(UNRELIABLEFS_SRC conf.c
                     unreliablefs.c
                     unreliablefs_errinj.c
                     unreliablefs_ops.c 
                     cppWrapper.cpp
                     grpc-client.cpp
                     )

# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")
# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# gRPC
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "./afs.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/afs.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/afs.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/afs.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/afs.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# custom_hw_grpc_proto
add_library(custom_hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(custom_hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

include(CheckFunctionExists)

add_definitions(${FUSE_DEFINITIONS})
add_executable(${PROJECT_NAME} ${UNRELIABLEFS_SRC}
  ${hw_proto_srcs}
  ${hw_grpc_srcs})
  
target_link_libraries(${PROJECT_NAME}
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  stdc++fs)
target_link_libraries(${PROJECT_NAME} gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES})
    
add_executable(grpc-server grpc-server.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs})
  target_link_libraries(grpc-server
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

target_include_directories(${PROJECT_NAME} PRIVATE ${FUSE_INCLUDE_DIRS})
include_directories(${BOOST_INCLUDE_DIRS})


target_compile_options(${PROJECT_NAME} PUBLIC -D_FILE_OFFSET_BITS=64 -Wall -Wextra -Wno-unused-parameter)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++11 -D_FILE_OFFSET_BITS=64>)

check_function_exists(fallocate HAVE_FALLOCATE)
check_function_exists(fallocate HAVE_FLOCK)
check_function_exists(utimensat HAVE_UTIMENSAT)
check_function_exists(setxattr HAVE_XATTR)
if (${HAVE_FALLOCATE})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_FALLOCATE)
endif ()
if (${HAVE_FLOCK})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_FLOCK)
endif ()
if (${HAVE_UTIMENSAT})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_UTIMENSAT)
endif ()
if (${HAVE_XATTR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_XATTR)
endif ()

# Tests that were used in the original repository
# add_subdirectory(./tests)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES "unreliablefs.1" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
install(FILES "unreliablefs.conf.5" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man5)
