FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/georges/*.h)

# SOURCE_GROUP(headers FILES ${HEADERS})

NL_TARGET_LIB(nelgeorges ${HEADERS} ${SRC})
add_project_library_alias(nelgeorges georges)

TARGET_LINK_LIBRARIES(nelgeorges
        PUBLIC
        nelmisc
        PRIVATE
        LibXml2::LibXml2
)
NL_DEFAULT_PROPS(nelgeorges "NeL, Library: NeL Georges")
NL_ADD_RUNTIME_FLAGS(nelgeorges)

NL_ADD_LIB_SUFFIX(nelgeorges)

IF(WITH_PCH)
  target_precompile_headers(nelgeorges PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdgeorges.h)
ENDIF()

NL_GEN_PC(nel-georges.pc)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nelgeorges EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
