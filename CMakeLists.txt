cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)

## When using vcpkg as a submodule of your project, you can add the following to your CMakeLists.txt before the first project() call, instead of passing CMAKE_TOOLCHAIN_FILE to the cmake invocation. This will still allow people to not use vcpkg, by passing the CMAKE_TOOLCHAIN_FILE directly, but it will make the configure-build step slightly easier.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dependency/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(custom-AFS-project VERSION 0.1.0)

find_package(gRPC CONFIG REQUIRED)
# # note: 7 additional targets are not displayed.
# target_link_libraries(main PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

# include(./script/PrintVariables.cmake) # log cmake variables
# include(CTest)
# enable_testing()


# add_executable(${PROJECT_NAME} ./src/server/main.cpp)
# include(${CMAKE_CURRENT_SOURCE_DIR}/src/server/CMakeLists.txt)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependency/unreliablefs/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server/)
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

