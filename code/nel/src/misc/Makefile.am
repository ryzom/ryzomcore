#
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = \
	aabbox.cpp \
	app_context.cpp \
	algo.cpp \
	async_file_manager.cpp \
	big_file.cpp \
	bit_mem_stream.cpp \
	bit_set.cpp \
	bitmap.cpp \
	bitmap_jpeg.cpp \
	bitmap_png.cpp \
	block_memory.cpp \
	bsphere.cpp \
	buf_fifo.cpp \
	class_id.cpp \
	class_registry.cpp \
	command.cpp \
	common.cpp \
	contiguous_block_allocator.cpp \
	cpu_info.cpp \
	cpu_time_stat.cpp \
	debug.cpp \
	di_event_emitter.cpp \
	di_game_device.cpp \
	di_game_device.h \
	di_keyboard_device.cpp \
	di_keyboard_device.h \
	di_mouse_device.cpp \
	di_mouse_device.h \
	diff_tool.cpp \
	displayer.cpp \
	eid_translator.cpp \
	entity_id.cpp \
	eval_num_expr.cpp \
	event_emitter.cpp \
	event_emitter_multi.cpp \
	event_listener.cpp \
	event_server.cpp \
	events.cpp \
	fast_floor.cpp \
	fast_mem.cpp \
	file.cpp \
	fixed_size_allocator.cpp \
	game_device.cpp \
	game_device_events.cpp \
	geom_ext.cpp \
	grid_traversal.cpp \
	gtk_displayer.cpp \
	heap_memory.cpp \
	hierarchical_timer.cpp \
	i18n.cpp \
	i_xml.cpp \
	input_device.cpp \
	input_device_server.cpp \
	keyboard_device.cpp \
	line.cpp \
	log.cpp \
	matrix.cpp \
	md5.cpp \
	mem_displayer.cpp \
	mem_stream.cpp \
	mouse_smoother.cpp \
	mutex.cpp \
	noise_value.cpp \
	o_xml.cpp \
	object_arena_allocator.cpp \
	object_vector.cpp \
	p_thread.cpp \
	path.cpp \
	plane.cpp \
	polygon.cpp \
	progress_callback.cpp \
	quad.cpp \
	quat.cpp \
	reader_writer.cpp \
	rect.cpp \
	report.cpp \
	rgba.cpp \
	sha1.cpp \
	shared_memory.cpp \
	sheet_id.cpp \
	smart_ptr.cpp \
	stdmisc.cpp \
	stdmisc.h \
	stl_block_allocator.cpp \
	stl_block_list.cpp \
	stop_watch.cpp \
	stream.cpp \
	string_common.cpp \
	string_id_array.cpp \
	string_mapper.cpp \
	system_info.cpp \
	task_manager.cpp \
	tds.cpp \
	time_nl.cpp \
	triangle.cpp \
	uv.cpp \
	unicode.cpp \
	value_smoother.cpp \
	variable.cpp \
	vector.cpp \
	vector_2d.cpp \
	vector_2f.cpp \
	vector_h.cpp \
	vectord.cpp \
	win_displayer.cpp \
	win_event_emitter.cpp \
	win_thread.cpp \
	window_displayer.cpp \
	words_dictionary.cpp \
	dynloadlib.cpp \
	sstring.cpp \
	co_task.cpp \
	xml_pack.cpp \
	inter_window_msg_queue.cpp \
	win32_util.cpp


noinst_HEADERS        = di_game_device.h \
                        di_keyboard_device.h \
                        di_mouse_device.h \
                        stdmisc.h


libnelmisc_la_LIBADD  = config_file/libconfig.la -lc -lpthread -lrt -ldl -lpng -ljpeg

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nel-misc.pc

# End of Makefile.am

