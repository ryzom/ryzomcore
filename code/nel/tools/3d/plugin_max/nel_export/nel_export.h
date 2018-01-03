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

#ifndef __NEL_EXPORT__H
#define __NEL_EXPORT__H

#include "resource.h"
#include "../nel_mesh_lib/export_nel.h"
#include "nel/3d/mesh.h"

#include <vector>
#include <string>

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define CNELEXPORT_CLASS_ID	Class_ID(0x8c02158, 0x5a9e252b)

namespace NL3D
{
	class CSkeletonShape;
}

class CNelExport : public UtilityObj 
{
private:

public:
	
	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void DeleteThis() { }		

	CNelExport ();

	virtual ~CNelExport();

	void init (bool view, bool errorInDialog, Interface	*ip, bool loadStruct);

	TCHAR* fixupName(TCHAR* name);
	bool nodeEnum(INode* pNode, bool selected);
	bool doExport(bool selected=false);
	bool freeExported(void);
	void getSelectedNode (std::vector<INode*>& vectNode);

	bool	exportZone	(const std::string &sName, INode& node, TimeValue time);
	bool	exportMesh	(const std::string &sPath, INode& node, TimeValue time);
	bool	exportAnim	(const std::string &sPath, std::vector<INode*>& vectNode, TimeValue time, bool scene);
	bool	exportSWT	(const std::string &sPath, std::vector<INode*>& vectNode);

	bool	exportInstanceGroup	(std::string filename, std::vector<INode*>& vectNode);
	bool	exportSkeleton	(const std::string &sPath, INode* pNode, TimeValue time);

	bool	exportCollision	(const std::string &sPath, std::vector<INode *> &nodes, TimeValue time);

	bool	exportPACSPrimitives (const std::string &sPath, std::vector<INode *> &nodes, TimeValue time);

	bool	exportVegetable (const std::string &sPath, INode& node, TimeValue time);

	bool	exportLodCharacter (const std::string &sPath, INode& node, TimeValue time);

	void	viewMesh (TimeValue time);

	static void initOptions(); // read the CNelExportSceneStruct from disk or init it
	static void deleteLM(INode& ZeNode); // the export scene struct MUST be initialized before calling this fn
	void			OnNodeProperties (const std::set<INode*> &listNode);

	ULONG SelectFileForSave(HWND Parent, const TCHAR* Title, const TCHAR* Mask, std::string &FileName);
	ULONG SelectDir(HWND Parent, const TCHAR* Title, std::string &Path);

	// The nel export objtect
	CExportNel		*_ExportNel;

	// The interface pointer
	Interface		*_Ip;

	// View
	// bool			_View;

	// View
	bool			_ErrorInDialog;

	// Handle problematic file handles with max.
	bool			_TerminateOnFileOpenIssues;
};

class CNelExportClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE);
	const MCHAR *	ClassName() {return _M("NeL Export");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return CNELEXPORT_CLASS_ID;}
	const MCHAR* 	Category() {return _M("NeL Tools");}
	const MCHAR*	InternalName() { return _M("NeL export and view"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

extern CNelExportClassDesc CNelExportDesc;

extern CNelExport theCNelExport;

void nelExportTerminateProcess();


#endif // __NEL_EXPORT__H
