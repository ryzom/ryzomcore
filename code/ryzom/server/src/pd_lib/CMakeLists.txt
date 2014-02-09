FILE(GLOB SRC *.cpp *.h)

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.h)

NL_TARGET_LIB(ryzom_pd ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_pd nelmisc nelnet ryzom_gameshare)
NL_DEFAULT_PROPS(ryzom_pd "Ryzom, Library: PD Lib")
NL_ADD_RUNTIME_FLAGS(ryzom_pd)
NL_ADD_LIB_SUFFIX(ryzom_pd)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_pd LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
