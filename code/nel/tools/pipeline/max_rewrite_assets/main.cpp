
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>

#include <gsf/gsf-outfile-msole.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-output-stdio.h>
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
#include <fstream>

#include <vector>
#include <utility>

#include <nel/misc/file.h>
#include <nel/misc/vector.h>

#include "../max/storage_stream.h"
#include "../max/storage_object.h"
#include "../max/dll_directory.h"
#include "../max/class_directory_3.h"
#include "../max/class_data.h"
#include "../max/config.h"
#include "../max/scene.h"
#include "../max/scene_class_registry.h"

// Testing
#include "../max/builtin/builtin.h"
#include "../max/update1/update1.h"
#include "../max/epoly/epoly.h"

#include "../max/builtin/storage/app_data.h"
#include "../max/builtin/storage/geom_buffers.h"
#include "../max/builtin/scene_impl.h"
#include "../max/builtin/i_node.h"
#include "../max/update1/editable_mesh.h"
#include "../max/epoly/editable_poly.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BUILTIN::STORAGE;
using namespace PIPELINE::MAX::UPDATE1;
using namespace PIPELINE::MAX::EPOLY;

CSceneClassRegistry SceneClassRegistry;

bool DisplayStream = false;
bool WriteModified = false;
bool WriteDummy = true;

void serializeStorageContainer(PIPELINE::MAX::CStorageContainer *storageContainer, GsfInfile *infile, const char *streamName)
{
	GsfInput *input = gsf_infile_child_by_name(infile, streamName);
	if (!input)
	{
		nlerror("GSF Could not read stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream instream(input);
		storageContainer->serial(instream);
	}
	g_object_unref(input);
}

void serializeStorageContainer(PIPELINE::MAX::CStorageContainer *storageContainer, GsfOutfile *outfile, const char *streamName)
{
	//nldebug("write");
	GsfOutput *output = GSF_OUTPUT(gsf_outfile_new_child(outfile, streamName, false));
	if (!output)
	{
		nlerror("GSF Could not write stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream outstream(output);
		storageContainer->serial(outstream);
	}
	gsf_output_close(output);
	g_object_unref(G_OBJECT(output));
}

void serializeRaw(std::vector<uint8> &rawOutput, GsfInfile *infile, const char *streamName)
{
	GsfInput *input = gsf_infile_child_by_name(infile, streamName);
	if (!input)
	{
		nlerror("GSF Could not read stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream instream(input);
		rawOutput.resize(instream.size());
		instream.serialBuffer(&rawOutput[0], rawOutput.size());
	}
	g_object_unref(input);
}

void serializeRaw(std::vector<uint8> &rawOutput, GsfOutfile *outfile, const char *streamName)
{
	GsfOutput *output = GSF_OUTPUT(gsf_outfile_new_child(outfile, streamName, false));
	if (!output)
	{
		nlerror("GSF Could not write stream %s", streamName);
		return;
	}
	{
		PIPELINE::MAX::CStorageStream outstream(output);
		outstream.serialBuffer(&rawOutput[0], rawOutput.size());
	}
	gsf_output_close(output);
	g_object_unref(G_OBJECT(output));
}

void handleFile(const std::string &path)
{
	GError *err = NULL;

	GsfDocMetaData *metadata = gsf_doc_meta_data_new();
	nlassert(metadata);

	GsfInput *src = gsf_input_stdio_new(path.c_str(), &err);
	if (err) { nlerror("GSF Failed to open %s", path.c_str()); return; }

	GsfInfile *infile = gsf_infile_msole_new(src, NULL);
	if (!infile) { nlerror("GSF Failed to recognize %s", path.c_str()); return; }
	g_object_unref(src);

	uint8 classId[16];
	if (!gsf_infile_msole_get_class_id((GsfInfileMSOle *)infile, classId))
		nlerror("GSF Did not find classId");

	PIPELINE::MAX::CStorageContainer videoPostQueue;
	serializeStorageContainer(&videoPostQueue, infile, "VideoPostQueue");
	PIPELINE::MAX::CStorageContainer config;
	serializeStorageContainer(&config, infile, "Config");
	PIPELINE::MAX::CStorageContainer classData;
	serializeStorageContainer(&classData, infile, "ClassData");

	std::vector<uint8> summaryInformation;
	serializeRaw(summaryInformation, infile, "\05SummaryInformation");

	std::vector<uint8> documentSummaryInformation;
	serializeRaw(documentSummaryInformation, infile, "\05DocumentSummaryInformation"); // Can't read this, don't care.

	PIPELINE::MAX::CDllDirectory dllDirectory;
	serializeStorageContainer(&dllDirectory, infile, "DllDirectory");
	dllDirectory.parse(VersionUnknown);

	PIPELINE::MAX::CClassDirectory3 classDirectory3(&dllDirectory);
	serializeStorageContainer(&classDirectory3, infile, "ClassDirectory3");
	classDirectory3.parse(VersionUnknown);

	PIPELINE::MAX::CScene scene(&SceneClassRegistry, &dllDirectory, &classDirectory3);
	serializeStorageContainer(&scene, infile, "Scene");

	/*
	// Not parsing the scene for this function.
	scene.parse(VersionUnknown);
	scene.clean();
	scene.build(VersionUnknown);
	scene.disown();
	*/

	/*
	PIPELINE::MAX::CStorageContainer dllDirectory;
	serializeStorageContainer(&dllDirectory, infile, "DllDirectory");

	PIPELINE::MAX::CStorageContainer classDirectory3;
	serializeStorageContainer(&classDirectory3, infile, "ClassDirectory3");

	PIPELINE::MAX::CStorageContainer scene;
	serializeStorageContainer(&scene, infile, "Scene");
	*/

	if (DisplayStream)
	{
		videoPostQueue.toString(std::cout);
		config.toString(std::cout);
		classData.toString(std::cout);
		dllDirectory.toString(std::cout);
		classDirectory3.toString(std::cout);
		scene.toString(std::cout);
	}

	g_object_unref(infile);

	dllDirectory.disown();
	classDirectory3.disown();
	if (WriteModified)
	{
		const char *outpath = (WriteDummy ? "testdummy.max" : path.c_str());
		GsfOutput  *output;
		GsfOutfile *outfile;

		output = gsf_output_stdio_new(outpath, &err);
		if (err) { nlerror("GSF Failed to create %s", outpath); return; }
		outfile = gsf_outfile_msole_new(output);
		g_object_unref(G_OBJECT(output));

		serializeStorageContainer(&videoPostQueue, outfile, "VideoPostQueue");
		serializeStorageContainer(&scene, outfile, "Scene");
		serializeStorageContainer(&dllDirectory, outfile, "DllDirectory");
		serializeStorageContainer(&config, outfile, "Config");
		serializeStorageContainer(&classDirectory3, outfile, "ClassDirectory3");
		serializeStorageContainer(&classData, outfile, "ClassData");
		serializeRaw(summaryInformation, outfile, "\05SummaryInformation");
		serializeRaw(documentSummaryInformation, outfile, "\05DocumentSummaryInformation");

		if (!gsf_outfile_msole_set_class_id((GsfOutfileMSOle *)outfile, classId))
			nlerror("GSF Cannot write class id");

		gsf_output_close(GSF_OUTPUT(outfile));
		g_object_unref(G_OBJECT(outfile));
	}

	g_object_unref(metadata);
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	// Initialise gsf
	printf("Pipeline Max Rewrite Assets\n");
	char const *me = (argv[0] ? argv[0] : "pipeline_max_rewrite_assets");
	g_set_prgname(me);
	gsf_init();

	// Register all plugin classes
	CBuiltin::registerClasses(&SceneClassRegistry);
	CUpdate1::registerClasses(&SceneClassRegistry);
	CEPoly::registerClasses(&SceneClassRegistry);

	handleFile("/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max");

	gsf_shutdown();

	return 0;
}

