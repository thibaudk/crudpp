project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

string(REPLACE ";" "," CLASSES_STRING "${CRUDPP_CLASSES}")

add_compile_definitions(CLIENT
                        QT
                        CLASSES_STRING=${CLASSES_STRING})

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC ON)

option(VERDIGRIS_PATH "Path to the verdigris repository" ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${VERDIGRIS_PATH}/src)

if(IS_DIRECTORY ${CRUDPP_CLIENT_RESOURCE_DIR})
    file(GLOB_RECURSE QRC_FILE
        ${CRUDPP_CLIENT_RESOURCE_DIR}/${PROJECT_NAME}.qrc)

    message("QRC_FILE =" ${QRC_FILE})

    if(WIN32)
        file(GLOB_RECURSE RC_FILE
            ${CRUDPP_CLIENT_RESOURCE_DIR}/${PROJECT_NAME}.rc)

        message("RC_FILE =" ${RC_FILE})
    endif()
endif()

set(PROJECT_SOURCES
    utils.cpp
    net_manager.cpp
    bridge.cpp
    sort_filter.cpp
    main.cpp)

if(EXISTS ${QRC_FILE})
    list(APPEND PROJECT_SOURCES ${QRC_FILE})
endif()

if(WIN32 AND EXISTS ${RC_FILE})
    qt_add_executable(${PROJECT_NAME}
        WIN32 ${RC_PATH}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES})
else()
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES})
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Quick
    Qt6::qtquickdialogsplugin
    Qt6::qquicklayoutsplugin
    Qt6::qtquickcontrols2plugin
    Qt6::qtquickcontrols2basicstyleplugin
    Qt6::qtquickcontrols2materialstyleplugin)

qt_import_qml_plugins(${PROJECT_NAME})

qt_finalize_executable(${PROJECT_NAME})
