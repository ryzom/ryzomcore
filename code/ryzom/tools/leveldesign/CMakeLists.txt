ADD_SUBDIRECTORY(alias_synchronizer)
ADD_SUBDIRECTORY(prim_export)
ADD_SUBDIRECTORY(uni_conv)
ADD_SUBDIRECTORY(csv_transform)
ADD_SUBDIRECTORY(icon_search)
ADD_SUBDIRECTORY(world_editor)

IF(WIN32)
  ADD_SUBDIRECTORY(export)
ENDIF(WIN32)
# Legacy folders, do not build
#ADD_SUBDIRECTORY(export)

# folders not handled yet.
#georges_convert
#georges_dll
#georges_exe
#georges_plugin_sound
#georges_plugin_test
#icon_search
#install
#master
#mission_compiler_fe
#mission_compiler_lib
#mission_simulator
#mp_generator
#named_items_2_csv
#
#primitive_id_assignator
#test_root
#
#variable_parser
#
