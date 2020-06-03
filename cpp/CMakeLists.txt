project(http)
set(CMAKE_CXX_COMPILER ccache)
set(CMAKE_CXX_COMPILER_ARG1 g++)
add_compile_options(-std=c++1z -ggdb3)
#add_compile_options(-std=c++1z -O3)

aux_source_directory(common COMMON)
aux_source_directory(http HTTP)
aux_source_directory(util UTIL)

add_executable(csdn_https_client example/csdn_https_client.cpp ${COMMON} ${HTTP})
target_link_libraries(csdn_https_client pthread ssl crypto)

add_executable(report example/test.cpp ${COMMON} ${HTTP})
target_link_libraries(report pthread ssl crypto)

add_executable(http_server example/http_server.cpp ${COMMON} ${HTTP})
target_link_libraries(http_server pthread ssl crypto)

add_executable(http_client_hello example/http_client_hello.cpp ${COMMON} ${HTTP})
target_link_libraries(http_client_hello pthread ssl crypto)
