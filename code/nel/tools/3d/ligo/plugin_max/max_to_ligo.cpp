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

#include <assert.h>

// From MAXSDK
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#endif

#include "max_to_ligo.h"

#undef min
#undef max

// From nel misc
#include "nel/misc/stream.h"

// From ligo library
#include "nel/../../src/ligo/zone_template.h"
#include "nel/ligo/ligo_config.h"
#include "nel/../../src/ligo/ligo_error.h"

using namespace std;
using namespace NLMISC;
extern HINSTANCE hInstance;

#define NEL3D_APPDATA_ZONE_SYMMETRY		1266703979

namespace NLLIGO
{

// ***************************************************************************

int getScriptAppDataPatchMesh (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (sscanf ((const char*)ap->data, "%d", &value)==1)
		return value;
	else
		return def;
}

// ***************************************************************************

bool CMaxToLigo::buildZoneTemplate (INode* pNode, const PatchMesh &patchMesh, CZoneTemplate &zoneTemplate, const CLigoConfig &config, CLigoError &errors, TimeValue time)
{	
	// Vertices
	std::vector<NLMISC::CVector> vertices;
	vertices.resize (patchMesh.numVerts);

	// Indexies
	std::vector< std::pair<uint, uint> > indexes;

	// Get node matrix
	Matrix3 local = pNode->GetObjectTM (time);

	// For each vertices
	for (uint vert=0; vert<(uint)patchMesh.numVerts; vert++)
	{
		// Transform the vertex
		Point3 v = local * patchMesh.verts[vert].p;

		// Copy it
		vertices[vert].x = v.x;
		vertices[vert].y = v.y;
		vertices[vert].z = v.z;
	}

	// Symetric ?
	bool sym = getScriptAppDataPatchMesh (pNode, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;

	// For each edges
	for (uint edge=0; edge<(uint)patchMesh.numEdges; edge++)
	{
		// Open edge ?
#if (MAX_RELEASE < 4000)
		if (patchMesh.edges[edge].patch2<0)
#else // (MAX_RELEASE < 4000)
		if (patchMesh.edges[edge].patches.Count()<2)
#endif // (MAX_RELEASE < 4000)
		{
			// Add this edge
			if (sym)
				indexes.push_back (pair<uint, uint> (patchMesh.edges[edge].v2, patchMesh.edges[edge].v1));
			else
				indexes.push_back (pair<uint, uint> (patchMesh.edges[edge].v1, patchMesh.edges[edge].v2));
		}
	}

	// Build it
	return zoneTemplate.build (vertices, indexes, config, errors);
}

// ***************************************************************************
bool CMaxToLigo::loadLigoConfigFile (CLigoConfig& config, Interface& it, bool dialog)
{
	// Get the module path
	HMODULE hModule = hInstance;
	if (hModule)
	{
		// Get the path
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);

		// Success ?
		if (res)
		{
			// Path
			char sDrive[256];
			char sDir[256];
			_splitpath (sModulePath, sDrive, sDir, NULL, NULL);
			_makepath (sModulePath, sDrive, sDir, "ligoscape", ".cfg");

			try
			{
				// Load the config file
				config.readConfigFile (sModulePath, false);

				// ok
				return true;
			}
			catch (Exception& e)
			{
				// Print an error message
				char msg[512];
				smprintf (msg, 512, "Error loading the config file ligoscape.cfg: %s", e.what());
				errorMessage (msg, "NeL Ligo load config file", it, dialog);
			}
		}
	}

	// Can't found the module
	return false;
}

// ***************************************************************************

void CMaxToLigo::errorMessage (const char *msg, const char *title, Interface& it, bool dialog)
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
		mprintf ((string(msg) + "\n").c_str());
	}

	// Output in log
	nlwarning ("LIGO ERROR : %s", msg);
}

// ***************************************************************************

}