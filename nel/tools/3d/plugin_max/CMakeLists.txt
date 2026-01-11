ADD_SUBDIRECTORY(nel_patch_lib)
ADD_SUBDIRECTORY(nel_mesh_lib)
ADD_SUBDIRECTORY(nel_patch_converter)
ADD_SUBDIRECTORY(nel_3dsmax_shared)
ADD_SUBDIRECTORY(nel_export)
ADD_SUBDIRECTORY(nel_patch_edit)
ADD_SUBDIRECTORY(nel_patch_paint)
ADD_SUBDIRECTORY(nel_vertex_tree_paint)
ADD_SUBDIRECTORY(tile_utility)

INSTALL(FILES 
		max_animation_support.txt
		max_lightmap_support.txt
		max_skinning_support.txt
		nel_water_material.txt
		install_plugin_max.txt
		max_light_support.txt
		max_material_support.txt
		resolve_troubles.txt
		DESTINATION maxplugin/docs)
INSTALL(DIRECTORY scripts/
        DESTINATION maxplugin/scripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY macroscripts/
        DESTINATION maxplugin/macroscripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)