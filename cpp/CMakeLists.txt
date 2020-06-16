project(http)
set(CMAKE_CXX_COMPILER ccache)
set(CMAKE_CXX_COMPILER_ARG1 g++)
add_compile_options(
    -std=c++1z
    -ggdb3
    -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    )
#add_compile_options(-std=c++1z -O3)

aux_source_directory(common COMMON)
aux_source_directory(curl_http CURLHTTP)
aux_source_directory(util UTIL)
aux_source_directory(example EXAMPLE)

message(STATUS ${CMAKE_SOURCE_DIR})
message(STATUS "${PROJECT_NAME}")
message(STATUS ${PROJECT_PATH})

include_directories(
    ${CMAKE_SOURCE_DIR}
    )

#add_executable(csdn_https_client example/csdn_https_client.cpp ${COMMON} ${HTTP})
#target_link_libraries(csdn_https_client pthread ssl crypto)

add_executable(curl_csdn_client example/curl_csdn_client.cpp ${COMMON} ${CURLHTTP} ${UTIL})
target_link_libraries(curl_csdn_client curl pthread boost_filesystem)
target_include_directories(curl_csdn_client PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(curl_file example/curl_file.cpp)
target_link_libraries(curl_file curl pthread)

#add_executable(report example/test.cpp ${COMMON} ${HTTP})
#target_link_libraries(report pthread ssl crypto)
#
#add_executable(http_server example/http_server.cpp ${COMMON} ${HTTP})
#target_link_libraries(http_server pthread ssl crypto)
#
#add_executable(http_client_hello example/http_client_hello.cpp ${COMMON} ${HTTP})
#target_link_libraries(http_client_hello pthread ssl crypto)
