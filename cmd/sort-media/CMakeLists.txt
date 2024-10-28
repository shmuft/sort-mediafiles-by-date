cmake_minimum_required(VERSION 3.0)
project(sort-images-go)

set(TARGET sort-images-go)
set(GOPATH ${CMAKE_CURRENT_BINARY_DIR})

set(SRCS main.go)
set(BINARY sort-media.exe)

add_dependencies(win-dep-install ${TARGET})

add_custom_command(
    OUTPUT ${BINARY}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND env GOPATH=${GOPATH} go fmt && go install && go build -o "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${SRCS}
    COMMENT "Building Go binary"
)

add_custom_target(${TARGET} ALL DEPENDS ${BINARY})
target_sources(${TARGET} PRIVATE ${SRCS})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${BINARY}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
