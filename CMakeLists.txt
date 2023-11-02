cmake_minimum_required(VERSION 3.2)

project(hl_server C CXX)

SET (EXAMPLE_BINARY "app")
SET (OTHER_SERVICE "other_service")
SET (USER_WRITER_SERVICE "user_writer_service")
set (STD_CXX "c++17")
set (REDISCPP_FLAGS "-DREDISCPP_HEADER_ONLY=ON")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=${STD_CXX} ${REDISCPP_FLAGS}")

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(CppKafka REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${EXAMPLE_BINARY} main.cpp 
                                 config/config.cpp 
                                 database/database.cpp
                                 database/user.cpp
                                 database/cache.cpp)


target_include_directories(${EXAMPLE_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${EXAMPLE_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${EXAMPLE_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${EXAMPLE_BINARY} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             "rdkafka++"
                             CppKafka::cppkafka
                             ZLIB::ZLIB)


add_executable(${USER_WRITER_SERVICE} user_writer_service/main.cpp 
                                      config/config.cpp 
                                      database/database.cpp
                                      database/user.cpp
                                      database/cache.cpp)


target_include_directories(${USER_WRITER_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${USER_WRITER_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${USER_WRITER_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${USER_WRITER_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${USER_WRITER_SERVICE} PRIVATE 
                         ${CMAKE_THREAD_LIBS_INIT} 
                         ${Poco_LIBRARIES}
                         "PocoData"
                         "PocoDataMySQL"
                         "mysqlclient"
                         "rdkafka++"
                         CppKafka::cppkafka
                         ZLIB::ZLIB)




add_executable(${OTHER_SERVICE} other_main.cpp
                                config/config.cpp
                                database/database.cpp
                                database/message.cpp
                                database/group.cpp)
target_include_directories(${OTHER_SERVICE} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${OTHER_SERVICE} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${OTHER_SERVICE} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${OTHER_SERVICE} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${OTHER_SERVICE} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "rdkafka++"
                             "mysqlclient"
                             ZLIB::ZLIB)





