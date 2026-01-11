/*
 * Copyright (C) 2012  by Jan Boon (Kaetemi)
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
#include <fstream>

#include <vector>
#include <utility>

#include <nel/misc/file.h>
#include <nel/misc/vector.h>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/class_data.h"
#include "../pipeline_max/config.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

// Testing
#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"

#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/storage/geom_buffers.h"
#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/update1/editable_mesh.h"
#include "../pipeline_max/epoly/editable_poly.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BUILTIN::STORAGE;
using namespace PIPELINE::MAX::UPDATE1;
using namespace PIPELINE::MAX::EPOLY;

//static const char *filename = "/srv/work/database/interfaces/anims_max/cp_fy_hof_species.max";
//static const char *filename = "/home/kaetemi/source/minimax/GE_Acc_MikotoBaniere.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/test2008.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/teapot_test_scene.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/testplane.max";
//static const char *filename = "/home/kaetemi/3dsMax/scenes/geomobjects.max";
static const char *filename = "/mnt/tsurugi/ryzom-assets/database/landscape/ligo/desert/pipeline_max/zonematerial-converted-165_eg.max";
static const char *streamname = "Scene";

#define PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID 0x0100
#define PBMS_GEOM_BUFFERS_POLY_A_EDGE_CHUNK_ID 0x010a
#define PBMS_GEOM_BUFFERS_POLY_A_FACE_CHUNK_ID 0x011a
// CStorageArraySizePre<CGeomPolyVertexInfo>
// CStorageArraySizePre<CGeomPolyEdgeInfo>
// CStorageArrayDynSize<CGeomPolyFaceInfo>

void exportObj(const std::string &fileName, const CReferenceMaker *geomObject)
{
	nlassert(dynamic_cast<const CGeomObject *>(geomObject));
	const CEditableMesh *mesh = dynamic_cast<const CEditableMesh *>(geomObject);
	if (mesh)
	{
		nlerror("Not implemented!");
		/*
		IStorageObject *bufferBlock = triObject->findStorageObject(0x08fe);
		nlassert(bufferBlock->isContainer());
		CStorageContainer *buffers = static_cast<CStorageContainer *>(bufferBlock);
		CStorageArraySizePre<NLMISC::CVector> *vertexBuffer = static_cast<CStorageArraySizePre<NLMISC::CVector> *>(buffers->findStorageObject(0x0914));
		CStorageArraySizePre<CGeomTriIndexInfo> *indexBuffer = static_cast<CStorageArraySizePre<CGeomTriIndexInfo> *>(buffers->findStorageObject(0x0912));

		std::ofstream ofs(fileName.c_str());
		for (uint i = 0; i < vertexBuffer->Value.size(); ++i)
			ofs << "v " << vertexBuffer->Value[i].x << " " << vertexBuffer->Value[i].y << " " << vertexBuffer->Value[i].z << "\n";
		for (uint i = 0; i < indexBuffer->Value.size(); ++i)
			ofs << "f " << (indexBuffer->Value[i].a + 1) << " " << (indexBuffer->Value[i].b + 1) << " " << (indexBuffer->Value[i].c + 1) << "\n"; // + 1 as .obj indexes at 1...
		*/
		return;
	}
	const CEditablePoly *poly = dynamic_cast<const CEditablePoly *>(geomObject);
	if (poly)
	{
		CGeomBuffers *geomBuffers = poly->geomBuffers();
		CStorageArraySizePre<CGeomPolyVertexInfo> *vertexBuffer = static_cast<CStorageArraySizePre<CGeomPolyVertexInfo> *>(geomBuffers->findStorageObject(PBMS_GEOM_BUFFERS_POLY_A_VERTEX_CHUNK_ID));
		CStorageArraySizePre<CGeomPolyEdgeInfo> *edgeBuffer = static_cast<CStorageArraySizePre<CGeomPolyEdgeInfo> *>(geomBuffers->findStorageObject(PBMS_GEOM_BUFFERS_POLY_A_EDGE_CHUNK_ID));
		CStorageArrayDynSize<CGeomPolyFaceInfo> *faceBuffer = static_cast<CStorageArrayDynSize<CGeomPolyFaceInfo> *>(geomBuffers->findStorageObject(PBMS_GEOM_BUFFERS_POLY_A_FACE_CHUNK_ID));
		nlassert(vertexBuffer);
		nlassert(edgeBuffer);
		nlassert(faceBuffer);
		std::ofstream ofs(fileName.c_str());
		for (uint i = 0; i < vertexBuffer->Value.size(); ++i)
			ofs << "v " << vertexBuffer->Value[i].v.x << " " << vertexBuffer->Value[i].v.y << " " << vertexBuffer->Value[i].v.z << "\n";
		std::vector<CGeomTriIndex> triangles;
		for (uint i = 0; i < faceBuffer->Value.size(); ++i)
		{
			CGeomObject::triangulatePolyFace(triangles, faceBuffer->Value[i]);
			for (uint j = 0; j < triangles.size(); ++j)
			{
				ofs << "f " << (triangles[j].a + 1) << " " << (triangles[j].b + 1) << " " << (triangles[j].c + 1) << "\n"; // + 1 as .obj indexes at 1...
			}
			triangles.clear();
		}
		return;
	}
	nlerror("Not handled!");
}

// int __stdcall WinMain(void *, void *, void *, int)
int main(int argc, char **argv)
{
	//printf("Pipeline Max Dump (Temporary Tool)\n");

	char const *me = (argv[0] ? argv[0] : "pipeline_max_dump");
	g_set_prgname(me);
	gsf_init();

	// Register all plugin classes
	CSceneClassRegistry sceneClassRegistry;
	CBuiltin::registerClasses(&sceneClassRegistry);
	CUpdate1::registerClasses(&sceneClassRegistry);
	CEPoly::registerClasses(&sceneClassRegistry);

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
	dllDirectory.parse(PIPELINE::MAX::VersionUnknown); // parse the structure to readable data
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
	classDirectory3.parse(PIPELINE::MAX::VersionUnknown); // parse the structure to readable data
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
	scene.parse(PIPELINE::MAX::VersionUnknown); // parse the structure to readable data
	scene.clean(); // cleanup unused file structure
	// TEST ->
	nldebug("BUILD");
	scene.build(PIPELINE::MAX::VersionUnknown);
	nldebug("DISOWN");
	scene.disown();
	nldebug("PARSE");
	scene.parse(PIPELINE::MAX::VersionUnknown); // parse the structure to readable data
	nldebug("CLEAN");
	//scene.clean(); // cleanup unused file structure, don't clean up if we want direct access to chunks as well
	// <- TEST
	scene.toString(std::cout);//##
	std::cout << "\n";
	//classDirectory3.build(PIPELINE::MAX::VersionUnknown);
	//classDirectory3.disown();
	//classDirectory3.toString(std::cout);
	//std::cout << "\n";

	std::cout << "\n";
	scene.container()->scene()->rootNode()->dumpNodes(std::cout);
	std::cout << "\n";

	//PIPELINE::MAX::BUILTIN::INode *node = scene.container()->scene()->rootNode()->find(ucstring("TR_HOF_civil01_gilet")); nlassert(node);
	//node->toString(std::cout);
	//exportObj("tr_hof_civil01_gilet.obj", node->getReference(1)->getReference(1)); // => CDerivedObject::getBase(node->object())

	/*INode *node = scene.container()->scene()->rootNode()->find(ucstring("GE_Acc_MikotoBaniere")); nlassert(node);
	//INode *node = scene.container()->scene()->rootNode()->find(ucstring("testplane")); nlassert(node);
	CReferenceMaker *object = node->getReference(1);
	object->toString(std::cout);
	exportObj("ge_acc_mikotobaniere.obj", object);*/


	//GE_Acc_MikotoBaniere

	// TEST APP DATA ->
/*
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
				CStorageRaw *raw = appData->get<CStorageRaw>(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL3D_APPDATA_ENV_FX);
				if (raw)
				{
					nlinfo("Found NEL3D_APPDATA_ENV_FX, size %i", raw->Value.size());
					//raw->Value.resize(200);
				}

			}
		}
	}*/
	// <- TEST APP DATA
	
	
/*
	scene.clean();
	scene.build(PIPELINE::MAX::VersionUnknown);
	scene.disown();
	scene.parse(PIPELINE::MAX::VersionUnknown);

	ssc = scene.container();
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
				const CStorageRaw *raw = appData->get<CStorageRaw>(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL3D_APPDATA_ENV_FX);
				if (raw)
				{
					nlinfo("Found NEL3D_APPDATA_ENV_FX, size %i", raw->Value.size());
				}

			}
		}
	}
	*/

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

