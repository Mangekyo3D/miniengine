cmake_minimum_required(VERSION 3.10)

project("Space Pirates")

set (CMAKE_CXX_STANDARD 14)

add_subdirectory(src)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/audio DESTINATION ${CMAKE_BINARY_DIR}/bin)
