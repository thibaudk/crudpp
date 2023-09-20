project("${PROJECT_NAME}_server")

include(CheckIncludeFileCXX)

string(REPLACE "," ")(" BOOST_FORMATED_CLASSES "(${CRUDPP_CLASSES})")

add_compile_definitions(SERVER
                        DROGON
                        BOOST_FORMATED_CLASSES=${BOOST_FORMATED_CLASSES})

add_executable(${PROJECT_NAME} main.cc)

find_package(Drogon CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

#aux_source_directory(filters FILTER_SRC)
#aux_source_directory(plugins PLUGIN_SRC)

#drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                    ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)
# use the following line to create views with namespace CHANGE_ME prefixed
# and path namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE CHANGE_ME)

target_include_directories(${PROJECT_NAME}
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR})

#target_sources(${PROJECT_NAME}
#               PRIVATE
#               ${SERVER_SOURCES})

# ##############################################################################
# uncomment the following line for dynamically loading views
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

add_subdirectory(test)
