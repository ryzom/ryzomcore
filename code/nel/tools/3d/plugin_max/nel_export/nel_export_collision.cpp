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
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"

#include "nel/pacs/collision_mesh_build.h"
#include "nel/pacs/primitive_block.h"

#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"

using namespace NLMISC;
using namespace NLPACS;

// --------------------------------------------------

bool CNelExport::exportCollision (const char *sPath, std::vector<INode *> &nodes, TimeValue time)
{
	// get list of CMB froms nodes.
	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> >	meshBuildList;
	if(!_ExportNel->createCollisionMeshBuildList(nodes, time, meshBuildList))
		return false;

	// Result to return
	bool bRet=false;

//	ULONG SelectDir(HWND Parent, char* Title, char* Path);

	std::string	path = std::string(sPath);
	if (path.size() == 0 || path[path.size()-1] != '\\' && path[path.size()-1] != '/')
		path.insert(path.end(), '/');

	if (meshBuildList.empty()) return true;
	for (uint i=0; i<meshBuildList.size(); ++i)
	{
		std::string				igname = meshBuildList[i].first;
		std::string				filename = path+igname+".cmb";
		CCollisionMeshBuild		*pCmb = meshBuildList[i].second;

		// Open a file
		COFile file;
		if (file.open (filename))
		{
			try
			{
				// Serialise the collision mesh build
				file.serial(*pCmb);

				// All is good
				bRet=true;
			}
			catch (...)
			{
			}
		}

		// Delete the pointer
		delete pCmb;
	}

/*
	// Object exist ?
	CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(nodes, time);

	// Conversion success ?
	if (pCmb)
	{
		// Open a file
		COFile file;
		if (file.open (sPath))
		{
			try
			{
				// Serialise the collision mesh build
				file.serial(*pCmb);

				// All is good
				bRet=true;
			}
			catch (...)
			{
			}
		}

		// Delete the pointer
		delete pCmb;
	}
*/
	return bRet;
}

/*
bool CNelExport::exportCollision (const char *sPath, INode& node, Interface& _Ip, TimeValue time, CExportNelOptions &opt)
{
	// Result to return
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(node, time);

		// Conversion success ?
		if (pCmb)
		{
			// Open a file
			COFile file;
			if (file.open (sPath))
			{
				try
				{
					// Serialise the collision mesh build
					file.serial(*pCmb);

					// All is good
					bRet=true;
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete pCmb;
		}
	}
	return bRet;
}
*/
// --------------------------------------------------

bool CNelExport::exportPACSPrimitives (const char *sPath, std::vector<INode *> &nodes, TimeValue time)
{
	// Build the primitive block
	NLPACS::CPrimitiveBlock primitiveBlock;
	if (_ExportNel->buildPrimitiveBlock (time, nodes, primitiveBlock))
	{
		// Open the file
		COFile file;
		if (file.open (sPath))
		{
			// Create the XML stream
			COXml output;

			// Init
			if (output.init (&file, "1.0"))
			{
				// Serial it
				primitiveBlock.serial (output);

				// Ok
				return true;
			}
			else
			{
				nlwarning ("Can't init XML stream with file %s", sPath);
			}
		}
		else
		{
			nlwarning ("Can't open the file %s for writing", sPath);
		}
	}
	return false;
}