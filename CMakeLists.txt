cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "macOS deployment target")
endif()

# Visual Studio: Enable parallel builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
# Visual Studio: Preselect qolibri as the solution's startup project
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT qolibri)

project(qolibri VERSION 2.1.4)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS LinguistTools Multimedia Network WebEngine WebEngineWidgets Widgets REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(qolibri MACOSX_BUNDLE WIN32
    images/qolibri.icns
    qolibri.qrc
    qolibri.rc
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

set(QOLIBRI_EB_SOURCE_DIR "" CACHE STRING "Optional path to cmake-enabled EB library source tree")
if(QOLIBRI_EB_SOURCE_DIR)
    add_subdirectory("${QOLIBRI_EB_SOURCE_DIR}" eb)
    set(EB_LIBRARY eb)
else()
    find_library(EB_LIBRARY eb REQUIRED)
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

set(QOLIBRI_WEBSITE "https://github.com/ludios/qolibri" CACHE STRING "The URL of the website. Adds a hyperlink to the about dialog if non-empty.")
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
    set(QOLIBRI_EB_UTF8_PATHS ON CACHE BOOL "Force UTF-8 encoding in path names at EB API boundary")
    if(QOLIBRI_EB_UTF8_PATHS)
        target_compile_definitions(qolibri PRIVATE QOLIBRI_EB_UTF8_PATHS)
    endif()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/qolibri.manifest.in" "${CMAKE_CURRENT_BINARY_DIR}/qolibri.manifest" @ONLY)
    target_sources(qolibri PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/qolibri.manifest")
endif()

# Link translations into the binary
configure_file(translations/translations.qrc "${CMAKE_CURRENT_BINARY_DIR}/translations/translations.qrc" COPYONLY)
set(TS_FILES translations/qolibri_ja_JP.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TS_FILES})
target_sources(qolibri PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/translations/translations.qrc" ${QM_FILES} ${TS_FILES})

target_link_libraries(qolibri Qt5::Multimedia Qt5::Network Qt5::WebEngine Qt5::WebEngineWidgets Qt5::Widgets "${EB_LIBRARY}" ZLIB::ZLIB)

install(TARGETS qolibri DESTINATION "${CMAKE_INSTALL_BINDIR}")
