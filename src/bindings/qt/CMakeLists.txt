project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_HEADERS
    ${BINDING_HEADERS}/qt/interface/bridge.hpp
    ${BINDING_HEADERS}/qt/interface/net_manager.hpp
    ${BINDING_HEADERS}/qt/interface/net_manager.hpp
    ${BINDING_HEADERS}/qt/wrappers/base_wrapper.hpp
    ${BINDING_HEADERS}/qt/wrappers/list_model.hpp
    ${BINDING_HEADERS}/qt/wrappers/model.hpp
    ${BINDING_HEADERS}/qt/wrappers/property_holder.hpp
    ${BINDING_HEADERS}/qt/wrappers/sort_filter.hpp
    ${BINDING_HEADERS}/qt/json_reader.hpp
    ${BINDING_HEADERS}/qt/utils.hpp

)

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

    message("QRC_FILE = " ${QRC_FILE})

    # file(GLOB_RECURSE RESSOURCES RELATIVE
    #     ${CRUDPP_CLIENT_RESOURCE_DIR}/*.qml
    #     ${CRUDPP_CLIENT_RESOURCE_DIR}/*.svg
    #     ${CRUDPP_CLIENT_RESOURCE_DIR}/*.png
    #     ${CRUDPP_CLIENT_RESOURCE_DIR}/*.jpeg
    # )

    # message("RESSOURCES = " ${RESSOURCES})

    if(WIN32)
        file(GLOB_RECURSE RC_FILE
            ${CRUDPP_CLIENT_RESOURCE_DIR}/${PROJECT_NAME}.rc)

        message("RC_FILE = " ${RC_FILE})
    endif()
endif()

set(PROJECT_SOURCES
    ${QT_HEADERS}
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
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES})
else()
    qt_add_executable(${PROJECT_NAME}
        WIN32 MACOSX_BUNDLE
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

# qt6_add_resources(${PROJECT_NAME} ${CRUDPP_CLIENT_RESOURCE_DIR}
#     PREFIX
#     "/"
#     FILES
#     ${RESSOURCES}
# )

qt_import_qml_plugins(${PROJECT_NAME})

qt_finalize_executable(${PROJECT_NAME})
