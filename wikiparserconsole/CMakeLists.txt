cmake_minimum_required(VERSION 3.17)
project(wikiparserconsole)

set(CMAKE_CXX_STANDARD 20)
find_package(pugixml CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_library(TIDY_LIBRARY tidys)

add_executable(wikiparserconsole main.cpp)
target_link_libraries(
        wikiparserconsole
        PRIVATE
        re2::re2
        pugixml
        ${TIDY_LIBRARY}
)
