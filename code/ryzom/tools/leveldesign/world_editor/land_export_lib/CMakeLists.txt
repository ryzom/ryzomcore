FILE(GLOB SRC *.cpp )
FILE(GLOB PRIV_H *.h)

NL_TARGET_LIB(ryzom_landexport ${PRIV_H} ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_landexport ryzom_export nelmisc nelligo nelgeorges nel3d nelpacs ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})
NL_DEFAULT_PROPS(ryzom_landexport "Ryzom, Library, World: Land Export")
NL_ADD_RUNTIME_FLAGS(ryzom_landexport)
NL_ADD_LIB_SUFFIX(ryzom_landexport)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_landexport LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)

