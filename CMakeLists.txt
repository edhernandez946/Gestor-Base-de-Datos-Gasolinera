cmake_minimum_required(VERSION 3.16)

project(ProyectoGasolinera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar Qt con Widgets y SQL
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_add_executable(ProyectoGasolinera
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Fotos.qrc
        crud.h crud.cpp crud.ui
        consultas.h consultas.cpp consultas.ui




    )
else()
    if(ANDROID)
        add_library(ProyectoGasolinera SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ProyectoGasolinera
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Linkear Widgets y SQL
target_link_libraries(ProyectoGasolinera PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(ProyectoGasolinera PRIVATE Qt6::Widgets)

# Propiedades para iOS/macOS/Windows
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ProyectoGasolinera)
endif()
set_target_properties(ProyectoGasolinera PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ProyectoGasolinera
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ProyectoGasolinera)
endif()
