cmake_minimum_required(VERSION 3.5)

project(sort-media-gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Install directory
set(CMAKE_INSTALL_MESSAGE LAZY)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output" CACHE PATH "..." FORCE)
endif()

# Output directory
if(CMAKE_CONFIGURATION_TYPES)
    set(CUSTOM_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtime/$<CONFIG>/bin)
    set(DEPENDENCY_INSTALL_DIRECTORY ${CMAKE_BINARY_DIR}/runtime/$<CONFIG>)
else()
    set(CUSTOM_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(DEPENDENCY_INSTALL_DIRECTORY ${CMAKE_BINARY_DIR})
endif()


set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(sort-media-gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET sort-media-gui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(sort-media-gui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(sort-media-gui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(sort-media-gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.sort-media-gui)
endif()
set_target_properties(sort-media-gui PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(sort-media-gui)
endif()

install(TARGETS sort-media-gui
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(Qtx Qt${QT_VERSION_MAJOR})

if(${Qtx}_FOUND AND WIN32 AND TARGET "${Qtx}::qmake" AND NOT TARGET "${Qtx}::windeployqt")
    get_target_property(QMAKE_LOCATION ${Qtx}::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${QMAKE_LOCATION}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE QT_INSTALL_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(Windeployqt_POSSIBLE_LOCATION "${QT_INSTALL_PREFIX}/bin/windeployqt.exe")

    if(EXISTS ${Windeployqt_POSSIBLE_LOCATION})
        set(Windeployqt_LOCATION ${Windeployqt_POSSIBLE_LOCATION})

        add_executable(Qt::windeployqt IMPORTED)
        set_target_properties(Qt::windeployqt PROPERTIES
            IMPORTED_LOCATION ${Windeployqt_LOCATION}
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Windeployqt
    REQUIRED_VARS Windeployqt_LOCATION
)

if(TARGET "${Qtx}::qmake")
    get_target_property(QMAKE_LOCATION ${Qtx}::qmake IMPORTED_LOCATION)

    function(query_qmake PROPERTY)
        execute_process(
            COMMAND "${QMAKE_LOCATION}" -query ${PROPERTY}
            RESULT_VARIABLE QMAKE_RESULT
            ERROR_VARIABLE QMAKE_ERROR
            OUTPUT_VARIABLE ${PROPERTY}
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(NOT ${QMAKE_RESULT} EQUAL 0)
            message (FATAL_ERROR "QMake query failed ${QMAKE_ERROR}")
        endif()
        set(${PROPERTY} ${${PROPERTY}} PARENT_SCOPE)
    endfunction()

    query_qmake(QT_INSTALL_PREFIX)
    query_qmake(QT_INSTALL_PLUGINS)
    query_qmake(QT_INSTALL_BINS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtDirs
    REQUIRED_VARS
        QT_INSTALL_PREFIX
        QT_INSTALL_PLUGINS
        QT_INSTALL_BINS
)

set(SUF $<$<CONFIG:Debug>:d>)

install(FILES
    "${QT_INSTALL_BINS}/${Qtx}Core${SUF}.dll"
    "${QT_INSTALL_BINS}/${Qtx}Gui${SUF}.dll"
    "${QT_INSTALL_BINS}/${Qtx}Widgets${SUF}.dll"

    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    "${QT_INSTALL_PLUGINS}/platforms/qwindows${SUF}.dll"
    DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins/platforms
)
