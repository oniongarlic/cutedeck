project(imageview LANGUAGES CXX C)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Multimedia)

qt6_policy(SET QTP0001 NEW)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(imageview
    main.cpp
)

qt_add_qml_module(imageview
    URI CuteDeckImageView
    VERSION 1.0
    QML_FILES Main.qml
    IMPORTS TARGET cutedeck
    DEPENDENCIES CuteDeck
    #QML_FILES ImageDrawer.qml
)

target_link_libraries(imageview PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Multimedia cutedeck)
