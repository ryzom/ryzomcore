FILE(GLOB SRC *.cpp )
FILE(GLOB PRIV_H *.h)

NL_TARGET_LIB(ryzom_export ${PRIV_H} ${SRC})
add_library(Ryzom::export ALIAS ryzom_export)

TARGET_LINK_LIBRARIES(ryzom_export
        PUBLIC
        NeL::misc
        NeL::ligo
        NeL::georges
        NeL::3d
        NeL::pacs
)
NL_DEFAULT_PROPS(ryzom_export "Ryzom, Library, World: Export Tools")
NL_ADD_RUNTIME_FLAGS(ryzom_export)
NL_ADD_LIB_SUFFIX(ryzom_export)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_export LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()
