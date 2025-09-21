FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/logic/*.h)

NL_TARGET_LIB(nellogic ${HEADERS} ${SRC})
add_project_library_alias(nellogic logic)

TARGET_LINK_LIBRARIES(nellogic
        PUBLIC
        nelmisc
        nelnet
        PRIVATE
        LibXml2::LibXml2
)
NL_DEFAULT_PROPS(nellogic "NeL, Library: NeL Logic")
NL_ADD_RUNTIME_FLAGS(nellogic)

NL_ADD_LIB_SUFFIX(nellogic)

IF(WITH_PCH)
  target_precompile_headers(nellogic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdlogic.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nellogic EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
