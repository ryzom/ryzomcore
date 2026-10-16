FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(sheets_packer_shard ${SRC})

# All service sheet code comes from the once-compiled libraries under
# ryzom/server/src/sheets; the seam symbols they expect from their host
# application (writeDirectory, EGSLight, LoadOutposts, readCreatureScriptComp)
# are defined in sheets_packer_shard.cpp.
TARGET_LINK_LIBRARIES(sheets_packer_shard
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

NL_DEFAULT_PROPS(sheets_packer_shard "Ryzom, Tools: Sheets Packer Shard")
NL_ADD_RUNTIME_FLAGS(sheets_packer_shard)

INSTALL(TARGETS sheets_packer_shard RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT tools)
