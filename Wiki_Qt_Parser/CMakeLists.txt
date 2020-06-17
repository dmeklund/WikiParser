cmake_minimum_required(VERSION 3.13)
project(WikiParser LANGUAGES CXX)
#enable_testing()

find_package(Boost 1.40 COMPONENTS iostreams filesystem REQUIRED)

set(SOURCE_FILES
        wiki_qt_parser.cpp
        wiki_qt_parser.h
        shared/CommonUtility.cpp
        shared/CommonUtility.h
)
include_directories(
        ${PROJECT_SOURCE_DIR}/shared
        ${Boost_INCLUDE_DIR}
)
add_executable(wiki_qt_parser ${SOURCE_FILES})
target_link_libraries(wiki_qt_parser ${Boost_LIBRARIES})
set_property(TARGET wiki_qt_parser PROPERTY CXX_STANDARD 17)
