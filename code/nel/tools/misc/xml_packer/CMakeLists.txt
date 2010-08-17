FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(xml_packer ${SRC})

TARGET_LINK_LIBRARIES(xml_packer ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(xml_packer "Tools, Misc: xml_packer")
NL_ADD_RUNTIME_FLAGS(xml_packer)

INSTALL(TARGETS xml_packer RUNTIME DESTINATION bin COMPONENT toolsmisc)
