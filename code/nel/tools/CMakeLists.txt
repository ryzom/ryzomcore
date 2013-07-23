
# Don't add other subdirectories if only max plugins are selected.
IF(WITH_NEL_TOOLS)
  ADD_SUBDIRECTORY(misc)
  ADD_SUBDIRECTORY(memory)
ENDIF(WITH_NEL_TOOLS)

# Max plugins are under the 3d directory as well.
IF(WITH_3D)
  ADD_SUBDIRECTORY(3d)
ENDIF(WITH_3D)

# Don't add other subdirectories if only max plugins are selected.
IF(WITH_NEL_TOOLS)
  IF(WITH_PACS)
    ADD_SUBDIRECTORY(pacs)
  ENDIF(WITH_PACS)
  
  IF(WITH_LOGIC)
    ADD_SUBDIRECTORY(logic)
  ENDIF(WITH_LOGIC)
  
  IF(WITH_GEORGES)
    ADD_SUBDIRECTORY(georges)
  ENDIF(WITH_GEORGES)
  
  IF(WITH_SOUND)
    ADD_SUBDIRECTORY(sound)
  ENDIF(WITH_SOUND)
  
  IF(WITH_NEL_TESTS)
    ADD_SUBDIRECTORY(nel_unit_test)
  ENDIF(WITH_NEL_TESTS)
ENDIF(WITH_NEL_TOOLS)
