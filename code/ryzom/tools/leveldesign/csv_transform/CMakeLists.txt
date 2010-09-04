FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(csv_transform ${SRC})

TARGET_LINK_LIBRARIES(csv_transform ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(csv_transform "Ryzom, Tools, Misc: CSV Transform")
NL_ADD_RUNTIME_FLAGS(csv_transform)

INSTALL(TARGETS csv_transform RUNTIME DESTINATION bin COMPONENT tools)
