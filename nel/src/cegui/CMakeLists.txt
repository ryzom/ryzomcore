FILE(GLOB SRC *.cpp *.h)
FILE(GLOB HEADERS ../../include/nel/cegui/*.h)

# a module, as nothing links against it: it is loaded at run time by its decorated name (CLibrary), which is the name
# CMake gives a module on every platform
ADD_LIBRARY(nelceguirenderer MODULE ${HEADERS} ${SRC} nelceguirenderer.def)

TARGET_INCLUDE_DIRECTORIES(nelceguirenderer BEFORE PRIVATE ${FREETYPE_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(nelceguirenderer PRIVATE ${CEGUI_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(nelceguirenderer NeL::misc NeL::3d LibXml2::LibXml2 ${CEGUI_LIBRARY})
NL_DEFAULT_PROPS(nelceguirenderer "NeL, Library: CEGUI Renderer")
NL_ADD_RUNTIME_FLAGS(nelceguirenderer)
NL_ADD_LIB_SUFFIX(nelceguirenderer)

TARGET_COMPILE_DEFINITIONS(nelceguirenderer PRIVATE NEL_CEGUIRENDERER_EXPORTS)

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC)
  # a module is installed as a LIBRARY on Windows too: next to the programs there, where the DLL was
  IF(WIN32)
    INSTALL(TARGETS nelceguirenderer LIBRARY DESTINATION ${NL_BIN_PREFIX} COMPONENT libraries)
  ELSE()
    INSTALL(TARGETS nelceguirenderer LIBRARY DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
  ENDIF()
ENDIF()
