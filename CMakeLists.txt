cmake_minimum_required(VERSION 3.5)

project("Space Pirates")

set (CMAKE_CXX_STANDARD 14)

add_subdirectory(src)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR}/bin)
