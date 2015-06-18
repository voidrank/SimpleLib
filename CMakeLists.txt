cmake_minimum_required(VERSION 2.8)

include_directories(${Boost_INCLUDE_DIR})
find_package(Boost)
find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Boost COMPONENTS random REQUIRED)
find_package(Threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

add_executable(app app.cpp)
target_link_libraries(app
    ${Boost_LIBARARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_RANDOM_LIBRARY}
    )
