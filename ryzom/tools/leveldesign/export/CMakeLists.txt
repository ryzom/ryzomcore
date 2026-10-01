FILE(GLOB SRC *.cpp )
FILE(GLOB PRIV_H *.h)

# CExport::newExport() reads continent.cfg through SContinentCfg (export.cpp
# includes "../master/ContinentCfg.h"), whose implementation lives in the
# master/ folder (an MFC app not otherwise built by CMake). Both files are
# platform-independent (their #include "stdafx.h" is guarded behind
# NL_OS_WINDOWS), so compile them directly into ryzom_export instead of
# pulling in all of master/.
LIST(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/../master/easy_cfg.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../master/ContinentCfg.cpp)
LIST(APPEND PRIV_H ${CMAKE_CURRENT_SOURCE_DIR}/../master/easy_cfg.h ${CMAKE_CURRENT_SOURCE_DIR}/../master/ContinentCfg.h)

NL_TARGET_LIB(ryzom_export ${PRIV_H} ${SRC})

# _CONSOLE makes master/StdAfx.h (pulled in transitively by easy_cfg.cpp/
# ContinentCfg.cpp on Windows) skip its #ifndef _CONSOLE block of MFC
# includes -- neither file actually uses MFC, and ryzom_export isn't an MFC
# app, so there's nothing to build without it besides an optional Visual
# Studio component.
TARGET_COMPILE_DEFINITIONS(ryzom_export PRIVATE _CONSOLE)

TARGET_LINK_LIBRARIES(ryzom_export nelmisc nelligo nelgeorges nel3d nelpacs)
NL_DEFAULT_PROPS(ryzom_export "Ryzom, Library, World: Export Tools")
NL_ADD_RUNTIME_FLAGS(ryzom_export)
NL_ADD_LIB_SUFFIX(ryzom_export)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS ryzom_export LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT libraries)
ENDIF()
