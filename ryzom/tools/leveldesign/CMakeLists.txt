ADD_SUBDIRECTORY(uni_conv)
ADD_SUBDIRECTORY(csv_transform)
ADD_SUBDIRECTORY(icon_search)
IF(WITH_LIGO)
  ADD_SUBDIRECTORY(alias_synchronizer)
  ADD_SUBDIRECTORY(prim_export)
  ADD_SUBDIRECTORY(mission_compiler_lib)
ENDIF()
ADD_SUBDIRECTORY(mp_generator)
ADD_SUBDIRECTORY(named_items_2_csv)

IF(WIN32)
  IF(WITH_LIGO)
    ADD_SUBDIRECTORY(export)
    ADD_SUBDIRECTORY(world_editor)
  ENDIF()
  IF(WITH_MFC)
    IF(WITH_LIGO)
      ADD_SUBDIRECTORY(mission_compiler_fe)
    ENDIF()
    IF(WITH_GEORGES)
      ADD_SUBDIRECTORY(georges_dll)
      ADD_SUBDIRECTORY(georges_exe)
      IF(WITH_SOUND)
        ADD_SUBDIRECTORY(georges_plugin_sound)
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()


IF(WITH_QT)
  IF(WITH_GEORGES)
    ADD_SUBDIRECTORY(georges_editor_qt)
  ENDIF()
ENDIF()

# folders not handled yet.
#georges_convert
#georges_dll
#georges_exe
#georges_plugin_sound
#georges_plugin_test
#icon_search
#install
#master
#mission_simulator
#
#primitive_id_assignator
#test_root
#
#variable_parser
#
