cmake_minimum_required(VERSION 3.0.0)
project(custom-AFS-project VERSION 0.1.0)


# include(./script/PrintVariables.cmake) # log cmake variables
# include(CTest)
# enable_testing()

# add_executable(${PROJECT_NAME} ./src/server/main.cpp)
# include(${CMAKE_CURRENT_SOURCE_DIR}/src/server/CMakeLists.txt)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependency/unreliablefs/)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client/)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server/)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# specify the output directories on a per-target basis:
# set_target_properties( targets...
#     PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
# )

