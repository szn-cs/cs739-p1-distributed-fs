cmake_minimum_required(VERSION 3.0.0)
project(custom-AFS-project VERSION 0.1.0)

# include(CTest)
# enable_testing()

# add_executable(${PROJECT_NAME} ./src/server/main.cpp)
# include(${CMAKE_CURRENT_SOURCE_DIR}/src/server/CMakeLists.txt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/server)
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

