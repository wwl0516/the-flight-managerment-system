cmake_minimum_required(VERSION 3.16)

project(the_flight_managerment_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

find_package(HuskarUI REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickEffects)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Widgets Sql)#Qt 数据库模块

set(CMAKE_AUTORCC ON)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appthe_flight_managerment_system
    main.cpp
    DBManager.cpp
    DBManager.h
)

qt_add_qml_module(appthe_flight_managerment_system
    URI the_flight_managerment_system
    VERSION 1.0
    
    QML_FILES
    
    Pages/Login.qml
    Pages/Main.qml
    Pages/MyFavouriteFlight.qml
    Pages/AboutUs.qml
    Components/SharedCard.qml
    RESOURCES images.qrc
    SOURCES 
    DBManager.cpp
    DBManager.h
    SOURCES DBUser.h
    SOURCES DBUser.cpp

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

target_link_libraries(appthe_flight_managerment_system PRIVATE
    Qt6::Quick
    HuskarUI::Basic
    Qt6::QuickEffects
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Widgets Qt6::Sql
    Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appthe_flight_managerment_system
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


