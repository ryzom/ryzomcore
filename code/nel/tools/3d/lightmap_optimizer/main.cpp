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

// lightmap_optimizer
// ------------------
// the goal is to regroup lightmap of a level into lightmap with a higher level

#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/log.h"
#include "nel/misc/path.h"

#include "nel/3d/mesh_base.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_multi_lod.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/register_3d.h"

#ifdef NL_OS_WINDOWS
	#include <windows.h>
#else
	#include <dirent.h> /* for directories functions */
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h> /* getcwd, chdir -- replacement for getCurDiretory & setCurDirectory on windows */
#endif



#include <vector>
#include <string>

// ---------------------------------------------------------------------------

using namespace std;
using namespace NL3D;


void outString (const string &sText) ;

// ---------------------------------------------------------------------------
#ifdef NL_OS_WINDOWS  // win32 code
void GetCWD (int length,char *dir)
{
	GetCurrentDirectoryA (length, dir);
}

bool ChDir(const char *path)
{
	return SetCurrentDirectoryA (path);
}
void dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	sAllFiles.clear ();
	GetCurrentDirectory (MAX_PATH, sCurDir);
	hFind = FindFirstFile ("*"+sFilter.c_str(), &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		DWORD res = GetFileAttributes(findData.cFileName);
		if (res != INVALID_FILE_ATTRIBUTES && !(res&FILE_ATTRIBUTE_DIRECTORY))
		{
			if (bFullPath)
				sAllFiles.push_back(string(sCurDir) + "\\" + findData.cFileName);
			else
				sAllFiles.push_back(findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
}
#else   // posix version  of the void dir(...) function.
void GetCWD (int length, char* directory)
{
	getcwd (directory,length);
}
bool ChDir(const char *path)
{
	return ( chdir (path) == 0 ? true : false );
}
void dir (const string &sFilter, vector<string> &sAllFiles, bool bFullPath)
{
	char sCurDir[MAX_PATH];
	DIR* dp = NULL;
	struct dirent *dirp= NULL;

	GetCWD ( MAX_PATH,sCurDir ) ;
	sAllFiles.clear ();
	if ( (dp = opendir( sCurDir )) == NULL)
	{
		string sTmp = string("ERROR :  Can't open the dir : \"")+string(sCurDir)+string("\"") ;
		outString ( sTmp ) ;
		return ;
	}

	while ( (dirp = readdir(dp)) != NULL)
	{
		std:string sFileName = std::string(dirp->d_name) ;
		if (sFileName.substr((sFileName.length()-sFilter.length()),sFilter.length()).find(sFilter)!= std::string::npos )
		{
			if (bFullPath)
				sAllFiles.push_back(string(sCurDir) + "/" + sFileName);
			else
				sAllFiles.push_back(sFileName);
		}

	}
	closedir(dp);
}
bool DeleteFile(const char* filename){
    if ( int res = unlink (filename) == -1 )
    	return false;
    return true;
}
#endif


// ---------------------------------------------------------------------------
char sExeDir[MAX_PATH];

void outString (const string &sText)
{
	char sCurDir[MAX_PATH];
	GetCWD (MAX_PATH, sCurDir);
	ChDir (sExeDir);
	NLMISC::createDebug ();
	NLMISC::InfoLog->displayRaw(sText.c_str());
	ChDir (sCurDir);
}

// ---------------------------------------------------------------------------
bool fileExist (const std::string &sFileName)
{
#ifdef NL_OS_WINDOWS
	return (GetFileAttributes(sFileName.c_str()) != INVALID_FILE_ATTRIBUTES);
#else // NL_OS_WINDOWS
	struct stat buf;
	return stat (sFileName.c_str (), &buf) == 0;
#endif // NL_OS_WINDOWS
}

// -----------------------------------------------------------------------------------------------
// Try all position to put pSrc in pDst
bool tryAllPos (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
{
	uint32 i, j;
	NLMISC::CObjectVector<uint8> &rSrcPix = pSrc->getPixels();
	NLMISC::CObjectVector<uint8> &rDstPix = pDst->getPixels();

	// Recalculate real size of the source (without padding to power of 2)
	uint32 nSrcWidth = 0, nSrcHeight = 0;
	for (j = 0; j < pSrc->getHeight(); ++j)
	for (i = 0; i < pSrc->getWidth(); ++i)
	{
		if (rSrcPix[4*(i+j*pSrc->getWidth())+3] != 0)
		{
			if ((i+1) > nSrcWidth)
				nSrcWidth = i+1;
			if ((j+1) > nSrcHeight)
				nSrcHeight= j+1;
		}
	}

	if (nSrcWidth > pDst->getWidth() ) return false;
	if (nSrcHeight > pDst->getHeight() ) return false;

	// For all position test if the Src plane can be put in
	for (j = 0; j < (pDst->getHeight() - nSrcHeight); ++j)
	for (i = 0; i < (pDst->getWidth() - nSrcWidth); ++i)
	{
		x = i; y = j;
		
		uint32 a, b;
		bool bCanPut = true;
		for (b = 0; b < nSrcHeight; ++b)
		{
			for (a = 0; a < nSrcWidth; ++a)
			{
				if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
				{
					if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0 )
					{
						bCanPut = false;
						break;
					}
				}
			}
			if (bCanPut == false)
				break;
		}
		if (bCanPut)
			return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool putIn (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y)
{
	uint32 a, b;

	NLMISC::CObjectVector<uint8> &rSrcPix = pSrc->getPixels();
	NLMISC::CObjectVector<uint8> &rDstPix = pDst->getPixels();

	for (b = 0; b < pSrc->getHeight(); ++b)
	for (a = 0; a < pSrc->getWidth(); ++a)
		if (rSrcPix[4*(a+b*pSrc->getWidth())+3] != 0)
		{
			if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
				return false;
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+0] = rSrcPix[4*(a+b*pSrc->getWidth())+0];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+1] = rSrcPix[4*(a+b*pSrc->getWidth())+1];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+2] = rSrcPix[4*(a+b*pSrc->getWidth())+2];
			rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] = rSrcPix[4*(a+b*pSrc->getWidth())+3];
		}
	return true;
}

// ---------------------------------------------------------------------------
string getBaseName (const string &fullname)
{
	string sTmp2;
	string::size_type pos = fullname.rfind('_');
	if (pos != string::npos)
		sTmp2 = fullname.substr(0, pos+1);
	return sTmp2;
}

// ---------------------------------------------------------------------------
uint8 getLayerNb (const string &fullname)
{
	uint8 nRet = 0;
	string::size_type beg = fullname.rfind('_');
	string::size_type end = fullname.rfind('.');
	if (beg != string::npos)
	{
		string sTmp2 = fullname.substr(beg+1, end-beg-1);
		NLMISC::fromString(sTmp2, nRet);
	}

	return nRet;
}

// Flag all vertices linked to face with material m
// ---------------------------------------------------------------------------
void FlagVertices (CMeshGeom &mg, uint InMatID, vector<bool> &verticesNeedRemap)
{
	CVertexBuffer &vertexBuffer = const_cast<CVertexBuffer&>(mg.getVertexBuffer());

	// For each matrix block
	uint matrixBlock;
	uint nbMatrixBlock=mg.getNbMatrixBlock();
	for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
	{
		// For each render pass
		uint renderPass;
		uint numRenderPass=mg.getNbRdrPass(matrixBlock);
		for (renderPass=0; renderPass<numRenderPass; renderPass++)
		{
			// Render pass material
			uint32 matId=mg.getRdrPassMaterial(matrixBlock, renderPass);

			if (matId == InMatID) // Same Material -> Flag all vertices of this pass
			{
				// Get primitives
				const CIndexBuffer &primitiveBlock=mg.getRdrPassPrimitiveBlock(matrixBlock,renderPass);
				CIndexBufferRead iba;
				primitiveBlock.lock (iba);

				// Set of vertex to remap
				std::set<uint> vertexToRemap;

				// Remap triangles
				uint index;
				if (iba.getFormat() == CIndexBuffer::Indices32)
				{
					const uint32 *indexPtr=(const uint32 *)iba.getPtr();
					uint32 numIndex=primitiveBlock.getNumIndexes();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);
				}
				else
				{
					const uint16 *indexPtr=(const uint16 *)iba.getPtr();
					uint32 numIndex=primitiveBlock.getNumIndexes();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);
				}

				// Remap the vertex
				std::set<uint>::iterator iteRemap=vertexToRemap.begin();
				while (iteRemap!=vertexToRemap.end())
				{
					// Remap the vertex
					verticesNeedRemap[*iteRemap] = true;

					// Next vertex
					iteRemap++;
				}
			}
		}
	}
}
void FlagVerticesMRM (CMeshMRMGeom &mg, uint InMatID, vector<bool> &verticesNeedRemap)
{
	CVertexBuffer &vertexBuffer = const_cast<CVertexBuffer&>(mg.getVertexBuffer());

	// For each matrix block
	uint matrixBlock;
	uint nbMatrixBlock=1;//mg.getNbMatrixBlock(); // ASK YOYO
	for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
	{
		// For each render pass
		uint renderPass;
		uint numRenderPass=mg.getNbRdrPass(matrixBlock);
		for (renderPass=0; renderPass<numRenderPass; renderPass++)
		{
			// Render pass material
			uint32 matId=mg.getRdrPassMaterial(matrixBlock, renderPass);

			if (matId == InMatID) // Same Material -> Flag all vertices of this pass
			{
				// Get primitives
				const CIndexBuffer &primitiveBlock=mg.getRdrPassPrimitiveBlock(matrixBlock,renderPass);
				CIndexBufferRead iba;
				primitiveBlock.lock (iba);

				// Set of vertex to remap
				std::set<uint> vertexToRemap;

				// Remap triangles
				uint index;
				if (iba.getFormat() == CIndexBuffer::Indices32)
				{
					const uint32 *indexPtr=(const uint32 *)iba.getPtr();
					uint32 numIndex=primitiveBlock.getNumIndexes();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);
				}
				else
				{
					const uint16 *indexPtr=(const uint16 *)iba.getPtr();
					uint32 numIndex=primitiveBlock.getNumIndexes();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);
				}

				// Remap the vertex
				std::set<uint>::iterator iteRemap=vertexToRemap.begin();
				while (iteRemap!=vertexToRemap.end())
				{
					// Remap the vertex
					verticesNeedRemap[*iteRemap] = true;

					// Next vertex
					iteRemap++;
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int nNbArg, char **ppArgs)
{
	
	if (nNbArg <3 || nNbArg >5)
	{
		outString ("ERROR : Wrong number of arguments\n");
		outString ("USAGE : lightmap_optimizer <path_lightmaps> <path_shapes> [path_tags] [path_flag8bit]\n");
		return -1;
	}
	
	vector<string> AllShapeNames;
	vector<CMeshBase*> AllShapes;
	std::vector<std::string> tags;	
	char sLMPDir[MAX_PATH];
	char sSHPDir[MAX_PATH];

	
	GetCWD (MAX_PATH, sExeDir);

	
	// Get absolute directory for lightmaps
	if (!ChDir(ppArgs[1]))
	{
		outString (string("ERROR : directory ") + ppArgs[1] + " do not exists or access is denied\n");
		return -1;
	}
	GetCWD (MAX_PATH, sLMPDir);
	ChDir (sExeDir);
	// Get absolute directory for shapes
	if (!ChDir(ppArgs[2]))
	{
		outString (string("ERROR : directory ") + ppArgs[2] + " do not exists or access is denied\n");
		return -1;
	}
	GetCWD (MAX_PATH, sSHPDir);
	dir (".shape", AllShapeNames, false);
	registerSerial3d ();
	for (uint32 nShp = 0; nShp < AllShapeNames.size(); ++nShp)
	{
		try
		{
			CShapeStream mesh;
			NLMISC::CIFile meshfile (AllShapeNames[nShp]);
			meshfile.serial( mesh );
			meshfile.close();

			// Add the shape to the map.
			CMeshBase *pMB = dynamic_cast<CMeshBase*>(mesh.getShapePointer());
			AllShapes.push_back (pMB);
		}
		catch (const NLMISC::EPathNotFound &e)
		{
			outString(string("ERROR: shape not found ")+AllShapeNames[nShp]+" - "+e.what());
			return -1;
		}
	}

	if (nNbArg > 3 && ppArgs[3] && strlen(ppArgs[3]) > 0)
	{
		ChDir (sExeDir);
		if (!ChDir(ppArgs[3]))
		{
			outString (string("ERROR : directory ") + ppArgs[3] + " do not exists\n");
			return -1;
		}
		dir (".tag", tags, false);
		for(uint k = 0; k < tags.size(); ++k)
		{
			std::string::size_type pos = tags[k].find('.');
			if (pos != std::string::npos)
			{
				tags[k] = tags[k].substr(0, pos);
			}
		}
	}


	// **** Parse all mesh loaded, to flag each lightmap if 8 bit or not (NB: all layers should be same mode)
	std::set<string>	setLM8Bit;
	for(uint i=0;i<AllShapes.size();i++)
	{
		CMeshBase *pMB= AllShapes[i];
		if(!pMB)
			continue;

		uint32		nbMat= pMB->getNbMaterial();
		for (uint32 m = 0; m < nbMat; ++m)
		{
			CMaterial& rMat = const_cast<CMaterial&>(pMB->getMaterial (m));
			if (rMat.getShader() == CMaterial::LightMap)
			{
				// Begin with stage 0
				uint8 stage = 0;
				while (rMat.getLightMap(stage) != NULL)
				{
					ITexture *pIT = rMat.getLightMap (stage);
					CTextureFile *pTF = dynamic_cast<CTextureFile*>(pIT);
					if (pTF != NULL)
					{
						string sTexName = NLMISC::toLower(pTF->getFileName());
						if(pTF->getUploadFormat()==ITexture::Luminance)
							setLM8Bit.insert(sTexName);
					}
					++stage;
				}
			}
		}
	}


	// **** Parse all lightmaps, sorted by layer, and 8 or 16 bit mode
	ChDir (sExeDir);
	for (uint32 lmc8bitMode = 0; lmc8bitMode < 2; ++lmc8bitMode)
	for (uint32 nNbLayer = 0; nNbLayer < 256; ++nNbLayer)
	{
		// Get all lightmaps with same number of layer == nNbLayer
		// merge lightmaps only if they are in same mode (8bits or 16 bits)

		vector<string> AllLightmapNames;
		vector<sint>   AllLightmapTags;
		vector<NLMISC::CBitmap*> AllLightmaps;
		sint32 i, j, k, m, n;
		string sFilter;

		// **** Get All Lightmaps that have this number of layer, and this mode
		sFilter = "_" + NLMISC::toString(nNbLayer) + ".tga";
		ChDir (sLMPDir);
		dir (sFilter, AllLightmapNames, false);

		// filter by layer
		vector<string>		tmpLMs;
		tmpLMs.reserve(AllLightmapNames.size());
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			string sTmp2 = getBaseName (AllLightmapNames[i]);
			sTmp2 += NLMISC::toString(nNbLayer+1) + ".tga";
			// if not More layer than expected, ok
			if (!fileExist(sTmp2))
			{	
				tmpLMs.push_back(AllLightmapNames[i]);
			}
		}
		AllLightmapNames= tmpLMs;
	
		// filter by 8bit or not mode.
		tmpLMs.clear();
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			bool	lm8Bit= setLM8Bit.find( NLMISC::toLower(AllLightmapNames[i]) ) !=setLM8Bit.end();
			// if same mode
			if( lm8Bit == (lmc8bitMode==1) )
			{
				tmpLMs.push_back(AllLightmapNames[i]);
			}
		}
		AllLightmapNames= tmpLMs;

		
		// **** Build tag info
		/*
		for(uint k = 0; k < tags.size(); ++k)
		{
			nlinfo("tag %d = %s", (int) k, tags[k].c_str());
		}
		*/
		AllLightmapTags.resize(AllLightmapNames.size());
		for(uint k = 0; k < AllLightmapNames.size(); ++k)
		{
			nlinfo("k = %d", (int) k);
			AllLightmapTags[k] = -1;
			// search for longest tag that match
			uint bestLength = 0;
			for(uint l = 0; l < tags.size(); ++l)
			{
				if (AllLightmapNames[k].size() > tags[l].size())
				{
					if (tags[l].size() > bestLength)
					{					
						std::string start = AllLightmapNames[k].substr(0, tags[l].size());
						if (NLMISC::nlstricmp(start, tags[l]) == 0)
						{
							bestLength = (uint)tags[l].size();
							// the tag matchs
							AllLightmapTags[k] = l;						
						}
					}
				}
			}						
			if (AllLightmapTags[k] == -1)
			{
				nlinfo(NLMISC::toString("Lightmap %s has no tag", AllLightmapNames[k].c_str()).c_str());
			}
			else
			{			
				nlinfo(NLMISC::toString("Lightmap %s has tag %d : %s", AllLightmapNames[k].c_str(), (int) AllLightmapTags[k], tags[AllLightmapTags[k]].c_str()).c_str());
			}			
		}




		// Check if all layer of the same lightmap has the same size
		if (nNbLayer > 0)
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			string sTmp2;
			sTmp2 = getBaseName (AllLightmapNames[i]) + "0.tga";
			uint32 wRef, hRef;
			try
			{
				NLMISC::CIFile inFile;
				inFile.open(sTmp2);
				CBitmap::loadSize(inFile, wRef, hRef);
			}
			catch (const NLMISC::Exception &e)
			{
				outString (string("ERROR :") + e.what());
				return -1;
			}

			bool bFound = false;
			for (k = 1; k <= (sint32)nNbLayer; ++k)
			{
				string sTmp3 = getBaseName (AllLightmapNames[i]) + NLMISC::toString(k) + ".tga";
				uint32 wCur = wRef, hCur = hRef;
				try
				{
					NLMISC::CIFile inFile;
					inFile.open(sTmp3);
					CBitmap::loadSize(inFile, wCur, hCur);
				}
				catch (const NLMISC::Exception &)
				{
				}

				if ((wCur != wRef) || (hCur != hRef))
				{
					bFound = true;
					break;
				}
			}
			// Should delete all layers of this lightmap (in fact in lightmapnames list we have
			// only the name of the current layer)
			if (bFound)
			{
				sTmp2 = getBaseName (AllLightmapNames[i]);
				outString(string("ERROR: lightmaps ")+sTmp2+"*.tga not all the same size\n");
				for (k = 0; k < (sint32)AllLightmapNames.size(); ++k)
				{
					if (NLMISC::strnicmp(AllLightmapNames[k].c_str(), sTmp2.c_str(), sTmp2.size()) == 0)
					{
						for (j = k+1; j < (sint32)AllLightmapNames.size(); ++j)
						{
							AllLightmapNames[j-1] = AllLightmapNames[j];
							AllLightmapTags[j - 1] = AllLightmapTags[j];
						}
						AllLightmapNames.resize (AllLightmapNames.size()-1);
						AllLightmapTags.resize(AllLightmapTags.size()  - 1);
						k = -1;
						i = -1;
					}
				}
			}
		}
		
		if (AllLightmapNames.size() == 0)
			continue;
		
		// Load all the lightmaps
		AllLightmaps.resize (AllLightmapNames.size());
		for (i = 0; i < (sint32)AllLightmaps.size(); ++i)
		{
			try
			{
				NLMISC::CBitmap *pBtmp = new NLMISC::CBitmap;
				NLMISC::CIFile inFile;
				inFile.open(AllLightmapNames[i]);
				pBtmp->load(inFile);
				AllLightmaps[i] = pBtmp;
			}
			catch (const NLMISC::Exception &e)
			{
				outString (string("ERROR :") + e.what());
				return -1;
			}
		}

		// Sort all lightmaps by decreasing size
		for (i = 0; i < (sint32)(AllLightmaps.size()-1); ++i)
		for (j = i+1; j < (sint32)AllLightmaps.size(); ++j)
		{
			NLMISC::CBitmap *pBI = AllLightmaps[i];
			NLMISC::CBitmap *pBJ = AllLightmaps[j];
			if ((pBI->getWidth()*pBI->getHeight()) < (pBJ->getWidth()*pBJ->getHeight()))
			{
				NLMISC::CBitmap *pBTmp = AllLightmaps[i];
				AllLightmaps[i] = AllLightmaps[j];
				AllLightmaps[j] = pBTmp;

				string sTmp = AllLightmapNames[i];
				AllLightmapNames[i] = AllLightmapNames[j];
				AllLightmapNames[j] = sTmp;

				sint tagTmp = AllLightmapTags[i];
				AllLightmapTags[i] = AllLightmapTags[j];
				AllLightmapTags[j] = tagTmp;
			}
		}
		nlassert(AllLightmapTags.size() == AllLightmapNames.size());
		for (i = 0; i < (sint32)AllLightmapNames.size(); ++i)
		{
			outString(NLMISC::toString("%d / %d\n", (int) i, (int) AllLightmapNames.size()));
			bool bAssigned = false;
			for (j = 0; j < i; ++j)
			{				
				// Tags of both textures must match. We don't want to spread lightmap chunk in bitmap whose other part aren't used by current ig lightmaps (this wastes vram for nothing)
				if (AllLightmapTags[i] != AllLightmapTags[j]) continue;

				// Try to place the texture i into the texture j
				// This can be done only if texture was exported from the same zone. To ensure that, check 
				NLMISC::CBitmap *pBI = AllLightmaps[i];
				NLMISC::CBitmap *pBJ = AllLightmaps[j];
				sint32 x, y;
				if (tryAllPos (pBI, pBJ, x, y))
				{
					bAssigned = true;

					if (!putIn (pBI, pBJ, x, y))
					{
						outString (string("ERROR : cannot put reference lightmap ")+AllLightmapNames[i]+
									" in "+AllLightmapNames[j]);
						return -1;
					}
					// Put texture i into texture j for all layers of the lightmap !
					for (k = 0; k <= (sint32)nNbLayer; ++k)
					{
						string sTexNameI = getBaseName (AllLightmapNames[i]) + NLMISC::toString(k) + ".tga";
						string sTexNameJ = getBaseName (AllLightmapNames[j]) + NLMISC::toString(k) + ".tga";
						NLMISC::CBitmap BitmapI;
						NLMISC::CBitmap BitmapJ;
						NLMISC::CIFile inFile;

						outString (NLMISC::toString("INFO : Transfering %s (tag = %d) in %s (tag = %d)", 
													sTexNameI.c_str(), (int) AllLightmapTags[i],
													sTexNameJ.c_str(), (int) AllLightmapTags[j]) +
													" at ("+NLMISC::toString(x)+","+NLMISC::toString(y)+")\n");

						try
						{
							inFile.open (sTexNameI);
							BitmapI.load (inFile);
							inFile.close ();
							inFile.open (sTexNameJ);
							BitmapJ.load (inFile);
							inFile.close ();
						}
						catch (const NLMISC::Exception &e)
						{
							outString (string("ERROR :") + e.what());
							return -1;
						}
						
						if (!putIn (&BitmapI, &BitmapJ, x, y))
						{
							outString (string("ERROR : cannot put lightmap ")+sTexNameI+" in "+sTexNameJ+"\n");
							return -1;
						}

						// Delete File
						DeleteFile (sTexNameI.c_str());
						outString (string("INFO : Deleting file ")+sTexNameI+"\n");

						// Save destination image
						NLMISC::COFile outFile;
						outFile.open (sTexNameJ);
						BitmapJ.writeTGA (outFile, 32);
						outString (string("INFO : Saving file ")+sTexNameJ+"\n");
					}

					// Change shapes uvs related and names to the lightmap
					// ---------------------------------------------------

					ChDir (sSHPDir);

					for (k = 0; k < (sint32)AllShapes.size(); ++k)
					{
						CMeshBase *pMB = AllShapes[k];
						if (!pMB)
							continue;

						uint nNbMat = pMB->getNbMaterial ();
						vector< vector<bool> > VerticesNeedRemap;
						bool bMustSave = false;
						// Initialize all VerticesNeedRemap
						CMesh *pMesh = dynamic_cast<CMesh*>(pMB);
						CMeshMRM *pMeshMRM = dynamic_cast<CMeshMRM*>(pMB);
						CMeshMultiLod *pMeshML = dynamic_cast<CMeshMultiLod*>(pMB);

						if (pMesh != NULL)
						{
							VerticesNeedRemap.resize(1); // Only one meshgeom
							vector<bool> &rVNR = VerticesNeedRemap[0];
							rVNR.resize (pMesh->getMeshGeom().getVertexBuffer().getNumVertices(), false);
						}
						else if (pMeshMRM != NULL)
						{
							VerticesNeedRemap.resize(1); // Only one meshmrmgeom
							vector<bool> &rVNR = VerticesNeedRemap[0];
							rVNR.resize (pMeshMRM->getMeshGeom().getVertexBuffer().getNumVertices(), false);
						}
						else if (pMeshML != NULL)
						{
							sint32 nNumSlot = pMeshML->getNumSlotMesh();
							VerticesNeedRemap.resize(nNumSlot);
							for (m = 0; m < nNumSlot; ++m)
							{
								vector<bool> &rVNR = VerticesNeedRemap[m];
								const CMeshGeom *pMG = dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(m));
								if (pMG != NULL)
									rVNR.resize (pMG->getVertexBuffer().getNumVertices(), false);
								else
									rVNR.resize(0);
							}
						}
						else continue; // Next mesh
						

						// All materials must have the lightmap names changed
						for (m = 0; m < (sint32)nNbMat; ++m)
						{
							bool bMustRemapUV = false;
							CMaterial& rMat = const_cast<CMaterial&>(pMB->getMaterial (m));
							if (rMat.getShader() == CMaterial::LightMap)
							{
								// Begin with stage 0
								uint8 stage = 0;
								while (rMat.getLightMap(stage) != NULL)
								{
									ITexture *pIT = rMat.getLightMap (stage);
									CTextureFile *pTF = dynamic_cast<CTextureFile*>(pIT);
									if (pTF != NULL)
									{
										string sTexName = NLMISC::toLower(getBaseName(pTF->getFileName()));
										string sTexNameMoved = NLMISC::toLower(getBaseName(AllLightmapNames[i]));
										if (sTexName == sTexNameMoved)
										{
											// We must remap the name and indicate to remap uvs
											bMustRemapUV = true;
											//string sNewTexName = NLMISC::toLower(getBaseName(AllLightmapNames[j]));
											//sNewTexName += NLMISC::toString(getLayerNb(pTF->getFileName())) + ".tga";
											//pTF->setFileName (sNewTexName);
										}
									}
									++stage;
								}
							}
							// We have to remap the uvs of this mesh for this material
							if (bMustRemapUV) // Flaggage of the vertices to remap
							{
								if (pMesh != NULL)
								{
									// Flag all vertices linked to face with material m
									FlagVertices (const_cast<CMeshGeom&>(pMesh->getMeshGeom()), m, VerticesNeedRemap[0]);
								}
								else if (pMeshMRM != NULL)
								{
									FlagVerticesMRM (const_cast<CMeshMRMGeom&>(pMeshMRM->getMeshGeom()), m, VerticesNeedRemap[0]);
								}
								else if (pMeshML != NULL)
								{
									sint32 nNumSlot = pMeshML->getNumSlotMesh();
									for (n = 0; n < nNumSlot; ++n)
									{
										// Get the mesh geom
										CMeshGeom *pMG = const_cast<CMeshGeom*>(dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(n)));
										if (pMG)
										{
											// Flag the vertices
											FlagVertices (*pMG, m, VerticesNeedRemap[n]);
										}
										else
										{
											// Get the mesh MRM geom
											CMeshMRMGeom *pMMRMG = const_cast<CMeshMRMGeom*>(dynamic_cast<const CMeshMRMGeom*>(&pMeshML->getMeshGeom(n)));
											if (pMMRMG)
											{
												// Flag the vertices
												FlagVerticesMRM (*pMMRMG, m, VerticesNeedRemap[n]);
											}
										}
									}
								}
							}
						}

						// Change lightmap names
						for (m = 0; m < (sint32)nNbMat; ++m)
						{
							CMaterial& rMat = const_cast<CMaterial&>(pMB->getMaterial (m));
							if (rMat.getShader() == CMaterial::LightMap)
							{
								// Begin with stage 0
								uint8 stage = 0;
								while (rMat.getLightMap(stage) != NULL)
								{
									ITexture *pIT = rMat.getLightMap (stage);
									CTextureFile *pTF = dynamic_cast<CTextureFile*>(pIT);
									if (pTF != NULL)
									{
										string sTexName = NLMISC::toLower(getBaseName(pTF->getFileName()));
										string sTexNameMoved = NLMISC::toLower(getBaseName(AllLightmapNames[i]));
										if (sTexName == sTexNameMoved)
										{
											string sNewTexName = NLMISC::toLower(getBaseName(AllLightmapNames[j]));
											sNewTexName += NLMISC::toString(getLayerNb(pTF->getFileName())) + ".tga";
											pTF->setFileName (sNewTexName);
										}
									}
									++stage;
								}
							}
						}

						// We have now the list of vertices to remap for all material that have been changed
						// So parse this list and apply the transformation : (uv * TexSizeI + decalXY) / TexSizeJ
						for (m = 0; m < (sint32)VerticesNeedRemap.size(); ++m)
						{
							CVertexBuffer *pVB;							
							if (pMesh != NULL)
							{
								pVB = const_cast<CVertexBuffer*>(&pMesh->getMeshGeom().getVertexBuffer());
							}
							else if (pMeshMRM != NULL)
							{
								pVB = const_cast<CVertexBuffer*>(&pMeshMRM->getMeshGeom().getVertexBuffer());
							}
							else if (pMeshML != NULL)
							{
								const CMeshGeom *pMG = dynamic_cast<const CMeshGeom*>(&pMeshML->getMeshGeom(m));
								pVB = const_cast<CVertexBuffer*>(&pMG->getVertexBuffer());
							}
							CVertexBufferReadWrite vba; 
							pVB->lock (vba);

							vector<bool> &rVNR = VerticesNeedRemap[m];
							for (n = 0; n < (sint32)rVNR.size(); ++n)
							if (rVNR[n])
							{
								CUV *pUV = (CUV*)vba.getTexCoordPointer (n,1);
								pUV->U = (pUV->U*pBI->getWidth() + x) / pBJ->getWidth();
								pUV->V = (pUV->V*pBI->getHeight() + y) / pBJ->getHeight();
								bMustSave = true;
							}
						}

						if (bMustSave)
						{
							try
							{
								if (AllShapes[k])
								{
									CShapeStream mesh;
									mesh.setShapePointer (AllShapes[k]);
									NLMISC::COFile meshfile (AllShapeNames[k]);
									meshfile.serial (mesh);
									meshfile.close ();
								}
							}
							catch (const NLMISC::EPathNotFound &e)
							{
								outString(string("ERROR: cannot save shape ")+AllShapeNames[k]+" - "+e.what());
								return -1;
							}
						}
					}

					ChDir (sLMPDir);

					// Get out of the j loop
					break;
				}
			}		
			// if assigned to another bitmap -> delete the bitmap i
			if (bAssigned)
			{
				// Delete Names && tags
				for (j = i+1; j < (sint32)AllLightmapNames.size(); ++j)
				{
					AllLightmapNames[j-1] = AllLightmapNames[j];
					AllLightmapTags[j-1] = AllLightmapTags[j];
				}
				AllLightmapNames.resize (AllLightmapNames.size()-1);
				AllLightmapTags.resize (AllLightmapTags.size()-1);
				// Delete Lightmaps
				delete AllLightmaps[i];
				for (j = i+1; j < (sint32)AllLightmaps.size(); ++j)
					AllLightmaps[j-1] = AllLightmaps[j];
				AllLightmaps.resize (AllLightmaps.size()-1);
				i = i - 1;
			}
		}

	}
	

	// **** Additionally, output or clear a "flag file" in a dir to info if a 8bit lihgtmap or not
	if (nNbArg >=5 && ppArgs[4] && strlen(ppArgs[4]) > 0)
	{
		ChDir (sExeDir);

		// out a text file, with list of
		FILE	*out= fopen(ppArgs[4], "wt");
		if(!out)
		{
			outString(string("ERROR: cannot save ")+ppArgs[4]);
		}

		set<string>::iterator	it(setLM8Bit.begin()), end(setLM8Bit.end());
		for(;it!=end;it++)
		{
			string	temp= (*it);
			temp+= "\n";
			fputs(temp.c_str(), out);
		}

		fclose(out);
	}

	return 0;
}
