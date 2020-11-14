FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(land_export ${SRC})

TARGET_LINK_LIBRARIES(land_export nelmisc nelligo ryzom_landexport)

NL_DEFAULT_PROPS(land_export "Ryzom, Tools, World: Land Export")
NL_ADD_RUNTIME_FLAGS(land_export)

INSTALL(TARGETS land_export RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
