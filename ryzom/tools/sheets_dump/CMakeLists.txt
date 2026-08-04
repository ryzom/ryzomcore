FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(sheets_dump ${SRC})

# Reads .packed_sheets through the same once-compiled sheet libraries the
# services and sheets_packer_shard use, so the dump can never drift from the
# real serialization code.
TARGET_LINK_LIBRARIES(sheets_dump
		PRIVATE
		Ryzom::egssheets
		Ryzom::aissheets
		Ryzom::gpmsheets
		Ryzom::iossheets
		Ryzom::gameshare
		Ryzom::servershare
		Ryzom::aishare
		NeL::misc
		NeL::georges
		NeL::net
		NeL::ligo
)

NL_DEFAULT_PROPS(sheets_dump "Ryzom, Tools: Sheets Dump")
NL_ADD_RUNTIME_FLAGS(sheets_dump)

INSTALL(TARGETS sheets_dump RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
