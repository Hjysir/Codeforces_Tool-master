cmake_minimum_required(VERSION 3.25)

project(Project)

set(CMAKE_CXX_STANDARD 23)

# windows 链接curl
set(CURL_INCLUDE_DIR "curl/include")
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
link_libraries(${CURL_LIBRARY})

add_executable(Project main.cpp CodeforcesAPI.cpp CodeforcesAPI.h cJSON/cJSON.h cJSON/cJSON.c)

