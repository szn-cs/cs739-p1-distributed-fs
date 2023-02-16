cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)

## When using vcpkg as a submodule of your project, you can add the following to your CMakeLists.txt before the first project() call, instead of passing CMAKE_TOOLCHAIN_FILE to the cmake invocation. This will still allow people to not use vcpkg, by passing the CMAKE_TOOLCHAIN_FILE directly, but it will make the configure-build step slightly easier.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dependency/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project (unreliablefs 
          VERSION 1.0.0
          LANGUAGES C CXX
         DESCRIPTION "A FUSE-based fault injection filesystem.")

# debug
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config/cmake;${CMAKE_MODULE_PATH}")


find_package(gRPC CONFIG REQUIRED)
# # note: 7 additional targets are not displayed.
# target_link_libraries(main PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

# include(./config/PrintVariables.cmake) # log cmake variables
# include(CTest)
# enable_testing()


# add_executable(${PROJECT_NAME} ./src/server/main.cpp)
# include(${CMAKE_CURRENT_SOURCE_DIR}/src/server/CMakeLists.txt)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/) # server & client directory
# add example grpc code: 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependency/grpc/examples/cpp/helloworld)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# specify the output directories on a per-target basis:
# set_target_properties( targets...
#     PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )


# mainly from unreliablefs project
option(ENABLE_ASAN "Enable AddressSanitizer, a fast memory error detector
                    based on compiler instrumentation" OFF)
if(ENABLE_ASAN)
    target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
endif()

option(ENABLE_UBSAN "Enable UBSanitizer, undefined behaviour
                     detector based on compiler instrumentation" OFF)
if(ENABLE_UBSAN)
    target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=undefined)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=undefined)
endif()

add_custom_target(check DEPENDS check-mandoc check-rockspec)

set(ROCKSPEC "unreliablefs-scm-1.rockspec")
add_custom_target(check-rockspec DEPENDS ${ROCKSPEC})
add_custom_command(TARGET check-rockspec
                   COMMAND luarocks --local build ${PROJECT_SOURCE_DIR}/${ROCKSPEC}
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                   COMMENT "Build Lua rockspec")

set(MANUAL_PAGES "unreliablefs/unreliablefs.1;unreliablefs/unreliablefs.conf.5")
add_custom_target(check-mandoc DEPENDS ${MANUAL_PAGES})
add_custom_command(TARGET check-mandoc
                   COMMAND mandoc -T lint -W warning,stop ${MANUAL_PAGES}
                   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                   COMMENT "Run mandoc static analysis")

