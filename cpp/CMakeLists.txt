project(http)
add_compile_options(-std=c++2a -ggdb3)

aux_source_directory(common COMMON)
aux_source_directory(http HTTP)
aux_source_directory(util UTIL)

add_executable(csdn example/csdn.cpp ${COMMON} ${HTTP})
target_link_libraries(csdn pthread ssl crypto)
