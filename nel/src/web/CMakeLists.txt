FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/web/*.h)

SOURCE_GROUP("" FILES ${SRC} ${HEADERS})

NL_TARGET_LIB(nelweb ${HEADERS} ${SRC})
add_project_library_alias(nelweb web)

TARGET_INCLUDE_DIRECTORIES(nelweb PRIVATE ../../3rdparty)

TARGET_LINK_LIBRARIES(nelweb
        PUBLIC
        nelmisc
        nel_sevenzip
        PRIVATE
        CURL::libcurl
        ${OPENSSL_LIBRARIES}
        LibXml2::LibXml2
)

NL_DEFAULT_PROPS(nelweb "NeL, Library: NeL Web")
NL_ADD_RUNTIME_FLAGS(nelweb)

NL_ADD_LIB_SUFFIX(nelweb)

IF(WITH_PCH)
  target_precompile_headers(nelweb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdweb.h)
ENDIF()

NL_GEN_PC(nel-web.pc)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS nelweb EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
