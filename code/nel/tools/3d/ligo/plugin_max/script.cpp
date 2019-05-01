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

#include <assert.h>

#ifdef _STLPORT_VERSION
namespace std
{
	float fabsf(float f);
	double fabsl(double f);
}
#endif

// Various MAX and MXS includes
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
#	include <MaxScrpt/MAXScrpt.h>
#	include <MaxScrpt/3dmath.h>
#	include <MaxScrpt/Numbers.h>
#	include <MaxScrpt/MAXclses.h>
#	include <MaxScrpt/Streams.h>
#	include <MaxScrpt/MSTime.h>
#	include <MaxScrpt/MAXObj.h>
#	include <MaxScrpt/Parser.h>
#	include <MaxScrpt/definsfn.h>
#endif
#include <max.h>
#include <stdmat.h>

// Visual
#include <direct.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// From nel patch lib
#include "../../plugin_max/nel_patch_lib/rpo.h"
#include "../../plugin_max/nel_mesh_lib/export_nel.h"


// From nel 3d
#include "nel/3d/zone.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/3d/nelu.h"
#include "nel/3d/landscape_model.h"

// From nel misc
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/config_file.h"

// From ligo library
#include "nel/ligo/zone_template.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/ligo_error.h"
#include "nel/ligo/ligo_material.h"
#include "nel/ligo/transition.h"
#include "nel/ligo/zone_bank.h"

#include "max_to_ligo.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

// APP DATA
#define NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX	((uint32)1342141818)

// ***************************************************************************

/// Definition of new MAXScript functions
def_visible_primitive( export_material,						"NeLLigoExportMaterial");
def_visible_primitive( export_transition,					"NeLLigoExportTransition");
def_visible_primitive( export_zone,							"NeLLigoExportZone");
//def_visible_primitive( export_transition_zone,				"NeLLigoExportTransitionZone");
def_visible_primitive( get_error_zone_template,				"NeLLigoGetErrorZoneTemplate");
def_visible_primitive( get_snap ,							"NeLLigoGetSnap");
def_visible_primitive( get_cell_size ,						"NeLLigoGetCellSize");
def_visible_primitive( check_zone_with_material,			"NeLLigoCheckZoneWithMaterial");
def_visible_primitive( check_zone_with_transition,			"NeLLigoCheckZoneWithTransition");
def_visible_primitive( get_error_string,					"NeLLigoGetErrorString");
def_visible_primitive( set_directory,						"NeLLigoSetDirectory");
def_visible_primitive( get_zone_mask,						"NeLLigoGetZoneMask");
def_visible_primitive( make_snapshot,						"NeLLigoMakeSnapShot");
def_visible_primitive( get_zone_size,						"NeLLigoGetZoneSize");

// ***************************************************************************

/// Zone template
CLigoError ScriptErrors[10];

// ***************************************************************************

bool MakeSnapShot (NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, const NL3D::CTileFarBank &tileFarBank, 
				   sint xmin, sint xmax, sint ymin, sint ymax, const CLigoConfig &config, bool errorInDialog);

bool MakeSnapShot (NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, const NL3D::CTileFarBank &tileFarBank, 
				   sint xmax, sint ymax, const CLigoConfig &config, bool errorInDialog);

// ***************************************************************************

/// Export a material
Value* export_material_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(export_material, 4, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoExportMaterial [Object] [Filename] [CheckOnly] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);
	type_check(arg_list[3], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	const std::string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The third arg
	bool checkOnly = (arg_list[2]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export material", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// Build a zone
					NL3D::CZone zone;
					CZoneSymmetrisation zoneSymmetry;
					if (tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, 0, config.CellSize, config.Snap, false))
					{
						// Build a material
						NLLIGO::CMaterial material;

						// No error ?
						if (res = material.build (zoneTemplate, config, ScriptErrors[0]))
						{
							// Save it ?
							if (!checkOnly)
							{

								// Make a name for the zone
								std::string path = CFile::getPath(fileName) + CFile::getFilenameWithoutExtension(fileName) + ".zone";

								// Ok ?
								bool ok = true;

								// Catch exception
								try
								{
									// Open a stream file
									COFile outputfile;
									if (outputfile.open (fileName))
									{
										// Create an xml stream
										COXml outputXml;
										nlverify (outputXml.init (&outputfile));

										// Serial the class
										material.serial (outputXml);

										// Another the stream
										COFile outputfile2;

										// Open another file
										if (outputfile2.open (path))
										{
											// Serial the zone
											zone.serial (outputfile2);
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", path);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
										ok = false;
									}
								}
								catch (const Exception &e)
								{
									// Error message
									char tmp[512];
									smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
									CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
									ok = false;
								}

								// Remove the files
								if (!ok)
								{
									CFile::deleteFile(fileName);
									CFile::deleteFile(path);
								}
							}
						}
					}
					else
					{
						// Error, zone can't be exported
						CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
							*MAXScript_interface, errorInDialog);
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export material", *MAXScript_interface, errorInDialog);
		}
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

/// Export a transition
Value* export_transition_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (6)
	check_arg_count(export_transition, 6, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoExportTransition [Object array (count=9)] [Output filename] [First material filename] [Second material filename] [CheckOnly] [Error in dialog]");
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], String, message);
	type_check(arg_list[3], String, message);
	type_check(arg_list[4], Boolean, message);
	type_check(arg_list[5], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *nodes = (Array*)arg_list[0];
	nlassert (is_array(nodes));

	// The second arg
	std::string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The second arg
	string matFilename[2];
	matFilename[0] = MCharStrToUtf8(arg_list[2]->to_string());
	matFilename[1] = MCharStrToUtf8(arg_list[3]->to_string());

	// The third arg
	bool checkOnly = (arg_list[4]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[5]->to_bool() != FALSE);

	// Ok ?
	bool ok = true;

	// All zone are present ?
	bool allPresent = true;

	// Clear error message
	for (uint error=0; error<10; error++)
		ScriptErrors[error].clear ();

	// Load the ligofile
	CLigoConfig config;
	if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export transition", 
			*MAXScript_interface, errorInDialog);
	}
	else
	{
		// CTransition::TransitionZoneCount elements in the array ?
		if (nodes->size == CTransition::TransitionZoneCount)
		{
			// Build a node array
			std::vector<CZoneTemplate>			arrayTemplate (CTransition::TransitionZoneCount);
			std::vector<const CZoneTemplate*>	arrayTemplatePtr (CTransition::TransitionZoneCount, NULL);
			std::vector<RPO*>					arrayTri (CTransition::TransitionZoneCount, NULL);
			std::vector<INode*>					arrayNode (CTransition::TransitionZoneCount, NULL);

			for (uint i=0; i<(uint)nodes->size; i++)
			{
				// Get a node zone
				if (nodes->get (i+1) != &undefined)
				{
					arrayNode[i] = nodes->get (i+1)->to_node();
					// Node builded ?
					bool builded = false;
					
					// Get a Object pointer
					ObjectState os=arrayNode[i]->EvalWorldState(ip->GetTime()); 

					// Ok ?
					if (os.obj)
					{
						// Convert in 3ds NeL patch mesh
						arrayTri[i] = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
						if (arrayTri[i])
						{
							// Build a zone template
							CZoneTemplate zoneTemplate;

							// Build the zone template
							if (CMaxToLigo::buildZoneTemplate (arrayNode[i], arrayTri[i]->patch, arrayTemplate[i], config, ScriptErrors[i], ip->GetTime()))
							{
								// Success, put the pointer
								arrayTemplatePtr[i] = &arrayTemplate[i];
								builded = true;
							}
						}
					}

					// Ok ?
					if (!builded)
						ok = false;
				}
				else
					allPresent = false;
			}

			// Ok, continue
			if (ok)
			{
				// Load the materials
				NLLIGO::CMaterial materials[2];

				// For each material
				for (uint mat=0; mat<2; mat++)
				{
					// Inputfile 0
					CIFile input;
					if (input.open (matFilename[mat]))
					{
						// Catch some errors
						try
						{
							// XML stream
							CIXml inputXml;
							nlverify (inputXml.init (input));

							// Serial
							materials[mat].serial (inputXml);
						}
						catch (const Exception &e)
						{
							// Error message
							char tmp[2048];
							smprintf (tmp, 2048, "Error while loading material file %s : %s", matFilename[mat], e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
							ok = false;
						}
					}
					else
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Can't open the file %s for reading.", matFilename[mat]);
						CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
						ok = false;
					}
				}

				// Ok ?
				if (ok)
				{
					// Build the transition
					CTransition transition;
					if (ok = transition.build (materials[0], materials[1], arrayTemplatePtr, config, ScriptErrors, ScriptErrors[CTransition::TransitionZoneCount]))
					{
						// Export ?
						if (!checkOnly)
						{
							// All transitions are ok ?
							if (allPresent)
							{
								// Build the zones
								NL3D::CZone zones[CTransition::TransitionZoneCount];
								uint zone;
								for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
								{
									// Build the zone
									CZoneSymmetrisation zoneSymmetry;
									if (!arrayTri[zone]->rpatch->exportZone (arrayNode[zone], &arrayTri[zone]->patch, zones[zone], zoneSymmetry, 0, config.CellSize, config.Snap, false))
									{
										// Error, zone can't be exported
										CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
											*MAXScript_interface, errorInDialog);
										ok = false;
									}
								}

								// Catch exceptions
								if (ok)
								{
									// File names
									vector<string> createdfiles;
									try
									{
										// Open a stream file
										COFile outputfile;
										if (outputfile.open (fileName))
										{
											// Add the filename
											createdfiles.push_back (fileName);

											// Create an xml stream
											COXml outputXml;
											nlverify (outputXml.init (&outputfile));

											// Serial the class
											transition.serial (outputXml);

											// Make a name for the zone
											char drive[512];
											char dir[512];
											char name[512];

											// Export each zones
											for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
											{
												// Final path
												std::string path = CFile::getPath(fileName) + toString("%s-%u", CFile::getFilenameWithoutExtension(fileName).c_str(), zone) + ".zone";

												// Another the stream
												COFile outputfile2;

												// Open another file
												if (outputfile2.open (path))
												{
													// Add the filename
													createdfiles.push_back (path);

													// Serial the zone
													zones[zone].serial (outputfile2);
												}
												else
												{
													// Error message
													char tmp[512];
													smprintf (tmp, 512, "Can't open the file %s for writing.", path);
													CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
													ok = false;
												}
											}
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									catch (const Exception &e)
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
										ok = false;
									}

									// Not ok ?
									if (!ok)
									{
										// Erase the files
										for (uint file=0; file<createdfiles.size(); file++)
										{
											// Removing files
											CFile::deleteFile(createdfiles[file]);
										}
									}
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "All transition are not present. Can't export..");
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
								ok = false;
							}
						}
					}
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: must have 9 cell in the node array", "NeL Ligo export transition", 
				*MAXScript_interface, errorInDialog);
		}
	}

	// Ok ?
	return ok?&true_value:&false_value;
}

// ***************************************************************************
 
Value* get_error_zone_template_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_error_zone_template, 4, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoGetErrorZoneTemplate [Array error codes] [Array vertex id] [Array messages] [Error index]");
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Integer, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *errorCodes = (Array *)arg_list[0];
	nlassert (is_array(errorCodes));

	// The second arg
	Array *vertexId = (Array *)arg_list[1];
	nlassert (is_array(vertexId));

	// The third arg
	Array *messages = (Array *)arg_list[2];
	nlassert (is_array(messages));

	// The third arg
	int errorIndex = arg_list[3]->to_int() - 1;
	clamp (errorIndex, 0, 8);

	// Num error
	uint numError = ScriptErrors[errorIndex].numVertexError ();

	// For each error
	for (uint i=0; i<numError; i++)
	{
		// Id and edge
		uint id, edge;
		uint error = (uint) ScriptErrors[errorIndex].getVertexError (i, id, edge);

		// Append error code
		errorCodes->append (Integer::intern (error+1));

		// Append vertex id
		vertexId->append (Integer::intern (id+1));

		// Append messages
		messages->append (new String(_T("[LIGO DEBUG] Opened edge")));
	}

	// Return the main error message
	return Integer::intern ((int)ScriptErrors[errorIndex].MainError+1);
}

// ***************************************************************************
 
Value* get_snap_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_snap, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.Snap);
	}
	else
		return &undefined;
}

// ***************************************************************************
 
Value* get_cell_size_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_cell_size, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.CellSize);
	}
	else
		return &undefined;
}

// ***************************************************************************
 
/// Check a ligo zone with a ligo material
Value* check_zone_with_material_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(check_zone_with_template, 3, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoCheckZoneWithMaterial [Object] [Material filename] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The fourth arg
	bool errorInDialog = (arg_list[2]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo check zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// Material to check ?
					if (fileName != "")
					{
						// The material
						NLLIGO::CMaterial material;

						// Read the template
						CIFile inputfile;

						// Catch exception
						try
						{
							// Open the selected template file
							if (inputfile.open (fileName))
							{
								// Create an xml stream
								CIXml inputXml;
								inputXml.init (inputfile);

								// Serial the class
								material.serial (inputXml);

								// Get the zone edges
								const std::vector<CZoneEdge> &zoneEdges = zoneTemplate.getEdges ();

								// All edge ok
								res = true;

								// For each zone edge
								for (uint edge=0; edge<zoneEdges.size(); edge++)
								{
									// Check it
									if (!material.getEdge().isTheSame (zoneEdges[edge], config, ScriptErrors[0]))
										res = false;
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "Can't open the zone template file %s for reading.", fileName);
								CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
							}
						}
						catch (const Exception &e)
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
						}
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}
 

// ***************************************************************************

/// Check a ligo zone with a ligo material
Value* check_zone_with_transition_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(check_zone_with_template, 4, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoCheckZoneWithTransition [Object] [Transition filename] [Transition number: 0~8] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Integer, message);
	type_check(arg_list[3], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The second arg
	int transitionNumber = arg_list[2]->to_int();

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo check zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// The material
					NLLIGO::CTransition transition;

					// Read the template
					CIFile inputfile;

					// Catch exception
					try
					{
						// Open the selected template file
						if (inputfile.open (fileName))
						{
							// Create an xml stream
							CIXml inputXml;
							inputXml.init (inputfile);

							// Serial the class
							transition.serial (inputXml);

							// Check it
							res = transition.check (zoneTemplate, transitionNumber, config, ScriptErrors[0]);
						}
						else
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Can't open the transition template file %s for reading.", fileName);
							CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
						}
					}
					catch (const Exception &e)
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
						CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

// Get the zone mask
void getSquareMask (CZone &zone, std::vector<bool> &mask, uint &width, uint &height, float cellSize)
{
	CZoneInfo zoneInfo;
	zone.retrieve (zoneInfo);

	// Look for bounding box of the zone, min size 1x1
	sint maxX = 1;
	sint maxY = 1;

	// For each patches
	uint i;
	for (i=0; i<zoneInfo.Patchs.size (); i++)
	{
		CPatchInfo &patch = zoneInfo.Patchs[i];
		
		// For each vertex
		uint v;
		for (v=0; v<4; v++)
		{
			// Get the cel position
			sint positionX = (uint)((patch.Patch.Vertices[v].x + cellSize/2) / cellSize);
			sint positionY = (uint)((patch.Patch.Vertices[v].y + cellSize/2) / cellSize);

			// PosX
			if (positionX>maxX)
				maxX = positionX;

			// PosY
			if (positionY>maxY)
				maxY = positionY;
		}
	}

	// Set the size
	width = (uint)maxX;
	height = (uint)maxY;

	// Set the mask
	uint size = width*height;
	mask.resize (0);
	mask.resize (size, true);
}

// ***************************************************************************

/// Export a ligo zone
Value* export_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (5)
	check_arg_count(check_zone_with_template, 5, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoExportZone [Object] [Ligozone filename] [Category Array] [Error in dialog] [Snapshot]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Boolean, message);
	type_check(arg_list[4], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The thrid arg
	Array *array = (Array*)arg_list[2];

	// Build an array of category
	vector<pair<string, string> > categories;
	categories.resize (array->size);

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// The fifth arg
	bool weWantToMakeASnapshot = (arg_list[4]->to_bool() != FALSE);

	// Load the ligofile
	CLigoConfig config;
	if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo check zone", 
			*MAXScript_interface, errorInDialog);
	}
	else
	{
		// For each array elements
		uint i;
		for (i=0; i<(uint)array->size; i++)
		{
			// Check that we have an array
			type_check(array->get (i+1), Array, message);

			// Check we have 2 strings
			Array *cell = (Array*)array->get (i+1);
			if (cell->size != 2)
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: category arguments are not 2 strings", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
				return &false_value;
			}
			type_check (cell->get(1), String, message);
			type_check (cell->get(2), String, message);

			// Get the strings
			categories[i].first = MCharStrToUtf8(cell->get(1)->to_string());
			categories[i].second = MCharStrToUtf8(cell->get(2)->to_string());
		}

		// Get a Object pointer
		ObjectState os=node->EvalWorldState(ip->GetTime()); 

		// Ok ?
		if (os.obj)
		{
			// Convert in 3ds NeL patch mesh
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				// Load the ligofile
				CLigoConfig config;
				if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
				{
					// Output an error
					CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{
					// Build a filename
					std::string path = NLMISC::CFile::getPath(fileName);
					std::string name = NLMISC::CFile::getFilenameWithoutExtension(fileName);

					// Build the zone filename
					std::string outputFilenameZone = path + "zones/" + name + ".zone";

					// Build the snap shot filename
					std::string outputFilenameSnapShot = path + "zonesBitmaps/" + name + ".tga";

					// Build the ligozone filename
					std::string outputFilenameLigozone = path + "zoneLigos/" + name + ".ligozone";

					// Build the zone
					CZone zone;
					CZoneSymmetrisation zoneSymmetry;
					if (!tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, 0, config.CellSize, config.Snap, false))
					{
						// Error, zone can't be exported
						CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
							*MAXScript_interface, errorInDialog);
					}
					else
					{
						// The zone mask
						std::vector<bool> mask;
						uint width;
						uint height;

						// Calc zone mask or force bounding mask ?
						bool useBoundingBox = CExportNel::getScriptAppData (node, NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX, 0) != 0;
						bool maskOk = false;
						
						if (useBoundingBox)
						{
							// Get the square zone
							getSquareMask (zone, mask, width, height, config.CellSize);
							maskOk = true;
						}
						else
						{
							// The zone template
							CZoneTemplate zoneTemplate;

							// Build the zone template
							bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

							// Success ?
							if (res)
							{
								// Build the zone mask
								zoneTemplate.getMask (mask, width, height);
								maskOk = true;
							}
						}

						// Continue ?
						if (maskOk)
						{
							// The bank
							static CTileBank *tileBank=NULL;
							static CTileFarBank *tileFarBank=NULL;

							// Catch exception
							try
							{
								// Load the bank
								if (tileBank == NULL)
								{
									CIFile fileBank;
									if (fileBank.open (GetBankPathName ()))
									{
										// Create an xml stream
										CTileBank *tileBankSerial = new CTileBank;
										tileBankSerial->serial (fileBank);
										tileBank = tileBankSerial;
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the bank file %s for reading.", GetBankPathName ().c_str() );
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
									}
								}

								if (tileBank != NULL)
								{
									// Continue ?
									bool cont = true;

									// The .TGA
									if (weWantToMakeASnapshot)
									{
										CIFile fileFarBank;

										std::string path = NLMISC::CFile::getPath(GetBankPathName());
										std::string name = NLMISC::CFile::getFilenameWithoutExtension(GetBankPathName());

										std::string farBankPathName = path + name + ".farbank";

										if (fileFarBank.open (farBankPathName))
										{
											// Create an xml stream
											CTileFarBank *tileFarBankSerial = new CTileFarBank;
											tileFarBankSerial->serial (fileFarBank);
											tileFarBank = tileFarBankSerial;

											CBitmap snapshot;
											if (MakeSnapShot (snapshot, *tileBank, *tileFarBank, width, height, config, errorInDialog))
											{

												// Output the snap shot
												COFile outputSnapShot;
												if (outputSnapShot.open (outputFilenameSnapShot))
												{
													// Write the tga file
													snapshot.writeTGA (outputSnapShot, 32);
												}
											}
											else
											{
												// Error message
												char tmp[512];
												smprintf (tmp, 512, "Can't open the tga file %s for writing.", outputFilenameSnapShot);
												CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
												cont = false;
											}
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the farbank file %s for reading.", farBankPathName );
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
											cont = false;
										}
									}

									// The .ZONE
									if (cont)
									{
										{
											COFile outputZone;
											if (outputZone.open (outputFilenameZone))
											{
												// Serial the NeL zone
												zone.serial (outputZone);
											}
											else
											{
												// Error message
												char tmp[512];
												smprintf (tmp, 512, "Can't open the NeL zone file %s for writing.", outputFilenameZone);
												CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
											}
										}


										// The .LIGOZONE
										{
											// Is filled ?
											uint j;
											for (j=0; j<mask.size(); j++)
												if (!mask[j]) break;
											
											// Add filled zone	
											if (j >= mask.size())
											{
												categories.push_back (pair<string,string> ("filled", "yes"));
											}
											else
											{
												categories.push_back (pair<string,string> ("filled", "no"));
											}

											// Add the zone categorie
											if (width == height)
											{
												categories.push_back (pair<string,string> ("square", "yes"));
											}
											else
											{
												categories.push_back (pair<string,string> ("square", "no"));
											}

											// Add the size category
											char size[30];
											smprintf (size, 30, "%dx%d", width, height);
											categories.push_back (pair<string,string> ("size", size));

											// Create the zone bank element
											CZoneBankElement bankElm;
											bankElm.setMask (mask, width,height);

											// Add the category
											for (j=0; j<categories.size(); j++)
											{
												bankElm.addCategory (strlwr (categories[j].first), strlwr (categories[j].second));
											}


											// Write the zone
											COFile outputLigoZone;

											// Catch exception
											try
											{
												// Open the selected zone file
												if (outputLigoZone.open (outputFilenameLigozone))
												{
													// Create an xml stream
													COXml outputXml;
													outputXml.init (&outputLigoZone);

													// Serial the class
													bankElm.serial (outputXml);

													// Return true
													return &true_value;
												}
												else
												{
													// Error message
													char tmp[512];
													smprintf (tmp, 512, "Can't open the ligozone file %s for writing.", outputFilenameLigozone );
													CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
												}
											}
											catch (const Exception &e)
											{
												// Error message
												char tmp[512];
												smprintf (tmp, 512, "Error while writing the file %s : %s", outputFilenameLigozone, e.what());
												CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
											}
										}
									}
								}
							}
							catch (const Exception &e)
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "Error while loading the file bank %s : %s", GetBankPathName ().c_str(), e.what());
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
							}
						}
					}
				}
			}
			else
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
					"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

Value* get_error_string_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(get_error_string, 1, count);

	// Checks arg
	TCHAR *message = _T("NeLLigoGetErrorString [error code]");
	type_check(arg_list[0], Integer, message);

	// The first arg
	int errorCode = arg_list[0]->to_int()-1;

	// Error code
	return new String(MaxTStrFromUtf8(CLigoError::getStringError ((CLigoError::TError)errorCode)));
}

// ***************************************************************************

Value* set_directory_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(set_directory, 1, count);

	// Checks arg
	TCHAR *message = _T("NeLLigoDirectory [path]");
	type_check(arg_list[0], String, message);

	// The first arg
	const std::string dir = MCharStrToUtf8(arg_list[0]->to_string());

	// Set the directory
	return (chdir (dir.c_str())==0)?&true_value:&false_value;
}

// ***************************************************************************

Value* get_zone_mask_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (5)
	check_arg_count(check_zone_with_template, 5, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoGetZoneMask [Object] [Mask Array] [Width Array] [Height Array] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Array, message);
	type_check(arg_list[4], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The first array
	Array *maskArray = (Array*)arg_list[1];

	// The second array
	Array *widthArray = (Array*)arg_list[2];

	// The second array
	Array *heightArray = (Array*)arg_list[3];

	// The fourth arg
	bool errorInDialog = (arg_list[4]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// Build the zone
				CZone zone;
				CZoneSymmetrisation zoneSymmetry;
				if (!tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, 0, config.CellSize, config.Snap, false))
				{
					// Error, zone can't be exported
					CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{
					// Get the object matrix
					Matrix3 nodeTM;
					nodeTM = node->GetObjectTM (ip->GetTime());

					// max x and y
					int maxx = 0x80000000;
					int maxy = 0x80000000;

					// For each vertex
					for (uint vert=0; vert<(uint)tri->patch.numVerts; vert++)
					{
						// Transform in the world
						Point3 worldPt = tri->patch.verts[vert].p * nodeTM;

						// Get cell coordinates
						int x = (int)(worldPt.x / config.CellSize) + 1;
						int y = (int)(worldPt.y / config.CellSize) + 1;

						// Max ?
						if (x>maxx)
							maxx = x;
						if (y>maxy)
							maxy = y;
					}

					// The second array
					maxx++;
					maxy++;
					widthArray->append (Float::intern ((float)maxx));
					heightArray->append (Float::intern ((float)maxy));

					// Cells
					vector<bool> cells (maxx * maxy, false);

					// For each patch
					for (uint patch=0; patch<(uint)tri->patch.numPatches; patch++)
					{
						// Average of the patch
						Point3 average (0,0,0);

						// For each vertex
						for (uint vert=0; vert<4; vert++)
						{
							// Index of the vertex
							int vertId = tri->patch.patches[patch].v[vert];

							// Sum
							average += tri->patch.verts[vertId].p * nodeTM;
						}

						// Average
						average /= 4;

						// Coordinates
						int x = (int)(average.x / config.CellSize) + 1;
						int y = (int)(average.y / config.CellSize) + 1;

						// Clip
						if ((x>=0) && (y>=0) && (x<maxx) && (y<maxy))
						{
							// Set this ok
							cells[x+y*maxx]=true;
						}
					}

					// For each cell
					for (uint k=0; k<cells.size(); k++)
					{
						// Build the result mask
						maskArray->append (cells[k]?&true_value:&false_value);
					}

					// ok
					return &true_value;
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

Value* get_zone_size_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (6)
	check_arg_count(check_zone_with_template, 6, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoGetZoneMask [Object] [minx Array] [maxy Array] [miny Array] [maxy Array] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Array, message);
	type_check(arg_list[4], Array, message);
	type_check(arg_list[5], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second array
	Array *minXArray = (Array*)arg_list[1];

	// The second array
	Array *maxXArray = (Array*)arg_list[2];

	// The second array
	Array *minYArray = (Array*)arg_list[3];

	// The second array
	Array *maxYArray = (Array*)arg_list[4];

	// The fourth arg
	bool errorInDialog = (arg_list[5]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// Build the zone
				CZone zone;
				CZoneSymmetrisation zoneSymmetry;
				if (!tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, 0, config.CellSize, config.Snap, false))
				{
					// Error, zone can't be exported
					CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{
					// Get the object matrix
					Matrix3 nodeTM;
					nodeTM = node->GetObjectTM (ip->GetTime());

					// max x and y
					int minx = 0x7fffffff;
					int miny = 0x7fffffff;
					int maxx = 0x80000000;
					int maxy = 0x80000000;

					// For each patch
					for (uint patch=0; patch<(uint)tri->patch.numPatches; patch++)
					{
						// Average of the patch
						Point3 average (0,0,0);

						// For each vertex
						for (uint vert=0; vert<4; vert++)
						{
							// Index of the vertex
							int vertId = tri->patch.patches[patch].v[vert];

							// Sum
							average += tri->patch.verts[vertId].p * nodeTM;
						}

						// Average
						average /= 4;

						// Coordinates
						int x = (int)(average.x / config.CellSize);
						int y = (int)(average.y / config.CellSize);

						if (x<minx)
							minx=x;
						if (x>maxx)
							maxx=x;
						if (y<miny)
							miny=y;
						if (y>maxy)
							maxy=y;
					}
					maxx++;
					maxy++;

					// The second array
					minXArray->append (Float::intern ((float)minx));
					maxXArray->append (Float::intern ((float)maxx));
					minYArray->append (Float::intern ((float)miny));
					maxYArray->append (Float::intern ((float)maxy));

					// ok
					return &true_value;
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// Make a snap shot of a zone

bool MakeSnapShot (NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, const NL3D::CTileFarBank &tileFarBank, 
				   sint xmax, sint ymax, const CLigoConfig &config, bool errorInDialog)
{
	return MakeSnapShot (snapshot, tileBank, tileFarBank, 0, xmax, 0, ymax, config, errorInDialog);
}

bool MakeSnapShot (NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, const NL3D::CTileFarBank &tileFarBank, 
				   sint xmin, sint xmax, sint ymin, sint ymax, const CLigoConfig &config, bool errorInDialog)
{
	// Result
	bool result = false;
		
	try
	{
		// Resolution
		sint widthPixel = config.ZoneSnapShotRes * (xmax-xmin);
		sint heightPixel = config.ZoneSnapShotRes * (ymax-ymin);

		sint oversampledWidth = widthPixel*4;
		sint oversampledHeight = heightPixel*4;

		// Oversampled size should be < 2048
		if (oversampledWidth > 2048)
			oversampledWidth = 2048;
		if (oversampledHeight > 2048)
			oversampledHeight = 2048;
		
		// Oversampled size should be < sreen size
		DEVMODE	devMode;
		devMode.dmSize= sizeof(DEVMODE);
		devMode.dmDriverExtra= 0;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
		if (oversampledWidth > (sint)devMode.dmPelsWidth)
			oversampledWidth = devMode.dmPelsWidth;
		if (oversampledHeight > (sint)devMode.dmPelsHeight)
			oversampledHeight = devMode.dmPelsHeight;

		// Region
		float width = config.CellSize * (float)(xmax-xmin);
		float height = config.CellSize * (float)(ymax-ymin);
		float posX = config.CellSize * (float)xmin;
		float posY = config.CellSize * (float)ymin;

		// Use NELU
		if (CNELU::init (oversampledWidth, oversampledHeight, CViewport(), 32, true, NULL, false, true)) // FIXME: OpenGL not working correctly, offscreen not available in Direct3D
		{
			// Setup the camera
			CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
			CMatrix view;
			view.setPos (CVector (width/2 + posX, height/2 + posY, width));
			view.setRot (CVector::I, -CVector::K, CVector::J);
			CNELU::Camera->setFrustum (width, height, 0.1f, 10000.f, false);
			CNELU::Camera->setMatrix (view);

			// Create a Landscape.
			CLandscapeModel	*theLand= (CLandscapeModel*)CNELU::Scene->createModel(LandscapeModelId);

			// Build the scene
			CExportNelOptions options;
			CExportNel export_ (errorInDialog, false, true, MAXScript_interface, "Snapshot ligozone", &options);
			export_.buildScene (*CNELU::Scene, *CNELU::ShapeBank, *CNELU::Driver, 0, &theLand->Landscape, NULL, false, false, false);

			theLand->Landscape.setTileNear (50.f);
			theLand->Landscape.TileBank=tileBank;
			theLand->Landscape.TileFarBank=tileFarBank;
			theLand->Landscape.initTileBanks ();

			// Enable additive tiles
			theLand->enableAdditive (true);
			theLand->Landscape.setRefineMode (true);

			// theLand->Landscape.setupStaticLight(CRGBA(255, 255, 255), CRGBA(0, 0, 0), 1.0f);
			// theLand->Landscape.setThreshold(0.0005);

			// Enbable automatique lighting
	#ifndef NL_DEBUG
			// theLand->Landscape.enableAutomaticLighting (true);
			// theLand->Landscape.setupAutomaticLightDir (CVector (0, 0, -1));
	#endif // NL_DEBUG

			// theLand->Landscape.updateLightingAll();

			// Clear the backbuffer and the alpha
			CNELU::clearBuffers(CRGBA(255,0,255,0));

			// Render the scene
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();
			CNELU::Scene->render();

			// Snapshot
			CNELU::Driver->getBuffer (snapshot);

			// Release the driver
			CNELU::Driver->release ();

			// Release NELU
			CNELU::release();

			// Resample the bitmap
			snapshot.resample (widthPixel, heightPixel);

			// Ok
			result = true;
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Can't initialise opengl offscreen renderer", "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}
	catch (const Exception &e)
	{
		// Error
		char tmp[512];
		smprintf (tmp, 512, "Error during the snapshot: %s", e.what());

		// Output an error
		CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return result
	return result;
}


// ***************************************************************************
 
/// Export a ligo zone
Value* make_snapshot_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (7)
	check_arg_count(NeLLigoMakeSnapShot, 7, count);

	// Check to see if the arguments match up to what we expect
	TCHAR *message = _T("NeLLigoMakeSnapShot [Object] [Snapshot filename] [xMin] [xMax] [yMin] [yMax] [Error in dialog]");
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Integer, message);
	type_check(arg_list[3], Integer, message);
	type_check(arg_list[4], Integer, message);
	type_check(arg_list[5], Integer, message);
	type_check(arg_list[6], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = MCharStrToUtf8(arg_list[1]->to_string());

	// The thrid arg
	int xMin = arg_list[2]->to_int();
	int xMax = arg_list[3]->to_int();
	int yMin = arg_list[4]->to_int();
	int yMax = arg_list[5]->to_int();

	// The fourth arg
	bool errorInDialog = (arg_list[6]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// Build a filename
				std::string nametga = CFile::getFilenameWithoutExtension(fileName);
				std::string pathtga = CFile::getPath(fileName);


				// Build the zone
				CZone zone;
				CZoneSymmetrisation zoneSymmetry;
				if (!tri->rpatch->exportZone (node, &tri->patch, zone, zoneSymmetry, 0, config.CellSize, config.Snap, false))
				{
					// Error, zone can't be exported
					CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{

					// The bank
					static CTileBank *tileBank=NULL;
					static CTileFarBank *tileFarBank=NULL;

					// Catch exception
					try
					{
						if (tileBank == NULL)
						{
							// Load the bank
							CIFile fileBank;
							if (fileBank.open (GetBankPathName ()))
							{
								// Create an xml stream
								CTileBank *bankSerial = new CTileBank;
								bankSerial->serial (fileBank);
								tileBank = bankSerial;
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "Can't open the bank file %s for writing.", GetBankPathName ().c_str() );
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
							}
						}
						
						if (tileBank != NULL)
						{
							CIFile fileFarBank;
							std::string name = CFile::getFilenameWithoutExtension(GetBankPathName());
							std::string farBankPathName = CFile::getPath(GetBankPathName()) + name + ".farbank";

							if (fileFarBank.open (farBankPathName))
							{
								// Create an xml stream
								CTileFarBank *tileFarBankSerial = new CTileFarBank;
								tileFarBankSerial->serial (fileFarBank);
								tileFarBank = tileFarBankSerial;

								// Build a screenshot of the zone
								CBitmap snapshot;
								if (MakeSnapShot (snapshot, *tileBank, *tileFarBank, xMin, xMax, yMin, yMax, config, errorInDialog))
								{
									// Build the snap shot filename
									std::string outputFilenameSnapShot = pathtga + nametga + ".tga";

									// Output the snap shot
									COFile outputSnapShot;
									if (outputSnapShot.open (outputFilenameSnapShot))
									{
										// Write the tga file
										snapshot.writeTGA (outputSnapShot, 32);

										// Some categories
										vector<pair<string, string> > categories;

										// Add filled zone	
										categories.push_back (pair<string,string> ("filled", "yes"));

										// Add the zone categorie
										categories.push_back (pair<string,string> ("square", "yes"));

										// Add the size category
										categories.push_back (pair<string,string> ("size", "1x1"));

										// Add the material category
										categories.push_back (pair<string,string> ("material", "fyros"));

										// Add the material category
										categories.push_back (pair<string,string> ("zone", name));

										// Create the zone bank element
										CZoneBankElement bankElm;
										std::vector<bool> mask;
										mask.push_back (true);
										bankElm.setMask (mask, 1, 1);

										// Add the category
										for (uint j=0; j<categories.size(); j++)
										{
											bankElm.addCategory (strlwr (categories[j].first), strlwr (categories[j].second));
										}

										// Catch exception
#if 0
										// Write the zone
										COFile outputLigoZone;
										std::string outputFilenameSnapShot = pathtga + nametga + ".ligozone";

										try
										{
											// Open the selected zone file
											if (outputLigoZone.open (outputFilenameSnapShot))
											{
												// Create an xml stream
												COXml outputXml;
												outputXml.init (&outputLigoZone);

												// Serial the class
												bankElm.serial (outputXml);

												// Return true
												return &true_value;
											}
											else
											{
												// Error message
												char tmp[512];
												smprintf (tmp, 512, "Can't open the ligozone file %s for writing.", fileName.c_str() );
												CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
											}
										}
										catch (const Exception &e)
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
										}
#endif
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the NeL snapshot file %s for writing.", outputFilenameSnapShot);
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
									}
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "Can't open the farbank file %s for reading.", farBankPathName );
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
							}
						}
					}
					catch (const Exception &e)
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Error while loading the file bank %s : %s", GetBankPathName ().c_str(), e.what());
						CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
					}
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

/// MAXScript Plugin Initialization

// ***************************************************************************

__declspec( dllexport ) void LibInit() { }

// ***************************************************************************

