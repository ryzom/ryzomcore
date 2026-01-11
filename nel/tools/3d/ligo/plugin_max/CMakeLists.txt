FILE(GLOB SRC *.cpp *.h ../../ig_lighter_lib/*.cpp ../../ig_lighter_lib/*.h *.rc *.rc2 *.def)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(ligoscape_utility SHARED ${SRC})

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ligoscape_utility
							nelmisc
							nelligo
							nel3d
							nelpacs
							${MAXSDK_LIBRARIES}
							nel_patch_lib
							nel_mesh_lib
							nel_3dsmax_shared
							comctl32)

SET_TARGET_PROPERTIES(ligoscape_utility PROPERTIES SUFFIX ".dlx")
NL_DEFAULT_PROPS(ligoscape_utility "MAX Plugin: Ligoscape Utility")
NL_ADD_RUNTIME_FLAGS(ligoscape_utility)
NL_ADD_LIB_SUFFIX(ligoscape_utility)

ADD_DEFINITIONS(${MAXSDK_DEFINITIONS})

INSTALL(TARGETS ligoscape_utility RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
INSTALL(FILES ligoscript.txt DESTINATION maxplugin/docs)
INSTALL(DIRECTORY scripts/
        DESTINATION maxplugin/scripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY macroscripts/
        DESTINATION maxplugin/macroscripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)