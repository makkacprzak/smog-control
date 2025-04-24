cmake_minimum_required(VERSION 3.16)

project(smog-control VERSION 0.16 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts)
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Copy dictionary file to program file
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/dictionary.json
  ${CMAKE_CURRENT_BINARY_DIR}/dictionary.json
  COPYONLY)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(smog-control
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        fetchdata.cpp
        jfile.h jfile.cpp
        fetchdata.h
        translate.h translate.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET smog-control APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(smog-control SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(smog-control
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(smog-control PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(smog-control PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(smog-control PRIVATE ${CURL_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.smog-control)
endif()
set_target_properties(smog-control PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS smog-control
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(smog-control)
endif()
