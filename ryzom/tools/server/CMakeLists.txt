IF(WITH_RYZOM_TOOLS)
  IF(WITH_LIGO)
    # Disabled because no server code in atys/yubo branch
    #ADD_SUBDIRECTORY(ai_build_wmap)
  ENDIF()

  IF(WITH_3D)
    # Disabled because no server code in atys/yubo branch
    #ADD_SUBDIRECTORY(build_world_packed_col)
  ENDIF()
ENDIF()

# Not done yet.
#admin
#brick_param_extractor
#build_spell_sheet
#build_world_packed_col
#CMakeLists.txt
#sql
#www
