
IF(WITH_LIGO)
  ADD_SUBDIRECTORY(ai_build_wmap)
ENDIF(WITH_LIGO)
IF(WITH_3D)
  ADD_SUBDIRECTORY(build_world_packed_col)
ENDIF(WITH_3D)

# Not done yet.
#admin
#brick_param_extractor
#build_spell_sheet
#build_world_packed_col
#CMakeLists.txt
#sql
#www
