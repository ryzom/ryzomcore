FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nel_launcher_ext2 WIN32 ${SRC} nel_launcher.rc)
IF (CURL_FOUND)
    target_compile_definitions(nel_launcher_ext2 PRIVATE USE_CURL)
    TARGET_LINK_LIBRARIES(nel_launcher_ext2
            PRIVATE
            CURL::libcurl
    )
ENDIF ()
target_compile_definitions(nel_launcher_ext2 PRIVATE NL_LAUNCHER_CFG="${NL_ETC_PREFIX}/")

TARGET_LINK_LIBRARIES(nel_launcher_ext2
        PRIVATE
        NeL::net
        NeL::misc
        ZLIB::ZLIB # TODO included in std_afx.h but seems to be unused so maybe remove
)

NL_DEFAULT_PROPS(nel_launcher_ext2 "NeLNS, Launcher: NeL Launcher Ext2")
NL_ADD_RUNTIME_FLAGS(nel_launcher_ext2)

IF (WITH_PCH)
    target_precompile_headers(nel_launcher_ext2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h)
ENDIF ()

INSTALL(TARGETS nel_launcher_ext2 RUNTIME DESTINATION bin COMPONENT launcher)
INSTALL(FILES nel_launcher.cfg DESTINATION ${NL_ETC_PREFIX} COMPONENT launcher)
