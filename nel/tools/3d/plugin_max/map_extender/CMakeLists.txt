FILE(GLOB SRC *.cpp *.h *.def)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(map_extender SHARED ${SRC})

TARGET_INCLUDE_DIRECTORIES(map_extender PRIVATE ${MAXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(map_extender ${MAXSDK_LIBRARIES})

# Drop-in replacement: the output file must be named exactly like the original plugin DLL the
# corpus files' DllDirectory references (mapext198m3.dlm), so Max binds it without a
# missing-plugin prompt. No NL lib suffix for the same reason.
SET_TARGET_PROPERTIES(map_extender PROPERTIES OUTPUT_NAME "mapext198m3" PREFIX "" SUFFIX ".dlm")
NL_DEFAULT_PROPS(map_extender "MAX Plugin: Map Extender (drop-in)")
NL_ADD_RUNTIME_FLAGS(map_extender)

TARGET_COMPILE_DEFINITIONS(map_extender PRIVATE ${MAXSDK_DEFINITIONS})

INSTALL(TARGETS map_extender RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
