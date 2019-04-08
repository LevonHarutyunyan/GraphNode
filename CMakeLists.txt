cmake_minimum_required (VERSION 3.10.2)

project (GraphNode)

file(GLOB SOURCES main.cpp GraphNode.cpp)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.67.0 COMPONENTS regex thread system date_time coroutine graph) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(${PROJECT_NAME} ${SOURCES}) 
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
