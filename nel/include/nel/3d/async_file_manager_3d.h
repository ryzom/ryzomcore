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

#ifndef NL_ASYNC_FILE_MANAGER_3D_H
#define NL_ASYNC_FILE_MANAGER_3D_H

#include "nel/misc/types_nl.h"
#include "nel/misc/thread.h"
#include "nel/misc/async_file_manager.h"


namespace NL3D
{

class IShape;
class IDriver;
class CInstanceGroup;
class UInstanceGroup;
class CTextureFile;

/**
 * CAsyncFileManager is a class that manage file loading in a seperate thread
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2002
 */
class CAsyncFileManager3D
{

	NLMISC_SAFE_RELEASABLE_SINGLETON_DECL(CAsyncFileManager3D);
	CAsyncFileManager3D();
public:

//	static CAsyncFileManager3D &getInstance (); // Must be called instead of constructing the object
	static void terminate (); // release singleton

	void loadMesh (const std::string &sMeshName, IShape **ppShp, IDriver *pDriver, const NLMISC::CVector &position, uint textureSlot);
	bool cancelLoadMesh (const std::string& sMeshName);

	void loadIG (const std::string &igName, CInstanceGroup **ppIG);
	void loadIGUser (const std::string &igName, UInstanceGroup **ppIG);

	void loadTexture (CTextureFile *textureFile, bool *pSgn, const NLMISC::CVector &position);
	bool cancelLoadTexture (CTextureFile *textFile);


	// Do not use these methods with the bigfile manager
	void loadFile (const std::string &fileName, uint8 **pPtr);
	void loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs);


	void signal (bool *pSgn); // Signal a end of loading for a group of "mesh or file" added
	void cancelSignal (bool *pSgn);

private:


//	CAsyncFileManager3D (); // Singleton mode -> access it with the getInstance function

//	static CAsyncFileManager3D *_Singleton;


	friend class CLoadMeshCancel;
	friend class CLoadTextureCancel;

	// All the tasks
	// -------------

	// Load a .shape
	class CMeshLoad : public NLMISC::IRunnablePos
	{
		IShape **_ppShp;
		IDriver *_pDriver;
		uint	_SelectedTexture;
	public:
		std::string MeshName;
	public:
		CMeshLoad (const std::string &meshName, IShape **ppShp, IDriver *pDriver, const NLMISC::CVector &position, uint selectedTexture);
		void run (void);
		void getName (std::string &result) const;
	};

	// Load a .ig
	class CIGLoad : public NLMISC::IRunnable
	{
		std::string _IGName;
		CInstanceGroup **_ppIG;
	public:
		CIGLoad (const std::string& meshName, CInstanceGroup **ppIG);
		void run (void);
		void getName (std::string &result) const;
	};

	// Load a .ig User Interface
	class CIGLoadUser : public NLMISC::IRunnable
	{
		std::string _IGName;
		UInstanceGroup **_ppIG;
	public:
		CIGLoadUser (const std::string& meshName, UInstanceGroup **ppIG);
		void run (void);
		void getName (std::string &result) const;
	};

	// Load a texture
	class CTextureLoad : public NLMISC::IRunnablePos
	{
	public:
		CTextureFile	*TextureFile;
		bool			*Signal;
	public:
		CTextureLoad(CTextureFile *textureFile, bool *psgn, const NLMISC::CVector &position)
			: TextureFile(textureFile), Signal(psgn)
		{
			Position = position;
		}

		void run();
		void getName (std::string &result) const;
	};

};


} // NL3D


#endif // NL_ASYNC_FILE_MANAGER_3D_H

/* End of async_file_manager.h */
