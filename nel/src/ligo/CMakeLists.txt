FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/ligo/*.h)

NL_TARGET_LIB(nelligo ${HEADERS} ${SRC})
add_project_library_alias(nelligo ligo)

TARGET_LINK_LIBRARIES(nelligo nelmisc LibXml2::LibXml2)
NL_DEFAULT_PROPS(nelligo "NeL, Library: NeL Ligo")
NL_ADD_RUNTIME_FLAGS(nelligo)

NL_ADD_LIB_SUFFIX(nelligo)

IF(WITH_PCH)
  target_precompile_headers(nelligo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdligo.h)
ENDIF()

NL_GEN_PC(nel-ligo.pc)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nelligo EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
