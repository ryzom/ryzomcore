FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(sheets_dump_shard ${SRC})

# Reads .packed_sheets through the same once-compiled sheet libraries the
# services and sheets_packer_shard use, so the dump can never drift from the
# real serialization code.
TARGET_LINK_LIBRARIES(sheets_dump_shard
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

NL_DEFAULT_PROPS(sheets_dump_shard "Ryzom, Tools: Sheets Dump Shard")
NL_ADD_RUNTIME_FLAGS(sheets_dump_shard)

INSTALL(TARGETS sheets_dump_shard RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
