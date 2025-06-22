cmake_minimum_required(VERSION 3.0)
project(sort-images LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)

# Добавить поддержку сборки golang
find_program(GO_EXECUTABLE go)
if(NOT GO_EXECUTABLE)
    message(FATAL_ERROR "Go compiler not found!")
endif()
set(GO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cmd/sort-media)
set(GO_OUTPUT ${CMAKE_BINARY_DIR}/bin/sort-media.exe)

add_custom_target(sort-media-go ALL
    COMMAND ${GO_EXECUTABLE} build -o ${GO_OUTPUT} ${GO_SOURCE_DIR}
    WORKING_DIRECTORY ${GO_SOURCE_DIR}
    COMMENT "Building Go project sort-media"
)

add_custom_target(
    win-dep-install ALL
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${CMAKE_BINARY_DIR} --config $<CONFIG>
)

add_subdirectory(cmd/sort-media)
add_subdirectory(gui/sort-media-gui)
