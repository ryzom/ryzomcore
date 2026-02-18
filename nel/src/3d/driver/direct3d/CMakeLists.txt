FILE(GLOB SRC *.cpp *.h *.def *.rc)

SOURCE_GROUP("" FILES
		driver_direct3d.cpp
		driver_direct3d.h
		driver_direct3d.def
		stddirect3d.cpp
		stddirect3d.h
		resources.rc)

SOURCE_GROUP(rendering FILES
		driver_direct3d_render.cpp
		driver_direct3d_material.cpp
		driver_direct3d_texture.cpp
		driver_direct3d_light.cpp
		driver_direct3d_matrix.cpp
		driver_direct3d_uniform.cpp
		driver_direct3d_shader.cpp
		driver_direct3d_index.cpp
		driver_direct3d_profile.cpp
		driver_direct3d_inputs.cpp)

SOURCE_GROUP(program FILES
		driver_direct3d_vertex_program.cpp
		driver_direct3d_pixel_program.cpp)

SOURCE_GROUP(vertex FILES
		driver_direct3d_vertex.cpp)

NL_TARGET_DRIVER(nel_drv_direct3d_win ${SRC})

TARGET_INCLUDE_DIRECTORIES(nel_drv_direct3d_win BEFORE PRIVATE ${DXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_drv_direct3d_win nel3d nelmisc ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY})

NL_DEFAULT_PROPS(nel_drv_direct3d_win "NeL, Driver, Video: Direct3D")
NL_ADD_RUNTIME_FLAGS(nel_drv_direct3d_win)
NL_ADD_LIB_SUFFIX(nel_drv_direct3d_win)

TARGET_COMPILE_DEFINITIONS(nel_drv_direct3d_win PRIVATE RIVER_DIRECT3D_EXPORTS) # TODO fix typo or remove

IF(WITH_PCH)
  target_precompile_headers(nel_drv_direct3d_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stddirect3d.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS nel_drv_direct3d_win EXPORT "${project_export_name}" LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_DRIVER_PREFIX} COMPONENT drivers3d)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS nel_drv_direct3d_win RUNTIME DESTINATION maxplugin COMPONENT drivers3d)
  ENDIF()
ENDIF()
