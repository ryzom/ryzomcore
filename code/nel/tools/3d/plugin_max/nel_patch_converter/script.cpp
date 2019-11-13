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

#define EXPORT_GET_ALLOCATOR

#pragma conform(forScope, push)
#pragma conform(forScope, off)

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <string>

#ifdef _STLPORT_VERSION
namespace std
{
	float fabsf(float f);
	double fabsl(double f);
}
#endif

#include <assert.h>

#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#	include <maxscript/foundation/3dmath.h>
#	include <maxscript/foundation/numbers.h>
#	include <maxscript/maxwrapper/maxclasses.h>
#	include <maxscript/foundation/streams.h>
#	include <maxscript/foundation/mxstime.h>
#	include <maxscript/maxwrapper/mxsobjects.h>
#	include <maxscript/compiler/parser.h>
#	include <maxscript/macros/define_instantiation_functions.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#	include <MaxScrpt/3dmath.h>
//	Various MAX and MXS includes
#	include <MaxScrpt/Numbers.h>
#	include <MaxScrpt/MAXclses.h>
#	include <MaxScrpt/Streams.h>
#	include <MaxScrpt/MSTime.h>
#	include <MaxScrpt/MAXObj.h>
#	include <MaxScrpt/Parser.h>
//	define the new primitives using macros from SDK
#	include <MaxScrpt/definsfn.h>
#endif

#include <modstack.h>
#include <decomp.h>

#include <max.h>
#include <stdmat.h>


#undef _CRT_SECURE_NO_DEPRECATE

#pragma conform(forScope, pop)

#undef min
#undef max

#include "PO2RPO.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"

#include "../nel_patch_lib/rpo.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"

#include "nel/3d/zone.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/3d/tile_bank.h"


using namespace NLMISC;
using namespace NL3D;
using namespace std;


/*===========================================================================*\
 |	Define our new functions
 |	These will turn on texture map display at all levels for a given object
\*===========================================================================*/

def_visible_primitive( set_steps,				"SetRykolPatchSteps");
def_visible_primitive( set_tile_steps,			"SetRykolTileSteps");
def_visible_primitive( get_tile_count,			"GetRykolTileCount");
def_visible_primitive( get_patch_count,			"GetRykolPatchCount");
def_visible_primitive( get_sel_edge,			"GetRykolSelEdges");
def_visible_primitive( get_edge_vert1,			"GetRykolEdgesVert1");
def_visible_primitive( get_edge_vert2,			"GetRykolEdgesVert2");
def_visible_primitive( get_edge_vect1,			"GetRykolEdgesVect1");
def_visible_primitive( get_edge_vect2,			"GetRykolEdgesVect2");
def_visible_primitive( get_vertex_pos,			"GetRykolVertexPos");
def_visible_primitive( set_vertex_pos,			"SetRykolVertexPos");
def_visible_primitive( get_vector_pos,			"GetRykolVectorPos");
def_visible_primitive( set_vector_pos,			"SetRykolVectorPos");
def_visible_primitive( set_vector_count,		"GetRykolVectorCount");
def_visible_primitive( set_vertex_count,		"GetRykolVertexCount");
def_visible_primitive( set_interior_mode,		"SetRykolInteriorMode");
def_visible_primitive( set_compute_interior,	"RykolComputeInterior");
def_visible_primitive( set_tile_mode,			"SetRykolTileMode");
def_visible_primitive( get_selected_vertex,		"GetRykolSelVertex");
def_visible_primitive( get_selected_patch,		"GetRykolSelPatch");
def_visible_primitive( get_selected_tile,		"GetRykolSelTile");
def_visible_primitive( get_patch_vertex,		"NeLGetPatchVertex");
def_visible_primitive( attach,					"NeLAttachPatchMesh");
def_visible_primitive( weld,					"NeLWeldPatchMesh");


def_visible_primitive( get_tile_tile_number,	"NelGetTileTileNumber");
def_visible_primitive( get_tile_noise_number,	"NelGetTileNoiseNumber");
def_visible_primitive( set_tile_noise_number,	"NelSetTileNoiseNumber");

def_visible_primitive( load_bank,				"NelLoadBank");
def_visible_primitive( get_tile_set,			"NelGetTileSet");

def_visible_primitive( set_tile_bank,			"NelSetTileBank");

def_visible_primitive( export_zone,				"ExportRykolZone");
def_visible_primitive( import_zone,				"NeLImportZone");

/* allows access to auto/manual interior edges
create a method to interface compute interior edge function
give access to tiledmode/patchmode (on/off)
use getselectedvertex
use getselectedpatch
use getselectedtile */

/*def_visible_primitive( set_interior_mode,		"SetRykolInteriorMode");
def_visible_primitive( set_vertex_count,		"GetRykolVertexCount");*/

/*===========================================================================*\
 |	Implimentations of our new function calls
 |	notice the appended '_cf' to show its the function implimentation
\*===========================================================================*/

/*def_visible_primitive( get_sel_edge,			"GetRykolSelEdges");
def_visible_primitive( get_edge_vertex,			"GetRykolEdgesVertex");
def_visible_primitive( get_vertex_pos,			"GetRykolVertexPos");
def_visible_primitive( set_vertex_pos,			"SetRykolVertexPos");
def_visible_primitive( get_vector_pos,			"GetRykolVectorPos");
def_visible_primitive( set_vector_pos,			"SetRykolVectorPos");*/

void errorMessage (const MCHAR *msg, const TCHAR *title, Interface& it, bool dialog)
{
	// Text or dialog ?
	if (dialog)
	{
		// Dialog message
		MessageBox (it.GetMAXHWnd(), msg, title, MB_OK|MB_ICONEXCLAMATION);
	}
	else
	{
		// Text message
		mprintf(msg);
		mprintf(_M("\n"));
	}
}

Value*
export_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_zone, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("ExportRykolZone [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Boolean *ret=&false_value;

	// Is the flag dont export set ?
	if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, 0))
		return ret;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nZone=arg_list[2]->to_int ();

			// Create a zone
			CZone zone;
			CZoneSymmetrisation zoneSymmetry;
			if (tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, nZone, 160, 1, false))
			{
				// Export path 
				const std::string sPath = MCharStrToUtf8(arg_list[1]->to_string());

				COFile file;
				if (file.open (sPath))
				{
					zone.serial (file);
					ret=&true_value;
				}
			}
		}
	}

	return ret;
}

Value* import_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count (export_zone, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	MCHAR *help = _M("NeLImportZone filename dialogError");
	type_check (arg_list[0], String, help);
	type_check (arg_list[1], Boolean, help);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get the filename
	string filename = MCharStrToUtf8(arg_list[0]->to_string());

	// Get the flip
	bool dialog = arg_list[1]->to_bool ()!=FALSE;

	// Return value
	Value *ret = &undefined;
		
	// Load the zone
	CIFile input;
	if (input.open (filename))
	{
		try
		{
			// The zone
			CZone zone;
			input.serial (zone);

			// Create an instance of NeL path mesh
			RPO* rpo = (RPO*) CreateInstance (GEOMOBJECT_CLASS_ID, RYKOLPATCHOBJ_CLASS_ID);
			rpo->rpatch = new RPatchMesh ();
			rpo->rpatch->rTess.TileTesselLevel = -5;

			// Convert the zone
			int zoneId;
			rpo->rpatch->importZone (&rpo->patch, zone, zoneId);

			// Create a derived object that references the rpo
			IDerivedObject *dobj = CreateDerivedObject(rpo);

			// Create a node in the scene that references the derived object
			INode *node = ip->CreateObjectNode (dobj);

    		// Return the result
			ret = new MAXNode (node);

			// Redraw the viewports
			ip->RedrawViews(ip->GetTime());
       	}
		catch (const Exception& e)
		{
			// Error message
			std::string msg = toString("Error when loading file %s: %s", filename.c_str(), e.what());
			errorMessage (MaxTStrFromUtf8(msg), _M("NeL import zone"), *ip, dialog);
		}
	}
	else
	{
		// Error message
		std::string msg = toString("Can't open the file %s for reading.", filename.c_str());
		errorMessage (MaxTStrFromUtf8(msg), _M("NeL import zone"), *ip, dialog);
	}

	return ret;
}

Value*
get_selected_tile_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_tile, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolSeltile [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->rpatch->tileSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->rpatch->tileSel.GetSize(); i++)
			{
				if (tri->rpatch->tileSel[i])
					array->append(Integer::intern(i+1));
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return array;
}

Value*
get_selected_patch_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_patch, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolSelPatch [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.patchSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.patchSel.GetSize(); i++)
			{
				if (tri->patch.patchSel[i])
					array->append(Integer::intern(i+1));
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return array;
}

Value*
get_selected_vertex_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_vertex, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolSelVertex [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.vertSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.vertSel.GetSize(); i++)
			{
				if (tri->patch.vertSel[i])
					array->append(Integer::intern(i+1));
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return array;
}

Value*
set_tile_mode_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_interior_mode, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolTileMode [Object]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				tri->rpatch->rTess.ModeTile=(arg_list[1]->to_int()!=0);
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;

			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_compute_interior_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_compute_interior, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("RykolComputeInterior [Object]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
				tri->patch.computeInteriors();
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_interior_mode_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_interior_mode, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolInteriorMode [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				if (arg_list[1]->to_int()<=tri->patch.numPatches)
					tri->patch.patches[arg_list[1]->to_int()-1].SetAuto (arg_list[2]->to_int());
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_vertex_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vertex_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolVertexCount [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->patch.numVerts;
		}
		if (os.obj != tri)
			delete tri;
	}

	return Integer::intern(nRet);
}

Value*
set_vector_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vector_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolVectorCount [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->patch.numVecs;
		}
		if (os.obj != tri)
			delete tri;
	}

	return Integer::intern(nRet);
}

Value*
set_vertex_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vertex_pos, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolVertexPos [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			Point3 point=arg_list[2]->to_point3();
			if ((nVertex>=0)&&(nVertex<tri->patch.numVerts))
			{
				bRet=true;
				tri->patch.verts[nVertex].p=point;
				tri->patch.InvalidateGeomCache();
				tri->rpatch->InvalidateChannels(PART_ALL);
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_vector_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vector_pos, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolVectorPos [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			Point3 point=arg_list[2]->to_point3();
			if ((nVertex>=0)&&(nVertex<tri->patch.numVecs))
			{
				bRet=true;
				tri->patch.vecs[nVertex].p=point;
				tri->patch.InvalidateGeomCache();
				tri->rpatch->InvalidateChannels(PART_ALL);
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
get_vertex_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_vertex_pos, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolVertexPos [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Value *vRet=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			if ((nVertex>=0)&&(nVertex<tri->patch.numVerts))
			{
				vRet=new Point3Value (tri->patch.verts[nVertex].p);
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return vRet;
}

Value*
get_vector_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_vector_pos, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolVectorPos [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Value *vRet=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			if ((nVertex>=0)&&(nVertex<tri->patch.numVecs))
			{
				vRet=new Point3Value (tri->patch.vecs[nVertex].p);
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return vRet;
}


Value*
get_edge_vect1_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vect1, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolEdgesVect1 [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].vec12;
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nVert);
}

Value*
get_edge_vect2_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vect2, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolEdgesVect2 [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].vec21;
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nVert);
}

Value*
get_edge_vert1_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vert1, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolEdgesVert1 [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].v1;
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nVert);
}


Value*
get_edge_vert2_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vert2, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolEdgesVert2 [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].v2;
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nVert);
}

Value*
get_sel_edge_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_tile_steps, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolSelEdges [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.edgeSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.edgeSel.GetSize(); i++)
			{
				if (tri->patch.edgeSel[i])
					array->append(Integer::intern(i+1));
					//array->data[j++]=;
			}
			if (os.obj != tri)
				delete tri;
		}
	}

	return array;
}

Value*
set_steps_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_steps, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykolPatchSteps [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		if (classId==RYKOLPATCHOBJ_CLASS_ID)
		{
			bRet=true;
			static_cast<RPO*>(os.obj)->SetMeshSteps(arg_list[1]->to_int());
		}
		if (os.obj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
		{
			bRet=true;
			PatchObject *tri = (PatchObject *) os.obj->ConvertToType(ip->GetTime(), 
				Class_ID(PATCHOBJ_CLASS_ID, 0));
			if (tri)
				tri->SetMeshSteps(arg_list[1]->to_int());
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}
		if (classId==Class_ID(PATCHOBJ_CLASS_ID,0))
		{
			bRet=true;
			static_cast<PatchObject*>(os.obj)->SetMeshSteps(arg_list[1]->to_int());
		}
		if (bRet)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}


Value*
set_tile_steps_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_tile_steps, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("SetRykoltileSteps [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			bRet=true;
			int nTess=arg_list[1]->to_int();
			if (nTess<-5)
				nTess=-5;
			if (nTess>5)
				nTess=5;
			tri->rpatch->rTess.TileTesselLevel=nTess;
			tri->rpatch->InvalidateChannels (PART_ALL);
			if (os.obj != tri)
				delete tri;
		}	
		if (bRet)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
get_tile_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_tile_count, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolTileCount [Zone] [PatchNumber]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nPatch=arg_list[1]->to_int();
			if ((nPatch<(int)tri->rpatch->getUIPatchSize())&&(nPatch>=0))
			{
				nRet=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
				(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));
			}

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}


		if (nRet!=-1)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return Integer::intern(nRet);
}

Value* get_patch_vertex_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(get_tile_count, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	const MCHAR *message= _M("NeLGetPatchVertex [NeLPatchMesh] [PatchId] [VertexId]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], Integer, message);
	type_check(arg_list[2], Integer, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// ok ?
	int nRet=-1;

	// Get the patch id
	int patchId = arg_list[1]->to_int() - 1;

	// Get the vertex id
	int vertId = arg_list[2]->to_int() - 1;

	// Valid vertex id ?
	if ((vertId>=0) && (vertId<4))
	{
		// Get a Object pointer
		ObjectState os=node->EvalWorldState(ip->GetTime()); 

		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				if ((patchId<(int)tri->patch.numPatches)&&(patchId>=0))
				{
					// Get the vertex id
					nRet = tri->patch.patches[patchId].v[vertId] + 1;
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}


			if (nRet!=-1)
			{
				// redraw and update
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
		}
	}

	return Integer::intern(nRet);
}



Value*
get_patch_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_patch_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("GetRykolPatchCount [Object]"));
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->rpatch->getUIPatchSize();

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}


		if (nRet!=-1)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime());
		}
	}

	return Integer::intern(nRet);
}

Value*
get_tile_tile_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 4, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]"));
	type_check(arg_list[1], Integer, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]"));
	type_check(arg_list[2], Integer, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]"));
	type_check(arg_list[3], Integer, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	// Get the layer
	uint layer=arg_list[3]->to_int()-1;
	if (layer>=3)
	{
		mprintf (_M("Error: layer must be 1, 2, or 3\n"));
	}
	else
	{
		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				// Get the patch number
				uint nPatch=arg_list[1]->to_int()-1;
				if (nPatch>=tri->rpatch->getUIPatchSize())
				{
					mprintf (_M("Error: patch index is invalid.\n"));
				}
				else
				{
					// Number of tile in this patch
					uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
						(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

					// Get the tile number
					uint tile=arg_list[2]->to_int()-1;
					if (tile>=nPatchCount)
					{
						mprintf (_M("Error: tile index is invalid.\n"));
					}
					else
					{
						// Tile index
						if (layer<(uint)tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getNumLayer())
						{
							nRet=tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getLayer (layer).Tile+1;
						}
						else
						{
							nRet=0;
						}
					}
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}
		}
	}

	return Integer::intern(nRet);
}

Value*
get_tile_noise_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]"));
	type_check(arg_list[1], Integer, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]"));
	type_check(arg_list[2], Integer, _M("NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Get the patch number
			uint nPatch=arg_list[1]->to_int()-1;
			if (nPatch>=tri->rpatch->getUIPatchSize())
			{
				mprintf (_M("Error: patch index is invalid.\n"));
			}
			else
			{
				// Number of tile in this patch
				uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
					(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

				// Get the tile number
				uint tile=arg_list[2]->to_int()-1;
				if (tile>=nPatchCount)
				{
					mprintf (_M("Error: patch index is invalid.\n"));
				}
				else
				{
					// Tile index
					nRet=tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getDisplace ()+1;
				}
			}

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nRet);
}

Value*
set_tile_noise_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 4, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]"));
	type_check(arg_list[1], Integer, _M("NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]"));
	type_check(arg_list[2], Integer, _M("NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]"));
	type_check(arg_list[3], Integer, _M("NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	// Get noise number
	uint noise=arg_list[3]->to_int()-1;
	if (noise>=16)
	{
		mprintf (_M("Error: noise value must be 1~16\n"));
	}
	else
	{
		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				// Get the patch number
				uint nPatch=arg_list[1]->to_int()-1;
				if (nPatch>=tri->rpatch->getUIPatchSize())
				{
					mprintf (_M("Error: patch index is invalid.\n"));
				}
				else
				{
					// Number of tile in this patch
					uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
						(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

					// Get the tile number
					uint tile=arg_list[2]->to_int()-1;
					if (tile>=nPatchCount)
					{
						mprintf (_M("Error: patch index is invalid.\n"));
					}
					else
					{
						// Tile index
						tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).setDisplace (noise);
						bRet=true;
					}
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}
		}
	}

	return bRet?&true_value:&false_value;
}

CTileBank scriptedBank;

Value*
load_bank_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 0, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	std::string bankName = GetBankPathName ();
	if (!bankName.empty())
	{
		try
		{
			// Open a file
			CIFile file;
			if (file.open (bankName))
			{
				// Read the bank
				file.serial (scriptedBank);

				// Build xref
				scriptedBank.computeXRef ();

				// Ok
				return &true_value;
			}
			else
			{
				mprintf (_M("Error: can't open bank file %s\n"), bankName.c_str());
			}
		}
		catch (const Exception& e)
		{
			// Error message
			mprintf (_M("Error: %s\n"), e.what());
		}
	}

	// Error
	return &false_value;
}

Value*
get_tile_set_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], Integer, _M("NelGetTileSet [tileId]"));

	// ok ?
	int nRet=-1;

	// Get tile number
	uint tile=arg_list[0]->to_int()-1;
	if (tile>=(uint)scriptedBank.getTileCount())
	{
		mprintf (_M("Error: tile number is wrong. (1 ~ %d)\n"), scriptedBank.getTileCount());
	}
	else
	{
		// Get the XRef for this bank
		int tileSet;
		int number;
		CTileBank::TTileType type;
		scriptedBank.getTileXRef (tile, tileSet, number, type);

		// Return value
		nRet=tileSet+1;
	}

	// Error
	return Integer::intern(nRet);
}

Value* set_tile_bank_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], String, _M("NelSetTileBank [tile bank pathname]"));

	// ok ?
	const std::string pathname = MCharStrToUtf8(arg_list[0]->to_string());

	// Get tile number
	SetBankPathName (pathname);

	// Error
	return &true_value;
}

//int attachReorient = 0;

void DoAttach(INode *node, PatchMesh *patch, RPatchMesh *rpatch, INode *attNode, PatchMesh *attPatch, RPatchMesh *rattPatch, Interface *ip)
{
	// Transform the shape for attachment:
	// If reorienting, just translate to align pivots
	// Otherwise, transform to match our transform
	Matrix3 attMat(1);
	if (0)
	{
		Matrix3 thisTM = node->GetNodeTM(ip->GetTime());
		Matrix3 thisOTMBWSM = node->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 thisPivTM = thisTM * Inverse(thisOTMBWSM);
		Matrix3 otherTM = attNode->GetNodeTM(ip->GetTime());
		Matrix3 otherOTMBWSM = attNode->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 otherPivTM = otherTM * Inverse(otherOTMBWSM);
		Point3 otherObjOffset = attNode->GetObjOffsetPos();
		attMat = Inverse(otherPivTM) * thisPivTM;
	}
	else 
	{
		attMat = attNode->GetObjectTM(ip->GetTime()) *
			Inverse(node->GetObjectTM(ip->GetTime()));
	}

	// RB 3-17-96 : Check for mirroring
	AffineParts parts;
	decomp_affine(attMat, &parts);
	if (parts.f < 0.0f)
	{
		int v[8], ct, ct2, j;
		Point3 p[9];
		
		for (int i = 0; i < attPatch->numPatches; i++)
		{

			// Re-order rpatch
			if (attPatch->patches[i].type == PATCH_QUAD)
			{
				UI_PATCH rpatch=rattPatch->getUIPatch (i);
				int ctU=rpatch.NbTilesU<<1;
				int ctV=rpatch.NbTilesV<<1;
				int nU;
				for (nU=0; nU<ctU; nU++)
				{
					for (int nV=0; nV<ctV; nV++)
					{
						rattPatch->getUIPatch (i).getTileDesc (nU+nV*ctU)=rpatch.getTileDesc (ctU-1-nU+(ctV-1-nV)*ctU);
					}
				}
				for (nU=0; nU<ctU+1; nU++)
				{
					for (int nV=0; nV<ctV+1; nV++)
					{
						rattPatch->getUIPatch (i).setColor (nU+nV*(ctU+1), rpatch.getColor (ctU-nU+(ctV-nV)*ctU));
					}
				}
			}

			// Re-order vertices
			ct = attPatch->patches[i].type == PATCH_QUAD ? 4 : 3;
			for (j = 0; j < ct; j++)
			{
				v[j] = attPatch->patches[i].v[j];
			}
			for (j = 0; j < ct; j++)
			{
				attPatch->patches[i].v[j] = v[ct - j - 1];
			}
			
			// Re-order vecs
			ct  = attPatch->patches[i].type == PATCH_QUAD ? 8 : 6;
			ct2 = attPatch->patches[i].type == PATCH_QUAD ? 5 : 3;
			for (j = 0; j < ct; j++)
			{
				v[j] = attPatch->patches[i].vec[j];
			}
			for (j = 0; j < ct; j++, ct2--)
			{
				if (ct2 < 0)
					ct2 = ct - 1;
				attPatch->patches[i].vec[j] = v[ct2];
			}
			
			// Re-order enteriors
			if (attPatch->patches[i].type == PATCH_QUAD)
			{
				ct = 4;
				for (j = 0; j < ct; j++)
				{
					v[j] = attPatch->patches[i].interior[j];
				}
				for (j = 0; j < ct; j++)
				{
					attPatch->patches[i].interior[j] = v[ct - j - 1];
				}
			}
			
			// Re-order aux
			if (attPatch->patches[i].type == PATCH_TRI)
			{
				ct = 9;
				for (j = 0; j < ct; j++)
				{
					p[j] = attPatch->patches[i].aux[j];
				}
				for (j = 0; j < ct; j++)
				{
					attPatch->patches[i].aux[j] = p[ct - j - 1];
				}
			}
			
			// Re-order TV faces if present
			for (int chan = 0; chan < patch->getNumMaps(); ++chan)
			{
				if (attPatch->tvPatches[chan])
				{
					ct = 4;
					for (j = 0; j < ct; j++)
					{
						v[j] = attPatch->tvPatches[chan][i].tv[j];
					}
					for (j = 0; j < ct; j++)
					{
						attPatch->tvPatches[chan][i].tv[j] = v[ct - j - 1];
					}
				}
			}
		}
	}

	int i;
	for (i = 0; i < attPatch->numVerts; ++i)
		attPatch->verts[i].p = attPatch->verts[i].p * attMat;
	for (i = 0; i < attPatch->numVecs; ++i)
		attPatch->vecs[i].p = attPatch->vecs[i].p * attMat;
	attPatch->computeInteriors();

	// theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset = 0;
	Mtl *m1 = node->GetMtl();
	Mtl *m2 = attNode->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 &&(m1 != m2))
	{
		//if (attachMat == ATTACHMAT_IDTOMAT)
		{
			int ct = 1;
			if (m1->IsMultiMtl())
				ct = m1->NumSubMtls();
			for (int i = 0; i < patch->numPatches; ++i)
			{
				int mtid = patch->getPatchMtlIndex(i);
				if (mtid >= ct)
					patch->setPatchMtlIndex(i, mtid % ct);
			}
			FitPatchIDsToMaterial(*attPatch, m2);
			/*if (condenseMat)
				condenseMe = TRUE;*/
		}
		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		// theHold.Suspend();
		/*if (attachMat == ATTACHMAT_MATTOID)
		{
			m1 = FitMaterialToPatchIDs(*patch, m1);
			m2 = FitMaterialToPatchIDs(*attPatch, m2);
		}*/
		
		Mtl *multi = CombineMaterials(m1, m2, mat2Offset);
		/*if (attachMat == ATTACHMAT_NEITHER)
			mat2Offset = 0;*/
		// theHold.Resume();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		DWORD roldSL = patch->selLevel;
		patch->selLevel = PATCH_OBJECT;
		rpatch->SetSelLevel (EP_OBJECT);
		node->SetMtl(multi);
		patch->selLevel = oldSL;
		rpatch->SetSelLevel (roldSL);
		m1 = multi;
		// canUndo = FALSE;	// Absolutely cannot undo material combinations.
	}
	if (!m1 && m2)
	{
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		DWORD roldSL = rpatch->GetSelLevel();
		patch->selLevel = PATCH_OBJECT;
		rpatch->SetSelLevel (EP_OBJECT);
		node->SetMtl(m2);
		patch->selLevel = oldSL;
		rpatch->SetSelLevel (roldSL);
		m1 = m2;
	}

	// Start a restore object...
	// if (theHold.Holding())
		// theHold.Put(new PatchRestore(patchData, this, patch, rpatch, "DoAttach"));

	// Do the attach
	patch->Attach(attPatch, mat2Offset);
	rpatch->Attach(rattPatch, *patch);
	// patchData->UpdateChanges(patch, rpatch);
	// patchData->TempData(this)->Invalidate(PART_TOPO | PART_GEOM);

	// Get rid of the original node
	ip->DeleteNode(attNode);

	// ResolveTopoChanges();
	// theHold.Accept(GetString(IDS_TH_ATTACH));

	if (m1 && condenseMe)
	{
		// Following clears undo stack.
		// patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		m1 = CondenseMatAssignments(*patch, m1);
	}
}



Value*
attach_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(attach, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("NeLAttachPatchMesh [RykolPatchMeshSrc] [RykolPatchMeshDest]"));
	type_check(arg_list[1], MAXNode, _M("NeLAttachPatchMesh [RykolPatchMeshSrc] [RykolPatchMeshDest]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *nodeSrc = arg_list[0]->to_node();
	INode *nodeDest = arg_list[1]->to_node();
	nlassert (nodeSrc);
	nlassert (nodeDest);

	// Get a Object pointer
	ObjectState osSrc = nodeSrc->EvalWorldState (ip->GetTime()); 
	ObjectState osDest = nodeDest->EvalWorldState (ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (osSrc.obj && osDest.obj)
	{
		// Get class id
		if ( (osSrc.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) && (osDest.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)))
		{
			RPO *triSrc = (RPO *) osSrc.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			RPO *triDest = (RPO *) osDest.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (triSrc && triDest)
			{
				DoAttach (nodeDest, &triDest->patch, triDest->rpatch, nodeSrc, &triSrc->patch, triSrc->rpatch, ip);
				bRet=true;
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (osSrc.obj != triSrc)
				delete triSrc;

			// redraw and update
			triDest->patch.InvalidateGeomCache();
			triDest->rpatch->InvalidateChannels(PART_ALL);
			nodeDest->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
weld_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(attach, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, _M("NeLWeldPatchMesh [RykolPatchMeshSrc] [threshold]"));
	type_check(arg_list[1], Float, _M("NeLWeldPatchMesh [RykolPatchMeshSrc] [threshold]"));

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *nodeSrc = arg_list[0]->to_node();
	float threshold = arg_list[1]->to_float();
	nlassert (nodeSrc);

	// Get a Object pointer
	ObjectState osSrc = nodeSrc->EvalWorldState (ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (osSrc.obj)
	{
		// Get class id
		if (osSrc.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID))
		{
			RPO *triSrc = (RPO *) osSrc.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (triSrc)
			{
				triSrc->patch.vertSel.SetAll ();
				triSrc->patch.Weld (threshold, TRUE);
				triSrc->rpatch->Weld (&triSrc->patch);
				bRet=true;
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (osSrc.obj != triSrc)
				delete triSrc;

			// redraw and update
			triSrc->patch.InvalidateGeomCache();
			triSrc->rpatch->InvalidateChannels(PART_ALL);
			nodeSrc->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

/*===========================================================================*\
 |	MAXScript Plugin Initialization
\*===========================================================================*/

__declspec( dllexport ) void
LibInit() { 
}


