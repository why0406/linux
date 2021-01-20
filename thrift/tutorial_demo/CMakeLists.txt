cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
include(ThriftCompiler)

#include_directories(SYSTEM "${Boost_INCLUDE_DIRS}")

#Make sure gen-cpp files can be included
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gen-cpp")
include_directories("${PROJECT_SOURCE_DIR}/lib/cpp/src")

set(tutorialgencpp_SOURCES
    gen-cpp/Calculator.cpp
    gen-cpp/SharedService.cpp
    gen-cpp/shared_constants.cpp
    gen-cpp/shared_types.cpp
    gen-cpp/tutorial_constants.cpp
    gen-cpp/tutorial_types.cpp
)
add_library(tutorialgencpp STATIC ${tutorialgencpp_SOURCES})
target_link_libraries(tutorialgencpp thrift)

add_custom_command(OUTPUT gen-cpp/Calculator.cpp gen-cpp/SharedService.cpp gen-cpp/shared_constants.cpp gen-cpp/shared_types.cpp gen-cpp/tutorial_constants.cpp gen-cpp/tutorial_types.cpp
    COMMAND ${THRIFT_COMPILER} --gen cpp -r ${PROJECT_SOURCE_DIR}/tutorial.thrift
)

add_executable(TutorialServer CppServer.cpp)
target_link_libraries(TutorialServer tutorialgencpp)
if (ZLIB_FOUND)
  target_link_libraries(TutorialServer ${ZLIB_LIBRARIES})
endif ()

add_executable(TutorialClient CppClient.cpp)
target_link_libraries(TutorialClient tutorialgencpp)
if (ZLIB_FOUND)
  target_link_libraries(TutorialClient ${ZLIB_LIBRARIES})
endif ()
