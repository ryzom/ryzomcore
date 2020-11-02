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

#include "nel/3d/coarse_mesh_build.h"

#include "nel/3d/mesh.h"
#include "nel/3d/debug_vb.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

bool CCoarseMeshBuild::build (const std::vector<CCoarseMeshDesc>& coarseMeshes, std::vector<NLMISC::CBitmap> &bitmaps, CStats& stats, float mulArea)
{
	// 1. build the bitmap
	MapBitmapDesc desc;
	if (buildBitmap (coarseMeshes, bitmaps, stats, desc, mulArea)==false)
		return false;

	// 2. remap coordinates
	remapCoordinates (coarseMeshes, desc, (uint)bitmaps.size ());

	// 3. ok
	return true;
}

// ***************************************************************************

// Class descriptor for bitmap inserted
class CInsertedBitmap
{
public:
	// Width and height
	uint Width;
	uint Height;

	// Coordinates
	uint U;
	uint V;
};

// ***************************************************************************

bool CCoarseMeshBuild::buildBitmap (const std::vector<CCoarseMeshDesc>& coarseMeshes, std::vector<NLMISC::CBitmap> &bitmaps, CStats& stats, MapBitmapDesc& desc, float mulArea)
{
	// Total area used by texture
	uint totalArea=0;

	// ***************************************************************************

	// 1. scan each bitmap: calc the area of the bitmap and it its name in the maps sorted by area
	typedef std::multimap<uint, CBitmapDesc*> MapAreaBitmap;
	MapAreaBitmap mapArea;
	uint mesh;
	for (mesh=0; mesh<coarseMeshes.size(); mesh++)
	{
		// Geom mesh pointer
		CMeshGeom *meshGeom=coarseMeshes[mesh].MeshGeom;

		// Base mesh pointer
		const CMeshBase *meshBase=coarseMeshes[mesh].MeshBase;

		// For each matrix block
		uint matrixBlock;
		uint nbMatrixBlock=meshGeom->getNbMatrixBlock();
		for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
		{
			// For each render pass
			uint renderPass;
			uint numRenderPass=meshGeom->getNbRdrPass(matrixBlock);
			for (renderPass=0; renderPass<numRenderPass; renderPass++)
			{
				// Render pass material
				uint32 matId=meshGeom->getRdrPassMaterial(matrixBlock, renderPass);

				// Checks
				nlassert (matId<meshBase->getNbMaterial());

				// Get the material
				const CMaterial &material=meshBase->getMaterial(matId);

				// Get the texture
				ITexture *texture=material.getTexture(0);
				if (texture)
				{
					// For each bitmaps
					uint i;
					std::string name;
					for (i=0; i<bitmaps.size (); i++)
					{
						// Select the good slot
						texture->selectTexture (i);

						// Get its name
						if (texture->supportSharing())
						{
							// Get sharing name
							name+=toLowerAscii(texture->getShareName());
						}
						else
						{
							// Build a name
							name+=toString ("%p", texture);
						}
					}

					// Already added ?
					if (desc.find (name)==desc.end())
					{
						// Add it..

						// Insert it in the maps
						MapBitmapDesc::iterator ite = desc.insert (MapBitmapDesc::value_type (name, CBitmapDesc ())).first;

						// Descriptor for this texture
						CBitmapDesc &descBitmap = ite->second;

						// Backup original size
						uint originalWidth = 0;
						uint originalHeight = 0;

						// For each bitmaps
						uint i;
						descBitmap.Bitmaps.resize (bitmaps.size ());
						for (i=0; i<bitmaps.size (); i++)
						{
							// Select the good slot
							texture->selectTexture (i);

							// Generate the texture
							texture->generate();

							// Convert to RGBA
							texture->convertToType (CBitmap::RGBA);

							// First texture ?
							if (i == 0)
							{
								// Backup original size
								originalWidth = texture->getWidth();
								originalHeight = texture->getHeight();
							}

							// Resample, if needed
							if (i != 0)
							{
								// New size
								if ( ( originalWidth != texture->getWidth () ) || originalHeight != texture->getHeight  () )
								{
									texture->resample (originalWidth, originalHeight);
								}
							}

							// Copy the texture
							descBitmap.Bitmaps[i] = *texture;

							// Expand the texture
							expand (descBitmap.Bitmaps[i]);
						}

						// Texture area
						uint area = descBitmap.Bitmaps[0].getWidth() * descBitmap.Bitmaps[0].getHeight();
						descBitmap.Name = name;
						descBitmap.FactorU = (float)originalWidth;
						descBitmap.FactorV = (float)originalHeight;

						// Insert in the map area
						mapArea.insert (MapAreaBitmap::value_type(area, &(ite->second)));

						// Sum area if added
						totalArea+=area;
					}
				}
			}
		}
	}

	// ***************************************************************************

	// 2. Calc the best area for the dest texture and resize the bitmap

	// Total area used by the textures + a little more
	uint newArea=getPowerOf2 (raiseToNextPowerOf2 (totalArea));
	while ((1<<newArea)<(sint)(mulArea*(float)totalArea))
	{
		newArea++;
	}

	// Calc width and height with HEIGHT==WIDTH or HEIGHT=2*WIDTH
	uint width=1<<(newArea/2);
	uint height=1<<(newArea/2 + (newArea&1));

	// Resize the bitmap and set the pixel format
	uint i;
	for (i=0; i<bitmaps.size (); i++)
		bitmaps[i].resize (width, height, CBitmap::RGBA);

	// Checks
	if (totalArea==0)
	{
		// No texture, ok computed.
		stats.TextureUsed=1;

		return true;
	}

	// ***************************************************************************

	// 3. Place each texture in the bitmap in uncreasing order
	typedef std::multimap<sint, CInsertedBitmap> mapInsertedBitmap;

	// For each texture
	MapAreaBitmap::iterator ite=mapArea.end();

	// Inserted bitmap desc
	mapInsertedBitmap inserted;

	// Max texture height
	uint maxTexHeight=0;

	do
	{
		ite--;
		nlassert (ite!=mapArea.end());

		// Size of the texture
		uint widthTex=ite->second->Bitmaps[0].getWidth();
		uint heightTex=ite->second->Bitmaps[0].getHeight();

		// Some checks
		nlassert (bitmaps.size () == ite->second->Bitmaps.size ());

		// Width and height max
		uint widthMax=width-widthTex;
		uint heightMax=height-heightTex;

		// Test against others..
		bool enter=false;

		// For each row and each column
		for (uint v=0; v<heightMax; v++)
		{
			for (uint u=0; u<widthMax; u++)
			{
				// Test against others..
				enter=true;

				// Get the first to test
				mapInsertedBitmap::iterator toTest=inserted.lower_bound ((sint)v-(sint)maxTexHeight);
				while (toTest!=inserted.end())
				{
					// Make a test ?
					if ((sint)(v+heightTex)<=(toTest->first))
					{
						// Ok, end test
						break;
					}

					// Test it
					uint otherU=toTest->second.U;
					uint otherV=toTest->second.V;
					uint otherWidth=toTest->second.Width;
					uint otherHeight=toTest->second.Height;
					if ((v<otherV+otherHeight) && (v+heightTex>otherV) &&
						(u<otherU+otherWidth) && (u+widthTex>otherU))
					{
						// Collision
						enter=false;
						u=toTest->second.U+otherWidth-1;
						break;
					}

					// Next to test
					toTest++;
				}

				// Enter ?
				if (enter)
				{
					// Ok, enter

					// Insert an inserted descriptor
					CInsertedBitmap descInserted;
					descInserted.Width=widthTex;
					descInserted.Height=heightTex;
					descInserted.U=u;
					descInserted.V=v;
					inserted.insert (mapInsertedBitmap::value_type (v, descInserted));

					// Max height
					if (heightTex>maxTexHeight)
						maxTexHeight=heightTex;

					// Blit in the texture
					uint i;
					for (i=0; i<bitmaps.size (); i++)
					{
						// Check..
						nlassert (	(ite->second->Bitmaps[0].getWidth () == ite->second->Bitmaps[i].getWidth ()) &&
									(ite->second->Bitmaps[0].getHeight () == ite->second->Bitmaps[i].getHeight ())	);

						// Blit it
						bitmaps[i].blit (&(ite->second->Bitmaps[i]), u, v);
					}

					// Set the U and V texture coordinates
					ite->second->U=(float)(u+1)/(float)width;
					ite->second->V=(float)(v+1)/(float)height;

					// Set ratio
					ite->second->FactorU /= (float)width;
					ite->second->FactorV /= (float)height;

					// End
					break;
				}

				// next..
			}

			// Enter ?
			if (enter)
				break;
		}

		// Not enter ?
		if (!enter)
			// Texture too small..
			return false;
	}
	while (ite!=mapArea.begin());

	// Some stats
	stats.TextureUsed=(float)totalArea/(float)(width*height);

	return true;
}

// ***************************************************************************

void CCoarseMeshBuild::expand (CBitmap& bitmap)
{
	// Get size
	uint width=bitmap.getWidth();
	uint height=bitmap.getHeight();

	// Valid size ?
	if ((width!=0) && (height!=0))
	{
		// Copy the bitmap
		CBitmap copy=bitmap;

		// Resize the bitmap
		bitmap.resize (width+2, height+2);

		// Copy old bitmap
		bitmap.blit (&copy, 1, 1);

		// Make a top and bottom border
		uint32 *topSrc=(uint32*)&(copy.getPixels()[0]);
		uint32 *topDest=((uint32*)&(bitmap.getPixels()[0]))+1;
		memcpy (topDest, topSrc, 4*width);
		uint32 *bottomSrc=topSrc+width*(height-1);
		uint32 *bottomDest=((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+1)+1;
		memcpy (bottomDest, bottomSrc, 4*width);

		// Make a left and right border
		uint32 *leftSrc=(uint32*)&(copy.getPixels()[0]);
		uint32 *leftDest=((uint32*)&(bitmap.getPixels()[0]))+width+2;
		uint32 *rightSrc=leftSrc+width-1;
		uint32 *rightDest=leftDest+width+1;
		uint i;
		for (i=0; i<height; i++)
		{
			// Copy the borders
			*leftDest=*leftSrc;
			*rightDest=*rightSrc;

			// Move pointers
			leftDest+=width+2;
			rightDest+=width+2;
			leftSrc+=width;
			rightSrc+=width;
		}

		// Make corners

		// Left top
		*(uint32*)&(bitmap.getPixels()[0])=*(uint32*)&(copy.getPixels()[0]);

		// Rigth top
		*(((uint32*)&(bitmap.getPixels()[0]))+width+1)=*(((uint32*)&(copy.getPixels()[0]))+width-1);

		// Rigth bottom
		*(((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+2)-1)=*(((uint32*)&(copy.getPixels()[0]))+width*height-1);

		// Left bottom
		*(((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+1))=*(((uint32*)&(copy.getPixels()[0]))+width*(height-1));
	}
}

// ***************************************************************************

void CCoarseMeshBuild::remapCoordinates (const std::vector<CCoarseMeshDesc>& coarseMeshes, const MapBitmapDesc& desc, uint outputBitmapCount)
{
	// 1. scan each bitmap: calc the area of the bitmap and it its name in the maps sorted by area
	typedef std::multimap<float, CBitmapDesc> MapAreaBitmap;
	MapAreaBitmap mapArea;
	uint mesh;
	for (mesh=0; mesh<coarseMeshes.size(); mesh++)
	{
		// Geom mesh pointer
		CMeshGeom *meshGeom=coarseMeshes[mesh].MeshGeom;

		// Base mesh pointer
		const CMeshBase *meshBase=coarseMeshes[mesh].MeshBase;

		// The vertex buffer
		CVertexBuffer &vertexBuffer=const_cast<CVertexBuffer&> (meshGeom->getVertexBuffer());
		CVertexBufferReadWrite vba;
		vertexBuffer.lock(vba);

		// For each matrix block
		uint matrixBlock;
		uint nbMatrixBlock=meshGeom->getNbMatrixBlock();
		for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
		{
			// For each render pass
			uint renderPass;
			uint numRenderPass=meshGeom->getNbRdrPass(matrixBlock);
			for (renderPass=0; renderPass<numRenderPass; renderPass++)
			{
				// Render pass material
				uint32 matId=meshGeom->getRdrPassMaterial(matrixBlock, renderPass);

				// Checks
				nlassert (matId<meshBase->getNbMaterial());

				// Get the material
				const CMaterial &material=meshBase->getMaterial(matId);

				// Get the texture
				ITexture *texture=material.getTexture(0);
				if (texture)
				{
					// Get its name
					std::string name;
					uint i;
					for (i=0; i<outputBitmapCount; i++)
					{
						// Select the good slot
						texture->selectTexture (i);

						// Get its name
						if (texture->supportSharing())
						{
							// Get sharing name
							name+=toLowerAscii(texture->getShareName());
						}
						else
						{
							// Build a name
							name+=toString ("%p", texture);
						}
					}

					// Find the texture
					MapBitmapDesc::const_iterator ite=desc.find (name);
					nlassert (ite!=desc.end());

					// Descriptor ref
					const CBitmapDesc& descBitmap=ite->second;

					// Get primitives
					const CIndexBuffer &primitiveBlock=meshGeom->getRdrPassPrimitiveBlock(matrixBlock,renderPass);

					// Set of vertex to remap
					std::set<uint> vertexToRemap;

					// Remap triangles
					uint index;
					CIndexBufferRead ibaRead;
					primitiveBlock.lock (ibaRead);
					if (ibaRead.getFormat() == CIndexBuffer::Indices32)
					{
						const uint32 *indexPtr=(uint32 *) ibaRead.getPtr();
						uint32 numIndex=primitiveBlock.getNumIndexes();
						for (index=0; index<numIndex; index++)
							vertexToRemap.insert (indexPtr[index]);
					}
					else
					{
						nlassert(ibaRead.getFormat() == CIndexBuffer::Indices16);
						const uint16 *indexPtr=(uint16 *) ibaRead.getPtr();
						uint32 numIndex=primitiveBlock.getNumIndexes();
						for (index=0; index<numIndex; index++)
							vertexToRemap.insert ((uint32) indexPtr[index]);
					}

					// Remap the vertex
					std::set<uint>::iterator iteRemap=vertexToRemap.begin();
					while (iteRemap!=vertexToRemap.end())
					{
						// Remap the vertex
						float *UVCoordinate=(float*)vba.getTexCoordPointer(*iteRemap);
						CHECK_VBA(vba, UVCoordinate);
						UVCoordinate[0]=UVCoordinate[0]*descBitmap.FactorU+descBitmap.U;
						UVCoordinate[1]=UVCoordinate[1]*descBitmap.FactorV+descBitmap.V;

						// Next vertex
						iteRemap++;
					}
				}
			}
		}
	}
}

// ***************************************************************************

} // NL3D
