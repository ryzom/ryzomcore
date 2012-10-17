// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std_afx.h"
#include "nel_export.h"
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/foundation/strings.h>
#else
#	include <MaxScrpt/strings.h>
#endif
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"

// From max
#include <notify.h>


#define EXPORT_GET_ALLOCATOR

#include "nel/misc/debug.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

/*===========================================================================*\
 |	Define our new functions
 |	These will turn on texture map display at all levels for a given object
\*===========================================================================*/

def_visible_primitive ( export_shape,		"NelExportShape");
def_visible_primitive ( export_shape_ex,	"NelExportShapeEx");
def_visible_primitive ( export_skeleton,	"NelExportSkeleton");
def_visible_primitive ( export_animation,	"NelExportAnimation");
def_visible_primitive ( export_ig,			"NelExportInstanceGroup");
def_visible_primitive ( export_skeleton_weight,		"NelExportSkeletonWeight");
def_visible_primitive ( view_shape,			"NelViewShape");
def_visible_primitive ( test_file_date,		"NeLTestFileDate");
def_visible_primitive ( export_vegetable,	"NelExportVegetable");
def_visible_primitive ( reload_texture,		"NelReloadTexture" );
def_visible_primitive ( export_collision,	"NelExportCollision" );
def_visible_primitive ( export_pacs_primitives,	"NelExportPACSPrimitives" );
def_visible_primitive ( export_lod_character,	"NelExportLodCharacter" );
def_visible_primitive ( node_properties,	"NelNodeProperties" );
def_visible_primitive ( mirror_physique, 	"NelMirrorPhysique" );
def_visible_primitive ( get_file_modification_date, 	"NeLGetFileModificationDate" );
def_visible_primitive ( set_file_modification_date, 	"NeLSetFileModificationDate" );

def_visible_primitive ( force_quit_on_msg_displayer,		"NelForceQuitOnMsgDisplayer");
def_visible_primitive ( force_quit_right_now,		"NelForceQuitRightNow");

char *sExportShapeErrorMsg = "NeLExportShape [Object] [Filename.shape]";
char *sExportShapeExErrorMsg = "NeLExportShapeEx [Object] [Filename.shape] [bShadow] [bExportLighting] [sLightmapPath] [nLightingLimit] [fLumelSize] [nOverSampling] [bExcludeNonSelected] [bShowLumel]";
char *sExportAnimationErrorMsg = "NelExportAnimation [node array] [Filename.anim] [bool_scene_animation]";
char *sExportCollisionErrorMsg = "NelExportCollision [node array] [output directory]";
char *sExportPACSPrimitivesErrorMsg = "NelExportPACSPrimitves [node array] [output filename]";

extern CExportNelOptions theExportSceneStruct;

Value* export_shape_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], MAXNode, sExportShapeErrorMsg);
	type_check (arg_list[1], String, sExportShapeErrorMsg);

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Ok ?
	Boolean *ret=&false_value;

	try
	{

		// Is the flag dont export set ?
		if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, 0))
			return ret;
		// Object is flagged as a collision?
		int	bCol= CExportNel::getScriptAppData(node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		if(bCol == BST_CHECKED)
			return ret;

		// Export
		theCNelExport._ExportNel->deleteLM( *node);
		if (theCNelExport.exportMesh (sPath, *node, ip->GetTime()))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportShape) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportShape) catch (...)");
	}
	nlinfo("ret");

	return ret;
}


Value* export_shape_ex_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (11)
	check_arg_count(export_shape, 11, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], MAXNode, sExportShapeExErrorMsg);
	type_check (arg_list[1], String, sExportShapeExErrorMsg);
	type_check (arg_list[2], Boolean, sExportShapeExErrorMsg);
	type_check (arg_list[3], Boolean, sExportShapeExErrorMsg);
	type_check (arg_list[4], String, sExportShapeExErrorMsg);
	type_check (arg_list[5], Integer, sExportShapeExErrorMsg);
	type_check (arg_list[6], Float, sExportShapeExErrorMsg);
	type_check (arg_list[7], Integer, sExportShapeExErrorMsg);
	type_check (arg_list[8], Boolean, sExportShapeExErrorMsg);
	type_check (arg_list[9], Boolean, sExportShapeExErrorMsg);
	type_check (arg_list[10], Boolean, sExportShapeExErrorMsg);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert(node);
	nlassert(node->GetName());

	// Export path 
	std::string sPath=arg_list[1]->to_string();

	// Ex argu
	theExportSceneStruct.bShadow = arg_list[2]->to_bool()!=FALSE;
	theExportSceneStruct.bExportLighting = arg_list[3]->to_bool()!=FALSE;
	theExportSceneStruct.sExportLighting = arg_list[4]->to_string();
	theExportSceneStruct.nExportLighting = arg_list[5]->to_int();
	theExportSceneStruct.rLumelSize = arg_list[6]->to_float();
	theExportSceneStruct.nOverSampling = arg_list[7]->to_int();
	theExportSceneStruct.bExcludeNonSelected = arg_list[8]->to_bool()!=FALSE;
	theExportSceneStruct.bShowLumel = arg_list[9]->to_bool()!=FALSE;
	theExportSceneStruct.OutputLightmapLog = arg_list[10]->to_bool()!=FALSE;

	theCNelExport.init (false, false, ip, false);

	// Ok ?
	Boolean *ret=&false_value;

	// Is the flag dont export set ?
	if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, 0))
		return ret;
	// Object is flagged as a collision?
	int	bCol= CExportNel::getScriptAppData(node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
	if(bCol == BST_CHECKED)
		return ret;

	try
	{
		// Export
		theCNelExport._ExportNel->deleteLM( *node );
		if (theCNelExport.exportMesh (sPath.c_str(), *node, ip->GetTime()))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportShapeEx) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportShapeEx) catch (...)");
	}
	nlinfo("ret");
	return ret;
}


Value* export_skeleton_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], MAXNode, "NelExportSkeleton [root node] [Filename]");
	type_check (arg_list[1], String, "NelExportSkeleton [root node] [Filename]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		// Export
		if (theCNelExport.exportSkeleton (sPath, node, ip->GetTime()))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportSkeleton) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportSkeleton) catch (...)");
	}

	return ret;
}


Value* export_animation_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 3, count);

	// Check to see if the arguments match up to what we expect
	type_check (arg_list[0], Array, sExportAnimationErrorMsg);
	type_check (arg_list[1], String, sExportAnimationErrorMsg);
	type_check (arg_list[2], Boolean, sExportAnimationErrorMsg);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Get time
	TimeValue time=MAXScript_interface->GetTime();
	
	// Get array
	Array* array=(Array*)arg_list[0];

	// Check each value in the array
	uint i;
	for (i=0; i<(uint)array->size; i++)
		type_check (array->get (i+1), MAXNode, sExportAnimationErrorMsg);

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		// Save all selected objects
		if (array->size)
		{
			// Make a list of nodes
			std::vector<INode*> vectNode;
			for (i=0; i<(uint)array->size; i++)
				vectNode.push_back (array->get (i+1)->to_node());

			// Scene anim ?
			BOOL scene=arg_list[2]->to_bool();

			// Export the zone
			if (theCNelExport.exportAnim (sPath, vectNode, time, scene!=FALSE))
			{
				// Ok
				ret=&true_value;
			}
			else
			{
				// Error message
				mprintf ("Error exporting animation %s in the file\n%s\n", (*vectNode.begin())->GetName(), sPath);
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportAnimation) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportAnimation) catch (...)");
	}
	return ret;
}


Value* export_ig_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], Array, "NelExportInstanceGroup [Object array] [Filename]");
	type_check (arg_list[1], String, "NelExportInstanceGroup [Object array] [Filename]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		if (is_array (arg_list[0]))
		{
			// Get array
			Array* array=(Array*)arg_list[0];

			// Check each value in the array
			uint i;
			for (i=0; i<(uint)array->size; i++)
				type_check (array->get (i+1), MAXNode, "NelExportInstanceGroup [Object array] [Filename]");

			// Create a STL array
			if (array->size)
			{
				std::vector<INode*> vect;
				for (uint i=0; i<(uint)array->size; i++)
					vect.push_back (array->get (i+1)->to_node());
					
				// Export path 
				const char* sPath=arg_list[1]->to_string();

				// Export
				if (theCNelExport.exportInstanceGroup (sPath, vect))
					ret=&true_value;
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportInstanceGroup) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportInstanceGroup) catch (...)");
	}

	return ret;
}


Value* export_skeleton_weight_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], Array, "NelExportSkeletonWeight [Object array] [Filename]");
	type_check (arg_list[1], String, "NelExportSkeletonWeight [Object array] [Filename]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		if (is_array (arg_list[0]))
		{
			// Get array
			Array* array=(Array*)arg_list[0];

			// Check each value in the array
			uint i;
			for (i=0; i<(uint)array->size; i++)
				type_check (array->get (i+1), MAXNode, "NelExportSkeletonWeight [Object array] [Filename]");

			// Create a STL array
			if (array->size)
			{
				std::vector<INode*> vect;
				for (uint i=0; i<(uint)array->size; i++)
					vect.push_back (array->get (i+1)->to_node());
					
				// Export path 
				const char* sPath=arg_list[1]->to_string();

				// Export
				if (theCNelExport.exportSWT (sPath, vect))
					ret=&true_value;
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportSkeletonWeight) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportSkeletonWeight) catch (...)");
	}

	return ret;
}

Value* view_shape_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(view_shape, 0, count);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	try 
	{
		theCNelExport.init (true, true, ip, true);

		theCNelExport.viewMesh (ip->GetTime());		
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR catch (...)");
	}

	return &true_value;
}

Value* test_file_date_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(view_shape, 2, count);

	type_check (arg_list[0], String, "NeLTestFileDate [DestFilename] [SrcFilename]");
	type_check (arg_list[1], String, "NeLTestFileDate [DestFilename] [SrcFilename]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// The 2 filenames
	string file0 = arg_list[0]->to_string();
	string file1 = arg_list[1]->to_string();

	// Open it
	FILE *file=fopen (file0.c_str(), "r");
	if (file == NULL)
		return &true_value;
	
	// Close it
	fclose (file);

	// Return value
	Value *ret = &undefined;

	// Create first file
	HANDLE h0 = CreateFile (file0.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h0!=INVALID_HANDLE_VALUE)
	{
		HANDLE h1 = CreateFile (file1.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h1!=INVALID_HANDLE_VALUE)
		{
			// Get file time
			FILETIME creationTime0;
			FILETIME lastAccessTime0;
			FILETIME lastWriteTime0;
			FILETIME creationTime1;
			FILETIME lastAccessTime1;
			FILETIME lastWriteTime1;

			if (GetFileTime (h0, &creationTime0, &lastAccessTime0, &lastWriteTime0))
			{
				if (GetFileTime (h1, &creationTime1, &lastAccessTime1, &lastWriteTime1))
				{
					// Compare date
					if (CompareFileTime (&lastWriteTime0, &lastWriteTime1)<=0)
						ret = &true_value;
					else
						ret= &false_value;
				}
			}
			CloseHandle (h1);
		}
		CloseHandle (h0);
	}

	return ret;
}

Value* export_vegetable_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(export_shape, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	char *message = "NelExportVegetable [node] [filename] [dialog error]";
	type_check (arg_list[0], MAXNode, message);
	type_check (arg_list[1], String, message);
	type_check (arg_list[2], Boolean, message);

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Message in dialog
	bool dialogMessage = arg_list[2]->to_bool() != FALSE;

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, dialogMessage, ip, true);

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		// Export
		if (theCNelExport.exportVegetable (sPath, *node, ip->GetTime()))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportVegetable) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportVegetable) catch (...)");
	}

	return ret;
}

Value* reload_texture_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(reload_texture, 1, count);
	char *message = "NelReloadTexture [BitmapTex]";
	//type_check (arg_list[0], TextureMap, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// The 2 filenames
	Texmap *texmap = arg_list[0]->to_texmap ();

	// BitmapTex ?
	if (texmap->ClassID() == Class_ID (BMTEX_CLASS_ID, 0))
	{
		// Cast
		BitmapTex *bitmap = (BitmapTex*)texmap;

		// Reload
		bitmap->ReloadBitmapAndUpdate ();

		// Tell the bitmap has changed
		BroadcastNotification (NOTIFY_BITMAP_CHANGED, (void *)bitmap->GetMapName());
		
		return &true_value;
	}

	return &false_value;
}

Value* export_collision_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	type_check (arg_list[0], Array, sExportCollisionErrorMsg);
	type_check (arg_list[1], String, sExportCollisionErrorMsg);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Export path 
	string sPath = arg_list[1]->to_string();

	// Get time
	TimeValue time = MAXScript_interface->GetTime();
	
	// Get array
	Array* array=(Array*)arg_list[0];

	// Array of INode *
	std::vector<INode *> nodes;
	nodes.reserve (array->size);

	// Check each value in the array
	uint i;
	for (i=0; i<(uint)array->size; i++)
	{
		type_check (array->get (i+1), MAXNode, sExportCollisionErrorMsg);

		// Add to the array of nodes
		nodes.push_back (array->get (i+1)->to_node());
	}

	// Ok ?
	Boolean *ret = &false_value;

	try
	{
		// Warning as the export options are not used, they are not loaded!

		// Export
		if (theCNelExport.exportCollision (sPath.c_str(), nodes, time))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportCollision) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportCollision) catch (...)");
	}
	return ret;
}

Value* export_pacs_primitives_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	type_check (arg_list[0], Array, sExportPACSPrimitivesErrorMsg);
	type_check (arg_list[1], String, sExportPACSPrimitivesErrorMsg);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, false, ip, true);

	// Export path 
	string sPath = arg_list[1]->to_string();

	// Get time
	TimeValue time = MAXScript_interface->GetTime();
	
	// Get array
	Array* array=(Array*)arg_list[0];

	// Array of INode *
	std::vector<INode *> nodes;
	nodes.reserve (array->size);

	// Check each value in the array
	uint i;
	for (i=0; i<(uint)array->size; i++)
	{
		type_check (array->get (i+1), MAXNode, sExportPACSPrimitivesErrorMsg);

		// Add to the array of nodes
		nodes.push_back (array->get (i+1)->to_node());
	}

	// Ok ?
	Boolean *ret = &false_value;

	try
	{
		// Export
		if (theCNelExport.exportPACSPrimitives (sPath.c_str(), nodes, time))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportPACSPrimitives) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportPACSPrimitives) catch (...)");
	}
	return ret;
}


Value* export_lod_character_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(export_lod_character, 3, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NelExportLodCharacter [node] [filename] [dialog error]";
	type_check (arg_list[0], MAXNode, message);
	type_check (arg_list[1], String, message);
	type_check (arg_list[2], Boolean, message);

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Message in dialog
	bool dialogMessage = arg_list[2]->to_bool() != FALSE;

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, dialogMessage, ip, true);

	// Ok ?
	Boolean *ret=&false_value;

	try
	{
		// Export
		if (theCNelExport.exportLodCharacter (sPath, *node, ip->GetTime()))
			ret = &true_value;
	}
	catch (Exception &e)
	{
		nlwarning ("ERROR (NelExportLodCharacter) %s", e.what());
	}
	catch (...)
	{
		nlwarning ("ERROR (NelExportLodCharacter) catch (...)");
	}

	return ret;
}


Value* node_properties_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(export_lod_character, 2, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NelNodeProperties [node_array] [dialog error]";

	//type_check (arg_list[0], MAXNode, message);
	type_check (arg_list[0], Array, message);
	type_check (arg_list[1], Boolean, message);

	// Get array
	Array* array=(Array*)arg_list[0];

	// Array of INode *
	std::set<INode *> nodes;

	// Check each value in the array
	uint i;
	for (i=0; i<(uint)array->size; i++)
	{
		type_check (array->get (i+1), MAXNode, "NelNodeProperties [node_array] [dialog error]");

		// Add to the array of nodes
		nodes.insert (array->get (i+1)->to_node());
	}

	// Message in dialog
	bool dialogMessage = arg_list[1]->to_bool() != FALSE;

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	theCNelExport.init (false, dialogMessage, ip, true);
	
	// Build a seleted set
	std::set<INode*> listNode;

	// Get the sel node count
	uint selNodeCount=ip->GetSelNodeCount();
	for (i=0; i<selNodeCount; i++)
	{
		// insert the node
		listNode.insert (ip->GetSelNode(i));
	}

	// Call the dialog
	theCNelExport.OnNodeProperties (listNode);

	return &true_value;
}


// ***************************************************************************
// Physique Mirror
Value* mirror_physique_cf (Value** arg_list, int count)
{
	uint i;
	
	// **** retrieve args.
	// Make sure we have the correct number of arguments (3)
	check_arg_count(NelMirrorPhysique , 3, count);
	
	// Check to see if the arguments match up to what we expect
	char *message = "NelMirrorPhysique [node] [vert_list_in] [threshold]";
	
	//type_check
	type_check (arg_list[0], MAXNode, message);
	type_check (arg_list[1], Array, message);
	type_check (arg_list[2], Float, message);
	
	// get the node
	INode	*node= arg_list[0]->to_node();
	
	// get vertices indices
	Array* array=(Array*)arg_list[1];
	std::vector<uint>	vertIn;
	vertIn.resize(array->size);
	for (i=0; i<(uint)array->size; i++)
	{
		type_check (array->get (i+1), Integer, message);
		vertIn[i]= array->get (i+1)->to_int() - 1;
	}
	
	// get threshold
	float	threshold= arg_list[2]->to_float();
	
	// **** Mirror!
	// Get time
	TimeValue time=MAXScript_interface->GetTime();
	
	// do it
	theCNelExport._ExportNel->mirrorPhysiqueSelection(*node, time, vertIn, threshold);
	
	return &true_value;
}


// ***************************************************************************

Value* get_file_modification_date_cf (Value** arg_list, int count)
{
	// **** retrieve args.
	// Make sure we have the correct number of arguments (3)
	check_arg_count(NeLGetFileModificationDate , 1, count);
	
	// Check to see if the arguments match up to what we expect
	char *message = "date NeLGetFileModificationDate [filename] - If an error occured, returns undefined.";
	
	//type_check
	type_check (arg_list[0], String, message);
	
	// get the node
	string sPath = arg_list[0]->to_string();

	// get vertices indices
	string result;
	HANDLE file = CreateFile (sPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		FILETIME lastWriteTime;
		if (GetFileTime(file, NULL, NULL, &lastWriteTime))
		{
			char number[512];
			sprintf (number, "%08x%08x", lastWriteTime.dwHighDateTime, lastWriteTime.dwLowDateTime);
			result = number;
		}
		CloseHandle (file);
	}
	
	if (result.empty())
		return &undefined;
	else
		return new String((char*)result.c_str());
}

// ***************************************************************************

Value* set_file_modification_date_cf (Value** arg_list, int count)
{
	// **** retrieve args.
	// Make sure we have the correct number of arguments (3)
	check_arg_count(NeLSetFileModificationDate , 2, count);
	
	// Check to see if the arguments match up to what we expect
	char *message = "bool NeLSetFileModificationDate [filename] [date] - If an error occured, returns false.";
	
	//type_check
	type_check (arg_list[0], String, message);
	type_check (arg_list[1], String, message);
	
	// get the node
	string sPath = arg_list[0]->to_string();
	string sDate = arg_list[1]->to_string();

	// get vertices indices
	string result;
	HANDLE file = CreateFile (sPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		FILETIME lastWriteTime;
		if (sscanf (sDate.c_str(), "%08x%08x", &lastWriteTime.dwHighDateTime, &lastWriteTime.dwLowDateTime) == 2)
		{
			if (SetFileTime(file, NULL, NULL, &lastWriteTime))
			{
				CloseHandle (file);
				return &true_value;
			}
		}
		CloseHandle (file);
	}
	
	return &false_value;
}

class CSuicideMsgBoxDisplayer : public CMsgBoxDisplayer
{
public:
	CSuicideMsgBoxDisplayer (const char *displayerName = "") : CMsgBoxDisplayer(displayerName) { }

protected:
	/// Put the string into the file.
    virtual void doDisplay( const CLog::TDisplayInfo& args, const char *message )
	{
		nelExportTerminateProcess();
	}
};

Value* force_quit_on_msg_displayer_cf(Value** arg_list, int count)
{
	nlwarning("Enable force quit on NeL report msg displayer");
	// disable the Windows popup telling that the application aborted and disable the dr watson report.
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	putenv("NEL_IGNORE_ASSERT=1");
	theCNelExport._ErrorInDialog = false;
	theCNelExport._TerminateOnFileOpenIssues = true;
	if (NLMISC::DefaultMsgBoxDisplayer || INelContext::getInstance().getDefaultMsgBoxDisplayer())
	{
		if (!NLMISC::DefaultMsgBoxDisplayer)
			NLMISC::DefaultMsgBoxDisplayer = INelContext::getInstance().getDefaultMsgBoxDisplayer();
		nldebug("Disable NeL report msg displayer");
		INelContext::getInstance().getAssertLog()->removeDisplayer(NLMISC::DefaultMsgBoxDisplayer);
		INelContext::getInstance().getErrorLog()->removeDisplayer(NLMISC::DefaultMsgBoxDisplayer);
		// TODO: Delete original NLMISC::DefaultMsgBoxDisplayer?
	}
	NLMISC::DefaultMsgBoxDisplayer = new CSuicideMsgBoxDisplayer("FORCEQUIT_MDB");
	INelContext::getInstance().setDefaultMsgBoxDisplayer(NLMISC::DefaultMsgBoxDisplayer);
	INelContext::getInstance().getAssertLog()->addDisplayer(NLMISC::DefaultMsgBoxDisplayer);
	INelContext::getInstance().getErrorLog()->addDisplayer(NLMISC::DefaultMsgBoxDisplayer);
	return &true_value;
}

Value* force_quit_right_now_cf(Value** arg_list, int count)
{
	// because quitMAX can fail
	nlwarning("Force quit");
	nelExportTerminateProcess();
	return &true_value;
}

/*===========================================================================*\
 |	MAXScript Plugin Initialization
\*===========================================================================*/

__declspec( dllexport ) void
LibInit() { 
}

