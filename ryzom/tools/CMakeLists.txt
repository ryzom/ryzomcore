
IF(NOT HUNTER_ENABLED)
  FIND_PACKAGE(MySQL)
ENDIF()

IF(WITH_RYZOM_TOOLS)
  ADD_SUBDIRECTORY(skill_extractor)
  ADD_SUBDIRECTORY(make_alias_file)
  ADD_SUBDIRECTORY(make_anim_by_race)
  ADD_SUBDIRECTORY(make_anim_melee_impact)
  ADD_SUBDIRECTORY(pd_parser)
  ADD_SUBDIRECTORY(assoc_mem)
  ADD_SUBDIRECTORY(leveldesign)

  IF(WITH_LIGO)
    ADD_SUBDIRECTORY(translation_tools)
  ENDIF()

  IF(WITH_NET)
    ADD_SUBDIRECTORY(stats_scan)
    ADD_SUBDIRECTORY(pdr_util)
    ADD_SUBDIRECTORY(patch_gen)

    IF(WIN32)
      IF(MYSQL_FOUND)
        ADD_SUBDIRECTORY(sheets_packer_shard)
      ENDIF()
    ENDIF()
  ENDIF()

  IF(WITH_LIGO AND WITH_NET)
    ADD_SUBDIRECTORY(sheets_packer)
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(client)

IF(MYSQL_FOUND)
  ADD_SUBDIRECTORY(server)
ENDIF()

# Old stuff that doesn't compile anymore.
#ADD_SUBDIRECTORY(occ2huff)
#ADD_SUBDIRECTORY(phrase_generator)
#ADD_SUBDIRECTORY(sheet_random_generator)

# Not building these
#xml_packer - this is in NeL tools.

# Folders with nothing to build
#data_tools
#translation
#test_files
#build_data

# folders not handled yet.
#connection_stats
#reynolds
