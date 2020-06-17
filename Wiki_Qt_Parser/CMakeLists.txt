cmake_minimum_required(VERSION 3.13)
project(WikiParser LANGUAGES CXX)
#enable_testing()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.40 COMPONENTS iostreams filesystem thread REQUIRED)

set(SOURCE_FILES
        main.cpp
        wiki_qt_parser.cpp
        wiki_qt_parser.h
        PageIndex.h
        PageIndex.cpp
        ThreadedParser.cpp
        ThreadedParser.h
        ThreadedWriter.cpp
        ThreadedWriter.h
        WpSavable.h
        WikipediaParser.h
        WikipediaParser.cpp
        shared/Common.h
        shared/Common.cpp
        shared/CommonUtility.cpp
        shared/CommonUtility.h
        shared/DizzyUtility.cpp
        shared/DizzyUtility.h
        shared/Savable.cpp
        shared/Savable.h
        shared/SimplestXml.h
        shared/SimplestXml.cpp
        shared/SimpleXml.cpp
        shared/SimpleXml.h
        shared/Timer.cpp
        shared/Timer.h
        shared/WordTrace.h
        shared/WordTrace.cpp
)
set(LIBRARIES
        re2
        pugixml
        ${CMAKE_DL_LIBS}
        ${Boost_LIBRARIES}
        Threads::Threads
        tidy
)
include_directories(
        ${PROJECT_SOURCE_DIR}/shared
        ${Boost_INCLUDE_DIR}
)
add_executable(wiki_qt_parser ${SOURCE_FILES})
target_link_libraries(wiki_qt_parser ${LIBRARIES})
set_property(TARGET wiki_qt_parser PROPERTY CXX_STANDARD 17)
