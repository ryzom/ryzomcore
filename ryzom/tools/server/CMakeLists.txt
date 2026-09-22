IF(WITH_RYZOM_TOOLS)
  IF(WITH_LIGO)
    # Re-enabled 2026-09-16: this checkout DOES have ryzom/server/src/
    # ai_data_service/ (ai_build_wmap/CMakeLists.txt compiles pacs_scan.cpp
    # from there directly into the tool, no running service needed) --
    # the "no server code in atys/yubo branch" comment above no longer
    # applies to this checkout.
    ADD_SUBDIRECTORY(ai_build_wmap)
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
