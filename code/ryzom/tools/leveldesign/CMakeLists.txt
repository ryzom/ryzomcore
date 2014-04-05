
ADD_SUBDIRECTORY(uni_conv)
ADD_SUBDIRECTORY(csv_transform)
ADD_SUBDIRECTORY(icon_search)
IF(WITH_LIGO)
  ADD_SUBDIRECTORY(alias_synchronizer)
  ADD_SUBDIRECTORY(prim_export)
  ADD_SUBDIRECTORY(mission_compiler_lib)
ENDIF(WITH_LIGO)
ADD_SUBDIRECTORY(mp_generator)
ADD_SUBDIRECTORY(named_items_2_csv)

IF(WIN32)
  IF(WITH_LIGO)
    ADD_SUBDIRECTORY(export)
    ADD_SUBDIRECTORY(world_editor)
  ENDIF(WITH_LIGO)
  IF(WITH_MFC)
    IF(WITH_LIGO)
      ADD_SUBDIRECTORY(mission_compiler_fe)
    ENDIF(WITH_LIGO)
    IF(WITH_GEORGES)
      ADD_SUBDIRECTORY(georges_dll)
      ADD_SUBDIRECTORY(georges_exe)
      IF(WITH_SOUND)
        ADD_SUBDIRECTORY(georges_plugin_sound)
      ENDIF(WITH_SOUND)
    ENDIF(WITH_GEORGES)
  ENDIF(WITH_MFC)
ENDIF(WIN32)


IF(WITH_QT)
  IF(WITH_GEORGES)
    ADD_SUBDIRECTORY(georges_editor_qt)
  ENDIF(WITH_GEORGES)
ENDIF(WITH_QT)

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
