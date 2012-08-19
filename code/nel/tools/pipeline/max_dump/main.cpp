
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

#include "../max/storage_stream.h"
#include "../max/storage_object.h"
#include "../max/dll_directory.h"
#include "../max/class_directory_3.h"
#include "../max/class_data.h"
#include "../max/config.h"
#include "../max/scene.h"

//static const char *filename = "/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max";
static const char *filename = "/home/kaetemi/source/minimax/GE_Acc_MikotoBaniere.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/test2008.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/teapot_test_scene.max";
static const char *streamname = "Scene";

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	printf("Pipeline Max Dump (Temporary Tool)\n");

	char const *me = (argv[0] ? argv[0] : "pipeline_max_dump");
	g_set_prgname(me);
	gsf_init();

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
	g_print("%s\n", display_name);
	g_free(display_name);
	g_print("%s\n", streamname);

	GsfInput *input = NULL;

	PIPELINE::MAX::CDllDirectory dllDirectory;
	input = gsf_infile_child_by_name(infile, "DllDirectory");
	{
		PIPELINE::MAX::CStorageStream instream(input);
		dllDirectory.serial(instream);
	}
	g_object_unref(input);
	dllDirectory.toString(std::cout);
	std::cout << "\n";
	dllDirectory.parse(PIPELINE::MAX::VersionUnknown, PIPELINE::MAX::PARSE_INTERNAL);
	dllDirectory.toString(std::cout);
	std::cout << "\n";


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

	return 0;
}

