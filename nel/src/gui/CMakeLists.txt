FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/gui/*.h)

SOURCE_GROUP("" FILES ${HEADERS} ${SRC})

NL_TARGET_LIB(nelgui ${SRC} ${HEADERS})
add_project_library_alias(nelgui gui)

TARGET_INCLUDE_DIRECTORIES(nelgui
        PRIVATE
        ${LUA_INCLUDE_DIR}
        ${LUABIND_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(nelgui
        PUBLIC
        NeL::misc
        NeL::web
        NeL::3d
        ${LUA_LIBRARIES}
        Luabind::Luabind
        PRIVATE
        CURL::libcurl
        LibXml2::LibXml2
)

NL_DEFAULT_PROPS(nelgui "NeL, Library: NeL GUI")
NL_ADD_RUNTIME_FLAGS(nelgui)

NL_ADD_LIB_SUFFIX(nelgui)

TARGET_COMPILE_DEFINITIONS(nelgui PRIVATE ${LUABIND_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(nelgui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nelgui EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
