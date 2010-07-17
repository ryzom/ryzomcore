CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/ryzom.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/ryzom.desktop")

INSTALL(FILES ryzom.png DESTINATION share/pixmaps)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/ryzom.desktop" DESTINATION share/applications)
