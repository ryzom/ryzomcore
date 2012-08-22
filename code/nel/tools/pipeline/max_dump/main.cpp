
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-doc-meta-data.h>
#include <gsf/gsf-msole-utils.h>
#include <glib/gi18n.h>
#include <string.h>
#include <cstdio>
#include <iostream>

#include <vector>
#include <utility>

#include <nel/misc/file.h>

#include "../max/storage_stream.h"
#include "../max/storage_object.h"
#include "../max/dll_directory.h"
#include "../max/class_directory_3.h"
#include "../max/class_data.h"
#include "../max/config.h"
#include "../max/scene.h"
#include "../max/scene_class_registry.h"

// Testing
#include "../max/builtin/storage/app_data.h"
#include "../max/builtin/builtin.h"

static const char *filename = "/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max";
//static const char *filename = "/home/kaetemi/source/minimax/GE_Acc_MikotoBaniere.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/test2008.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/teapot_test_scene.max";
static const char *streamname = "Scene";

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	//printf("Pipeline Max Dump (Temporary Tool)\n");

	char const *me = (argv[0] ? argv[0] : "pipeline_max_dump");
	g_set_prgname(me);
	gsf_init();


	PIPELINE::MAX::CSceneClassRegistry sceneClassRegistry;
	PIPELINE::MAX::BUILTIN::CBuiltin::registerClasses(&sceneClassRegistry);


	GsfInfile *infile;
	GError *error = NULL;
	GsfInput *src;
	char *display_name;

	src = gsf_input_stdio_new(filename, &error);

	if (error)
	{
		display_name = g_filename_display_name(filename);
		g_printerr (_("%s: Failed to open %s: %s\n"),
			    g_get_prgname (),
			    display_name,
			    error->message);
		g_free(display_name);
		return 1;
	}

	infile = gsf_infile_msole_new(src, NULL);

	if (!infile)
	{
		display_name = g_filename_display_name(filename);
		g_printerr (_("%s: Failed to recognize %s as an archive\n"),
				g_get_prgname (),
				display_name);
		g_free (display_name);
		return 1;
	}

	display_name = g_filename_display_name(filename);
	//g_print("%s\n", display_name);
	g_free(display_name);
	// g_print("%s\n", streamname);
	std::cout << "\n";

	GsfInput *input = NULL;


	PIPELINE::MAX::CDllDirectory dllDirectory;
	input = gsf_infile_child_by_name(infile, "DllDirectory");
	{
		PIPELINE::MAX::CStorageStream instream(input);
		PIPELINE::MAX::CStorageContainer ctr;
		ctr.serial(instream);
		{
			NLMISC::COFile of("temp.bin");
			ctr.serial(of); // out
			// nldebug("Written %i bytes", of.getPos());
		}
		{
			NLMISC::CIFile inf("temp.bin");
			dllDirectory.serial(inf); // in
		}
		//dllDirectory.serial(instream);
	}
	g_object_unref(input);
	//dllDirectory.toString(std::cout);
	//std::cout << "\n";
	dllDirectory.parse(PIPELINE::MAX::VersionUnknown, PIPELINE::MAX::PARSE_INTERNAL); // parse the structure to readable data
	dllDirectory.clean(); // cleanup unused file structure
	dllDirectory.toString(std::cout);
	std::cout << "\n";
	//dllDirectory.build(PIPELINE::MAX::VersionUnknown);
	//dllDirectory.disown();
	//dllDirectory.toString(std::cout);
	//std::cout << "\n";


	std::cout << "\n";


	PIPELINE::MAX::CClassDirectory3 classDirectory3(&dllDirectory);
	input = gsf_infile_child_by_name(infile, "ClassDirectory3");
	{
		PIPELINE::MAX::CStorageStream instream(input);
		classDirectory3.serial(instream);
	}
	g_object_unref(input);
	//classDirectory3.toString(std::cout);
	//std::cout << "\n";
	classDirectory3.parse(PIPELINE::MAX::VersionUnknown, PIPELINE::MAX::PARSE_INTERNAL); // parse the structure to readable data
	classDirectory3.clean(); // cleanup unused file structure
	classDirectory3.toString(std::cout);
	std::cout << "\n";
	//classDirectory3.build(PIPELINE::MAX::VersionUnknown);
	//classDirectory3.disown();
	//classDirectory3.toString(std::cout);
	//std::cout << "\n";


	std::cout << "\n";


	PIPELINE::MAX::CScene scene(&sceneClassRegistry, &dllDirectory, &classDirectory3);
	//PIPELINE::MAX::CStorageContainer scene;
	input = gsf_infile_child_by_name(infile, "Scene");
	{
		PIPELINE::MAX::CStorageStream instream(input);
		scene.serial(instream);
	}
	g_object_unref(input);
	//classDirectory3.toString(std::cout);
	//std::cout << "\n";
	scene.parse(PIPELINE::MAX::VersionUnknown, (PIPELINE::MAX::TParseLevel)(
		PIPELINE::MAX::PARSE_INTERNAL
		| PIPELINE::MAX::PARSE_BUILTIN)); // parse the structure to readable data
	scene.clean(); // cleanup unused file structure
	// TEST ->
	scene.build(PIPELINE::MAX::VersionUnknown);
	scene.disown();
	scene.parse(PIPELINE::MAX::VersionUnknown, (PIPELINE::MAX::TParseLevel)(
		PIPELINE::MAX::PARSE_INTERNAL
		| PIPELINE::MAX::PARSE_BUILTIN)); // parse the structure to readable data
	scene.clean(); // cleanup unused file structure
	// <- TEST
	scene.toString(std::cout);
	std::cout << "\n";
	//classDirectory3.build(PIPELINE::MAX::VersionUnknown);
	//classDirectory3.disown();
	//classDirectory3.toString(std::cout);
	//std::cout << "\n";


	// TEST APP DATA

#define MAXSCRIPT_UTILITY_CLASS_ID (NLMISC::CClassId(0x04d64858, 0x16d1751d))
#define UTILITY_CLASS_ID (4128)
#define NEL3D_APPDATA_ENV_FX (84682543)

	PIPELINE::MAX::CSceneClassContainer *ssc = scene.container();
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(), end = ssc->chunks().end(); it != end; ++it)
	{
		PIPELINE::MAX::CStorageContainer *subc = static_cast<PIPELINE::MAX::CStorageContainer *>(it->second);
		for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt subit = subc->chunks().begin(), subend = subc->chunks().end(); subit != subend; ++subit)
		{
			PIPELINE::MAX::IStorageObject *storageChunk = subit->second;
			PIPELINE::MAX::BUILTIN::STORAGE::CAppData *appData = dynamic_cast<PIPELINE::MAX::BUILTIN::STORAGE::CAppData *>(storageChunk);
			if (appData)
			{
				nlinfo("Found AppData");
				uint32 size;
				const uint8 *buffer = appData->read(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL3D_APPDATA_ENV_FX, size);
				if (buffer)
				{
					nlinfo("Found NEL3D_APPDATA_ENV_FX, size %i", size);
				}
			}
		}
	}

/*
	GsfInput *input = gsf_infile_child_by_name(infile, streamname);

	{
		//gsf_input_dump(input, 1); // just a regular hex dump of this input stream
		PIPELINE::MAX::CStorageStream instream(input);
		//dumpContainer(instream, "");
		PIPELINE::MAX::CScene ctr;
		ctr.serial(instream);
		ctr.toString(std::cout);
		std::cout << "\n";
		//ctr.dump("");
	}

	g_object_unref(input);
	*/


	g_object_unref(infile);
	g_object_unref(src);

	gsf_shutdown();

	return 0;
}

