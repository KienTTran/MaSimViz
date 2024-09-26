cmake_minimum_required(VERSION 3.16)

set(QT_WASM FALSE)
option(LLAMA_METAL "Enable Metal support for Apple Silicon" ON)

# Check if the target is Emscripten (WebAssembly)
if (QT_WASM)
    set(QT_VERSION_MAJOR 6)
    set(DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/Users/ktt/SDK/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake)
    set(DEMSCRIPTEN ON)
    set(VCPKG_TARGET_TRIPLET wasm32-emscripten)
else()
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
endif()

project(MaSimViz VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(QT_WASM)
    message(STATUS "Configuring for WebAssembly")
    find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Sql OpenGLWidgets Concurrent Charts)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql OpenGLWidgets Concurrent Charts)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql OpenGLWidgets Concurrent Charts)
endif()

set(TS_FILES MaSimViz_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MaSimViz
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        .gitignore
        loader.h loader.cpp
        loadersqlite.h loadersqlite.cpp
        graphicsviewcustom.h graphicsviewcustom.cpp
        glwidgetcustom.h glwidgetcustom.cpp
        vizdata.h vizdata.cpp
        loaderyml.h loaderyml.cpp
        loaderraster.h loaderraster.cpp
        dataprocessor.h dataprocessor.cpp
        squareitem.h squareitem.cpp
        chartcustom.h chartcustom.cpp
        ascfile.h ascfile.cpp
        colormapwidget.h
        README.md
        llamacpp

        chatbotwithapi.h chatbotwithapi.cpp
        chatbotwithmodel.h chatbotwithmodel.cpp
        webengineviewcustom.h webengineviewcustom.cpp
        chatbotinterface.h

    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MaSimViz SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(MaSimViz
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()


add_subdirectory(llamacpp)
# Set up includes and link necessary libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/llamacpp/build/include)
# include_directories(/opt/homebrew/include)

if(QT_WASM)
    target_link_libraries(MaSimViz PRIVATE Qt6::Widgets Qt6::OpenGLWidgets Qt6::Concurrent Qt6::Charts)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Sql OpenGLWidgets Concurrent Charts Network webenginewidgets)
    target_link_libraries(MaSimViz PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::WebEngineWidgets)

    find_package(date CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE date::date date::date-tz)

    find_package(yaml-cpp CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/llamacpp/build/lib/libllama.dylib)
endif()

# If building for WebAssembly, ensure WASM-specific properties
if(QT_WASM)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1")
    set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -s USE_PTHREADS=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets")
endif()

if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_finalize_executable(MaSimViz)
endif()

# Installation settings
include(GNUInstallDirs)
install(TARGETS MaSimViz
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)




