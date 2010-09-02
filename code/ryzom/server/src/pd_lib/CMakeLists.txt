FILE(GLOB SRC *.cpp *.h)

# Filter out the source files not actually compiled.
LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/pd_string_manager.h)

NL_TARGET_LIB(pd_lib ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(pd_lib nelmisc)
  
NL_DEFAULT_PROPS(pd_lib "Ryzom, Library: PD Lib")
NL_ADD_LIB_SUFFIX(pd_lib)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS pd_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
