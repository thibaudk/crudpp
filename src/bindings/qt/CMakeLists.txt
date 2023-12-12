project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string(REPLACE ";" "," CLASSES_STRING "${CRUDPP_CLASSES}")

add_compile_definitions(CLIENT
                        QT
                        CLASSES_STRING=${CLASSES_STRING})

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick)

option(VERDIGRIS_PATH "Path to the verdigris repository" ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${VERDIGRIS_PATH}/src)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${CRUDPP_EXTRA_CLIENT_SOURCES}
    main.cpp)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Quick)

qt_import_qml_plugins(${PROJECT_NAME})

qt_finalize_executable(${PROJECT_NAME})
