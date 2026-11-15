project(intensity LANGUAGES CXX C)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Multimedia)

qt6_policy(SET QTP0001 NEW)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(intensity
    main.cpp
)

qt_add_qml_module(intensity
    URI CuteDeckIntensity
    VERSION 1.0
    QML_FILES Intensity.qml
    IMPORTS TARGET cutedeck
    DEPENDENCIES CuteDeck
)

target_link_libraries(intensity PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Multimedia cutedeck)
