# shape_lightmapper — standalone lightmapper build step (design doc §11-lm): reads the
# .lmscene scene-graph intermediates written at 1_export, runs the NL3D CMeshLightmapper
# (the calc_lm port) and emits lightmapped .shape files + lightmap textures for the
# existing lightmap_optimizer. Pure NeL consumer — no pipeline_max dependency.
FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(shape_lightmapper ${SRC})

TARGET_LINK_LIBRARIES(shape_lightmapper
	NeL::misc
	NeL::3d
)

NL_DEFAULT_PROPS(shape_lightmapper "NeL, Tools, 3D: Shape Lightmapper")
NL_ADD_RUNTIME_FLAGS(shape_lightmapper)

INSTALL(TARGETS shape_lightmapper RUNTIME DESTINATION bin COMPONENT toolsmisc)
