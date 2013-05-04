FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(xml_packer ${SRC})

TARGET_LINK_LIBRARIES(xml_packer nelmisc)
NL_DEFAULT_PROPS(xml_packer "NeL, Tools, Misc: xml_packer")
NL_ADD_RUNTIME_FLAGS(xml_packer)

INSTALL(TARGETS xml_packer RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
