FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nel_launcher_ext2 WIN32 ${SRC} nel_launcher.rc)
IF(CURL_FOUND)
  ADD_DEFINITIONS(-DUSE_CURL)
ENDIF()
ADD_DEFINITIONS(-DNL_LAUNCHER_CFG="${NL_ETC_PREFIX}/")

TARGET_LINK_LIBRARIES(nel_launcher_ext2
						nelnet
						nelmisc
						${ZLIB_LIBRARY}
						${CURL_LIBRARIES})

NL_DEFAULT_PROPS(nel_launcher_ext2 "NeLNS, Launcher: NeL Launcher Ext2")
NL_ADD_RUNTIME_FLAGS(nel_launcher_ext2)

IF(WITH_PCH)
  target_precompile_headers(nel_launcher_ext2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h)
ENDIF()

INSTALL(TARGETS nel_launcher_ext2 RUNTIME DESTINATION bin COMPONENT launcher)
INSTALL(FILES nel_launcher.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT launcher)
