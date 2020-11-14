FILE(GLOB SRC *.cpp *.h)

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.h)

NL_TARGET_LIB(ryzom_pd ${SRC})

TARGET_LINK_LIBRARIES(ryzom_pd nelmisc nelnet ryzom_gameshare)
NL_DEFAULT_PROPS(ryzom_pd "Ryzom, Library: PD Lib")
NL_ADD_RUNTIME_FLAGS(ryzom_pd)
NL_ADD_LIB_SUFFIX(ryzom_pd)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_pd LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()

