cmake_minimum_required(VERSION 3.11)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
    # Touch files extracted from downloaded archives
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

# Minimum macOS version
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target")
# Visual Studio: Enable parallel builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
# Visual Studio: UTF-8 source and execution encoding, invalid UTF-8 in sources is an error
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8;/we4828>")
# Visual Studio: Preselect qolibri as the solution's startup project
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT qolibri)
# Copyright info, compiled into application metadata on some platforms
set(QOLIBRI_COPYRIGHT_YEARS "2007-2024")
set(QOLIBRI_AUTHORS "BOP et al.")

project(qolibri VERSION 2.1.4)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(QOLIBRI_QT5 OFF CACHE BOOL "Use Qt 5, don't try to find Qt 6 first")
if(NOT QOLIBRI_QT5)
    find_package(Qt6 6.2 COMPONENTS Core5Compat LinguistTools Multimedia Network WebEngineCore WebEngineWidgets Widgets)
endif()
if(NOT Qt6_FOUND)
    if("${QT_DEFAULT_MAJOR_VERSION}")
        message(FATAL_ERROR
            "Qt 6 was found, but it didn't include all required components. To "
            "proceed with Qt 6, install the components reported missing above. "
            "To ignore the Qt 6 installation and build against Qt 5, configure "
            "with -DQOLIBRI_QT5=ON in a fresh build directory.")
    endif()
    find_package(Qt5 5.15 COMPONENTS LinguistTools Multimedia Network WebEngine WebEngineWidgets Widgets REQUIRED)
endif()

add_executable(qolibri MACOSX_BUNDLE WIN32
    images/qolibri.icns
    qolibri.qrc
    src/allpage.cpp
    src/allpage.h
    src/book.cpp
    src/book.h
    src/bookbrowser.cpp
    src/bookbrowser.h
    src/bookbrowserpopup.cpp
    src/bookbrowserpopup.h
    src/booksetting.cpp
    src/booksetting.h
    src/bookview.cpp
    src/bookview.h
    src/bookwidget.cpp
    src/bookwidget.h
    src/browsefileedit.cpp
    src/browsefileedit.h
    src/client.h
    src/clipboardoptionswidget.cpp
    src/clipboardoptionswidget.h
    src/clipboardoptionswidget.ui
    src/configure.cpp
    src/configure.h
    src/ebcache.cpp
    src/ebcache.h
    src/ebcore.cpp
    src/ebcore.h
    src/ebhook.cpp
    src/ebook.cpp
    src/ebook.h
    src/fontsetting.cpp
    src/fontsetting.h
    src/globaleventfilter.cpp
    src/globaleventfilter.h
    src/groupdock.cpp
    src/groupdock.h
    src/groupwidget.cpp
    src/groupwidget.h
    src/infopage.cpp
    src/infopage.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/menupage.cpp
    src/menupage.h
    src/method.cpp
    src/method.h
    src/model.cpp
    src/model.h
    src/optiondialog.cpp
    src/optiondialog.h
    src/optiondialog.ui
    src/pageitems.cpp
    src/pageitems.h
    src/pagewidget.cpp
    src/pagewidget.h
    src/qeb.cpp
    src/qeb.h
    src/qolibri.cpp
    src/referencepopup.cpp
    src/referencepopup.h
    src/searchpage.cpp
    src/searchpage.h
    src/searchwholepage.cpp
    src/searchwholepage.h
    src/server.h
    src/settings.cpp
    src/settings.h
    src/ssheetsetting.cpp
    src/ssheetsetting.h
    src/toolbar.cpp
    src/toolbar.h
    src/treescrollpopup.cpp
    src/treescrollpopup.h
    src/webpage.cpp
    src/webpage.h
    src/webview.cpp
    src/webview.h
)

set(QOLIBRI_USE_SYSTEM_EB OFF CACHE BOOL "Use shared EB library provided by the OS")
set(QOLIBRI_EB_SOURCE_DIR "" CACHE STRING "Path to cmake-enabled EB sources")
set(QOLIBRI_EB_SOURCE_URL "https://github.com/mvf/eb/archive/qolibri.tar.gz" CACHE STRING "URL to cmake-enabled EB source tarball")
set(EB_ZLIB_VERSION "1.3.1" CACHE STRING "ZLIB version when building from source")

if(QOLIBRI_USE_SYSTEM_EB)
    message(STATUS "Using system EB library")
    find_library(EB_LIBRARY eb REQUIRED)
    message(STATUS "Found EB library: ${EB_LIBRARY}")
    add_library(eb SHARED IMPORTED)
    if(WIN32)
        set_target_properties(eb PROPERTIES IMPORTED_IMPLIB "${EB_LIBRARY}")
    else()
        set_target_properties(eb PROPERTIES IMPORTED_LOCATION "${EB_LIBRARY}")
    endif()
    add_library(eb::eb ALIAS eb)
elseif(QOLIBRI_EB_SOURCE_DIR)
    message(STATUS "Using EB library from ${QOLIBRI_EB_SOURCE_DIR}")
    add_subdirectory("${QOLIBRI_EB_SOURCE_DIR}" eb)
else()
    message(STATUS "Using EB sources from ${QOLIBRI_EB_SOURCE_URL}")
    include(FetchContent)
    FetchContent_Declare(eb URL "${QOLIBRI_EB_SOURCE_URL}")
    FetchContent_MakeAvailable(eb)
endif()

target_compile_definitions(qolibri PRIVATE QT_USE_QSTRINGBUILDER)
target_include_directories(qolibri PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(QOLIBRI_FIXED_POPUP OFF CACHE BOOL "Show popups fixed at window height")
if (${QOLIBRI_FIXED_POPUP})
    target_compile_definitions(qolibri PRIVATE FIXED_POPUP)
endif()

if ("${QOLIBRI_COMMIT_DATE}" STREQUAL "")
    execute_process(COMMAND git log -1 --format=%ci WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE QOLIBRI_COMMIT_DATE)
    string(REGEX REPLACE "(\r?\n)+$" "" QOLIBRI_COMMIT_DATE "${QOLIBRI_COMMIT_DATE}")
    set(QOLIBRI_COMMIT_DATE "${QOLIBRI_COMMIT_DATE}" CACHE INTERNAL "HEAD's commit date in ISO format")
endif()
target_compile_definitions(qolibri PRIVATE QOLIBRI_GIT_COMMIT_DATE="${QOLIBRI_COMMIT_DATE}")

if ("${QOLIBRI_VERSION_STR}" STREQUAL "")
    execute_process(COMMAND git describe --tags --always WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE QOLIBRI_VERSION_STR)
    string(REGEX REPLACE "(\r?\n)+$" "" QOLIBRI_VERSION_STR "${QOLIBRI_VERSION_STR}")
    if ("${QOLIBRI_VERSION_STR}" STREQUAL "")
        set(QOLIBRI_VERSION_STR "${PROJECT_VERSION}")
    endif()
    set(QOLIBRI_VERSION_STR "${QOLIBRI_VERSION_STR}" CACHE STRING "The version number string. Auto-detected from Git with a fallback to the CMake project version." FORCE)
endif()
target_compile_definitions(qolibri PRIVATE QOLIBRI_VERSION_STR="${QOLIBRI_VERSION_STR}")

set(QOLIBRI_WEBSITE "https://github.com/mvf/qolibri" CACHE STRING "The URL of the website. Adds a hyperlink to the about dialog if non-empty.")
target_compile_definitions(qolibri PRIVATE QOLIBRI_WEBSITE="${QOLIBRI_WEBSITE}")

if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "[Cc][Ll][Aa][Nn][Gg]|[Gg][Nn][Uu]")
    target_compile_options(qolibri PRIVATE -pedantic)
endif ()

if (APPLE)
    set_source_files_properties(images/qolibri.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(MACOSX_BUNDLE_ICON_FILE qolibri.icns)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
    set_target_properties(qolibri PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
endif()

if (WIN32) # Includes Win64
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qolibri.manifest.in" "${CMAKE_CURRENT_BINARY_DIR}/qolibri.manifest" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qolibri.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/qolibri.rc" @ONLY)
    target_sources(qolibri PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/qolibri.manifest"
        "${CMAKE_CURRENT_BINARY_DIR}/qolibri.rc"
    )
endif()

# Link translations into the binary
configure_file(translations/translations.qrc "${CMAKE_CURRENT_BINARY_DIR}/translations/translations.qrc" COPYONLY)
set(TS_FILES translations/qolibri_ja_JP.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt_add_translation(QM_FILES ${TS_FILES})
target_sources(qolibri PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/translations/translations.qrc" ${QM_FILES} ${TS_FILES})

target_link_libraries(qolibri PRIVATE Qt::Multimedia Qt::Network Qt::WebEngineWidgets Qt::Widgets eb::eb)
foreach(lib Core5Compat WebEngine WebEngineCore)
    if(TARGET Qt::${lib})
        target_link_libraries(qolibri PRIVATE Qt::${lib})
    endif()
endforeach()

install(TARGETS qolibri DESTINATION "${CMAKE_INSTALL_BINDIR}")
if(UNIX AND NOT APPLE)
    install(FILES qolibri.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
endif()
