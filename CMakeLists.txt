cmake_minimum_required(VERSION 3.16)

project(the_flight_managerment_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

find_package(HuskarUI REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickEffects)

set(CMAKE_AUTORCC ON)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appthe_flight_managerment_system
    main.cpp
)

qt_add_qml_module(appthe_flight_managerment_system
    URI the_flight_managerment_system
    VERSION 1.0
    QML_FILES
    Login.qml
    Main.qml
    MyFavouriteFlight.qml
    AboutUs.qml
    RESOURCES images.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appthe_flight_managerment_system PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appthe_flight_managerment_system
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appthe_flight_managerment_system
    PRIVATE Qt6::Quick
    PRIVATE HuskarUI::Basic
    PRIVATE Qt6::QuickEffects
)

include(GNUInstallDirs)
install(TARGETS appthe_flight_managerment_system
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


