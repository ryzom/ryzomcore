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

	~CNelExport();

	void init (bool view, bool errorInDialog, Interface	*ip, bool loadStruct);

	TCHAR* fixupName(TCHAR* name);
	bool nodeEnum(INode* pNode, bool selected);
	bool doExport(bool selected=false);
	bool freeExported(void);
	void getSelectedNode (std::vector<INode*>& vectNode);

	bool	exportZone	(const char *sName, INode& node, TimeValue time);
	bool	exportMesh	(const char *sPath, INode& node, TimeValue time);
	bool	exportAnim	(const char *sPath, std::vector<INode*>& vectNode, TimeValue time, bool scene);
	bool	exportSWT	(const char *sPath, std::vector<INode*>& vectNode);

	bool	exportInstanceGroup	(std::string filename, std::vector<INode*>& vectNode);
	bool	exportSkeleton	(const char *sPath, INode* pNode, TimeValue time);

	bool	exportCollision	(const char *sPath, std::vector<INode *> &nodes, TimeValue time);

	bool	exportPACSPrimitives (const char *sPath, std::vector<INode *> &nodes, TimeValue time);

	bool	exportVegetable (const char *sPath, INode& node, TimeValue time);

	bool	exportLodCharacter (const char *sPath, INode& node, TimeValue time);

	void	viewMesh (TimeValue time);

	static void initOptions(); // read the CNelExportSceneStruct from disk or init it
	static void deleteLM(INode& ZeNode); // the export scene struct MUST be initialized before calling this fn
	void			OnNodeProperties (const std::set<INode*> &listNode);

	ULONG ExtractFileName(char* Path, char* Name);
	ULONG ExtractPath(char* FullPath, char* Path);
	ULONG SelectFileForLoad(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectFileForSave(HWND Parent, char* Title, const char* Mask, char* FileName);
	ULONG SelectDir(HWND Parent, char* Title, char* Path);
	static ULONG FileExists(const char* FileName);
	ULONG GetFileSize(char* FileName);
	ULONG ProcessDir(char* Dir, const char* Mask, unsigned long flag, ULONG Fnct(char* FileName) );
	ULONG CleanFileName(char* FileName);
	ULONG CreateBAKFile(char* FileName);

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
	const TCHAR *	ClassName() {return _T("NeL Export");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return CNELEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return _T("NeL Tools");}
	const TCHAR*	InternalName() { return _T("NeL export and view"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

extern CNelExportClassDesc CNelExportDesc;

extern CNelExport theCNelExport;

void nelExportTerminateProcess();


#endif // __NEL_EXPORT__H
