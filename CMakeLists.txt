cmake_minimum_required (VERSION 3.10.2)

project (GraphNode)

file(GLOB SOURCES PrivateKey.cpp PublicKey.cpp 
				GraphNode.cpp HDWallet.cpp Transaction.cpp main.cpp)

set(MY_SOURCE_DIR "~/Desktop/GraphNode")

include_directories(${MY_SOURCE_DIR}/third_parties/secp256k1/include)
include_directories(${MY_SOURCE_DIR}/include)
include_directories(/usr/local/ssl/include)
include_directories(${GTEST_INCLUDE_DIRS})


set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 
set(OPENSSL_USE_STATIC_LIBS ON) 

find_package(Boost COMPONENTS regex thread system date_time coroutine graph serialization) 
find_package(GTest REQUIRED)


if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(${PROJECT_NAME} ${SOURCES}) 

    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
	
	target_link_libraries(${PROJECT_NAME} /usr/local/lib/libsecp256k1.so)
	target_link_libraries(${PROJECT_NAME} /usr/local/ssl/lib/libssl.so)
    target_link_libraries(${PROJECT_NAME} /usr/local/ssl/lib/libcrypto.so)
	
	target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} pthread)
endif()

