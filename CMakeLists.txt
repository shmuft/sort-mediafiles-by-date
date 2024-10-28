cmake_minimum_required(VERSION 3.0)
project(sort-images LANGUAGES CXX)

add_custom_target(
    win-dep-install ALL
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_BINARY_DIR} --config $<CONFIG>
)

add_subdirectory(cmd/sort-media)
add_subdirectory(gui/sort-media-gui)
