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

#include "std3d.h"

#include "nel/3d/async_file_manager_3d.h"
#include "nel/3d/shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/scene_group.h"

#include "nel/3d/instance_group_user.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{

//CAsyncFileManager3D *CAsyncFileManager3D::_Singleton = NULL;
NLMISC_SAFE_SINGLETON_IMPL(CAsyncFileManager3D);

// ***************************************************************************

CAsyncFileManager3D::CAsyncFileManager3D()
{
}

// ***************************************************************************

/*CAsyncFileManager3D &CAsyncFileManager3D::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManager3D();
	}
	return *_Singleton;
}
*/
// ***************************************************************************

void CAsyncFileManager3D::terminate ()
{
	if (_Instance != NULL)
	{
		CAsyncFileManager3D *afm = _Instance;
		INelContext::getInstance().releaseSingletonPointer("CAsyncFileManager3D", _Instance);
		_Instance = NULL;
		delete afm;
	}
}

// ***************************************************************************

void CAsyncFileManager3D::loadMesh(const std::string& meshName, IShape **ppShp, IDriver *pDriver, const CVector &position, uint textureSlot)
{
	if (meshName.empty())
		nlwarning ("Can't load an empty filename");
	CAsyncFileManager::getInstance().addLoadTask(new CMeshLoad(meshName, ppShp, pDriver, position, textureSlot));
}

// ***************************************************************************

// Callback class for canceling a loadMesh
class CLoadMeshCancel : public NLMISC::CAsyncFileManager::ICancelCallback
{
public:
	CLoadMeshCancel (const std::string &meshName)
		: _MeshName(meshName)
	{}
	virtual ~CLoadMeshCancel() { }

private:
	std::string	_MeshName;

	bool callback(const NLMISC::IRunnable *prunnable) const
	{
		const CAsyncFileManager3D::CMeshLoad *pML = dynamic_cast<const CAsyncFileManager3D::CMeshLoad*>(prunnable);
		if (pML != NULL)
		{
			if (pML->MeshName == _MeshName)
			{
				return true;
			}
		}
		return false;
	}

	void getName (std::string &result) const
	{
		result = "LoadMeshCancel (" + _MeshName + ")";
	}
};

bool CAsyncFileManager3D::cancelLoadMesh(const std::string& sMeshName)
{
	return CAsyncFileManager::getInstance().cancelLoadTask(CLoadMeshCancel(sMeshName));
}

// ***************************************************************************

void CAsyncFileManager3D::loadIG (const std::string& IGName, CInstanceGroup **ppIG)
{
	CAsyncFileManager::getInstance().addLoadTask(new CIGLoad(IGName, ppIG));
}

// ***************************************************************************

void CAsyncFileManager3D::loadIGUser (const std::string& IGName, UInstanceGroup **ppIG)
{
	CAsyncFileManager::getInstance().addLoadTask (new CIGLoadUser(IGName, ppIG));
}

// ***************************************************************************
void CAsyncFileManager3D::loadTexture (CTextureFile *textureFile, bool *pSgn, const NLMISC::CVector &position)
{
	nlassert(textureFile && pSgn);
	CAsyncFileManager::getInstance().addLoadTask(new CTextureLoad(textureFile, pSgn, position));
}

// Callback class for canceling a load texture
class CLoadTextureCancel : public NLMISC::CAsyncFileManager::ICancelCallback
{
public:
	CLoadTextureCancel (CTextureFile *ptextureFile)
		: _TextureFile(ptextureFile)
	{}

	virtual ~CLoadTextureCancel() {}

private:
	CTextureFile	*_TextureFile;

	bool callback(const NLMISC::IRunnable *prunnable) const
	{
		const CAsyncFileManager3D::CTextureLoad *pTL = dynamic_cast<const CAsyncFileManager3D::CTextureLoad*>(prunnable);
		if (pTL != NULL)
		{
			if (pTL->TextureFile == _TextureFile)
			{
				return true;
			}
		}
		return false;
	}

	void getName (std::string &result) const
	{
		result = "LoadTextureCancel (" + _TextureFile->getFileName() + ")";
	}
};


// ***************************************************************************
bool CAsyncFileManager3D::cancelLoadTexture (CTextureFile *textFile)
{
	return CAsyncFileManager::getInstance().cancelLoadTask(CLoadTextureCancel(textFile));
}


// ***************************************************************************

void CAsyncFileManager3D::loadFile (const std::string& sFileName, uint8 **ppFile)
{
	CAsyncFileManager::getInstance().loadFile (sFileName, ppFile);
}

// ***************************************************************************

void CAsyncFileManager3D::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	CAsyncFileManager::getInstance().loadFiles (vFileNames, vPtrs);
}

// ***************************************************************************

void CAsyncFileManager3D::signal (bool *pSgn)
{
	CAsyncFileManager::getInstance().signal(pSgn);
}

// ***************************************************************************

void CAsyncFileManager3D::cancelSignal (bool *pSgn)
{
	CAsyncFileManager::getInstance().cancelSignal(pSgn);
}

// ***************************************************************************
// TASKS
// ***************************************************************************

// ***************************************************************************
// MeshLoad
// ***************************************************************************

CAsyncFileManager3D::CMeshLoad::CMeshLoad(const std::string& sMeshName, IShape** ppShp, IDriver *pDriver, const CVector &position, uint selectedTexture)
{
	_pDriver = pDriver;
	MeshName = sMeshName;
	_ppShp = ppShp;
	Position = position;
	_SelectedTexture = selectedTexture;
}

// ***************************************************************************

void CAsyncFileManager3D::CMeshLoad::run()
{
	// This set represent the texture already loaded in memory
	// We have to have this set because the driver load the textures only on the
	// setupTexture, done in CShapeBank::isPresent. This must be done in the main
	// thread because setupTexture upload texture to VRAM.
	typedef set<string> TAlreadyPresentTextureSet;
	TAlreadyPresentTextureSet AlreadyPresentTextureSet;

	try
	{
		// Load from file the mesh
		CShapeStream mesh;
		CIFile meshfile;
		meshfile.setAsyncLoading (true);
		meshfile.setCacheFileOnOpen (true);
		meshfile.open (CPath::lookup(MeshName));
		meshfile.serial (mesh);
		meshfile.close ();

		// Is the pointer is invalid return -1
		if (mesh.getShapePointer() == NULL)
		{
			nlwarning ("Couldn't load '%s'", MeshName.c_str());
			*_ppShp = (IShape*)-1;
			delete this;
			return;
		}

		CMeshBase *pMesh = dynamic_cast<CMeshBase *>(mesh.getShapePointer());

		// If the shape is a mesh but the driver is not given or if the shape is not a mesh
		// then do not try to load the textures

		if ((pMesh == NULL) || ((pMesh != NULL) && (_pDriver == NULL)))
		{
			if (_pDriver == NULL || mesh.getShapePointer() == NULL)
			{
				nlwarning ("mesh or driver is NULL for file '%s'", MeshName.c_str());
			}

			*_ppShp = mesh.getShapePointer();
			delete this;
			return;
		}

		// Here we are sure that the shape is a mesh and the driver is not null
		// Parse all materials of the mesh
		uint i, j;
		uint nNbMat = pMesh->getNbMaterial();
		ITexture *pText;

		for(i = 0; i < nNbMat; ++i)
		{
			const CMaterial &rMat = pMesh->getMaterial(i);
			// Parse all textures from this material and generate them
			for(j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
			if (rMat.texturePresent(uint8(j)))
			{
				pText = rMat.getTexture (uint8(j));

				// Does this texture is a texture file ?
				if ((pText != NULL) && (pText->supportSharing()))
				{
					// Set texture slot
					pText->selectTexture(_SelectedTexture);

					// Yes -> Does the texture is already present in the driver ?
					if( ! _pDriver->isTextureExist(*pText) )
					{
						// No -> So we have perhaps to load it
						TAlreadyPresentTextureSet::iterator aptmIt = AlreadyPresentTextureSet.find (pText->getShareName());
						// Is the texture already loaded ?
						if(aptmIt == AlreadyPresentTextureSet.end())
						{
							// Texture not already present
							// add it
							AlreadyPresentTextureSet.insert (pText->getShareName());
							// And load it (to RAM only (upload in VRAM is done in the shape bank))
							pText->generate(true);
						}
					}
				}
			}

			// Do the same with lightmaps
			if (rMat.getShader() == CMaterial::LightMap)
			{
				j = 0; pText = rMat.getLightMap (j);
				while (pText != NULL)
				{
					// Does this texture is a texture file ?
					if ((pText != NULL) && (pText->supportSharing()))
					{
						// Yes -> Does the texture is already present in the driver ?
						if (!_pDriver->isTextureExist(*pText))
						{
							// No -> So we have perhaps to load it
							TAlreadyPresentTextureSet::iterator aptmIt = AlreadyPresentTextureSet.find (pText->getShareName());
							// Is the texture already loaded ?
							if(aptmIt == AlreadyPresentTextureSet.end())
							{
								// Texture not already present -> add it and load it to RAM
								AlreadyPresentTextureSet.insert (pText->getShareName());
								pText->generate(true);
							}
						}
					}
					++j; pText = rMat.getLightMap (j);
				}
			}
		}
		// Finally affect the pointer (Trans-Thread operation -> this operation must be atomic)
		*_ppShp = mesh.getShapePointer();
	}
	catch(const EPathNotFound &)
	{
		nlwarning ("Couldn't load '%s'", MeshName.c_str());
		*_ppShp = (IShape*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
void CAsyncFileManager3D::CMeshLoad::getName (std::string &result) const
{
	result = "LoadMesh (" + MeshName + ")";
}



// ***************************************************************************
// IGLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager3D::CIGLoad::CIGLoad (const std::string &IGName, CInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager3D::CIGLoad::run (void)
{
	try
	{
		CIFile igfile;
		igfile.setAsyncLoading (true);
		igfile.setCacheFileOnOpen (true);
		igfile.open (CPath::lookup (_IGName));
		CInstanceGroup *pIG = new CInstanceGroup();
		pIG->serial (igfile);
		igfile.close();

		*_ppIG = pIG;
	}
	catch(const EPathNotFound &)
	{
		nlwarning ("Couldn't load '%s'", _IGName.c_str());
		*_ppIG = (CInstanceGroup*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
void CAsyncFileManager3D::CIGLoad::getName (std::string &result) const
{
	result = "LoadIG(" + _IGName + ")";
}


// ***************************************************************************
// IGLoadUser
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager3D::CIGLoadUser::CIGLoadUser (const std::string &IGName, UInstanceGroup **ppIG)
{
	_IGName = IGName;
	_ppIG = ppIG;
}

// ***************************************************************************
void CAsyncFileManager3D::CIGLoadUser::run (void)
{
	CInstanceGroupUser *pIG = new CInstanceGroupUser();
	try
	{
		if (pIG->init (_IGName, true))
		{
			*_ppIG = pIG;
		}
		else
		{
			nlwarning ("Couldn't init '%s'", _IGName.c_str());
			delete pIG;
			*_ppIG = (UInstanceGroup*)-1;
			delete this;
			return;
		}
	}
	catch(const EPathNotFound &)
	{
		nlwarning ("Couldn't load '%s'", _IGName.c_str());
		delete pIG;
		*_ppIG = (UInstanceGroup*)-1;
		delete this;
		return;
	}
	delete this;
}

// ***************************************************************************
void CAsyncFileManager3D::CIGLoadUser::getName (std::string &result) const
{
	result = "LoadIGUser(" + _IGName + ")";
}

// ***************************************************************************
// CTextureLoad
// ***************************************************************************

// ***************************************************************************
void	CAsyncFileManager3D::CTextureLoad::run()
{
	// Load the texture.
	TextureFile->generate(true);
	// Ok
	*Signal= true;

	delete this;
}

// ***************************************************************************
void CAsyncFileManager3D::CTextureLoad::getName (std::string &result) const
{
	if (TextureFile)
		result = "LoadTexture(" + TextureFile->getFileName() + ")";
	else
		result = "LoadTexture(NULL)";
}

} // NL3D
