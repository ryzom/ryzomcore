FILE(GLOB SRC *.cpp *.h)

NL_TARGET_LIB(gameplay_module_lib ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(gameplay_module_lib nelmisc)

NL_DEFAULT_PROPS(gameplay_module_lib "Ryzom, Library: Gameplay Module")
NL_ADD_LIB_SUFFIX(gameplay_module_lib)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS gameplay_module_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
