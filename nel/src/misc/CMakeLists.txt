FILE(GLOB SRC *.cpp *.h config_file/*.cpp config_file/*.h)
FILE(GLOB HEADERS ../../include/nel/misc/*.h)

FILE(GLOB NLMISC_CDB
	cdb.cpp ../../include/nel/misc/cdb.h
	cdb_*.cpp ../../include/nel/misc/cdb_*.h
)

FILE(GLOB NLMISC_EVENT
	events.cpp ../../include/nel/misc/events.h
	event_*.cpp ../../include/nel/misc/event_*.h
	*_event_*.cpp ../../include/nel/misc/*_event_*.h
)

FILE(GLOB NLMISC_DEBUG
	debug.cpp ../../include/nel/misc/debug.h
	report.cpp ../../include/nel/misc/report.h
	log.cpp ../../include/nel/misc/log.h
)

FILE(GLOB NLMISC_FILESYSTEM
	async_file_manager.cpp ../../include/nel/misc/async_file_manager.h
	file.cpp ../../include/nel/misc/file.h
	path.cpp ../../include/nel/misc/path.h
	big_file.cpp ../../include/nel/misc/big_file.h
	*_xml.cpp ../../include/nel/misc/*_xml.h
	xml_*.cpp ../../include/nel/misc/xml_*.h
	streamed_*.cpp ../../include/nel/misc/streamed_*.h
	seven_zip.cpp ../../include/nel/misc/seven_zip.h
)

FILE(GLOB NLMISC_STREAM
	*_stream.cpp ../../include/nel/misc/*_stream.h
	stream.cpp ../../include/nel/misc/stream.h
	stream_*.cpp ../../include/nel/misc/stream_*.h
)

FILE(GLOB NLMISC_DISPLAYER
	displayer.cpp ../../include/nel/misc/displayer.h
	*_displayer.cpp ../../include/nel/misc/*_displayer.h
)

FILE(GLOB NLMISC_MATH
	plane.cpp ../../include/nel/misc/plane.h
	../../include/nel/misc/plane_inline.h
	polygon.cpp ../../include/nel/misc/polygon.h
	../../include/nel/misc/quad.h
	quat.cpp ../../include/nel/misc/quat.h
	rect.cpp ../../include/nel/misc/rect.h
	rgba.cpp ../../include/nel/misc/rgba.h
	triangle.cpp ../../include/nel/misc/triangle.h
	../../include/nel/misc/uv.h
	vector*.cpp ../../include/nel/misc/vector*.h
	aabbox.cpp ../../include/nel/misc/aabbox.h
	algo.cpp ../../include/nel/misc/algo.h
	bsphere.cpp ../../include/nel/misc/bsphere.h
	fast_floor.cpp ../../include/nel/misc/fast_floor.h
	../../include/nel/misc/geom_ext.h
	line.cpp ../../include/nel/misc/line.h
	matrix.cpp ../../include/nel/misc/matrix.h
)

FILE(GLOB NLMISC_PLATFORM
	*_nl.cpp ../../include/nel/misc/*_nl.h
	common.cpp ../../include/nel/misc/common.h
	app_context.cpp ../../include/nel/misc/app_context.h
	check_fpu.cpp ../../include/nel/misc/check_fpu.h
	cpu_time_stat.cpp ../../include/nel/misc/cpu_time_stat.h
	dummy_window.cpp ../../include/nel/misc/dummy_window.h
	dynloadlib.cpp ../../include/nel/misc/dynloadlib.h
	fast_mem.cpp ../../include/nel/misc/fast_mem.h
	inter_window_msg_queue.cpp ../../include/nel/misc/inter_window_msg_queue.h
	system_*.cpp ../../include/nel/misc/system_*.h
	win32_util.cpp ../../include/nel/misc/win32_util.h
)

FILE(GLOB NLMISC_GENERIC
	../../include/nel/misc/array_2d.h
	*_memory.cpp ../../include/nel/misc/*_memory.h
	buf_fifo.cpp ../../include/nel/misc/buf_fifo.h
	../../include/nel/misc/callback.h
	*_allocator.cpp ../../include/nel/misc/*_allocator.h
	../../include/nel/misc/enum_bitset.h
	../../include/nel/misc/fast_id_map.h
	hierarchical_timer.cpp ../../include/nel/misc/hierarchical_timer.h
	../../include/nel/misc/historic.h
	../../include/nel/misc/mutable_container.h
	../../include/nel/misc/random.h
	smart_ptr.cpp ../../include/nel/misc/smart_ptr.h
	../../include/nel/misc/smart_ptr_inline.h
	../../include/nel/misc/resource_ptr.h
	../../include/nel/misc/resource_ptr_inline.h
	bit_set.cpp ../../include/nel/misc/bit_set.h
	stop_watch.cpp ../../include/nel/misc/stop_watch.h
	../../include/nel/misc/twin_map.h
	../../include/nel/misc/object_vector.h
	../../include/nel/misc/singleton.h
	speaker_listener.cpp ../../include/nel/misc/speaker_listener.h
	../../include/nel/misc/static_map.h
	../../include/nel/misc/stl_block_list.h
)

FILE(GLOB NLMISC_UTILITY
	config_file.cpp ../../include/nel/misc/config_file.h
	cf_*.cpp ../../include/nel/misc/cf_*.h
	config_file/config_file.cpp config_file/config_file.h
	config_file/cf_*.cpp config_file/cf_*.h
	class_id.cpp ../../include/nel/misc/class_id.h
	class_registry.cpp ../../include/nel/misc/class_registry.h
	cmd_args.cpp ../../include/nel/misc/cmd_args.h
	command.cpp ../../include/nel/misc/command.h
	eid_translator.cpp ../../include/nel/misc/eid_translator.h
	entity_id.cpp ../../include/nel/misc/entity_id.h
	eval_num_expr.cpp ../../include/nel/misc/eval_num_expr.h
	factory.cpp ../../include/nel/misc/factory.h
	grid_traversal.cpp ../../include/nel/misc/grid_traversal.h
	mouse_smoother.cpp ../../include/nel/misc/mouse_smoother.h
	noise_value.cpp ../../include/nel/misc/noise_value.h
	progress_callback.cpp ../../include/nel/misc/progress_callback.h
	sheet_id.cpp ../../include/nel/misc/sheet_id.h
	variable.cpp ../../include/nel/misc/variable.h
	../../include/nel/misc/value_smoother.h
)

FILE(GLOB NLMISC_STRING
	string_*.cpp ../../include/nel/misc/string_*.h
	../../include/nel/misc/ucstring.h
	unicode.cpp
	sstring.cpp ../../include/nel/misc/sstring.h
)

FILE(GLOB NLMISC_I18N
	diff_tool.cpp ../../include/nel/misc/diff_tool.h
	i18n.cpp ../../include/nel/misc/i18n.h
	words_dictionary.cpp ../../include/nel/misc/words_dictionary.h
)

FILE(GLOB NLMISC_THREAD
	co_task.cpp ../../include/nel/misc/co_task.h
	mutex.cpp ../../include/nel/misc/mutex.h
	*_thread.cpp ../../include/nel/misc/*_thread.h
	task_*.cpp ../../include/nel/misc/task_*.h
	reader_writer.cpp ../../include/nel/misc/reader_writer.h
	tds.cpp ../../include/nel/misc/tds.h
	thread.cpp ../../include/nel/misc/thread.h
)

FILE(GLOB NLMISC_BITMAP
	bitmap.cpp ../../include/nel/misc/bitmap.h
	bitmap_*.cpp
)

FILE(GLOB NLMISC_CRYPT
	md5.cpp ../../include/nel/misc/md5.h
	sha1.cpp ../../include/nel/misc/sha1.h
	../../include/nel/misc/wang_hash.h
)

SOURCE_GROUP("" FILES ${SRC} ${HEADERS})
SOURCE_GROUP("cdb" FILES ${NLMISC_CDB})
SOURCE_GROUP("event" FILES ${NLMISC_EVENT})
SOURCE_GROUP("debug" FILES ${NLMISC_DEBUG})
SOURCE_GROUP("platform" FILES ${NLMISC_PLATFORM})
SOURCE_GROUP("filesystem" FILES ${NLMISC_FILESYSTEM})
SOURCE_GROUP("stream" FILES ${NLMISC_STREAM})
SOURCE_GROUP("displayer" FILES ${NLMISC_DISPLAYER})
SOURCE_GROUP("math" FILES ${NLMISC_MATH})
SOURCE_GROUP("generic" FILES ${NLMISC_GENERIC})
SOURCE_GROUP("utility" FILES ${NLMISC_UTILITY})
SOURCE_GROUP("bitmap" FILES ${NLMISC_BITMAP})
SOURCE_GROUP("thread" FILES ${NLMISC_THREAD})
SOURCE_GROUP("i18n" FILES ${NLMISC_I18N})
SOURCE_GROUP("crypt" FILES ${NLMISC_CRYPT})
SOURCE_GROUP("string" FILES ${NLMISC_STRING})

NL_TARGET_LIB(nelmisc ${HEADERS} ${SRC})

IF(WITH_GTK)
  IF(GTK2_FOUND)
    INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
    ADD_DEFINITIONS(-DNL_USE_GTK)
    TARGET_LINK_LIBRARIES(nelmisc ${GTK2_LIBRARIES})
  ENDIF()
ENDIF()

IF(JPEG_FOUND)
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
  ADD_DEFINITIONS(-DUSE_JPEG)
  TARGET_LINK_LIBRARIES(nelmisc ${JPEG_LIBRARY})
ENDIF()

IF(GIF_FOUND)
  INCLUDE_DIRECTORIES(${GIF_INCLUDE_DIR})
  ADD_DEFINITIONS(-DUSE_GIF)
  TARGET_LINK_LIBRARIES(nelmisc ${GIF_LIBRARY})
ENDIF()

IF(WITH_STATIC OR WIN32)
  TARGET_LINK_LIBRARIES(nelmisc ${PNG_LIBRARIES})
ELSE()
  # Link only with libpng shared library
  TARGET_LINK_LIBRARIES(nelmisc ${PNG_LIBRARY})
ENDIF()

IF(UNIX)
  TARGET_LINK_LIBRARIES(nelmisc -lc -ldl)
  IF(APPLE)
    TARGET_LINK_LIBRARIES(nelmisc ${CARBON_FRAMEWORK})
  ELSE()
    TARGET_LINK_LIBRARIES(nelmisc -lrt)
  ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(../../3rdparty)
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${PNG_INCLUDE_DIR} config_file)

TARGET_LINK_LIBRARIES(nelmisc ${CMAKE_THREAD_LIBS_INIT} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARY} nel_sevenzip)

NL_DEFAULT_PROPS(nelmisc "NeL, Library: NeL Misc")
NL_ADD_RUNTIME_FLAGS(nelmisc)

NL_ADD_LIB_SUFFIX(nelmisc)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nelmisc ${CMAKE_CURRENT_SOURCE_DIR}/stdmisc.h ${CMAKE_CURRENT_SOURCE_DIR}/stdmisc.cpp)
ENDIF()

NL_GEN_PC(nel-misc.pc)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS nelmisc LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
ENDIF()
