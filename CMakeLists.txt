cmake_minimum_required(VERSION 3.16)

project(cutedeck LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX ${CMAKE_PREFIX_PATH})
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui Qml)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Multimedia)

qt6_policy(SET QTP0001 NEW)
qt_standard_project_setup(REQUIRES 6.8)

add_library(cutedeck SHARED
  cutedeck_global.h
  cutedeck.cpp
  cutedeck.h

  decklink.cpp
  decklink.h
  decklinkaudiodevice.cpp
  decklinkaudiodevice.h
  decklinksink.cpp
  decklinksink.h
  decklinksource.cpp
  decklinksource.h
)

target_link_libraries(cutedeck PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt6::Gui Qt6::Qml Qt6::Multimedia)

target_compile_definitions(cutedeck PRIVATE CUTEDECK_LIBRARY)

if(WIN32)
    set(DECKLINK_DIR "C:/Qt/decklink")

    target_include_directories(cutedeck PRIVATE "${DECKLINK_DIR}/include")
    target_sources(cutedeck PRIVATE "${DECKLINK_DIR}/include/DeckLinkAPI_i.c")
    target_link_libraries(cutedeck PRIVATE ole32 uuid oleaut32)
elseif(UNIX)
    set(DECKLINK_DIR "/opt/decklink")

    target_include_directories(cutedeck PRIVATE "${DECKLINK_DIR}/include")
    target_sources(cutedeck PRIVATE "${DECKLINK_DIR}/include/DeckLinkAPIDispatch.cpp")
    target_link_libraries(cutedeck PRIVATE dl)
endif()

set_target_properties(cutedeck PROPERTIES PUBLIC_HEADER cutedeck.h)

target_include_directories(cutedeck SYSTEM PUBLIC)

qt_add_qml_module(cutedeck
    URI "CuteDeck"
    PLUGIN_TARGET cutedeckplugin
    DEPENDENCIES QtQuick
    OUTPUT_DIRECTORY CuteDeck
)

include(GNUInstallDirs)

# Install

install(TARGETS cutedeck
    BUNDLE DESTINATION .
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/CuteDeck/ DESTINATION ${CMAKE_INSTALL_PREFIX}/qml/CuteDeck/)

set_target_properties(cutedeckplugin PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")
install(TARGETS cutedeckplugin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/qml/CuteDeck/)