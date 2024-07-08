project("${PROJECT_NAME}_server")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckIncludeFileCXX)

set(DRGN_HEADERS
    ${BINDING_HEADERS}/drogon/visitors/json_handler.hpp
    ${BINDING_HEADERS}/drogon/visitors/json_reader.hpp
    ${BINDING_HEADERS}/drogon/visitors/offset_row_handler.hpp
    ${BINDING_HEADERS}/drogon/visitors/offset_row_reader.hpp
    ${BINDING_HEADERS}/drogon/visitors/row_handler.hpp
    ${BINDING_HEADERS}/drogon/visitors/row_reader.hpp
    ${BINDING_HEADERS}/drogon/wrappers/model.hpp
    ${BINDING_HEADERS}/drogon/wrappers/restful_ctrl_auth.hpp
    ${BINDING_HEADERS}/drogon/wrappers/restful_ctrl_base.hpp
    ${BINDING_HEADERS}/drogon/wrappers/restful_ctrl_w_spk.hpp
    ${BINDING_HEADERS}/drogon/wrappers/restful_ctrl.hpp
    ${BINDING_HEADERS}/drogon/ctrl_macro.hpp
    ${BINDING_HEADERS}/drogon/utils.hpp
)
set(CRUDPP_HEADERS ${CRUDPP_HEADERS} PARENT_SCOPE)

string(REPLACE ";" ")(" BOOST_FORMATED_CLASSES "(${CRUDPP_CLASSES})")

add_compile_definitions(SERVER
                        DROGON
                        BOOST_FORMATED_CLASSES=${BOOST_FORMATED_CLASSES})

add_executable(${PROJECT_NAME}
    ${DRGN_HEADERS}
    main.cc
)

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

# target_include_directories(${PROJECT_NAME}
#                            PRIVATE
#                            ${CMAKE_CURRENT_SOURCE_DIR})

# set(DRGN_HEADERS ${CRUDPP_SOURCE_DIR}/include/crudpp/bindings/drogon)

# target_sources(${PROJECT_NAME}
#               PRIVATE
#               ${DRGN_HEADERS}/utils.hpp)

# ##############################################################################
# uncomment the following line for dynamically loading views
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

# add_subdirectory(test)
