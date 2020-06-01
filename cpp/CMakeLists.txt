project(http)
add_compile_options(-std=c++1z -ggdb3)
#add_compile_options(-std=c++1z -O3)

aux_source_directory(common COMMON)
aux_source_directory(http HTTP)
aux_source_directory(util UTIL)

add_executable(csdn example/csdn.cpp ${COMMON} ${HTTP})
target_link_libraries(csdn pthread ssl crypto)

add_executable(report example/test.cpp ${COMMON} ${HTTP})
target_link_libraries(report pthread ssl crypto)
