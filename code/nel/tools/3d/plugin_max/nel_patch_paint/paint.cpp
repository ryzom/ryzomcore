#include "stdafx.h"
#include "nel_patch_paint.h"
#include "resource.h"

#include "nel/3d/scene.h"
#include "nel/3d/camera.h"
#include "nel/3d/nelu.h"
#include "nel/3d/light.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/landscape.h"
#include "nel/3d/event_mouse_listener.h"
#include "nel/3d/dru.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/zone_corner_smoother.h"
#include "nel/3d/zone_symmetrisation.h"

#include "nel/misc/vector.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/events.h"
#include "nel/misc/file.h"

#include "nel/ligo/ligo_config.h"

#include "paint_ui.h"
#include "paint_vcolor.h"
#include "paint_to_nel.h"
#include "paint_fill.h"
#include "paint_tileset.h"
#include "paint_light.h"
#include "../nel_mesh_lib/export_nel.h"

using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;

// Paint mouse proc

#define MAIN_Width 800
#define MAIN_Height 600

#define MOD_WIDTH 8
#define MOD_HEIGHT 6

#define TILE_SIZE_COUNT 2

#define DEPTH_SEARCH_MAX 8

#define REGKEY_EDIT_PATCH _T("Software\\Nevrax\\Ryzom\\edit_patch")

// Bank bitmaps
CBankCont*	bankCont;

// Tileset land
CTileSetSelection	tileSetSelector;

bool bWarningInvalidTileSet;

CVector maxToNel (const Point3& p)
{
	return CVector (p.x, p.y, p.z);
}

CRGBA maxToNel (COLORREF ref)
{
	return CRGBA ((uint8)(ref&0xff), (uint8)((ref>>8)&0xff), (uint8)((ref>>16)&0xff));
}

COLORREF nelToMax (CRGBA ref)
{
	return RGB (ref.R, ref.G, ref.B);
}

void WarningInvalidTileSet ()
{
	if (!bWarningInvalidTileSet)
	{
		bWarningInvalidTileSet=true;
		MessageBox (NULL, _T("The tile bank is not compatible with your zone.\nPlease use the good bank or erase and repaint the zone."), _T("Tile paint"), MB_OK|MB_ICONEXCLAMATION);
	}
}

int brushValue[BRUSH_COUNT]=
{
	0, 4, 8
};

// Alocate and load a bitmap
ITexture* allocateAndLoadBitmap (uint8* bitmap, uint size)
{
	return new CTextureMem (bitmap, size, false);
}

// Call when painter starts
void enterPainter (CTileBank& banktoLoad)
{
	// Load some tiles from the bank
	bankCont=new CBankCont (banktoLoad, hInstance);

	// Get background color
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_EDIT_PATCH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=4;
		DWORD type;
		RegQueryValueEx (hKey, _T("Background"), 0, &type, (LPBYTE)&backGround, &len);
		RegQueryValueEx (hKey, _T("Color1"), 0, &type, (LPBYTE)&color1, &len);
		RegQueryValueEx (hKey, _T("Color2"), 0, &type, (LPBYTE)&color2, &len);
		RegQueryValueEx (hKey, _T("Opa1"), 0, &type, (LPBYTE)&opa1, &len);
		RegQueryValueEx (hKey, _T("Opa2"), 0, &type, (LPBYTE)&opa2, &len);
		RegQueryValueEx (hKey, _T("Hard1"), 0, &type, (LPBYTE)&hard1, &len);
		RegQueryValueEx (hKey, _T("Hard2"), 0, &type, (LPBYTE)&hard2, &len);
		RegCloseKey (hKey);
	}
}

// Call when painter quit
void exitPainter ()
{
	// Delete some tiles from the bank
	delete bankCont;

	// Set background color
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_EDIT_PATCH, &hKey)==ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, _T("Background"), 0, REG_DWORD, (LPBYTE)&backGround, 4);
		RegSetValueEx(hKey, _T("Color1"), 0, REG_DWORD, (LPBYTE)&color1, 4);
		RegSetValueEx(hKey, _T("Color2"), 0, REG_DWORD, (LPBYTE)&color2, 4);
		RegSetValueEx(hKey, _T("Opa1"), 0, REG_DWORD, (LPBYTE)&opa1, 4);
		RegSetValueEx(hKey, _T("Opa2"), 0, REG_DWORD, (LPBYTE)&opa2, 4);
		RegSetValueEx(hKey, _T("Hard1"), 0, REG_DWORD, (LPBYTE)&hard1, 4);
		RegSetValueEx(hKey, _T("Hard2"), 0, REG_DWORD, (LPBYTE)&hard2, 4);
		RegCloseKey (hKey);
	}
}

/*-------------------------------------------------------------------*/

std::vector<CZoneSymmetrisation> symVector;

/*-------------------------------------------------------------------*/

// Painter modes
enum TModePaint { ModeTile, ModeColor, ModeDisplace};
enum TModeMouse { ModePaint, ModeSelect, ModePick, ModeFill, ModeGetState, ModeResetPatch };

/*-------------------------------------------------------------------*/

// Draw GUI
void drawVertexColorBox (float x0, float y0, float x1, float y1, CRGBA color, float opacity, float hardness, IDriver& driver)
{
	// Draw a white frame behind
	CDRU::drawQuad (x0, y0, x1, y1, driver, CRGBA (0x40, 0x40, 0x40), CViewport());

	// Compute the colors with alpha to show the hardness
	CRGBA fullFull=color;
	CRGBA full=color;
	full.A=(uint8)(opacity*255.f);
	CRGBA medium=color;
	medium.A=(uint8)(255.f*hardness*opacity);
	CRGBA empty=color;
	empty.A=(uint8)(255.f*std::max (std::min (opacity*(1.f+1.414213f*(hardness-1.f)), 1.f), 0.f));

	// Center
	float xc=(x0+x1)/2.f;
	float yc=(y0+y1)/2.f;

	// Draw 4 quads if texture exist
	CDRU::drawQuad (x0, y0, xc, yc, empty, medium, full, medium, driver, CViewport());
	CDRU::drawQuad (xc, y0, x1, yc, fullFull, fullFull, fullFull, fullFull, driver, CViewport());
	CDRU::drawQuad (xc, yc, x1, y1, fullFull, fullFull, fullFull, fullFull, driver, CViewport());
	CDRU::drawQuad (x0, yc, xc, y1, medium, full, medium, empty, driver, CViewport());
}


// Draw GUI
void drawInterface (TModeMouse select, TModePaint mode, PaintPatchMod *pobj, IDriver& driver, CLandscapeModel& landscape, CPaintColor &paintColor)
{
	// Select a texture..
	if (select==ModeSelect)
	{
		// Select a tileSet
		if (mode==ModeTile)
		{
			uint num=tileSetSelector.getTileCount ();
			for (uint t=0; t<num; t++)
			{
				// TileSet number
				int tileSet=tileSetSelector.getTileSet (t);

				// 128 tile in this bank ?
				if (bank.getTileSet (tileSet)->getNumTile128()>0)
				{
					// Get a 128 tile number
					int nTile=bank.getTileSet (tileSet)->getTile128(0);
					nlassert (nTile!=-1);

					// Get a texture of this tile
					ITexture *texture=bankCont->TileSet[tileSet].MainBitmap;
					if (texture)
						CDRU::drawBitmap ( (float)((t+1)%MOD_WIDTH)/(float)MOD_WIDTH, (float)((t+1)/MOD_WIDTH)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH,
							1.f/(float)MOD_HEIGHT, *texture, driver, CViewport(), false);
				}
			}
		}
		// Select a tileSet
		else if (mode==ModeDisplace)
		{
			// No tileset
			if (pobj->DisplaceTileSet!=-1)
			{
				// A is a valid tile set selected ?
				for (uint t=0; t<CTileSet::CountDisplace; t++)
				{
					// Get the texture
					ITexture *texture=bankCont->TileSet[pobj->DisplaceTileSet].DisplaceBitmap[t];

					// Draw the texture
					if (texture)
						CDRU::drawBitmap ( (float)(t%MOD_WIDTH)/(float)MOD_WIDTH, (float)(t/MOD_WIDTH)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH,
							1.f/(float)MOD_HEIGHT, *texture, driver, CViewport(), false);
					else
						CDRU::drawQuad ( (float)(t%MOD_WIDTH)/(float)MOD_WIDTH, (float)(t/MOD_WIDTH)/(float)MOD_HEIGHT,
							(float)(t%MOD_WIDTH)/(float)MOD_WIDTH+1.f/(float)MOD_WIDTH,
							(float)(t/MOD_WIDTH)/(float)MOD_HEIGHT+1.f/(float)MOD_HEIGHT, driver,
							CRGBA (128, 128, 128), CViewport());
				}
			}
		}
	}
	else
	{
		// Mode paint, draw the GUI

		// Automatic lighting ?
		if (pobj->automaticLighting)
		{
			// Get pointer
			ITexture *tex=bankCont->lightBitmap;

			// Draw the bitmap
			CDRU::drawBitmap ( (float)(MOD_WIDTH-1)/(float)MOD_WIDTH, (float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
		}

		// Lock borders ?
		if (pobj->lockBorders)
		{
			// Get pointer
			ITexture *tex=bankCont->lockBitmap;

			// Draw the bitmap
			CDRU::drawBitmap ( (float)(MOD_WIDTH-1)/(float)MOD_WIDTH, (float)(MOD_HEIGHT-2)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
		}

		// *** Draw the current tile
		if ((pobj->CurrentTileSet!=-1)&&(mode==ModeTile))
		{
			// Draw the oriented icon
			if (bank.getTileSet (pobj->CurrentTileSet)->getOriented())
			{
				// Get pointer
				ITexture *tex=bankCont->orientedBitmap;

				// Draw the bitmap
				CDRU::drawBitmap ( (float)(MOD_WIDTH-1)/(float)MOD_WIDTH, (float)(0)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
			}

			if (pobj->TileGroup==0)
			{
				// Get pointer
				ITexture *tex=bankCont->TileSet[pobj->CurrentTileSet].MainBitmap;

				// Default: all tile are used
				if (tex)
					CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
			}
			else
			{
				bool full=false;

				// 128 or 256 ?
				if (pobj->tileSize==0)
				{
					if (bankCont->TileSet[pobj->CurrentTileSet].GroupTile128[pobj->TileGroup-1].size()!=0)
						full=true;
				}
				else
				{
					if (bankCont->TileSet[pobj->CurrentTileSet].GroupTile256[pobj->TileGroup-1].size()!=0)
						full=true;
				}

				if (full)
				{
					// Get pointer
					ITexture *tex=bankCont->TileSet[pobj->CurrentTileSet].GroupBitmap[pobj->TileGroup-1];

					// Show the tile from a group
					if (tex)
						CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
				}
			}
		}

		// *** Draw the current noise
		if (mode==ModeDisplace)
		{
			if (pobj->DisplaceTileSet!=-1)
			{
				// Get pointer
				ITexture *tex=bankCont->TileSet[pobj->DisplaceTileSet].DisplaceBitmap[pobj->DisplaceTile];

				// Draw if texture exist
				if (tex)
					CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *tex, driver, CViewport(), true);
				else
				{
					// 4 Corners
					float x0=0.f;
					float y0=(float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT;
					float x1=1.f/(float)MOD_WIDTH;
					float y1=1.f;

					CDRU::drawQuad (x0, y0, x1, y1, driver, CRGBA (128, 128, 128), CViewport());
				}
			}
		}

		// *** Draw the current color
		if (mode==ModeColor)
		{

			// 4 Corners
			float x0=0.f;
			float y0=(float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT;
			float x1=1.f/(float)MOD_WIDTH;
			float y1=1.f;

#define		BORDER_COLOR (0.2f)

			// Draw second color
			drawVertexColorBox (x0+(x1-x0)*BORDER_COLOR, y0, x1, y1+(y0-y1)*BORDER_COLOR, maxToNel (color2), opa2, hard2, driver);

			// Draw first color
			drawVertexColorBox (x0, y0+(y1-y0)*BORDER_COLOR, x1+(x0-x1)*BORDER_COLOR, y1, maxToNel (color1), opa1, hard1, driver);
		}

		// *** Draw the current bursh size
		if ((mode==ModeTile)||(mode==ModeDisplace))
		{
			ITexture* burshSize[BRUSH_COUNT]={bankCont->_smallBitmap, bankCont->mediumBitmap, bankCont->largeBitmap};

			// Draw brush icon
			CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-3)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *(burshSize[pobj->brushSize]), driver, CViewport(), true);
		}
		else if (mode==ModeColor)
		{
			// 4 points
			float x0=0.f;
			float y0=(float)(MOD_HEIGHT-2)/(float)MOD_HEIGHT;
			float x1=1.f/(float)MOD_WIDTH;
			float y1=(float)(MOD_HEIGHT-1)/(float)MOD_HEIGHT;

			// Center
			float xc=(x0+x1)/2.f;
			float yc=(y0+y1)/2.f;

			// Scale factor
			float scaleFactor=0.9f*(float)PaintPatchMod::ColorBushSize/(float)COLOR_BRUSH_STEP+0.1f;
			clamp (scaleFactor, 0.f, 1.f);

			// Scale it
			x0=(x0-xc)*scaleFactor+xc;
			y0=(y0-yc)*scaleFactor+yc;
			x1=(x1-xc)*scaleFactor+xc;
			y1=(y1-yc)*scaleFactor+yc;

			// Draw brush icon
			CDRU::drawBitmap ( x0, y0, x1-x0, y1-y0, *bankCont->largeBitmap, driver, CViewport(), true);
		}

		// For tiles only
		if (mode==ModeTile)
		{
			// *** Draw the current size
			ITexture* size[2]={bankCont->_128Bitmap, bankCont->_256Bitmap};

			// Draw brush icon
			CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-2)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *(size[pobj->tileSize]), driver, CViewport(), true);
		}

		if ((pobj->CurrentTileSet!=-1)&&(mode==ModeTile))
		{
			// *** Draw the current group
			ITexture* group[NL3D_CTILE_NUM_GROUP+1]={bankCont->allBitmap, bankCont->_0Bitmap, bankCont->_1Bitmap, bankCont->_2Bitmap, bankCont->_3Bitmap,
								bankCont->_4Bitmap, bankCont->_5Bitmap, bankCont->_6Bitmap, bankCont->_7Bitmap, bankCont->_8Bitmap, bankCont->_9Bitmap,
								bankCont->_10Bitmap, bankCont->_11Bitmap};

			// Draw group icon
			CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-4)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *(group[pobj->TileGroup]), driver, CViewport(), true);
		}

		// For color only
		if (paintColor.getBrushMode ()&&(mode==ModeColor))
		{
			// Draw brush icon
			CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-3)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, paintColor.getBrush (), driver, CViewport(), false);
		}

		// For color only
		if (pobj->ShowCurrentState && ((int)pobj->CurrentState<3) && ((int)pobj->CurrentState>=0))
		{
			// Draw brush icon
			ITexture* bitmaps[3]={bankCont->nothingBitmap, bankCont->regularBitmap, bankCont->goofyBitmap};
			CDRU::drawBitmap ( 0, (float)(MOD_HEIGHT-5)/(float)MOD_HEIGHT, 1.f/(float)MOD_WIDTH, 1.f/(float)MOD_HEIGHT, *(bitmaps[(int)pobj->CurrentState]), driver, CViewport(), true);
		}
	}
}

/*-------------------------------------------------------------------*/

#define WELD_THRESOLD 1.f

/*-------------------------------------------------------------------*/

void addNeLZone ( Object *object, INode *current, std::vector<EPM_Mesh> &vectMesh, PatchMesh *patch, RPatchMesh *rpatch, PaintPatchData *patchData, ModContext *mod, INode *original, uint &nelIndex)
{
	// Not the same
	if ((current->GetObjectRef() == object) && (!current->IsHidden()))
	{
		vectMesh.push_back (EPM_Mesh (patch, rpatch, patchData, current, mod, nelIndex++, original == current));
	}

	// Call to child
	for (uint child=0; child<(uint)current->NumberOfChildren(); child++)
		addNeLZone ( object, current->GetChildNode(child), vectMesh, patch, rpatch, patchData, mod, original, nelIndex);
}

// Tile painting algorithm. watch "doc/3d/tile_algorithm.doc" for details

void makeVectMesh (std::vector<EPM_Mesh>& vectMesh, INodeTab& nodes, ModContextList& mcList, PaintPatchMod *pobj, TimeValue t)
{
	// Clear the mesh vector
	vectMesh.clear ();

	// Nel zone indexies
	uint nelIndex = 0;

	for (int i = 0; i < mcList.Count (); i++)
	{
		//
		PaintPatchData *patchData = (PaintPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		if (patchData->GetFlag(EPD_BEENDONE))
			continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(pobj)->GetPatch(t, rpatch);
		if (!patch)
			continue;

		// Look for another node
		addNeLZone ( nodes[i]->GetObjectRef(), pobj->ip->GetRootNode(), vectMesh, patch, rpatch, patchData, mcList[i], nodes[i], nelIndex );
	}
}

void transformDesc (tileDesc &desc, bool symmetry, uint rotate, uint mesh, uint tile, std::vector<EPM_Mesh>& vectMesh)
{
	uint patch=tile/NUM_TILE_SEL;
	uint ttile=tile%NUM_TILE_SEL;
	uint v=ttile/MAX_TILE_IN_PATCH;
	uint u=ttile%MAX_TILE_IN_PATCH;

	// Get the patch
	int OrderS=(1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesU);
	uint symTile = OrderS*v+u;

	// 256 ?
	if (desc.getCase()!=0)
	{
		// Transform the case
		uint8 case256 = desc.getCase()-1;

		// Get rot and symmetry for this tile
		uint tileRotate = rotate;
		bool tileSymmetry = symmetry;
		uint tile = desc.getLayer (0).Tile;

		// XRef
		int tileSet;
		int number;
		CTileBank::TTileType type;
		bank.getTileXRef (tile, tileSet, number, type);

		// Transform the transfo
		CPatchInfo::getTileSymmetryRotate (bank, desc.getLayer(0).Tile, tileSymmetry, tileRotate);

		// Get the state of the layer 0
		bool goofy = false;
		uint tileRotation = desc.getLayer (0).Rotate;
		if (symmetry)
		{
			if (bank.getTileSet (tileSet)->getOriented ())
			{
				if ((symVector[mesh].getOrientedTileBorderState (patch, symTile) != CZoneSymmetrisation::Nothing))
					goofy = (symVector[mesh].getOrientedTileBorderState (patch, symTile) == CZoneSymmetrisation::Goofy);
				else
					goofy = (symVector[mesh].getTileState (patch, symTile, 0) == CZoneSymmetrisation::Goofy);
			}
			else
			{
				if ((symVector[mesh].getTileBorderState (patch, symTile) != CZoneSymmetrisation::Nothing))
					goofy = (symVector[mesh].getTileBorderState (patch, symTile) == CZoneSymmetrisation::Goofy);
				else
					goofy = (symVector[mesh].getTileState (patch, symTile, 0) == CZoneSymmetrisation::Goofy);
			}
		}

		CPatchInfo::transform256Case (bank, case256, tileRotation, tileSymmetry, tileRotate, goofy);
		desc.setCase (case256+1);
	}

	for (int l=0; l<desc.getNumLayer (); l++)
	{
		// Tile and rot
		uint tile = desc.getLayer (l).Tile;
		uint tileRotation = desc.getLayer (l).Rotate;

		// XRef
		int tileSet;
		int number;
		CTileBank::TTileType type;
		bank.getTileXRef (tile, tileSet, number, type);

		// Get rot and symmetry for this tile
		uint tileRotate = rotate;
		bool tileSymmetry = symmetry;

		// Get the state of the
		bool goofy = false;
		if (symmetry)
		{
			if (bank.getTileSet (tileSet)->getOriented ())
			{
				if ((symVector[mesh].getOrientedTileBorderState (patch, symTile) != CZoneSymmetrisation::Nothing))
					goofy = (symVector[mesh].getOrientedTileBorderState (patch, symTile) == CZoneSymmetrisation::Goofy);
				else
					goofy = (symVector[mesh].getTileState (patch, symTile, l) == CZoneSymmetrisation::Goofy);
			}
			else
			{
				if ((symVector[mesh].getTileBorderState (patch, symTile) != CZoneSymmetrisation::Nothing))
					goofy = (symVector[mesh].getTileBorderState (patch, symTile) == CZoneSymmetrisation::Goofy);
				else
					goofy = (symVector[mesh].getTileState (patch, symTile, l) == CZoneSymmetrisation::Goofy);
			}
		}

		// Transform the transfo
		if (CPatchInfo::getTileSymmetryRotate (bank, tile, tileSymmetry, tileRotate))
		{
			// Transform it
			if (CPatchInfo::transformTile (bank, tile, tileRotation, tileSymmetry, tileRotate, goofy))
			{
				desc.getLayer (l).Tile = tile;
				desc.getLayer (l).Rotate = tileRotation;
			}
		}
	}
}

void transformInvDesc (tileDesc &desc, bool symmetry, uint rotate, uint mesh, uint tile, std::vector<EPM_Mesh>& vectMesh)
{
	transformDesc (desc, false, 4-rotate, mesh, tile, vectMesh);
	transformDesc (desc, symmetry, 0, mesh, tile, vectMesh);
}

void EPM_PaintMouseProc::SetTile (int mesh, int tile, const tileDesc& desc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land,
								  CNelPatchChanger& nelPatchChg, std::vector<CBackupValue>* backupStack, bool undo, bool updateDisplace)
{
	// Undo: get old value
	tileDesc oldDesc;
	GetTile (mesh, tile, oldDesc, vectMesh, land);

	// New desc
	tileDesc maxDesc=desc;

	// Backup if error
	if (backupStack)
		backupStack->push_back (CBackupValue (oldDesc, mesh, tile));

	// Update displace ?
	if (!updateDisplace)
	{
		// Copy old displacement map
		maxDesc.setDisplace (oldDesc.getDisplace());
	}

	// Transform tile
	tileDesc copyDesc=maxDesc;
	transformInvDesc (maxDesc, vectMesh[mesh].Symmetry, 4-vectMesh[mesh].Rotate, mesh, tile, vectMesh);
	transformInvDesc (copyDesc, vectMesh[mesh].Symmetry, 0, mesh, tile, vectMesh);

	// 3dsmax update
	vectMesh[mesh].RMesh->setTileDesc (tile, maxDesc);

	// NeL update
	if (land)
	{
		// For each mesh
		for (uint i=0; i<vectMesh.size(); i++)
		{
			// Same mesh ?
			if (vectMesh[i].RMesh == vectMesh[mesh].RMesh)
			{
				int patch=tile/NUM_TILE_SEL;
				int ttile=tile%NUM_TILE_SEL;
				int v=ttile/MAX_TILE_IN_PATCH;
				int u=ttile%MAX_TILE_IN_PATCH;

				// Get the patch
				std::vector<CTileElement>& copyZone = *nelPatchChg.getTileArray (i, patch);
				RPatchMesh *pMesh=vectMesh[i].RMesh;
				int OrderS=(1<<pMesh->getUIPatch (patch).NbTilesU);

				// Get a desc for this tile
				tileDesc nelDesc=copyDesc;
				transformInvDesc (nelDesc, vectMesh[mesh].Symmetry, 4-vectMesh[mesh].Rotate, mesh, tile, vectMesh);
				transformDesc (nelDesc, vectMesh[i].Symmetry, 4-vectMesh[i].Rotate, mesh, tile, vectMesh);

				// Symmetry ?
				if (vectMesh[i].Symmetry)
				{
					u = OrderS - u - 1;
				}

				for (int l=0; l<3; l++)
				{
					if (l>=nelDesc.getNumLayer ())
					{
						copyZone[u+v*OrderS].Tile[l]=0xffff;
					}
					else
					{
						int toto=copyZone[u+v*OrderS].Tile[l];
						copyZone[u+v*OrderS].Tile[l]=nelDesc.getLayer (l).Tile;
						copyZone[u+v*OrderS].setTileOrient (l, nelDesc.getLayer (l).Rotate);

					}
				}
				if (copyZone[u+v*OrderS].Tile[0]==0xffff)
					copyZone[u+v*OrderS].setTile256Info (false, 0);
				else
				{
					if (nelDesc.getCase()==0)
						copyZone[u+v*OrderS].setTile256Info (false, 0);
					else
						copyZone[u+v*OrderS].setTile256Info (true, nelDesc.getCase()-1);
				}

				// Displace tile
				copyZone[u+v*OrderS].setTileSubNoise (nelDesc.getDisplace());

				// Undo: push new value
				if (undo)
				{
					CUndoElement undoEmlt (i, tile, CUndoStruct (oldDesc), CUndoStruct (desc));
					bankCont->Undo.toUndo (undoEmlt);
				}
			}
		}
	}
}

void EPM_PaintMouseProc::GetTile (int mesh, int tile, tileDesc& desc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land)
{
	// NeL update
	if (land)
	{
		// Get the patch
		RPatchMesh *pMesh=vectMesh[mesh].RMesh;

		// Get tile desc
		desc=pMesh->getTileDesc (tile);

		// Transform it
		transformDesc (desc, vectMesh[mesh].Symmetry, 4-vectMesh[mesh].Rotate, mesh, tile, vectMesh);
	}
}

int EPM_PaintMouseProc::selectTile (uint tileSet, bool selectCycle, bool _256, uint group, const CTileBank& bank)
{
	// TileSet
	const CTileSet *TileSet=bank.getTileSet (tileSet);

	// The number
	int nTile;

	// Calc next tile index
	uint32 index;
	if (selectCycle)
		// Select a tile in a cycle
		index=TileIndex++;
	else
		// Select a random tile
		index=(uint32)rand();

	if (_256)
	{
		// Select in a group ?
		if (group==0)
		{
			// No, select in the global list
			nTile=TileSet->getTile256 (index%(uint32)TileSet->getNumTile256 ());
		}
		else
		{
			// Ref on the group tile array
			const std::vector<uint>& groupArray=bankCont->TileSet[tileSet].GroupTile256[group-1];

			// Check there is tiles in this group
			if (groupArray.size()==0)
				nTile=-1;
			else
			// No, select in the global list
			nTile=TileSet->getTile256 (groupArray[index%(uint32)groupArray.size ()]);
		}
	}
	else
	{
		// Select in a group ?
		if (group==0)
		{
			// No, select in the global list
			nTile=TileSet->getTile128 (index%(uint32)TileSet->getNumTile128 ());
		}
		else
		{
			// Ref on the group tile array
			const std::vector<uint>& groupArray=bankCont->TileSet[tileSet].GroupTile128[group-1];

			// Check there is tiles in this group
			if (groupArray.size()==0)
				nTile=-1;
			else
				// No, select in the global list
				nTile=TileSet->getTile128 (groupArray[index%(uint32)groupArray.size ()]);
		}
	}

	return nTile;
}

bool EPM_PaintMouseProc::isLockedEx (PaintPatchMod *pobj, EPM_PaintTile* pTile, std::vector<EPM_Mesh>& vectMesh, CLandscape* land)
{
	// Lock border ?
	if (pobj->lockBorders)
	{
		// get the tile desc
		tileDesc backup;
		GetTile (pTile->Mesh, pTile->tile, backup, vectMesh, land);

		// It is a 256 ?
		if (backup.getCase()>0)
		{
			// Align on the grid
			if (pTile->u&1)
				pTile=pTile->voisins[0];
			if (pTile->v&1)
				pTile=pTile->voisins[3];

			// Return status
			int nRot;
			return (pTile->locked!=0) || (pTile->getRight256 (0, nRot)->locked!=0) || (pTile->getBottom256 (0, nRot)->locked!=0) || (pTile->getRightBottom256 (0, nRot)->locked!=0);
		}
		else
			// Return lock status
			return (pTile->locked!=0);
	}

	// Not locked
	return false;
}

bool EPM_PaintMouseProc::isLocked256 (PaintPatchMod *pobj, EPM_PaintTile* pTile)
{
	// Lock border ?
	if (pobj->lockBorders)
	{
		// Align on the grid
		if (pTile->u&1)
			pTile=pTile->voisins[0];
		if (pTile->v&1)
			pTile=pTile->voisins[3];

		// Return status
		return (pTile->locked!=0) || (pTile->voisins[2]->locked!=0) || (pTile->voisins[1]->locked!=0) || (pTile->voisins[2]->voisins[1]->locked!=0);
	}

	// Not locked
	return false;
}

bool EPM_PaintMouseProc::ClearATile ( EPM_PaintTile* pTile, std::vector<EPM_Mesh>& vectMesh,
								   CLandscape* land, CNelPatchChanger& nelPatchChg, bool _256, bool _force128)
{
	// ** 1) Backup of the tile
	tileDesc backup;
	GetTile (pTile->Mesh, pTile->tile, backup, vectMesh, land);

	// If we are over a 256, force mode 256
	if ((backup.getCase()>0) && !_force128)
		_256 = true;

	// If tile 256, must have delta pos aligned on 2x2
	if (_256)
	{
		if (pTile->u&1)
			pTile=pTile->voisins[0];
		if (pTile->v&1)
			pTile=pTile->voisins[3];
	}

	// Erase a 256 area ?
	if (_256)
	{
		tileDesc desc;
		desc.setTile (0, 0, 0, tileIndex (0,0), tileIndex (0,0), tileIndex (0,0));

		// Get right tile
		int nRot;
		EPM_PaintTile *neighbor[4] = { pTile, pTile->getRight256 (0, nRot), pTile->getBottom256 (0, nRot), pTile->getRightBottom256 (0, nRot) };

		// Check not locked
		uint n;
		for (n=0; n<4; n++)
		{
			// Ok ?
			nlassert (neighbor[n]);

			// Locked ?
			if (isLocked (pobj, neighbor[n]))
				return false;
		}

		// Clear them
		for (n=0; n<4; n++)
		{
			// Get the displace index
			tileDesc descOrig;
			GetTile (neighbor[n]->Mesh, neighbor[n]->tile, descOrig, vectMesh, land);

			// Not compatible, clear it
			desc.setDisplace (descOrig.getDisplace());
			SetTile (neighbor[n]->Mesh, neighbor[n]->tile, desc, vectMesh, land, nelPatchChg, NULL);
		}
	}
	else
	{
		// Check locked
		if (isLocked (pobj, pTile))
			return false;

		// Cleared descriptor
		tileDesc desc;
		desc.setTile (0, 0, 0, tileIndex (0,0), tileIndex (0,0), tileIndex (0,0));

		// Get the displace index
		tileDesc descOrig;
		GetTile (pTile->Mesh, pTile->tile, descOrig, vectMesh, land);

		// Not compatible, clear it
		desc.setDisplace (descOrig.getDisplace());
		SetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land, nelPatchChg, NULL);
	}

	// Erased
	return true;
}

bool EPM_PaintMouseProc::PutATile ( EPM_PaintTile* pTile, int tileSet, int curRotation, const CTileBank& bank,
								   bool selectCycle, std::set<EPM_PaintTile*>& visited, std::vector<EPM_Mesh>& vectMesh,
								   CLandscape* land, CNelPatchChanger& nelPatchChg, bool _256)
{
	// If tile 256, must have delta pos aligned on 2x2
	if (_256)
	{
		if (pTile->u&1)
			pTile=pTile->voisins[0];
		if (pTile->v&1)
			pTile=pTile->voisins[3];
	}

	// 256 valide ?
	if ((_256)&&(!pTile->validFor256 (0)))
		return false;

	// Frozen ?
	if (pTile->frozen)
		return false;

	// Locked ?
	if (isLocked (pobj, pTile))
		return false;

	// *** Clip select patch
	// Patch number
	int patch=pTile->tile/NUM_TILE_SEL;

	// Check if we are in patch subobject and if this patch is selected

	if ((vectMesh[pTile->Mesh].PMesh->selLevel==EP_PATCH)&&(!vectMesh[pTile->Mesh].PMesh->patchSel[patch]))
		return false;

	// ** 1) Backup of the tile
	tileDesc backup;
	GetTile (pTile->Mesh, pTile->tile, backup, vectMesh, land);
	tileDesc backupRight, backupBottom, backupCorner;

	// Backup stack
	static std::vector<CBackupValue> backupStack;
	backupStack.reserve (300);
	backupStack.resize(0);

	if (_256)
	{
		// get right
		int nRot;
		EPM_PaintTile* other=pTile->getRight256 (0, nRot);
		nlassert (other);
		if (isLocked (pobj, other))
			return false;
		GetTile (other->Mesh, other->tile, backupRight, vectMesh, land);

		// get bottom
		other=pTile->getBottom256 (0, nRot);
		nlassert (other);
		if (isLocked (pobj, other))
			return false;
		GetTile (other->Mesh, other->tile, backupBottom, vectMesh, land);

		// get corner
		other=pTile->getRightBottom256 (0, nRot);
		nlassert (other);
		nlassert (other==pTile->getBottomRight256 (0, nRot));
		if (isLocked (pobj, other))
			return false;
		GetTile (other->Mesh, other->tile, backupCorner, vectMesh, land);
	}

	// ** 2) Add to visited tile set
	visited.insert (pTile);

	// Clear...
	if (tileSet==-1)
	{
		return ClearATile ( pTile, vectMesh, land, nelPatchChg, _256);
	}
	else
	{
		// Select a tile
		int nTile=selectTile (tileSet, selectCycle, _256, pobj->TileGroup, bank);

		// No tile, return
		if (nTile==-1)
			return false;

		// ** 3) Put the tile

		if (_256)
		{
			// Main tile
			tileDesc desc;
			desc.setTile (1, 1+((-curRotation)&3), 0, tileIndex (nTile, curRotation), tileIndex (0,0), tileIndex (0,0));

			// Main tile
			SetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land, nelPatchChg, &backupStack);

			// get right
			int nRot;
			EPM_PaintTile* other=pTile->getRight256 (0, nRot);
			desc.setTile (1, 1+((-curRotation-1)&3), 0, tileIndex (nTile, (curRotation-nRot)&3), tileIndex (0,0), tileIndex (0,0));
			nlassert (other);
			SetTile (other->Mesh, other->tile, desc, vectMesh, land, nelPatchChg, &backupStack);

			// Add to visited tile set
			visited.insert (other);

			// get bottom
			other=pTile->getBottom256 (0, nRot);
			desc.setTile (1, 1+((-curRotation+1)&3), 0, tileIndex (nTile, (curRotation-nRot)&3), tileIndex (0,0), tileIndex (0,0));
			nlassert (other);
			SetTile (other->Mesh, other->tile, desc, vectMesh, land, nelPatchChg, &backupStack);

			// Add to visited tile set
			visited.insert (other);

			// get corner
			other=pTile->getRightBottom256 (0, nRot);
			desc.setTile (1, 1+((-curRotation+2)&3), 0, tileIndex (nTile, (curRotation-nRot)&3), tileIndex (0,0), tileIndex (0,0));
			nlassert (other);
			nlassert (other==pTile->getBottomRight256 (0, nRot));
			SetTile (other->Mesh, other->tile, desc, vectMesh, land, nelPatchChg, &backupStack);

			// Add to visited tile set
			visited.insert (other);
		}
		else
		{
			tileDesc desc;
			desc.setTile (1, 0, 0, tileIndex (nTile, curRotation), tileIndex (0,0), tileIndex (0,0));
			SetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land, nelPatchChg, &backupStack);
		}
	}

	// return value
	bool bContinue=true;

	// Random offset
	uint offset=rand();

	if (_256)
	{

		// Visit neighbourhood in a random order
		for (int n=0; n<4; n++)
		{
			int nRot;
			EPM_PaintTile* other;

			switch ((offset+n)&0x3)
			{
			case 0:
				// Main
				if (pTile->voisins[3])
					if (!PropagateBorder (pTile->voisins[3], (pTile->rotate[3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				if (pTile->voisins[0])
					if (!PropagateBorder (pTile->voisins[0], (pTile->rotate[0]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				break;

			case 1:
				// Bottom
				other=pTile->getBottom256 (0, nRot);
				if (other->voisins[(0-nRot)&3])
					if (!PropagateBorder (other->voisins[(0-nRot)&3], (other->rotate[(0-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				if (other->voisins[(1-nRot)&3])
					if (!PropagateBorder (other->voisins[(1-nRot)&3], (other->rotate[(1-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				break;

			case 2:
				// Corner
				other=pTile->getBottomRight256 (0, nRot);
				if (other->voisins[(1-nRot)&3])
					if (!PropagateBorder (other->voisins[(1-nRot)&3], (other->rotate[(1-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				if (other->voisins[(2-nRot)&3])
					if (!PropagateBorder (other->voisins[(2-nRot)&3], (other->rotate[(2-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				break;

			case 3:
				// Right
				other=pTile->getRight256 (0, nRot);
				if (other->voisins[(2-nRot)&3])
					if (!PropagateBorder (other->voisins[(2-nRot)&3], (other->rotate[(2-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				if (other->voisins[(3-nRot)&3])
					if (!PropagateBorder (other->voisins[(3-nRot)&3], (other->rotate[(3-nRot)&3]+curRotation)&3, tileSet, visited, bank,
							vectMesh, land, nelPatchChg, backupStack))
					{
						bContinue=false;
						goto zap;
					}
				break;
			default:
				nlassert (0);		// no!
			}
		}
zap:;
	}
	else
	{
		// For all patches voisins
		for (int i=0; i<4; i++)
		{
			// Random offset
			uint ii=(offset+i)&0x3;

			// 4) Propagate the borders to voisins
			if (pTile->voisins[ii])
			{
				if (!PropagateBorder (pTile->voisins[ii], (pTile->rotate[ii]+curRotation)&3, tileSet, visited, bank,
					vectMesh, land, nelPatchChg, backupStack))
				{
					bContinue=false;
					break;
				}
			}
		}
	}

	// Backup !!
	if (!bContinue)
	{
		// Backup all tiles
		for (int back=backupStack.size()-1; back>=0; back--)
		{
			// Yo!
			SetTile (backupStack[back].Mesh, backupStack[back].Tile, backupStack[back].Desc, vectMesh, land, nelPatchChg, NULL);
		}

		// Try to put a transition tile ?
		bool backup256 = (backup.getCase()>0);
		if (!_256 && !backup256)
		{
			// 4 cases
			// *****
			// *0*3*
			// *****
			// *1*2*
			// *****
			tileSetIndex tileSetCases[4][4];
			for (uint i=0; i<4; i++)
			for (uint j=0; j<4; j++)
			{
				tileSetCases[i][j].TileSet = -1;
				tileSetCases[i][j].Rotate = 0;
			}
			CTileSet::TFlagBorder borderEdges[4][2];

			// For each edge
			for (uint edge=0; edge<4; edge++)
			{
				// Neighbor ?
				if (pTile->voisins[edge])
				{
					// Get the neighbor corner
					tileSetIndex pVoisinCorner[4];
					CTileSet::TFlagBorder pBorder[4][3];
					tileDesc pVoisinIndex;

					// Tile is filled ?
					if (GetBorderDesc (pTile->voisins[edge], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land))
					{
						// Neighbor edge
						int neigborEdge = (2+edge+pTile->rotate[edge])&3;

						// Copy the tiles
						tileSetCases[edge][edge] = pVoisinCorner[(neigborEdge+1)&3];
						tileSetCases[edge][edge].Rotate -= pTile->rotate[edge];
						tileSetCases[edge][edge].Rotate &= 3;
						tileSetCases[edge][(edge+1)&3] = pVoisinCorner[neigborEdge];
						tileSetCases[edge][(edge+1)&3].Rotate -= pTile->rotate[edge];
						tileSetCases[edge][(edge+1)&3].Rotate &= 3;

						// Change the rotation

						// Get the transition used
						for (uint subTile=0; subTile<2; subTile++)
						{
							// Tileset
							int slot=getLayer (pTile, edge, pVoisinCorner[(neigborEdge+subTile)&3].TileSet, (pVoisinCorner[(neigborEdge+subTile)&3].Rotate - pTile->rotate[edge])&3, vectMesh, land);

							// Should be found
							nlassert (slot>=0);

							// Get the border
							borderEdges[edge][1-subTile] = CTileSet::getInvertBorder (pBorder[neigborEdge][slot]);
						}
					}
				}
			}

			// Make the final corner descriptor
			tileSetIndex finalCorner[4];
			for (uint corner=0; corner<4; corner++)
			{
				finalCorner[corner].TileSet = -1;

				// All the same or empty ?
				for (uint layer=0; layer<4; layer++)
				{
					// Compatible ?
					if ( ( finalCorner[corner].TileSet == -1 )
						|| ( tileSetCases[layer][corner].TileSet == -1 )
						|| ( tileSetCases[layer][corner] == finalCorner[corner] ) )
					{
						// Copy the tile
						if ( tileSetCases[layer][corner].TileSet  != -1 )
							finalCorner[corner] = tileSetCases[layer][corner];
					}
					else
					{
						// Not compatible
						return false;
					}
				}

				// Empty ?
				if (finalCorner[corner].TileSet == -1)
				{
					// New tileSet
					finalCorner[corner].TileSet = tileSet;
					finalCorner[corner].Rotate = curRotation;
				}
			}

			// Set of index
			std::vector<tileSetIndex> setIndex;

			// Set count
			for (uint v=0; v<4; v++)
			{
				// Should not be empty
				nlassert (finalCorner[v].TileSet!=-1);

				// Check for same tile with a +2 rotation
				bool bFind=false;

				for (int vv=0; vv<(int)setIndex.size(); vv++)
				{
					if (setIndex[vv].TileSet==finalCorner[v].TileSet)
					{
						tileSetIndex complet=finalCorner[v];
						complet.Rotate=(complet.Rotate+2)&3;
						if (setIndex[vv].Rotate==complet.Rotate)
							return false;
						if (finalCorner[v]==setIndex[vv])
							bFind=true;
					}
				}

				// no, ok push it back.
				if (!bFind)
					setIndex.push_back (finalCorner[v]);
			}

			// Sort the tile set
			std::sort (setIndex.begin(), setIndex.end());

			// Check for more than 3 materials
			if (setIndex.size()>3)
				return false;

			// Count materiaux
			tileIndex finalIndex[3];
			finalIndex[0].Tile=0;
			finalIndex[1].Tile=0;
			finalIndex[2].Tile=0;

			// For each layer
			for (int l=0; l<(int)setIndex.size(); l++)
			{
				if (l==0)
				{
					// Look for a tile without group
					finalIndex[l].Tile=selectTile (setIndex[l].TileSet, false, false, 0, bank);
					finalIndex[l].Rotate=(setIndex[l].Rotate&3);
				}
				else
				{
					// The 4 borders
					CTileSet::TFlagBorder border[4];

					// Corner filled or not
					bool bFilled[4];
					for (int c=0; c<4; c++)
						bFilled[c]=!(finalCorner[c]<setIndex[l]);

					// Fill the edge
					for (uint e=0; e<4; e++)
					{
						// Two filled ?
						if (bFilled[e] && bFilled[(e+1)&3])
							border[e]=CTileSet::_1111;
						else if ( (!bFilled[e]) && (!bFilled[(e+1)&3]) )
							border[e]=CTileSet::_0000;
						else
						{
							// Found
							bool found = false;

							// Get neighbor edge
							if (pTile->voisins[e])
							{
								// Filled corner
								uint filledCorner = bFilled[e]?e:(e+1)&3;

								// Get the neighbor corner
								tileSetIndex pVoisinCorner[4];
								CTileSet::TFlagBorder pBorder[4][3];
								tileDesc pVoisinIndex;

								// Tile is filled ?
								if (GetBorderDesc (pTile->voisins[e], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land))
								{
									// Neighbor edge
									int neigborEdge = (2+e+pTile->rotate[e])&3;

									// Get the slot
									int slot=getLayer (pTile, e, setIndex[l].TileSet, setIndex[l].Rotate, vectMesh, land);

									if ((slot != -1) && (pBorder[neigborEdge][slot] != CTileSet::dontcare))
									{
										// Invert the border
										border[e]=CTileSet::getInvertBorder (pBorder[neigborEdge][slot]);

										// Found it
										found = true;
									}
								}
							}

							// Found ?
							if ( !found )
							{
								// First filled ?
								if (bFilled[e])
									border[e]=CTileSet::_1000;
								else
									border[e]=CTileSet::_0001;
							}
						}
					}

					// Find the transitions
					const CTileSetTransition *tileTrans = FindTransition (setIndex[l].TileSet, setIndex[l].Rotate, border, bank);
					if (!tileTrans)
					{
						return false;
					}
					finalIndex[l].Rotate=(setIndex[l].Rotate&3);
					finalIndex[l].Tile=tileTrans->getTile();
				}
			}

			// Set the border desc
			tileDesc desc;
			GetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land);
			switch (setIndex.size())
			{
			case 1:
				desc.setTile (1, 0, desc.getDisplace (), finalIndex[0], tileIndex (0,0), tileIndex (0,0));
				break;
			case 2:
				desc.setTile (2, 0, desc.getDisplace (), finalIndex[0], finalIndex[1], tileIndex (0,0));
				break;
			case 3:
				desc.setTile (3, 0, desc.getDisplace (), finalIndex[0], finalIndex[1], finalIndex[2]);
				break;
			default:
				nlassert (0);		// no!
				break;
			}
			SetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land, nelPatchChg, NULL);

		}
		else
		{
			// Can't pos 256 transition tile
			return false;
		}
	}

	return true;
}

/*-------------------------------------------------------------------*/

void EPM_PaintMouseProc::PutADisplacetile ( EPM_PaintTile* pTile, const CTileBank& bank,
								   std::vector<EPM_Mesh>& vectMesh,
								   CLandscape* land, CNelPatchChanger& nelPatchChg)
{
	// Get tile description
	tileDesc desc;
	GetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land);

	// Get the info about this tile
	int tileSet;
	int number;
	CTileBank::TTileType type;
	int tile=desc.getLayer (0).Tile;
	if ((tile>=0)&&(tile<bank.getTileCount ()))
	{
		bank.getTileXRef (tile, tileSet, number, type);

		// Pointer on the tileset
		const CTileSet *pTileSet=bank.getTileSet (tileSet);

		// Set the new displacement map
		desc.setDisplace (pobj->DisplaceTile);
		SetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land, nelPatchChg, NULL, true, true);
	}
}

/*-------------------------------------------------------------------*/

bool EPM_PaintMouseProc::GetBorderDesc (EPM_PaintTile* tile, tileSetIndex *pVoisinCorner, CTileSet::TFlagBorder pBorder[4][3],
										tileDesc *pVoisinIndex, const CTileBank& bank, std::vector<EPM_Mesh>& vectMesh,
										CNelPatchChanger& nelPatchChg, CLandscape *land)
{
	// Tile info
	tileDesc backup;
	GetTile (tile->Mesh, tile->tile, backup, vectMesh, land);
	if (backup.isEmpty())
		return false;

	int nLayer=backup.getNumLayer ();
	tileIndex pIndexx[3];
	for (int nL=0; nL<nLayer; nL++)
	{
		// GetTileIndex
		tileIndex index=backup.getLayer (nL);
		pIndexx[nL]=index;

		// Get layer info from base
		int tileSet;
		int number;
		CTileBank::TTileType type;

		if ((int)index.Tile>=bank.getTileCount())
		{
			WarningInvalidTileSet ();
			return false;	// Problem in tile info. Wrong tileSet ?
		}

		bank.getTileXRef (index.Tile, tileSet, number, type);

		// For each edge
		for (int i=0; i<4; i++)
		{
			// Border type
			CTileSet::TBorder toBorder[4]={CTileSet::left, CTileSet::bottom, CTileSet::right, CTileSet::top};
			CTileSet::TFlagBorder border=CTileSet::_1111;
			if (type==CTileBank::transition)
				border=CTileSet::getOrientedBorder (toBorder[i], CTileSet::getEdgeType ((CTileSet::TTransition)number, toBorder[i]));

			if (nL==0)
			{
				//nlassert (border==CTileSet::_1111);
				if (border!=CTileSet::_1111)
				{
					WarningInvalidTileSet ();
					return false;	// Problem in tile info. Wrong tileSet ?
				}
			}

			// Setup corner
			switch (border)
			{
			case CTileSet::_1111:
			case CTileSet::_1110:
			case CTileSet::_1000:
				pVoisinCorner[(i+backup.getLayer(nL).Rotate)&3].TileSet=tileSet;
				pVoisinCorner[(i+backup.getLayer(nL).Rotate)&3].Rotate=index.Rotate;
				break;
			}

			// Store border
			pBorder[(i+backup.getLayer(nL).Rotate)&3][nL]=border;
		}
	}

	pVoisinIndex->setTile (nLayer, backup.getCase(), backup.getDisplace(), pIndexx[0], pIndexx[1], pIndexx[2]);

	return true;
}

/*-------------------------------------------------------------------*/

const CTileSetTransition* EPM_PaintMouseProc::FindTransition (int nTileSet, int nRotate, const CTileSet::TFlagBorder *border, const CTileBank& bank)
{
	// Convert border to tile format
	CTileSet::TFlagBorder pBorderConverted[4];
	for (int i=0; i<4; i++)
	{
		CTileSet::TBorder toBorder[4]={CTileSet::left, CTileSet::bottom, CTileSet::right, CTileSet::top};
		pBorderConverted[i]=CTileSet::getOrientedBorder (toBorder[i], border[(i+nRotate)&3]);
	}

	// Look for good tile..
	CTileSet::TTransition nTransition=CTileSet::getTransitionTile
		(pBorderConverted[3], pBorderConverted[1], pBorderConverted[0], pBorderConverted[2]);
	//nlassert (nTransition!=CTileSet::notfound);
	if (nTransition==CTileSet::notfound)
		return NULL;

	// Tile description
	return bank.getTileSet (nTileSet)->getTransition (nTransition);
}

/*-------------------------------------------------------------------*/

int EPM_PaintMouseProc::getLayer (EPM_PaintTile* tile, int border, int tileSet, int rotate, std::vector<EPM_Mesh>& vectMesh, CLandscape *land)
{
	int nLayer=-1;
	tileDesc desc;
	GetTile (tile->voisins[border]->Mesh, tile->voisins[border]->tile, desc, vectMesh, land);
	for (int o=0; o<desc.getNumLayer(); o++)
	{
		tileIndex index=desc.getLayer(o);
		index.Rotate-=tile->rotate[border];
		index.Rotate&=3;

		CTileBank::TTileType type;
		int TileSet, number;

		if ((int)index.Tile<bank.getTileCount())
		{
			bank.getTileXRef (index.Tile, TileSet, number, type);
			if ((TileSet==tileSet)&&(index.Rotate==rotate))
				nLayer=o;
		}
	}

	return nLayer;
}

/*-------------------------------------------------------------------*/

bool EPM_PaintMouseProc::PropagateBorder (EPM_PaintTile* tile, int curRotation, int curTileSet, std::set<EPM_PaintTile*>& visited,
										  const CTileBank& bank, std::vector<EPM_Mesh>& vectMesh,
										  CLandscape* land, CNelPatchChanger& nelPatchChg, std::vector<CBackupValue>& backupStack, bool recurseNoDiff)
{
	// 1) Already visited
	if (visited.find (tile)!=visited.end())
		return true;

	// Frozen ?
	/* if (vectMesh[tile->Mesh].Node->IsFrozen())
		return true;*/

	// Big trick
	if (pobj->TileTrick)
		return true;

	// 3) Backup tile
	tileDesc backup;
	GetTile (tile->Mesh, tile->tile, backup, vectMesh, land);

	// 2) Tile empty
	if (backup.isEmpty())
		return true;

	// *** Clip select patch
	// Patch number
	int patch=tile->tile/NUM_TILE_SEL;

	// Check if we are in patch subobject and if this patch is selected
	if ((vectMesh[tile->Mesh].PMesh->selLevel==EP_PATCH)&&(!vectMesh[tile->Mesh].PMesh->patchSel[patch]))
		return false;

	// 3) Add to visited tiles
	visited.insert (tile);

	// 4) 256 ?
	bool _256=(backup.getCase()>0);

	// Corner type
	bool bModified[4]=
	{
		false,
		false,
		false,
		false
	};
	// Corner type
	bool bTouched[4]=
	{
		false,
		false,
		false,
		false
	};
	bool bSameEdge[4]=
	{
		true,
		true,
		true,
		true
	};
	bool bVisited[4]=
	{
		false,
		false,
		false,
		false
	};
	int extraOrdinary[4]=
	{
		0,
		0,
		0,
		0
	};
	int extraOrdinarySmallEdge[4]=
	{
		0,
		0,
		0,
		0
	};
	tileSetIndex nCorner[4];
	int i;
	for (i=0; i<4; i++)
		nCorner[i].TileSet=-1;
	CTileSet::TFlagBorder nBorder[4][3];
	tileDesc pIndex;
	bool bFill=GetBorderDesc (tile, nCorner, nBorder, &pIndex, bank, vectMesh, nelPatchChg, land);
	//nlassert (bFill);	// Problem in tile info. Wrong tileSet ?
	if (!bFill)
	{
		WarningInvalidTileSet ();
		return false;
	}
	bool bDiff=false;

	// For each voisin
	int v;
	for (v=0; v<4; v++)
	{
		// Voisin already visited ?
		if (tile->voisins[v])
		{
			// ok.. already visited, so copy border
			tileSetIndex pVoisinCorner[4];
			CTileSet::TFlagBorder pBorder[4][3];
			tileDesc pVoisinIndex;
			bFill=GetBorderDesc (tile->voisins[v], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land);
			if (bFill)
			{
				int edge=(2+v+tile->rotate[v])&3;

				// Already visited or frozen ?
				if ((visited.find (tile->voisins[v])!=visited.end()) /*|| (vectMesh[tile->voisins[v]->Mesh].Node->IsFrozen())*/)
				{
					bVisited[v]=true;
					pVoisinCorner[(edge+1)&3].Rotate-=tile->rotate[v];
					pVoisinCorner[(edge+1)&3].Rotate&=3;
					pVoisinCorner[edge].Rotate-=tile->rotate[v];
					pVoisinCorner[edge].Rotate&=3;
					if (bTouched[v])
					{
						if (nCorner[v]!=pVoisinCorner[(edge+1)&3])
						{
							// Check if it is a valid corner
							int delta=(pVoisinCorner[(edge+1)&3].Rotate-nCorner[v].Rotate)&3;
							if (delta != 2)
							{
								if (nCorner[v]<pVoisinCorner[(edge+1)&3])
								{
									nCorner[v]=pVoisinCorner[(edge+1)&3];
									extraOrdinarySmallEdge[v]=(v-1)&3;
								}
								else
									extraOrdinarySmallEdge[v]=v;
							}
							else
							{
								// Bad corner
								return false;
							}
							extraOrdinary[v]++;
							bDiff=true;
						}
					}
					else
					{
						if (nCorner[v]!=pVoisinCorner[(edge+1)&3])
						{
							nCorner[v]=pVoisinCorner[(edge+1)&3];
							bDiff=true;
							bModified[v]=true;
						}
						bTouched[v]=true;
					}
					int nNextCorner=(v+1)&3;
					if (bTouched[nNextCorner])
					{
						if (nCorner[nNextCorner]!=pVoisinCorner[edge])
						{
							// Check if it is a valid corner
							int delta=(pVoisinCorner[edge].Rotate-nCorner[nNextCorner].Rotate)&3;
							if (delta != 2)
							{
								if (nCorner[nNextCorner]<pVoisinCorner[edge])
								{
									nCorner[nNextCorner]=pVoisinCorner[edge];
									extraOrdinarySmallEdge[nNextCorner]=nNextCorner;
								}
								else
									extraOrdinarySmallEdge[nNextCorner]=v;
							}
							else
							{
								// Bad corner
								return false;
							}
							extraOrdinary[nNextCorner]++;
							bDiff=true;
						}
					}
					else
					{
						if (nCorner[nNextCorner]!=pVoisinCorner[edge])
						{
							nCorner[nNextCorner]=pVoisinCorner[edge];
							bDiff=true;
							bModified[nNextCorner]=true;
						}
						bTouched[nNextCorner]=true;
					}
				}
			}
		}
	}

	// Frozen ?
	bool _isLocked = isLocked (pobj, tile);

	// Force to visite tile in the same 256 in 256 mode..
	if (_256 && !tile->frozen)
	{
		// Case number
		int nCase=backup.getCase()-1;
		int nRotate=backup.getLayer(0).Rotate;
		nlassert (nCase>=0);
		nlassert (nCase<4);

		// Flag the voisin to force the visite
		EPM_PaintTile* other=tile->voisins[(1+nCase+nRotate)&3];
		if (other)
		{
			int	rot=tile->rotate[(1+nCase+nRotate)&3];
			tileDesc desc1;
			GetTile (other->Mesh, other->tile, desc1, vectMesh, land);
			if (!desc1.isEmpty())
			{
				if (
					(desc1.isEmpty())||
					(desc1.getCase()!=(1+((nCase+1)&3)))||
					(desc1.getLayer(0).Tile!=backup.getLayer(0).Tile)||
					(desc1.getLayer(0).Rotate!=((backup.getLayer(0).Rotate-rot)&3))
					)
					bDiff=true;
			}
			else
				bDiff=true;

			if (isLocked (pobj, other))
				_isLocked = true;
		}

		other=tile->voisins[(2+nCase+nRotate)&3];
		if (other)
		{
			int rot=tile->rotate[(2+nCase+nRotate)&3];
			tileDesc desc1;
			GetTile (other->Mesh, other->tile, desc1, vectMesh, land);
			if (!desc1.isEmpty())
			{
				if (
					(desc1.isEmpty())||
					(desc1.getCase()!=(1+((nCase+3)&3)))||
					(desc1.getLayer(0).Tile!=backup.getLayer(0).Tile)||
					(desc1.getLayer(0).Rotate!=((backup.getLayer(0).Rotate-rot)&3))
					)
					bDiff=true;
			}
			else
				bDiff=true;

			if (isLocked (pobj, other))
				_isLocked = true;
		}
	}

	// C) Invalide tile (same tile and rotation only on the diagonal)
	for (i=0; i<2; i++)
	{
		if ((nCorner[i]==nCorner[(i+2)&3])&&
			(nCorner[(i+1)&3]!=nCorner[(i+2)&3])&&
			(nCorner[(i+3)&3]!=nCorner[(i+2)&3]))
		{
			return false;
		}
	}

	// Same edge ?
	for (v=0; v<4; v++)
	{
		if (tile->voisins[v]==NULL)
			bSameEdge[v]=false;
		else
		{
			tileDesc desc;
			GetTile (tile->voisins[v]->Mesh, tile->voisins[v]->tile, desc, vectMesh, land);
			if (bModified[v]||
				bModified[(v+1)&3]||
				(desc.isEmpty()))
				bSameEdge[v]=false;
		}
	}

	if ((!bDiff)&&pIndex.getNumLayer()==1)
		return true;

	// Frozen or locked ?
	if (tile->frozen || _isLocked)
		return false;

	// Count materiaux
	//set<tileSetIndex> setIndex;
	std::vector<tileSetIndex> setIndex;

	// Set count
	for (v=0; v<4; v++)
	{
		nlassert (nCorner[v].TileSet!=-1);
		// B) Check for same tile with a +2 rotation
		bool bFind=false;

		for (int vv=0; vv<(int)setIndex.size(); vv++)
		{
			if (setIndex[vv].TileSet==nCorner[v].TileSet)
			{
				tileSetIndex complet=nCorner[v];
				complet.Rotate=(complet.Rotate+2)&3;
				if (setIndex[vv].Rotate==complet.Rotate)
					return false;
				if (nCorner[v]==setIndex[vv])
					bFind=true;
			}
		}

		// no, ok push it back.
		if (!bFind)
			setIndex.push_back (nCorner[v]);
	}

	std::sort (setIndex.begin(), setIndex.end());

	// Check validity

	// A) Check for more than 3 materials
	if (setIndex.size()>3)
		return false;

	// Count materiaux
	std::vector<tileSetIndex>::iterator ite=setIndex.begin();
	tileIndex finalIndex[3];
	finalIndex[0].Tile=0;
	finalIndex[1].Tile=0;
	finalIndex[2].Tile=0;

	// Look for a tile resolving the constraints
	// Fill ?
	for (int l=0; l<(int)setIndex.size(); l++)
	{
		if (l==0)
		{
			// Filled, choose a random tile

			// Get tge tileSet
			// TileSet
			const CTileSet *TileSet=bank.getTileSet (ite->TileSet);

			// Select group
			int group=0;
			int nTile=-1;

			if (backup.getNumLayer ()==1)
			{
				// Get information about the tileSet
				int tileSet;
				int number;
				CTileBank::TTileType type;

				// Get XRef
				bank.getTileXRef (backup.getLayer (0).Tile, tileSet, number, type);

				// Check if it is the same tileSet
				if (tileSet==ite->TileSet)
				{
					// Get the group flags
					uint flags=bank.getTile (backup.getLayer (0).Tile)->getGroupFlags ();
					for (int f=0; f<32; f++)
					{
						// Group ?
						if (flags&(1<<f))
						{
							// Try to get a tile from this group
							nTile=selectTile (ite->TileSet, false, false, f+1, bank);

							// Find a tile ?
							if (nTile!=-1)
								break;
						}
					}
				}
			}

			// A tile as been found ?
			if (nTile==-1)
				// Look for a tile without group
				nTile=selectTile (ite->TileSet, false, false, 0, bank);

			// Set the tile slot
			finalIndex[0].Tile=nTile;
			finalIndex[0].Rotate=(ite->Rotate&3);
		}
		else
		{
			// Transition, choose the good tile

			// Get tge tileSet
			// TileSet
			// Borders
			CTileSet::TFlagBorder border[4];

			bool bFilled[4];
			int c;
			for (c=0; c<4; c++)
				bFilled[c]=!(nCorner[c]<*ite);

			for (c=0; c<4; c++)
			{
				if (bFilled[c])
				{
					if (bFilled[(c+1)&3])
						border[c]=CTileSet::_1111;
					else
					{
						// If extraordinary vertex and the small edge
						if ((extraOrdinary[c])&&(extraOrdinarySmallEdge[c]==c))
						{
							border[c]=CTileSet::_1000;
						}
						// If 3 mat, use a 3/4 border
						else if (setIndex.size()==3)
						{
							CTileSet::TFlagBorder wanted;
							CTileSet::TFlagBorder invWanted;

							// Last on the stack ?
							if (*ite<nCorner[c])
							{
								// no,
								wanted=CTileSet::_1000;
								invWanted=CTileSet::_1110;
							}
							else
							{
								// yes,
								wanted=CTileSet::_1110;
								invWanted=CTileSet::_1000;
							}
							border[c]=wanted;

							// If voisin already visited, force his transition to 3/4
							if (tile->voisins[c])
							{
								tileSetIndex pVoisinCorner[4];
								CTileSet::TFlagBorder pBorder[4][3];
								tileDesc pVoisinIndex;
								if (GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land))
								{
									// Find the good layer in the dest tileDesc
									int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);

									if (nLayer!=-1)
									{
										int edge=(2+c+tile->rotate[c])&3;
										if (pBorder[edge][nLayer]==CTileSet::getInvertBorder (invWanted))
										{
											pBorder[edge][nLayer]=CTileSet::getInvertBorder (wanted);

											// Voisin tile set
											CTileSet::TFlagBorder newBorder[4];
											for (int nB=0; nB<4; nB++)
												newBorder[nB]=pBorder[nB][nLayer];

											const CTileSetTransition* pTile=FindTransition (ite->TileSet, ite->Rotate+tile->rotate[c], newBorder, bank);
											if (!pTile)
											{
												WarningInvalidTileSet ();
												return false;
											}
											pVoisinIndex.getLayer(nLayer).Tile=pTile->getTile();

											// Is frozen ?
											if (tile->voisins[c]->frozen)
												// Yes, can't change it!
												return false;

											// Is locked ?
											if (isLocked (pobj, tile->voisins[c]))
												return false;

											// Set the tile..
											SetTile (tile->voisins[c]->Mesh, tile->voisins[c]->tile, pVoisinIndex, vectMesh,
												land, nelPatchChg, &backupStack);
										}
										/*if (pBorder[edge][nLayer]==CTileSet::_0001)
											border[c]=CTileSet::_1000;*/
									}
									/*else
										border[c]=CTileSet::_1000;*/
								}
							}
						}
						// Normal,
						else
						{
							// Voisin visited or frozen ?
							if (tile->voisins[c]&&((visited.find (tile->voisins[c])!=visited.end()) /*||
								(vectMesh[tile->voisins[c]->Mesh].Node->IsFrozen())*/ ))
							{
								// Yes, visited. Copy the border
								tileSetIndex pVoisinCorner[4];
								CTileSet::TFlagBorder pBorder[4][3];
								tileDesc pVoisinIndex;
								bool bOk=GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land);

								// Should not be empty
								nlassert (bOk);
								int edge=(2+c+tile->rotate[c])&3;

								// Should have one of the following transition
								int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);
								if (nLayer!=-1)
								{
									nlassert ((pBorder[edge][nLayer]==CTileSet::_0111)||(pBorder[edge][nLayer]==CTileSet::_0001));
								}

								// Copy inverted!
								border[c]=CTileSet::getInvertBorder (pBorder[edge][nLayer]);
							}
							else
							{
								// No, not yet visited

								// Choose transition by random
								bool bComputed=false;
								if ((bVisited[c]||!recurseNoDiff)&&bSameEdge[c])
								{
									bSameEdge[c]=false;
									// Yes, visited. Copy the border of the voisin
									tileSetIndex pVoisinCorner[4];
									CTileSet::TFlagBorder pBorder[4][3];
									tileDesc pVoisinIndex;
									bool bOk=GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land);

									// ok voisin is here ?
									if (bOk)
									{
										// edge in the voisin
										int edge=(2+c+tile->rotate[c])&3;

										// layer where to find transition in the voisin
										int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);

										// transition ok?
										if ((pBorder[edge][nLayer]==CTileSet::_0111)||(pBorder[edge][nLayer]==CTileSet::_0001))
										{
											// copy!
											border[c]=CTileSet::getInvertBorder (pBorder[edge][nLayer]);
											bSameEdge[c]=true;
											bComputed=true;
										}
									}
								}
								// No transition computed, random
								if (!bComputed)
								{
									bSameEdge[c]=false;
									if (rand()&1)
										border[c]=CTileSet::_1000;
									else
										border[c]=CTileSet::_1110;
								}
							}
						}
					}
				}
				else
				{
					int nNextCorner=(c+1)&3;
					if (!bFilled[nNextCorner])
						border[c]=CTileSet::_0000;
					else
					{
						// If extraordinary vertex and the small edge
						if ((extraOrdinary[nNextCorner])&&(extraOrdinarySmallEdge[nNextCorner]==c))
						{
							border[c]=CTileSet::_0001;
						}
						// If 3 mat, use a 3/4 border
						else if (setIndex.size()==3)
						{
							CTileSet::TFlagBorder wanted;
							CTileSet::TFlagBorder invWanted;

							// Last on the stack ?
							if (*ite<nCorner[c])
							{
								// no,
								wanted=CTileSet::_0001;
								invWanted=CTileSet::_0111;
							}
							else
							{
								// yes,
								wanted=CTileSet::_0111;
								invWanted=CTileSet::_0001;
							}
							border[c]=wanted;

							// If voisin already visited, force his transition to 3/4
							if (tile->voisins[c])//&&visited.find (tile->voisins[c])!=visited.end())
							{
								tileSetIndex pVoisinCorner[4];
								CTileSet::TFlagBorder pBorder[4][3];
								tileDesc pVoisinIndex;
								if (GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land))
								{
									// Find the good layer in the dest tileDesc
									int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);

									if (nLayer!=-1)
									{
										int edge=(2+c+tile->rotate[c])&3;
										if (pBorder[edge][nLayer]==CTileSet::getInvertBorder (invWanted))
										{
											pBorder[edge][nLayer]=CTileSet::getInvertBorder (wanted);

											// Voisin tile set
											CTileSet::TFlagBorder newBorder[4];
											for (int nB=0; nB<4; nB++)
												newBorder[nB]=pBorder[nB][nLayer];

											const CTileSetTransition* pTile=FindTransition (ite->TileSet, ite->Rotate+tile->rotate[c], newBorder, bank);
											if (!pTile)
											{
												WarningInvalidTileSet ();
												return false;
											}
											pVoisinIndex.getLayer(nLayer).Tile=pTile->getTile();

											// Is frozen ?
											if (tile->voisins[c]->frozen)
												// Yes, can't change it!
												return false;

											// Is locked ?
											if (isLocked (pobj, tile->voisins[c]))
												return false;

											// Set the tile..
											SetTile (tile->voisins[c]->Mesh, tile->voisins[c]->tile, pVoisinIndex, vectMesh,
												land, nelPatchChg, &backupStack);
										}
										/*if (pBorder[edge][nLayer]==CTileSet::_1000)
											border[c]=CTileSet::_0001;*/
									}
									/*else
										border[c]=CTileSet::_0001;*/
								}
							}

						}
						// Normal,
						else
						{
							// Voisin visited ?
							if ((tile->voisins[c])&& ((visited.find (tile->voisins[c])!=visited.end()) /*|| (vectMesh[tile->voisins[c]->Mesh].Node->IsFrozen())*/) )
							{
								// Yes, visited. Copy the border
								tileSetIndex pVoisinCorner[4];
								CTileSet::TFlagBorder pBorder[4][3];
								tileDesc pVoisinIndex;
								bool bOk=GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land);

								// Should not be empty
								nlassert (bOk);
								int edge=(2+c+tile->rotate[c])&3;

								// Should have one of the following transition
								// Find the layer of the tile..
								int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);
								if (nLayer!=-1)
								{
									nlassert ((pBorder[edge][nLayer]==CTileSet::_1110)||(pBorder[edge][nLayer]==CTileSet::_1000));
								}

								// Copy inverted!
								border[c]=CTileSet::getInvertBorder (pBorder[edge][nLayer]);
							}
							else
							{
								// No, not yet visited

								// Choose transition by random
								bool bComputed=false;
								if ((bVisited[c]||!recurseNoDiff)&&bSameEdge[c])
								{
									bSameEdge[c]=false;
									// Yes, visited. Copy the border of the voisin
									tileSetIndex pVoisinCorner[4];
									CTileSet::TFlagBorder pBorder[4][3];
									tileDesc pVoisinIndex;
									bool bOk=GetBorderDesc (tile->voisins[c], pVoisinCorner, pBorder, &pVoisinIndex, bank, vectMesh, nelPatchChg, land);

									// ok voisin is here ?
									if (bOk)
									{
										// edge in the voisin
										int edge=(2+c+tile->rotate[c])&3;

										// layer where to find transition in the voisin
										int nLayer=getLayer (tile, c, ite->TileSet, ite->Rotate, vectMesh, land);

										// transition ok?
										if ((pBorder[edge][nLayer]==CTileSet::_1110)||(pBorder[edge][nLayer]==CTileSet::_1000))
										{
											// copy!
											border[c]=CTileSet::getInvertBorder (pBorder[edge][nLayer]);
											bSameEdge[c]=true;
											bComputed=true;
										}
									}
								}
								// No transition computed, random
								if (!bComputed)
								{
									bSameEdge[c]=false;
									if (rand()&1)
										border[c]=CTileSet::_0001;
									else
										border[c]=CTileSet::_0111;
								}
							}
						}
					}
				}
			}

			// ok, find the good transition..
			const CTileSetTransition* pTile=FindTransition (ite->TileSet, ite->Rotate, border, bank);
			if (!pTile)
			{
				WarningInvalidTileSet ();
				return false;
			}
			finalIndex[l].Rotate=ite->Rotate;
			finalIndex[l].Tile=pTile->getTile();
		}
		ite++;
	}

	// Set the border desc
	tileDesc desc;
	GetTile (tile->Mesh, tile->tile, desc, vectMesh, land);
	switch (setIndex.size())
	{
	case 1:
		desc.setTile (1, 0, desc.getDisplace (), finalIndex[0], tileIndex (0,0), tileIndex (0,0));
		break;
	case 2:
		desc.setTile (2, 0, desc.getDisplace (), finalIndex[0], finalIndex[1], tileIndex (0,0));
		break;
	case 3:
		desc.setTile (3, 0, desc.getDisplace (), finalIndex[0], finalIndex[1], finalIndex[2]);
		break;
	default:
		nlassert (0);		// no!
		break;
	}
	SetTile (tile->Mesh, tile->tile, desc, vectMesh, land, nelPatchChg, &backupStack);

	// Force to visite tile in the same 256 in 256 mode..
	if (_256)
	{
		// Case number
		int nCase=backup.getCase()-1;
		int nRotate=backup.getLayer(0).Rotate;
		nlassert (nCase>=0);
		nlassert (nCase<4);

		// Flag the voisin to force the visite
		bSameEdge[(1+nCase+nRotate)&3]=false;
		bSameEdge[(2+nCase+nRotate)&3]=false;
	}

	// ** For all voisin tiles not visited on a border with modified corners :

	// Idem ?
	if ((!bDiff)&&(!recurseNoDiff))
		return true;

	bool bContinue=true;

	// For each voisin
	for (v=0; v<4; v++)
	{
		// Voisin not already visited and not frozen ?
		if ((tile->voisins[v]) && (visited.find (tile->voisins[v])==visited.end()) /* && (vectMesh[tile->voisins[v]->Mesh].Node->IsFrozen()==0)*/)
		{
			// ok.. not visited, border with modified corner ?
			if (bModified[v]||bModified[(v+1)&3]||(!bSameEdge[v]))
			if (!PropagateBorder (tile->voisins[v], (tile->rotate[v]+curRotation)&3, curTileSet, visited, bank, vectMesh, land, nelPatchChg,
									backupStack, false))
			{
				bContinue=false;
				break;
			}
		}
	}
	if (!bContinue)
	{
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------*/

uint8 EPM_PaintMouseProc::CalcRotPath (EPM_PaintTile* from, EPM_PaintTile* to, int depth, int rotate, int& deltaX, int& deltaY, int& cost)
{
	static const int x[4]={-1, 0, 1, 0};
	static const int y[4]={0, 1, 0, -1};
	if (from==to)
	{
		cost=0;
		deltaX=0;
		deltaY=0;
		return 0;
	}
	if (depth>0)
	{
		uint8 ret=0xff;
		cost=1000000;
		int best;
		for (int i=0; i<4; i++)
		{
			if (from->voisins[i])
			{
				int myDeltaX;
				int myDeltaY;
				int myCost;
				int myRet=CalcRotPath (from->voisins[i], to, depth-1, (from->rotate[i]+rotate)&3, myDeltaX, myDeltaY, myCost);
				if (myRet!=0xff)
				{
					myDeltaX+=x[(i+rotate)&3];
					myDeltaY+=y[(i+rotate)&3];
					myCost++;
					if (myCost<cost)
					{
						cost=myCost;
						deltaX=myDeltaX;
						deltaY=myDeltaY;
						best=i;
						ret=myRet;
					}
				}
			}
		}
		if (ret!=0xff)
			return (from->rotate[best]+ret)&3;
	}
	return 0xff;
}

/*-------------------------------------------------------------------*/

static TModePaint nModeTexture=ModeTile;
static TModeMouse modeSelect=ModePaint;

void EPM_PaintMouseProc::RecursTile (EPM_PaintTile* pTile, const CTileBank& bank, int tileSet, std::vector<EPM_Mesh>& vectMesh, CLandscape* land,
									 int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, bool first, int rotation,
									 CNelPatchChanger& nelPatchChg, bool _256)
{
	if (alreadyRecursed.find (pTile)==alreadyRecursed.end())
	{
		alreadyRecursed.insert (pTile);
		std::set<EPM_PaintTile*> visited;

		// Mode displace ?
		if (nModeTexture==ModeDisplace)
			PutADisplacetile ( pTile, bank, vectMesh, land, nelPatchChg);
		else
			PutATile ( pTile, tileSet, rotation, bank, first, visited, vectMesh, land, nelPatchChg, _256);
	}

	// Call fill
	if (recurs>0)
	{
		for (int i=0; i<4; i++)
		{
			if (_256)
			{
				if (pTile->get2Voisin(i))
					RecursTile (pTile->get2Voisin(i), bank, tileSet, vectMesh, land, recurs-2, alreadyRecursed, false,
						(rotation+pTile->get2VoisinRotate(i))&3, nelPatchChg, true);
			}
			else
			{
				if (pTile->voisins[i])
					RecursTile (pTile->voisins[i], bank, tileSet, vectMesh, land, recurs-1, alreadyRecursed, false, (rotation+pTile->rotate[i])&3,
						nelPatchChg, false);
			}
		}
	}
}

BOOL EPM_PaintMouseProc::PutDisplace (int tile, int mesh, const CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, CLandscape* land,
								  int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, CNelPatchChanger& nelPatchChg)
{
	static sint32 MeshOld=-1;
	static int	tileOld=-1;
	bool lost=false;

	EPM_PaintTile* pTile=&metaTile[mesh][tile];

	// Ok recurse tiles
	RecursTile (pTile, bank, 0, vectMesh, land, brushValue[PaintPatchMod::brushSize], alreadyRecursed, true,
		EPM_PaintMouseProc::Rotation, nelPatchChg, false);

	MeshOld=mesh;
	tileOld=tile;

	return TRUE;
}

BOOL EPM_PaintMouseProc::PutTile (int tile, int mesh, bool first, const CTileBank& bank, int tileSet, std::vector<EPM_Mesh>& vectMesh, CLandscape* land,
								  int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, CNelPatchChanger& nelPatchChg, bool _256)
{
	static sint32 MeshOld=-1;
	static int	tileOld=-1;
	bool lost=false;

	EPM_PaintTile* pTile=&metaTile[mesh][tile];

	alreadyRecursed.insert (pTile);
	if (first)
	{
		tileDesc desc;
		GetTile (pTile->Mesh, pTile->tile, desc, vectMesh, land);
		if (desc.isEmpty ())
			EPM_PaintMouseProc::Rotation=0;
		else
		{
			int i;
			for (i=0; i<desc.getNumLayer(); i++)
			{
				if ((sint)desc.getLayer (i).Tile==tile)
				{
					EPM_PaintMouseProc::Rotation=desc.getLayer (i).Rotate;
					break;
				}
			}
			if (i==desc.getNumLayer())
				EPM_PaintMouseProc::Rotation=desc.getLayer (0).Rotate;
		}
	}
	else
	{
		EPM_PaintTile* pOldTile=&metaTile[MeshOld][tileOld];
		int deltaX;
		int deltaY;
		int cost;
		uint8 deltaRot=CalcRotPath (pOldTile, pTile, DEPTH_SEARCH_MAX, 0, deltaX, deltaY, cost);

		if (deltaRot!=0xff)
		{
			EPM_PaintMouseProc::Rotation+=deltaRot;
			EPM_PaintMouseProc::Rotation&=3;
		}
		else
			lost=true;
	}

	if (!lost)
	{
		{
			std::set<EPM_PaintTile*> alreadyRecursed;

			// Ok recurse tiles
			RecursTile (pTile, bank, tileSet, vectMesh, land, brushValue[PaintPatchMod::brushSize], alreadyRecursed, first,
				EPM_PaintMouseProc::Rotation, nelPatchChg, _256);
		}
		MeshOld=mesh;
		tileOld=tile;
	}

	return TRUE;
}

/*-------------------------------------------------------------------*/

int getOffset (int edge, int nU, int nV, bool symmetry)
{
	switch ((edge+(symmetry?1:0))&3)
	{
	case 0:
		return 0;
	case 1:
		return (nV-1)*MAX_TILE_IN_PATCH;
	case 2:
		return (nV-1)*MAX_TILE_IN_PATCH+nU-1;
	case 3:
		return nU-1;
	}
	nlassert (0);	//no!
	return 0;
}
/*int offset[4]={ 0, (nV-1)*MAX_TILE_IN_PATCH, (nV-1)*MAX_TILE_IN_PATCH+nU-1, nU-1 };
int offsetOther[4]={ 0, (nVOther-1)*MAX_TILE_IN_PATCH, (nVOther-1)*MAX_TILE_IN_PATCH+nUOther-1, nUOther-1 };*/

struct callThread
{
	callThread (std::vector<EPM_Mesh>&	vectMesh) : VectMesh (vectMesh) {}
	EPM_PaintMouseProc		*eproc;
	PaintPatchMod			*pobj;
	CVector					center;
	TimeValue				T;
	std::vector<EPM_Mesh>&	VectMesh;
};

void	mainproc(CScene& scene, CEventListenerAsync& AsyncListener, CEvent3dMouseListener& mouseListener, CLandscapeModel& landscape, IDriver& driver, callThread *pData, CPaintColor &paintColor)
{
	// Default mode is paint
	modeSelect=ModePaint;

	// Mode selection
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Select]))
		// Set mode
		modeSelect=ModeSelect;

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Pick]))
		// Set mode
		modeSelect=ModePick;

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[GetState]))
		// Set mode
		modeSelect=ModeGetState;

	// Mode reset zone
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[ResetPatch]))
	{
		// Set mode
		modeSelect=ModeResetPatch;
	}

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Fill0]))
	{
		// Set mode
		modeSelect=ModeFill;

		// Set fill rotation
		pData->pobj->TileFillRotation=0;
	}

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Fill1]))
		{
		// Set mode
		modeSelect=ModeFill;

		// Set fill rotation
		pData->pobj->TileFillRotation=1;
	}

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Fill2]))
		{
		// Set mode
		modeSelect=ModeFill;

		// Set fill rotation
		pData->pobj->TileFillRotation=2;
	}

	// Mode picking
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Fill3]))
		{
		// Set mode
		modeSelect=ModeFill;

		// Set fill rotation
		pData->pobj->TileFillRotation=3;
	}

	// Choose a color
	if ((AsyncListener.isKeyPushed ((TKey)PainterKeys[Select]))&&(nModeTexture==ModeColor))
		chooseAColor ();

	// Mode tile ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[MModeTile]))
		nModeTexture=ModeTile;

	// Mode color ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[MModeColor]))
		nModeTexture=ModeColor;

	// Mode displace ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[MModeDisplace]))
		nModeTexture=ModeDisplace;

	// Toggle couleur ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[ToggleColor]))
	{
		if (nModeTexture==ModeColor)
		{
			COLORREF tmp=color1;
			color1=color2;
			color2=tmp;
			float tmpF=opa1;
			opa1=opa2;
			opa2=tmpF;
			tmpF=hard1;
			hard1=hard2;
			hard2=tmpF;
		}
	}

	// Change brush ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[SizeDown]))
	{
		if ((nModeTexture==ModeTile)||(nModeTexture==ModeDisplace))
		{
			pData->pobj->brushSize--;
			if (pData->pobj->brushSize<0)
				pData->pobj->brushSize=0;
		}
		else if (nModeTexture==ModeColor)
		{
			pData->pobj->ColorBushSize--;
			if (pData->pobj->ColorBushSize<0)
				pData->pobj->ColorBushSize=0;
		}
	}

	// Change brush ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[SizeUp]))
	{
		if ((nModeTexture==ModeTile)||(nModeTexture==ModeDisplace))
		{
			pData->pobj->brushSize++;
			if (pData->pobj->brushSize>2)
				pData->pobj->brushSize=2;
		}
		else if (nModeTexture==ModeColor)
		{
			pData->pobj->ColorBushSize++;
			if (pData->pobj->ColorBushSize>COLOR_BRUSH_STEP)
				pData->pobj->ColorBushSize=COLOR_BRUSH_STEP;
		}
	}

	// Change size of tile ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[ToggleTileSize]))
	{
		pData->pobj->tileSize++;
		pData->pobj->tileSize&=1;
	}

	// Change group ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[GroupUp]))
	{
		pData->pobj->TileGroup++;
		if (pData->pobj->TileGroup>NL3D_CTILE_NUM_GROUP)
			pData->pobj->TileGroup=0;
	}

	// Change group ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[GroupDown]))
	{
		pData->pobj->TileGroup--;
		if (pData->pobj->TileGroup<0)
			pData->pobj->TileGroup=NL3D_CTILE_NUM_GROUP;
	}

	// Change background color ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[BackgroundColor]))
	{
		setBackgroundColor ();
	}

	// Toggle arrows ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[ToggleArrows]))
	{
		// Toggle
		pData->pobj->additiveTile^=true;

		// Go
		if (pData->pobj->additiveTile)
		{
			// Add a additive tile...
			int i;
			for (i=0; i<landscape.Landscape.TileBank.getTileCount(); i++)
			{
				landscape.Landscape.TileBank.getTile(i)->setFileName (CTile::additive, "arrow.tga");
				landscape.Landscape.releaseTiles(i, 1);
			}
		}
		else
		{
			// Copy original bank
			landscape.Landscape.TileBank = bank;

			// Add a additive tile...
			int i;
			for (i=0; i<landscape.Landscape.TileBank.getTileCount(); i++)
			{
				landscape.Landscape.releaseTiles(i, 1);
			}
		}

		// Touch all patches
		for (uint zone=0; zone<pData->VectMesh.size(); zone++)
		{
			// Get the zone
			CZone *pZone=landscape.Landscape.getZone (zone);
			if (pZone)
			{
				// For each patch
				uint numPatch=pZone->getNumPatchs();
				for (uint patch=0; patch<numPatch; patch++)
				{
					// Invalidate this patch
					pZone->changePatchTextureAndColor (patch, NULL, NULL);
					//pZone->refreshTesselationGeometry (patch);
				}
			}
		}
	}

	// Toggle automatic lighting ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[AutomaticLighting]))
	{
		// Toggle
		pData->pobj->automaticLighting^=true;

		// Enable / disable automatic lighting
		landscape.Landscape.enableAutomaticLighting (pData->pobj->automaticLighting);

		// Touch all patches
		for (uint zone=0; zone<pData->VectMesh.size(); zone++)
		{
			// Get the zone
			CZone *pZone=landscape.Landscape.getZone (zone);
			nlassert (pZone);

			// For each patch
			uint numPatch=pZone->getNumPatchs();
			for (uint patch=0; patch<numPatch; patch++)
			{
				// Invalidate this patch
				pZone->changePatchTextureAndColor (patch, NULL, NULL);
				pZone->refreshTesselationGeometry (patch);
			}
		}
	}


	// Toggle lock borders ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[LockBorders]))
	{
		// Toggle
		pData->pobj->lockBorders^=true;
	}

	// Select color brush ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[SelectColorBrush]))
	{
		// Create a dialog filter
		static TCHAR szFilter[] =
			_T("Targa Files (*.tga)\0*.tga\0")
			_T("All Files (*.*)\0*.*\0\0");

		// Filename buffer
		TCHAR buffer[65535];
		buffer[0]=0;

		// Fill the (big) struct
		OPENFILENAME openFile;
		memset (&openFile, 0, sizeof (OPENFILENAME));
		openFile.lStructSize = sizeof (OPENFILENAME);
		openFile.hwndOwner = (HWND)CNELU::Driver->getDisplay();
		openFile.lpstrFilter = szFilter;
		openFile.nFilterIndex = 0;
		openFile.lpstrFile = buffer;
		openFile.nMaxFile = 65535;
		openFile.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_EXPLORER;
		openFile.lpstrDefExt = _T("*.tga");

		// Open the dialog
		if (GetOpenFileName(&openFile))
		{
			// Load the file
			paintColor.loadBrush (MCharStrToUtf8(buffer));
			paintColor.setBrushMode (true);
		}
	}

	// Toggle brush mode ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[ToggleColorBrushMode]))
	{
		paintColor.setBrushMode (!paintColor.getBrushMode());
	}

	// Change hardness ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[HardnessUp]))
	{
		hard1+=0.2f;
		if (hard1>1.f)
			hard1=1.f;
	}

	// Change hardness ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[HardnessDown]))
	{
		hard1-=0.2f;
		if (hard1<0.f)
			hard1=0.f;
	}

	// Change opacity ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[OpacityUp]))
	{
		opa1+=0.2f;
		if (opa1>1.f)
			opa1=1.f;
	}

	// Change opacity ?
	if (AsyncListener.isKeyPushed ((TKey)PainterKeys[OpacityDown]))
	{
		opa1-=0.2f;
		if (opa1<0.f)
			opa1=0.f;
	}

	// Change hardness ?
	if (AsyncListener.isKeyDown ((TKey)PainterKeys[Zouille]))
		pData->pobj->TileTrick=true;
	else
		pData->pobj->TileTrick=false;

	// Set cursor
	if (modeSelect==ModeGetState)
		SetCursor (bankCont->HInspect);
	else if (modeSelect==ModePick)
		SetCursor (bankCont->HCur);
	else if (modeSelect==ModeFill)
		SetCursor (bankCont->HFill);
	else if (pData->pobj->TileTrick)
		SetCursor (bankCont->HTrick);
	else if (modeSelect==ModeResetPatch)
		SetCursor (LoadCursor (NULL, IDC_NO));
	else
		SetCursor (LoadCursor (NULL, IDC_ARROW));

	// Clear buffers
	CNELU::clearBuffers(CRGBA((uint8)(backGround&0xff), (uint8)((backGround>>8)&0xff), (uint8)((backGround>>16)&0xff)));

	if (modeSelect!=ModeSelect)
	{
		CVector	CameraRot(CVector::Null), CameraPos(0,0,0);


		// time mgt.
		static float t0= timeGetTime()*0.001f;
		static float t1;
		t1= timeGetTime()*0.001f;
		float	dt= t1-t0;

		// User Interaction.
		//==================
		// Speed.
		#define NSPEED	5
		static	float	Speed[NSPEED]= {1.66f, 3.33f, 8.33f, 40.0f, 200.0f};	// 6km/h, 12km/h, 30km/h, 144km/h, 700km/h.
		static	sint	idSpeed= 2;
		float	speed= Speed[idSpeed];
		float	rotSpeed= 80*(float)Pi/180;

		// Rot.
		if(AsyncListener.isKeyDown(KeyLEFT))		CameraRot.z+=rotSpeed*dt;
		if(AsyncListener.isKeyDown(KeyRIGHT))	CameraRot.z-=rotSpeed*dt;

		// Move.
		CVector	dir(0,speed*dt,0);
		CMatrix	mat;
		mat.identity();
		mat.rotateZ(CameraRot.z);
		mat.rotateX(CameraRot.x);
		dir= mat.mulVector(dir);
		if(AsyncListener.isKeyDown(KeyUP))		CameraPos+= dir;
		if(AsyncListener.isKeyDown(KeyDOWN))		CameraPos-= dir;

		/*if(AsyncListener.isKeyDown(KeyR))
		{
			CameraPos.set(0,0,0);
			CameraRot.set(0,0,0);
		}*/

		// Straff.
		float	straff=0;
		if(straff)
		{
			CMatrix	m;
			m.identity();
			m.rotateZ(CameraRot.z);
			m.rotateX(CameraRot.x);
			CameraPos+= m*CVector(straff,0,0);
		}

		CMatrix camKey;
		camKey.identity ();
		camKey.rotate(CameraRot, CMatrix::ZXY);

		// Zoom distance
		float zoom = 0;
		if (AsyncListener.isKeyDown ((TKey)PainterKeys[ZoomIn]))
			zoom -= ZoomSpeed * dt;
		if (AsyncListener.isKeyDown ((TKey)PainterKeys[ZoomOut]))
			zoom += ZoomSpeed * dt;

		// Add zoom
		CameraPos += (zoom * -camKey.getJ());

		camKey.setPos(CameraPos);

		camKey=mouseListener.getViewMatrix()*camKey;
		CNELU::Camera->setMatrix(camKey);
		mouseListener.setMatrix(camKey);

		// Render.
		//==================
		landscape.enableAdditive (true);

		scene.render();

		// first time: disable RefineMode, and compute all patchs tesselation.
		if(landscape.Landscape.getRefineMode())
		{
			landscape.Landscape.setRefineMode (false);
			landscape.Landscape.refineAll(camKey.getPos());
		}

		t0= t1;
	}

	// Draw interface
	drawInterface (modeSelect, nModeTexture, pData->pobj, driver, landscape, paintColor);

	CNELU::swapBuffers();
}

class MouseListener : public IEventListener
{
private:
	IObjParam*				_Ip;
	CCamera*				_Camera;
	CViewport*				_Viewport;
	PaintPatchMod*			_Pobj;
	EPM_PaintMouseProc*		_Eproc;
	CLandscape*				_Land;
	CEventListenerAsync*	_Async;
	CEvent3dMouseListener*	_3dMouseListener;
	CFillPatch				_FillTile;
	std::vector<EPM_Mesh>&	_VectMesh;
	TimeValue				_T;
public:
	bool					WindowActive;
	CPaintColor				PaintColor;
public:
	MouseListener (IObjParam *ip, CCamera *camera, CViewport *viewport, PaintPatchMod *pobj, EPM_PaintMouseProc *eproc, CLandscape* land,
					CEventListenerAsync* async, CEvent3dMouseListener* mouseList, std::vector<EPM_Mesh>& vectMesh, TimeValue t)
		: PaintColor (pobj, land, &bankCont->Undo, eproc), _FillTile (pobj, land, &bankCont->Undo, eproc), _VectMesh(vectMesh)
	{
		_Ip=ip;
		_Camera=camera;
		_Viewport=viewport;
		_Pobj=pobj;
		_Eproc=eproc;
		_Land=land;
		_Async=async;
		_3dMouseListener=mouseList;
		WindowActive=true;
		_T=t;
	}
private:

	// Pick a tile
	void pick (int mesh, int tile, const CVector& hit, TModePaint mode);

	// Get the state of a tile
	CZoneSymmetrisation::TState MouseListener::getState (int mesh, int tile, const CVector& hit, std::vector<EPM_Mesh>& vectMesh);

	// Callback on mouse events
	virtual void operator ()(const CEvent& event)
	{
		if (event==EventDestroyWindowId || event==EventCloseWindowId)
		{
			WindowActive=false;
		}
		if (modeSelect!=ModeSelect)
		{
			int res = TRUE;
			static PatchMesh *shape1 = NULL;
			static int poly1, tile1, tile2, mesh1, mesh2, seg1;
			static bool pressed=false;
			static IPoint2 anchor, lastPoint;

			// Mouse down ?
			CEventMouse*	mouse=(CEventMouse*)&event;
			CEventKeyDown*	keyDown=(CEventKeyDown*)&event;

			if (event==EventMouseDownId)
			{
				if (pressed&&((mouse->Button&rightButton)&&((mouse->Button&(ctrlButton|shiftButton|altButton))==0)))
				{
					pressed=false;

					// Undo step
					bankCont->Undo.pushUndo ();

					// Call undo now
					bankCont->Undo.undo (*_Eproc, _VectMesh, _Land, _Pobj, PaintColor);
				}
				if (mouse->Button==leftButton)
				{
					CVector hotSpot;
					CVector topVector;
					if (_Eproc->HitATile(*_Viewport, *_Camera, mouse->X, mouse->Y, &tile1, &mesh1, _T, _VectMesh, hotSpot, topVector))
					{
						// Set hit as next hotspot
						if ( (modeSelect!=ModePick) && (modeSelect!=ModeGetState) )
							_3dMouseListener->setHotSpot (hotSpot);

						// Patch number
						int patch=tile1/NUM_TILE_SEL;

						// Mode paint ?
						if (modeSelect==ModePaint)
						{
							if ((nModeTexture==ModeTile)||(nModeTexture==ModeDisplace))
							{
								// Check if we are in patch subobject and if this patch is selected
								if ((_VectMesh[mesh1].PMesh->selLevel!=EP_PATCH)||(_VectMesh[mesh1].PMesh->patchSel[patch]))
								{
									std::set<EPM_PaintTile*> alreadyRecursed;

									// A nel manager
									CNelPatchChanger nelPatchChg (_Land);

									if (nModeTexture==ModeTile)
									{
										// Put the tile
										_Eproc->PutTile (tile1, mesh1, true, bank, _Pobj->CurrentTileSet, _VectMesh, _Land, brushValue[PaintPatchMod::brushSize],
											alreadyRecursed, nelPatchChg, _Pobj->tileSize!=0);
									}
									else // (nModeTexture==ModeDisplace)
									{
										// Put the tile
										_Eproc->PutDisplace (tile1, mesh1, bank, _VectMesh, _Land, brushValue[PaintPatchMod::brushSize],
											alreadyRecursed, nelPatchChg);
									}

									// Flush nel chgt
									nelPatchChg.applyChanges ((nModeTexture==ModeDisplace)&&(_Pobj->automaticLighting));
								}
							}
							else if (nModeTexture==ModeColor)
							{
								// Paint
								PaintColor.paint (mesh1, tile1, hotSpot, topVector, _VectMesh);
							}

							// Button pressed
							pressed = true;
						}
						else if (modeSelect==ModePick)
						{
							// Pick tile
							pick (mesh1, tile1, hotSpot, nModeTexture);
						}
						else if (modeSelect==ModeGetState)
						{
							// Pick tile state
							CZoneSymmetrisation::TState state = getState (mesh1, tile1, hotSpot, _VectMesh);
							_Pobj->CurrentState = state;

							// Active show state
							_Pobj->ShowCurrentState = true;
						}
						else if (modeSelect==ModeFill)
						{
							// Paint mode
							if (nModeTexture==ModeTile)
								// Fill this patch with the current tile
								_FillTile.fillTile (mesh1, patch, _VectMesh, _Pobj->CurrentTileSet, _Pobj->TileFillRotation, _Pobj->TileGroup, _Pobj->tileSize!=0,
													bank);
							else if (nModeTexture==ModeColor)
								// Fill this patch with the current color
								_FillTile.fillColor (mesh1, patch, _VectMesh, maxToNel (color1), (uint16)(256.f*opa1), PaintColor);

							else if (nModeTexture==ModeDisplace)
								// Fill this patch with the current displace
								_FillTile.fillDisplace (mesh1, patch, _VectMesh, bank);
						}
						else if (modeSelect==ModeResetPatch)
						{
							int np = _VectMesh[mesh1].PMesh->numPatches;
							for (int pp = 0; pp < np; ++pp)
							{
								// Fill default tile
								_FillTile.fillTile (mesh1, pp, _VectMesh, -1, 0, 0, true, bank);

								// Fill default color
								_FillTile.fillColor (mesh1, pp, _VectMesh, CRGBA(255, 255, 255), 256, PaintColor);

								// Backup current displace, fill default, restore
								int bkdt = _Pobj->DisplaceTile;
								int bkdts = _Pobj->DisplaceTileSet;
								_Pobj->DisplaceTile = 0;
								_Pobj->DisplaceTileSet = -1;
								_FillTile.fillDisplace (mesh1, pp, _VectMesh, bank);
								_Pobj->DisplaceTile = bkdt;
								_Pobj->DisplaceTileSet = bkdts;
							}
						}
					}
				}
				// Pick with right mouse
				if (mouse->Button==rightButton)
				{
					// Pick tile
					CVector hotSpot;
					pick (mesh1, tile1, hotSpot, nModeTexture);
				}
			}
			if (event==EventMouseUpId)
			{
				if (mouse->Button==leftButton)
				{
					pressed = false;
					_Pobj->ShowCurrentState = false;

					// Undo step
					bankCont->Undo.pushUndo ();
				}
			}
			// Mouse move ?
			if (event==EventMouseMoveId)
			{
				if ((pressed)&&(mouse->Button==leftButton))
				{
					CVector hotSpot;
					CVector topVector;
					if (_Eproc->HitATile(*_Viewport, *_Camera, mouse->X, mouse->Y, &tile2, &mesh2, _T, _VectMesh, hotSpot, topVector))
					{
						_3dMouseListener->setHotSpot (hotSpot);
						if ((tile1!=tile2)||(mesh1!=mesh2))
						{
							// Paint tiles ?
							if ((nModeTexture==ModeTile)||(nModeTexture==ModeDisplace))
							{
								// Patch number
								int patch=tile2/NUM_TILE_SEL;

								// Check if we are in patch subobject and if this patch is selected
								if ((_VectMesh[mesh1].PMesh->selLevel!=EP_PATCH)||(_VectMesh[mesh2].PMesh->patchSel[patch]))
								{
									std::set<EPM_PaintTile*> alreadyRecursed;

									// A nel manager
									CNelPatchChanger nelPatchChg (_Land);

									if (nModeTexture==ModeTile)
									{
										// Put the tile
										_Eproc->PutTile (tile2, mesh2, false, bank, _Pobj->CurrentTileSet, _VectMesh, _Land,
											brushValue[PaintPatchMod::brushSize], alreadyRecursed, nelPatchChg, _Pobj->tileSize!=0);
									}
									else // (nModeTexture==ModeDisplace)
									{
										// Put the tile
										_Eproc->PutDisplace (tile2, mesh2, bank, _VectMesh, _Land, brushValue[PaintPatchMod::brushSize],
											alreadyRecursed, nelPatchChg);
									}

									// Flush nel chgt
									nelPatchChg.applyChanges ((nModeTexture==ModeDisplace)&&(_Pobj->automaticLighting));
								}
							}
							// Paint colors ?
							else if (nModeTexture==ModeColor)
							{
								// Paint
								PaintColor.paint (mesh2, tile2, hotSpot, topVector, _VectMesh);
							}

							// New tile touched
							tile1=tile2;
							mesh1=mesh2;
						}
					}
				}
			}
			// Key down ?
			if (event==EventKeyDownId)
			{
				// First time ?
				if (keyDown->FirstTime)
				{
					// Undo ?
					if ((keyDown->Key==KeyZ)&&(keyDown->Button==ctrlKeyButton))
					{
						// Undo !
						bankCont->Undo.undo (*_Eproc, _VectMesh, _Land, _Pobj, PaintColor);
					}

					// Redo ?
					if ((keyDown->Key==KeyE)&&(keyDown->Button==ctrlKeyButton))
					{
						// Redo !
						bankCont->Undo.redo (*_Eproc, _VectMesh, _Land, _Pobj, PaintColor);
					}
				}
			}
		}
		else if ((nModeTexture==ModeTile)||(nModeTexture==ModeDisplace))
		{
			// bTexture
			CEventMouse* mouse=(CEventMouse*)&event;
			if (event==EventMouseDownId)
			{
				int x=(int)(mouse->X*(float)MOD_WIDTH);
				int y=(int)((mouse->Y)*(float)MOD_HEIGHT);
				int tile=x+y*MOD_WIDTH;

				// Mode tiles ?
				if (nModeTexture==ModeTile)
				{
					if ((tile>=0)&&(tile<=bank.getTileSetCount()))
					{
						// Select bank tileset ?
						if (tile==0)
						{
							_Pobj->CurrentTileSet=-1;
						}
						// no...
						else
						{
							// Ask for the tileSet
							_Pobj->CurrentTileSet=tileSetSelector.getTileSet (tile-1);
							_Pobj->DisplaceTileSet=_Pobj->CurrentTileSet;
						}
					}
				}

				// Mode displace ?
				if (nModeTexture==ModeDisplace)
				{
					if ((tile>=0)&&(tile<CTileSet::CountDisplace))
					{
						_Pobj->DisplaceTile=tile;
					}
				}
			}
		}
	}
};

/*-------------------------------------------------------------------*/

// Pick a tile
void MouseListener::pick (int mesh, int tile, const CVector& hit, TModePaint mode)
{
	// Pick tile
	if ((mode==ModeTile)||(mode==ModeDisplace))
	{
		// Pick tile under the cursor
		tileDesc desc;
		_Eproc->GetTile (mesh, tile, desc, _VectMesh, _Land);

		if (desc.getNumLayer ()>0)
		{
			// Get info about this tile
			int tileSet;
			int number;
			CTileBank::TTileType type;
			bank.getTileXRef (desc.getLayer (0).Tile, tileSet, number, type);

			// Pickup a tile ?
			if (mode==ModeTile)
			{
				// Set the tileSet if this tile set is in the land
				if (tileSetSelector.isInArray (tileSet))
					_Pobj->CurrentTileSet=tileSet;
			}
			// Pickup a displace ?
			else
			{
				// Get the displace tile index
				_Pobj->DisplaceTile=desc.getDisplace ();
				_Pobj->DisplaceTileSet=tileSet;
			}
		}
	}
	else if (mode==ModeColor)
	{
		// Get the tile pointer
		EPM_PaintTile *pTile=&_Eproc->metaTile[mesh][tile];

		// Coordoninates
		int u[4]={pTile->u, pTile->u, pTile->u+1, pTile->u+1};
		int v[4]={pTile->v, pTile->v+1, pTile->v+1, pTile->v};

		CRGBA bestColor;
		float fBestDist=FLT_MAX;

		// 4 coordinates
		for (int i=0; i<4; i++)
		{
			// Get another color
			CVector pos;
			CRGBA color;
			PaintColor.pickVertexColor (mesh, pTile->patch, u[i], v[i], pos, color, _VectMesh);

			// Get new dist
			float fDist=(pos-hit).norm();

			// Better dist ?
			if (fDist<fBestDist)
			{
				bestColor=color;
				fBestDist=fDist;
			}
		}

		// Set the color
		color1=nelToMax (bestColor);
	}
}

/*-------------------------------------------------------------------*/
// Pick a tile
CZoneSymmetrisation::TState MouseListener::getState (int mesh, int tile, const CVector& hit, std::vector<EPM_Mesh>& vectMesh)
{
	if (vectMesh[mesh].Symmetry || vectMesh[mesh].Rotate)
	{
		uint patch=tile/NUM_TILE_SEL;
		uint ttile=tile%NUM_TILE_SEL;
		uint v=ttile/MAX_TILE_IN_PATCH;
		uint u=ttile%MAX_TILE_IN_PATCH;

		// Get the patch
		int OrderS=(1<<vectMesh[mesh].RMesh->getUIPatch (patch).NbTilesU);
		uint symTile = OrderS*v+u;

		// Pick tile under the cursor
		return symVector[mesh].getTileState (patch, symTile, 0);
	}
	else
		return CZoneSymmetrisation::Nothing;
}

/*-------------------------------------------------------------------*/

DWORD WINAPI myThread (LPVOID vData);

int getBindedEdge (int nPatch, int nVertInPatch, const PatchMesh& patch, const RPatchMesh& rpatch)
{
	// Vert number in the patxhmesh
	int nVertInMesh=patch.patches[nPatch].v[nVertInPatch];

	// Should be binded
	nlassert (rpatch.getUIVertex (nVertInMesh).Binding.bBinded);

	// Some vertex
	int nVertexBefore=patch.patches[nPatch].v[(nVertInPatch-1)&3];
	int nVertexAfter=patch.patches[nPatch].v[(nVertInPatch+1)&3];

	// switch binding sort
	switch (rpatch.getUIVertex (nVertInMesh).Binding.nType)
	{
	case BIND_SINGLE:
		// Check if the point after is binded to the same patch, same edge
		if (patch.patches[rpatch.getUIVertex (nVertInMesh).Binding.nPatch].v[rpatch.getUIVertex (nVertInMesh).Binding.nEdge]==
				patch.patches[nPatch].v[(nVertInPatch+1)&3])
		{
			return nVertInPatch;
		}
		else
		{
#ifdef NL_DEBUG
			const UI_VERTEX &uiv = rpatch.getUIVertex (nVertInMesh);
			Patch &patch0 = patch.patches[uiv.Binding.nPatch];
			Patch &patch1 = patch.patches[nPatch];
			uint vertIndex0 = (uiv.Binding.nEdge+1)&3;
			uint vertIndex1 = (nVertInPatch-1)&3;
			uint vert0 = patch0.v[vertIndex0];
			uint vert1 = patch1.v[vertIndex1];
#endif // NL_DEBUG
			nlassert (patch.patches[rpatch.getUIVertex (nVertInMesh).Binding.nPatch].v[(rpatch.getUIVertex (nVertInMesh).Binding.nEdge+1)&3]==
				patch.patches[nPatch].v[(nVertInPatch-1)&3]);
			return (nVertInPatch-1)&3;
		}
		break;
	case BIND_25:
		// Check if the point after is binded to the same patch, same edge
		if ((rpatch.getUIVertex (nVertexBefore).Binding.bBinded)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nPatch==rpatch.getUIVertex (nVertInMesh).Binding.nPatch)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nEdge==rpatch.getUIVertex (nVertInMesh).Binding.nEdge)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nType==BIND_50))
		{
			return (nVertInPatch-1)&3;
		}
		else
		{
			nlassert (patch.patches[rpatch.getUIVertex (nVertInMesh).Binding.nPatch].v[rpatch.getUIVertex (nVertInMesh).Binding.nEdge]==
				patch.patches[nPatch].v[(nVertInPatch+1)&3]);
			return nVertInPatch;
		}
		break;
	case BIND_50:
		// Check if the point after is binded to the same patch, same edge
		if ((rpatch.getUIVertex (nVertexBefore).Binding.bBinded)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nPatch==rpatch.getUIVertex (nVertInMesh).Binding.nPatch)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nEdge==rpatch.getUIVertex (nVertInMesh).Binding.nEdge)&&
			(rpatch.getUIVertex (nVertexBefore).Binding.nType==BIND_75))
		{
			return (nVertInPatch-1)&3;
		}
		else
		{
			nlassert (rpatch.getUIVertex (nVertexAfter).Binding.bBinded);
			nlassert (rpatch.getUIVertex (nVertexAfter).Binding.nPatch==rpatch.getUIVertex (nVertInMesh).Binding.nPatch);
			nlassert (rpatch.getUIVertex (nVertexAfter).Binding.nEdge==rpatch.getUIVertex (nVertInMesh).Binding.nEdge);
			nlassert (rpatch.getUIVertex (nVertexAfter).Binding.nType==BIND_25);
			return nVertInPatch;
		}
		break;
	case BIND_75:
		// Check if the point after is binded to the same patch, same edge
		if ((rpatch.getUIVertex (nVertexAfter).Binding.bBinded)&&
			(rpatch.getUIVertex (nVertexAfter).Binding.nPatch==rpatch.getUIVertex (nVertInMesh).Binding.nPatch)&&
			(rpatch.getUIVertex (nVertexAfter).Binding.nEdge==rpatch.getUIVertex (nVertInMesh).Binding.nEdge)&&
			(rpatch.getUIVertex (nVertexAfter).Binding.nType==BIND_50))
		{
			return nVertInPatch;
		}
		else
		{
			nlassert (patch.patches[rpatch.getUIVertex (nVertInMesh).Binding.nPatch].v[(rpatch.getUIVertex (nVertInMesh).Binding.nEdge+1)&3]==
				patch.patches[nPatch].v[(nVertInPatch-1)&3]);
			return (nVertInPatch-1)&3;
		}
		break;
	}
	nlassert (0);	// no!
	return 0;
}

void EPM_PaintCMode::EnterMode ()
{
	if (pobj->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_PAINT));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
}

void EPM_PaintCMode::DoPaint ()
{
	// Toremove
	static float best = 10000.f;

	// Set local to english
	setlocale (LC_NUMERIC, "C");

	if (pobj->hOpsPanel)
	{
		bWarningInvalidTileSet=false;

		// Build paint tile infos..
		nlassert (eproc.ip);
		ModContextList mcList;
		INodeTab nodes;
		TimeValue t = eproc.ip->GetTime();

		// Build modifier context list
		eproc.ip->GetModContexts(mcList, nodes);
		pobj->ClearPatchDataFlag(mcList, EPD_BEENDONE);
		theHold.Begin();

		// Mesh table
		std::vector<EPM_Mesh> vectMesh;
		makeVectMesh (vectMesh, nodes, mcList, pobj, t);

		// Size of the map
		eproc.metaTile.resize (vectMesh.size());
		eproc.bitArray.resize (vectMesh.size());

		// Calculate the boundind box..
		float fMinX=FLT_MAX;
		float fMinY=FLT_MAX;
		float fMinZ=FLT_MAX;
		float fMaxX=-FLT_MAX;
		float fMaxY=-FLT_MAX;
		float fMaxZ=-FLT_MAX;
		int i;
		for (i = 0; i < (int)vectMesh.size(); i++)
		{
			// Get pointers
			PaintPatchData *patchData = vectMesh[i].PatchData;
			RPatchMesh *rpatch = vectMesh[i].RMesh;
			PatchMesh *patch = vectMesh[i].PMesh;
			if ((!patchData)||(!patch)||(!rpatch))
				continue;

			// hold
			patchData->RecordTopologyTags(patch);
			patchData->BeginEdit(t);
			if (theHold.Holding())
				theHold.Put(new PatchRestore(patchData, pobj, patch, rpatch));

			// fill the tile map
			for (int p=0; p<patch->numPatches; p++)
			{
				int nU=1<<rpatch->getUIPatch (p).NbTilesU;
				int nV=1<<rpatch->getUIPatch (p).NbTilesV;
				for (int u=0; u<=nU; u++)
				for (int v=0; v<=nV; v++)
				{
					Point3 pos=patch->patches[p].interp (patch, (float)u/(float)(nU), (float)v/(float)(nV));
					pos=pos*(vectMesh[i].Node->GetObjectTM (t));
					if (fMaxX<pos.x)
						fMaxX=pos.x;
					if (fMinX>pos.x)
						fMinX=pos.x;
					if (fMaxY<pos.y)
						fMaxY=pos.y;
					if (fMinY>pos.y)
						fMinY=pos.y;
					if (fMaxZ<pos.z)
						fMaxZ=pos.z;
					if (fMinZ>pos.z)
						fMinZ=pos.z;
				}
			}
		}

		// Create the quad to select
		CVector center ((fMinX+fMaxX)/2.f, (fMinY+fMaxY)/2.f, (fMinZ+fMaxZ)/2.f);
		CVector lookFrom (fMaxX, fMaxY, fMaxZ);
		eproc.quadTreeSelect.create (8, center, (float)std::max (std::max (fMaxX-fMinX, fMaxY-fMinY), fMaxZ-fMinZ));

		for (i = 0; i < (int)vectMesh.size(); i++)
		{
			// Get pointers
			PaintPatchData *patchData = vectMesh[i].PatchData;
			RPatchMesh *rpatch = vectMesh[i].RMesh;
			PatchMesh *patch = vectMesh[i].PMesh;
			if ((!patchData)||(!patch)||(!rpatch))
				continue;

			// paint mode
			rpatch->paint=true;
			rpatch->BuildMesh(t, *patch);

			// copy edge eproc.bitArray
			eproc.bitArray[i]=patch->edgeSel;

			// clear tile map
			eproc.metaTile[i].clear();
			eproc.metaTile[i].resize (NUM_TILE_SEL*patch->numPatches);

			// fill the tile map
			for (int p=0; p<patch->numPatches; p++)
			{
				int nU=1<<rpatch->getUIPatch (p).NbTilesU;
				int nV=1<<rpatch->getUIPatch (p).NbTilesV;
				for (int u=0; u<nU; u++)
				for (int v=0; v<nV; v++)
				{
					EPM_PaintTile *pTile=&eproc.metaTile[i][p*NUM_TILE_SEL+v*MAX_TILE_IN_PATCH+u];
					pTile->Mesh=i;
					pTile->patch=p;
					pTile->tile=p*NUM_TILE_SEL+v*MAX_TILE_IN_PATCH+u;
					pTile->voisins[3]= v==0 ? NULL : &eproc.metaTile[i][p*NUM_TILE_SEL+(v-1)*MAX_TILE_IN_PATCH+u];
					pTile->voisins[1]= v==(nV-1) ? NULL : &eproc.metaTile[i][p*NUM_TILE_SEL+(v+1)*MAX_TILE_IN_PATCH+u];
					pTile->voisins[0]= u==0 ? NULL : &eproc.metaTile[i][p*NUM_TILE_SEL+v*MAX_TILE_IN_PATCH+u-1];
					pTile->voisins[2]= u==(nU-1) ? NULL : &eproc.metaTile[i][p*NUM_TILE_SEL+v*MAX_TILE_IN_PATCH+u+1];
					pTile->rotate[3]=0;
					pTile->rotate[1]=0;
					pTile->rotate[0]=0;
					pTile->rotate[2]=0;
					pTile->u=u;
					pTile->v=v;

					// Compute bouding box of the tile
					fMinX=FLT_MAX;
					fMinY=FLT_MAX;
					fMaxX=-FLT_MAX;
					fMaxY=-FLT_MAX;

					// Compute the 4 corners of the tile
					Point3 pos[4];
					pos[0]=patch->patches[p].interp (patch, (float)u/(float)(nU), (float)v/(float)(nV));
					pos[0]=pos[0]*(vectMesh[i].Node->GetObjectTM (t));
					pos[1]=patch->patches[p].interp (patch, (float)(u+1)/(float)(nU), (float)v/(float)(nV));
					pos[1]=pos[1]*(vectMesh[i].Node->GetObjectTM (t));
					pos[2]=patch->patches[p].interp (patch, (float)(u+1)/(float)(nU), (float)(v+1)/(float)(nV));
					pos[2]=pos[2]*(vectMesh[i].Node->GetObjectTM (t));
					pos[3]=patch->patches[p].interp (patch, (float)u/(float)(nU), (float)(v+1)/(float)(nV));
					pos[3]=pos[3]*(vectMesh[i].Node->GetObjectTM (t));

					// Store its center
					pTile->Center=(maxToNel (pos[0])+maxToNel (pos[1])+maxToNel (pos[2])+maxToNel (pos[3]))/4.f;

					// Store its radius
					pTile->Radius=std::max
									(
										std::max ( (maxToNel (pos[0])-pTile->Center).norm(), (maxToNel (pos[1])-pTile->Center).norm() ),
										std::max ( (maxToNel (pos[2])-pTile->Center).norm(), (maxToNel (pos[3])-pTile->Center).norm() )
									);

					// Bounding
					for (int i=0; i<4; i++)
					{
						if (fMaxX<pos[i].x)
							fMaxX=pos[i].x;
						if (fMinX>pos[i].x)
							fMinX=pos[i].x;
						if (fMaxY<pos[i].y)
							fMaxY=pos[i].y;
						if (fMinY>pos[i].y)
							fMinY=pos[i].y;
						if (fMaxZ<pos[i].z)
							fMaxZ=pos[i].z;
						if (fMinZ>pos[i].z)
							fMinZ=pos[i].z;
					}

					// Insert the tile in the quad tree
					eproc.quadTreeSelect.insert (CVector (fMinX, fMinY, fMinZ), CVector (fMaxX, fMaxY, fMaxZ), pTile);
				}
			}
		}

		// Watch for patch voisin
		for (i = 0; i < (int)vectMesh.size(); i++)
		{
			// Get pointers
			PaintPatchData *patchData = vectMesh[i].PatchData;
			RPatchMesh *rpatch = vectMesh[i].RMesh;
			PatchMesh *patch = vectMesh[i].PMesh;
			if ((!patchData)||(!patch)||(!rpatch))
				continue;

			// fill the tile map
			for (int p=0; p<patch->numPatches; p++)
			{
				// Find a voisin
				for (int e=0; e<4; e++)
				{
					EPM_PaintPatch patchVoisin;
					patchVoisin.patch=-1;
					int edgeVoisin;
					int offsetEdge=0;
					int dividEdge=0;
					int mYedge=patch->patches[p].edge[e];
					if (mYedge == -1)
					{
					 	std::string error = NLMISC::toString("Invalid edge '%i' with value '%i' in patch '%i' in PatchMesh", p, mYedge, e);
					 	nlwarning(error.c_str());
					 	MessageBox(NULL, MaxTStrFromUtf8(error).data(), _T("NeL Patch Painter"), MB_OK | MB_ICONSTOP);
					 	return;
					}
#if (MAX_RELEASE < 4000)
					int otherPatch=(patch->edges[mYedge].patch1==p)?patch->edges[mYedge].patch2:patch->edges[mYedge].patch1;
#else // (MAX_RELEASE < 4000)
					int otherPatch = -1;
					if(patch->edges[mYedge].patches.Count()>0)
					{
						if (patch->edges[mYedge].patches[0]==p) {
							if (patch->edges[mYedge].patches.Count() > 1)
								otherPatch = patch->edges[mYedge].patches[1];
						}
						else
							otherPatch = patch->edges[mYedge].patches[0];
					}
#endif // (MAX_RELEASE < 4000)
					int nMeshIndex;
					if (otherPatch!=-1)
					{
						patchVoisin.patch=otherPatch;
						patchVoisin.Mesh=i;
						edgeVoisin=WhereIsTheEdge (otherPatch, mYedge, *patch);
						nlassert (edgeVoisin!=-1);
						nMeshIndex=i;
					}
					else
					{
						// Binding ?
						int vertBinded=-1;
						if (rpatch->getUIVertex (patch->patches[p].v[e]).Binding.bBinded)
							vertBinded=e;
						if (rpatch->getUIVertex (patch->patches[p].v[(e+1)&3]).Binding.bBinded)
							vertBinded=(e+1)&3;
						if ((vertBinded!=-1)&&(getBindedEdge (p, vertBinded, *patch, *rpatch)==e))
						{
							int nVert=patch->patches[p].v[vertBinded];
							patchVoisin.patch=rpatch->getUIVertex (nVert).Binding.nPatch;
							otherPatch=rpatch->getUIVertex (nVert).Binding.nPatch;
							patchVoisin.Mesh=i;
							edgeVoisin=rpatch->getUIVertex (nVert).Binding.nEdge;
							nlassert (edgeVoisin!=-1);
							nMeshIndex=i;
							switch (rpatch->getUIVertex (nVert).Binding.nType)
							{
							case BIND_25:
								dividEdge=2;
								if (vertBinded==e)
									offsetEdge=3;
								else
									offsetEdge=2;
								break;
							case BIND_75:
								dividEdge=2;
								if (vertBinded==e)
									offsetEdge=1;
								else
									offsetEdge=0;
								break;
							case BIND_50:
								dividEdge=2;
								if (vertBinded==e)
									offsetEdge=2;
								else
									offsetEdge=1;
								break;
							case BIND_SINGLE:
								dividEdge=1;
								if (vertBinded==e)
									offsetEdge=2;
								else
									offsetEdge=0;
								break;
							}
						}
						else	// Look in the neighborhood for a voisin
						{
							// Vertex Number
							//Matrix3 m1=*mcList[i]->tm;
							Matrix3 m1=vectMesh[i].Node->GetObjectTM(t);
							Point3 vA1=patch->verts[patch->patches[p].v[e]].p * m1;
							Point3 vB1=patch->verts[patch->patches[p].v[(e+1)&3]].p * m1;

							// If symmetry, invert
							if (vectMesh[i].Symmetry)
							{
								Point3 tmp = vA1;
								vA1 = vB1;
								vB1 = tmp;
							}

							// Look for in other patchMesh
							for (int ii = 0; ii < (int)vectMesh.size(); ii++)
							{
								if (ii!=i)
								{
									// Get pointers
									PaintPatchData *patchData2 = vectMesh[ii].PatchData;
									RPatchMesh *rpatch2 = vectMesh[ii].RMesh;
									PatchMesh *patch2 = vectMesh[ii].PMesh;
									if ((!patchData2)||(!patch2)||(!rpatch2))
										continue;

									// Look for an open edge
									for (int ee=0; ee<patch2->numEdges; ee++)
									{
#if (MAX_RELEASE < 4000)
										if ((patch2->edges[ee].patch1==-1)||(patch2->edges[ee].patch2==-1))
										{
											//
											int pp=patch2->edges[ee].patch1==-1 ? patch2->edges[ee].patch2:patch2->edges[ee].patch1;
#else // (MAX_RELEASE < 4000)
										if (patch2->edges[ee].patches.Count()>0)
										{
											//
											int pp=patch2->edges[ee].patches[0];
#endif // (MAX_RELEASE < 4000)
											nlassert (pp!=-1);

											// Edge number
											int edge=WhereIsTheEdge (pp, ee, *patch2);
											//Matrix3 m2=*mcList[ii]->tm;
											Matrix3 m2=vectMesh[ii].Node->GetObjectTM(t);
											Point3 vA2=patch2->verts[patch2->patches[pp].v[edge]].p * m2;
											Point3 vB2=patch2->verts[patch2->patches[pp].v[(edge+1)&3]].p * m2;

											// If symmetry, invert
											if (vectMesh[ii].Symmetry)
											{
												Point3 tmp = vA2;
												vA2 = vB2;
												vB2 = tmp;
											}

											// The same ?

											// Toremove
											if ((vA1-vB2).Length () < best)
											{
												best = (vA1-vB2).Length ();
											}
											if ((vA2-vB1).Length () < best)
											{
												best = (vA2-vB1).Length ();
											}

											if (((vA1-vB2).Length ()<WELD_THRESOLD)&&((vA2-vB1).Length ()<WELD_THRESOLD))
											{
												// The same!!
												patchVoisin.patch=pp;
												patchVoisin.Mesh=ii;
												edgeVoisin=edge;
												nMeshIndex=ii;
												break;
											}
										}
									}
								}
							}
						}
					}
					if (patchVoisin.patch!=-1)
					{
						std::string first = MCharStrToUtf8(vectMesh[i].Node->GetName());
						std::string second = MCharStrToUtf8(vectMesh[patchVoisin.Mesh].Node->GetName());
						int rot = (2-((vectMesh[i].Symmetry)?(2-e):e)+((vectMesh[patchVoisin.Mesh].Symmetry)?(2-edgeVoisin):edgeVoisin))&3;
						int nU = 1 << rpatch->getUIPatch (p).NbTilesU;
						int nV = 1 << rpatch->getUIPatch (p).NbTilesV;
						int nUOther = 1 << vectMesh[patchVoisin.Mesh].RMesh->getUIPatch (patchVoisin.patch).NbTilesU;
						int nVOther = 1 << vectMesh[patchVoisin.Mesh].RMesh->getUIPatch (patchVoisin.patch).NbTilesV;
						int nTile= (e&1) ? nU : nV;
						int nTile2= (edgeVoisin&1) ? nUOther : nVOther;
						if (nTile==(nTile2>>dividEdge))
						{
							int offset=getOffset (e, nU, nV, vectMesh[i].Symmetry);
							int offsetOther=getOffset (edgeVoisin, nUOther, nVOther, vectMesh[patchVoisin.Mesh].Symmetry);
							static int delta[4] = { MAX_TILE_IN_PATCH, 1, -MAX_TILE_IN_PATCH, -1 };

							int symIndex = vectMesh[i].Symmetry?-1:1;
							int symIndexOther = vectMesh[patchVoisin.Mesh].Symmetry?-1:1;

							EPM_PaintTile *pTile1=&eproc.metaTile[i][p*NUM_TILE_SEL+offset];
							EPM_PaintTile *pTile2=&eproc.metaTile[nMeshIndex][patchVoisin.patch*NUM_TILE_SEL+offsetOther];
							pTile2+=(nTile2-1)*delta[edgeVoisin]*symIndexOther;
							pTile2-=(delta[edgeVoisin]*symIndexOther*nTile2*offsetEdge)>>2;

							for (int end=0; end<nTile; end++)
							{
								// tile dest
								pTile1->voisins[e]=pTile2;
								pTile1->rotate[e]=rot;

								// tile src..
								nlassert ((pTile2->voisins[edgeVoisin]==NULL)||(pTile2->voisins[edgeVoisin]==pTile1));
								pTile2->voisins[edgeVoisin]=pTile1;
								pTile2->rotate[edgeVoisin]=(-rot)&3;

								pTile1+=delta[e]*symIndex;
								pTile2-=delta[edgeVoisin]*symIndexOther;
							}
						}
						patch->edgeSel.Clear (mYedge);
					}
					else
					{
						patch->edgeSel.Set (mYedge);
					}
				}
			}
		}

		// Flag frozen and locked tiles
		for (i = 0; i < (int)vectMesh.size(); i++)
		{
			// Get pointers
			PaintPatchData *patchData = vectMesh[i].PatchData;
			RPatchMesh *rpatch = vectMesh[i].RMesh;
			PatchMesh *patch = vectMesh[i].PMesh;
			if ((!patchData)||(!patch)||(!rpatch))
				continue;

			// fill the tile map
			for (int p=0; p<patch->numPatches; p++)
			{
				int nU=1<<rpatch->getUIPatch (p).NbTilesU;
				int nV=1<<rpatch->getUIPatch (p).NbTilesV;
				for (int u=0; u<nU; u++)
				for (int v=0; v<nV; v++)
				{
					EPM_PaintTile *pTile=&eproc.metaTile[i][p*NUM_TILE_SEL+v*MAX_TILE_IN_PATCH+u];

					// frozen ?
					pTile->frozen = vectMesh[pTile->Mesh].Node->IsFrozen() != 0;

					// Not locked
					pTile->locked = 0;

					// Check that neighbor tiles are not frozen
					uint neighbor;
					for (neighbor=0; neighbor<4; neighbor++)
					{
						// Neighbor exist ?
						EPM_PaintTile *neighborTile = pTile->voisins[neighbor];
						if (neighborTile)
						{
							// Not freezed ?
							if (vectMesh[neighborTile->Mesh].Node->IsFrozen())
								pTile->locked |= 1<<neighbor;
						}
						else
							pTile->locked |= 1<<neighbor;
					}
				}
			}
		}


		std::string sName=GetBankPathName ();
		if (!sName.empty())
		{
			CIFile file;
			if (file.open (sName))
			{
				try
				{
					bank.clear();
					bank.serial (file);
					bank.computeXRef ();
				}
				catch (const EStream& stream)
				{
					MessageBox (NULL, MaxTStrFromUtf8(stream.what()).data(), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
				}
			}
		}

		// Enter painter mode
		enterPainter (bank);

		// Select the tilesets to use
		tileSetSelector.setSelection (GetBankTileSetSet (), bank);

		// Create a paint thread..
		//DWORD id;
		callThread *pData=new callThread (vectMesh);
		pData->eproc=&eproc;
		pData->pobj=pobj;
		pData->center=center;
		pData->T=t;

		myThread (pData);		// Do it without thread

		// Invalidate all objects
		for (i = 0; i < (int)vectMesh.size(); i++)
		{
			// Get pointers
			PaintPatchData *patchData = vectMesh[i].PatchData;
			RPatchMesh *rpatch = vectMesh[i].RMesh;
			PatchMesh *patch = vectMesh[i].PMesh;
			if ((!patchData)||(!patch)||(!rpatch))
				continue;

			// End of mode paint
			rpatch->paint=false;

			// Invalidate all modified parts
			patchData->UpdateChanges(patch, rpatch);
			patchData->TempData(pobj)->Invalidate(PART_ALL);

			patch->edgeSel=eproc.bitArray[vectMesh[i].McListIndex];

			patchData->SetFlag(EPD_BEENDONE, TRUE);
		}

		theHold.Accept(_M("Patch change"));

		nodes.DisposeTemporary();
		pobj->ClearPatchDataFlag(mcList, EPD_BEENDONE);
		pobj->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		eproc.ip->RedrawViews(t, REDRAW_NORMAL);

		// Exit painter mode
		exitPainter ();
	}

	// Set locale to current
	setlocale (LC_NUMERIC, "");
}

extern HINSTANCE hInstance;
bool loadLigoConfigFile (CLigoConfig& config, Interface& it)
{
	// Get the module path
	HMODULE hModule = hInstance;
	if (hModule)
	{
		// Get the path
		TCHAR sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		// Success ?
		if (res)
		{
			// Path
			std::string modulePath = NLMISC::CFile::getPath(MCharStrToUtf8(sModulePath)) + "ligoscape.cfg";

			try
			{
				// Load the config file
				config.readConfigFile (modulePath, false);
				// ok
				return true;
			}
			catch (const Exception& e)
			{
				// Print an error message
				nlwarning("Error loading the config file ligoscape.cfg: %s", e.what());
			}
		}
	}
	// Can't found the module
	return false;
}

DWORD WINAPI myThread (LPVOID vData)
{
	// Mega try
	try
	{
		/*************** new mode paint.. **************/
		callThread *pData=(callThread*)vData;

		// Build paint tile infos..
		nlassert (pData->eproc->ip);

		// Viewport parameters
		Matrix3		affineTM;
		float		minx,maxx,miny,maxy;
#if MAX_VERSION_MAJOR >= 19
		ViewExp *vp = &pData->eproc->ip->GetActiveViewExp();
#else
		ViewExp *vp = pData->eproc->ip->GetActiveViewport();
#endif
		vp->GetAffineTM(affineTM);
		if ( vp->IsPerspView() )
		{
			vp->GetGridDims(&minx,&maxx,&miny,&maxy);
		}

		// The scene

		// Loaf cfg files
		LoadKeyCfg ();
		LoadVarCfg ();

		// Init the scene
		CViewport viewport;

		try
		{
			CNELU::init(MAIN_Width, MAIN_Height, viewport);

			// Create a Landscape.
			CLandscapeModel	*TheLand= (CLandscapeModel*)CNELU::Scene->createModel(LandscapeModelId);
			TheLand->Landscape.setTileNear (10000.f);
			TheLand->Landscape.TileBank=bank;

			// Enbable automatique lighting
			TheLand->Landscape.enableAutomaticLighting (false);
			TheLand->Landscape.setupAutomaticLightDir (LightDirection);
			TheLand->Landscape.setupStaticLight (LightDiffuse, LightAmbiant, LightMultiply);

			// *******************
			CExportNel export_ (true, true, true, pData->eproc->ip, "NeL Patch Painter", NULL);

			// Add meshes in the scene
			if (pData->pobj->includeMeshes)
			{
				// Get the root node
				INode *root=pData->eproc->ip->GetRootNode ();

				// View all selected objects
				for (uint nNode=0; nNode<(uint)root->NumberOfChildren(); nNode++)
				{
					// Get the node
					INode* pNode=root->GetChildNode (nNode);

					// It is a zone ?
					if (RPO::isZone (*pNode, pData->T))
					{
					}
					// Try to export a mesh
					else if (CExportNel::isMesh (*pNode, pData->T))
					{
						// Build skined ?
						bool skined=false;

						// Export the shape
						IShape *pShape;
						pShape=export_.buildShape (*pNode, pData->T, NULL, true);

						// Export successful ?
						if (pShape)
						{
							// Add the shape to the view
							CNELU::ShapeBank->add (CExportNel::getName (*pNode), pShape);

							// Create an instance
							CTransformShape *tShape=CNELU::Scene->createInstance (CExportNel::getName (*pNode));

							// Big hack to sort
							TheLand->clipAddChild(tShape);
						}
					}
				}

				// Setup ambient light
				CNELU::Driver->setAmbientColor (export_.getAmbientColor (pData->T));

				// Build light vector
				std::vector<CLight> vectLight;
				export_.getLights (vectLight, pData->T);

				// Insert each lights
				for (uint light=0; light<vectLight.size(); light++)
					CNELU::Driver->setLight (light, vectLight[light]);
			}

			// *******************

			// Init the camera
			CMatrix	mat;
			mat.identity();
			CVector	I,J,K,P;

			Matrix3 matInvert;
			matInvert.SetRow (0, Point3(1.f, 0.f, 0.f));
			matInvert.SetRow (1, Point3(0.f, 0.f, 1.f));
			matInvert.SetRow (2, Point3(0.f, -1.f, 0.f));
			matInvert.SetRow (3, Point3(0.f, 0.f, 0.f));
			matInvert.Invert();
			affineTM.Invert();
			affineTM=matInvert*affineTM;

			I.x= affineTM.GetRow(0).x;
			I.y= affineTM.GetRow(0).y;
			I.z= affineTM.GetRow(0).z;
			J.x= affineTM.GetRow(1).x;
			J.y= affineTM.GetRow(1).y;
			J.z= affineTM.GetRow(1).z;
			K.x= affineTM.GetRow(2).x;
			K.y= affineTM.GetRow(2).y;
			K.z= affineTM.GetRow(2).z;

			P.x= affineTM.GetTrans().x;
			P.y= affineTM.GetTrans().y;
			P.z= affineTM.GetTrans().z;
			mat.setRot(I, J, K);
			mat.setPos(P);
			CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
			CNELU::Camera->setMatrix (mat);
			CNELU::Camera->setPerspective( 75.f*(float)Pi/180.f/*vp->GetFOV()*/, 1.33f, 0.1f, 10000.f);

			// Resize the sym vector
			symVector.resize (pData->VectMesh.size());

			// Form each zone
			uint i;
			for (i = 0; i <(int)pData->VectMesh.size(); i++)
			{
				// Get pointers
				PaintPatchData *patchData = pData->VectMesh[i].PatchData;
				RPatchMesh *rpatch = pData->VectMesh[i].RMesh;
				PatchMesh *patch = pData->VectMesh[i].PMesh;
				if ((!patchData)||(!patch)||(!rpatch))
					continue;

				// Get the symmetry flag
				CLigoConfig config;
				if (!loadLigoConfigFile (config, *pData->eproc->ip))
				{
					config.CellSize = 100;
					config.Snap = 1;
					config.ZoneSnapShotRes = 128;
				}

				// Create the zone..
				CZone	zone;
				if (rpatch->exportZone (pData->VectMesh[i].Node, patch, zone, symVector[i], i, config.CellSize, config.Snap, true))
				{
					// Smooth corner
					CZoneCornerSmoother cornerSmoother;
					std::vector<CZone*> emptyVector;
					cornerSmoother.computeAllCornerSmoothFlags (&zone, emptyVector);

					// Add the zone
					TheLand->Landscape.addZone (zone);
				}
				else
				{
					std::string message = toString("Can't build the zone named %s", MCharStrToUtf8(pData->VectMesh[i].Node->GetName()).c_str());
					MessageBox (pData->eproc->ip->GetMAXHWnd(), MaxTStrFromUtf8(message).data(), _T("NeL Painter"), MB_OK|MB_ICONEXCLAMATION);
				}
			}

			// Check zones
	#ifdef NL_DEBUG
			TheLand->Landscape.checkBinds();
	#endif // NL_DEBUG

			// Refine les zones
			TheLand->Landscape.setRefineMode (true);

			// Go.
			//========
			CEvent3dMouseListener	mouseListener;
			MouseListener			listener (pData->eproc->ip, CNELU::Camera, &viewport, pData->pobj, pData->eproc, &TheLand->Landscape,
				&CNELU::AsyncListener, &mouseListener, pData->VectMesh, pData->T);

			// Mouse listener
			CNELU::EventServer.addListener (EventMouseMoveId, &listener);
			CNELU::EventServer.addListener (EventMouseDownId, &listener);
			CNELU::EventServer.addListener (EventMouseUpId, &listener);
			CNELU::EventServer.addListener (EventMouseDblClkId, &listener);
			CNELU::EventServer.addListener (EventDestroyWindowId, &listener);
			CNELU::EventServer.addListener (EventCloseWindowId, &listener);
			CNELU::EventServer.addListener (EventKeyDownId, &listener);

			// Camera position

			// Mouse listener
			mouseListener.setMatrix (CNELU::Camera->getMatrix());
			mouseListener.setFrustrum (CNELU::Camera->getFrustum());
			mouseListener.setViewport (viewport);
			mouseListener.setHotSpot (pData->center);
			mouseListener.setMouseMode (CEvent3dMouseListener::edit3d);

			mouseListener.addToServer(CNELU::EventServer);

			// *** Flush all selected tileset...
			if (pData->pobj->preloadTiles)
			{
				// For all the tileset selected
				for (sint tss=0; tss<(sint)tileSetSelector.getTileCount (); tss++)
				{
					// Get the tileset index
					sint ts=tileSetSelector.getTileSet (tss);

					// Get the tileset pointer
					CTileSet *tileSet=bank.getTileSet (ts);
					nlassert (tileSet);

					// Flush all its 128x128 tiles
					sint tl;
					for (tl=0; tl<tileSet->getNumTile128(); tl++)
						TheLand->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTile128(tl), 1);

					// Flush all its 256x256 tiles
					for (tl=0; tl<tileSet->getNumTile256(); tl++)
						TheLand->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTile256(tl), 1);

					// Flush all its transisitons tiles
					for (tl=0; tl<CTileSet::count; tl++)
						TheLand->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
				}
			}

			// Setup lights
			CPaintLight lights;
			lights.build (*pData->eproc->ip);
			lights.setup (TheLand->Landscape, *CNELU::Scene);

			// MAIN LOOP
			do
			{
				// Pump events
				CNELU::EventServer.pump();

				// Call the proc
				mainproc(*CNELU::Scene, CNELU::AsyncListener, mouseListener, *TheLand, *CNELU::Scene->getDriver(), pData, listener.PaintColor);
			}
			while (!CNELU::AsyncListener.isKeyPushed(KeyESCAPE)&&listener.WindowActive);

			// Release the emitter from the server
			mouseListener.removeFromServer (CNELU::EventServer);

			CNELU::Scene->getDriver()->release ();

			// Mouse listener
			CNELU::EventServer.removeListener (EventMouseMoveId, &listener);
			CNELU::EventServer.removeListener (EventMouseDownId, &listener);
			CNELU::EventServer.removeListener (EventMouseUpId, &listener);
			CNELU::EventServer.removeListener (EventMouseDblClkId, &listener);
			CNELU::EventServer.removeListener (EventKeyDownId, &listener);
			CNELU::EventServer.removeListener (EventDestroyWindowId, &listener);
			CNELU::EventServer.removeListener (EventCloseWindowId, &listener);

			// End.
			//========
			CNELU::release();
		}
		catch (const EDru& druExcept)
		{
			MessageBox (NULL, MaxTStrFromUtf8(druExcept.what()).data(), _T("NeL driver utility"), MB_OK|MB_ICONEXCLAMATION);
		}

		delete pData;
	}
	catch (const Exception& e)
	{
		MessageBox (NULL, MaxTStrFromUtf8(e.what()).data(), _T("NeL Painter"), MB_OK|MB_ICONEXCLAMATION);
	}

	return 0;
}

/*-------------------------------------------------------------------*/

void EPM_PaintCMode::ExitMode()
{
	pobj->channelModified=EDITPAT_CHANNELS;
	if (pobj->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_PAINT));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);

		// Build paint tile infos..
		nlassert (eproc.ip);
	}
}

/*-------------------------------------------------------------------*/

class NelPatchHitData : public HitData
{
public:
	NelPatchHitData (int index, int type, int mesh)
	{
		Index=index;
		Type=type;
		Mesh=mesh;
	}
	int Index;
	int Type;
	int Mesh;
};

BOOL EPM_PaintMouseProc::HitATile(ViewExp *vpt, IPoint2 *p, int *tile, int *mesh, TimeValue t, std::vector<EPM_Mesh>& vectMesh, CVector &hit, CVector &topVector)
{
	// Get a world ray with the mouse 2d point
	Ray ray;
	vpt->MapScreenToWorldRay((float)p->x,  (float)p->y, ray);

	// Select tree's node with the world ray
	quadTreeSelect.clearSelection ();
	quadTreeSelect.selectRay (CVector (ray.p.x, ray.p.y, ray.p.z), CVector (ray.dir.x, ray.dir.y, ray.dir.z));

	// Get selected nodes..
	CQuadTree<EPM_PaintTile*>::CIterator it=quadTreeSelect.begin();
	while (it!=quadTreeSelect.end())
	{
		// Check if the ray intersect the tile..
		if ((*it)->intersect (ray, vectMesh, t, hit, topVector))
		{
			*tile = (*it)->tile;
			*mesh = (*it)->Mesh;
			return TRUE;
		}
		it++;
	}

	return FALSE;
}

BOOL EPM_PaintMouseProc::HitATile(const CViewport& viewport, const CCamera& camera, float x, float y, int *tile, int *mesh, TimeValue t,
								  std::vector<EPM_Mesh>& vectMesh, NLMISC::CVector& hit, NLMISC::CVector &topVector)
{
	// Get a world ray with the mouse 2d point
	CVector pos, dir;
	viewport.getRayWithPoint (x, y, pos, dir, camera.getMatrix(), camera.getFrustum());

	// Select tree's node with the world ray
	quadTreeSelect.clearSelection ();
	quadTreeSelect.selectRay (pos, dir);

	// Get selected nodes..
	CQuadTree<EPM_PaintTile*>::CIterator it=quadTreeSelect.begin();

	BOOL bRet=FALSE;

	float fMin=FLT_MAX;
	while (it!=quadTreeSelect.end())
	{
		// Check if the ray intersect the tile..

		Ray ray;
		ray.p.x=pos.x;
		ray.p.y=pos.y;
		ray.p.z=pos.z;
		ray.dir.x=dir.x;
		ray.dir.y=dir.y;
		ray.dir.z=dir.z;
		CVector hit2;
		if ((*it)->intersect (ray, vectMesh, t, hit2, topVector))
		{
			float newDist=(hit2-camera.getMatrix().getPos()).norm();
			if (newDist<fMin)
			{
				*tile = (*it)->tile;
				*mesh = (*it)->Mesh;
				fMin=newDist;
				hit=hit2;
				bRet=TRUE;
			}
		}
		it++;
	}

	return bRet;
}

/*-------------------------------------------------------------------*/

bool CheckTri (const Point3& pos0, const Point3& pos1, const Point3& pos2, const Ray& ray, CVector& hit)
{
	// Vectors in Nel format
	CVector v0 (pos0.x, pos0.y, pos0.z);
	CVector v1 (pos1.x, pos1.y, pos1.z);
	CVector v2 (pos2.x, pos2.y, pos2.z);
	CVector pos (ray.p.x, ray.p.y, ray.p.z);
	CVector dir (ray.dir.x, ray.dir.y, ray.dir.z);
	CVector center=v0+v1+v2;
	center/=3.f;
	dir.normalize ();

	// A second point on the ray
	hit=pos+dir;

	// Plane of the tri
	CPlane plane;
	plane.make (v0, v1, v2);

	// Normale
	CVector	normal=plane.getNormal();

	// Devant ?
	if ((plane*pos)<0.f)
		return false;

	// Behind ?
	if ((dir*(center-pos))<0.f)
		return false;

	// Point on the plane
	hit=plane.intersect (pos, pos+dir); //(D*p/(D-d))*dir+pos;

	// Check the point...
	bool positive=(((v0-hit)^(v1-hit))*normal>0.f);

	if ((((v1-hit)^(v2-hit))*normal>0.f)!=positive)
		return false;
	return ((((v2-hit)^(v0-hit))*normal>0.f)==positive);
}

/*-------------------------------------------------------------------*/

bool EPM_PaintTile::intersect (const Ray& ray, std::vector<EPM_Mesh>& vectMesh, TimeValue t, CVector& hit, CVector& topVector)
{
	// Pointer on the patch mesh
	PatchMesh *patchPtr=vectMesh[Mesh].PMesh;
	RPatchMesh *rpatch=vectMesh[Mesh].RMesh;
	INode *node=vectMesh[Mesh].Node;

	// Nb tile in this patch
	int nU=1<<rpatch->getUIPatch (patch).NbTilesU;
	int nV=1<<rpatch->getUIPatch (patch).NbTilesV;

	// 4 corners
	Point3 pos[4];
	pos[0]=patchPtr->patches[patch].interp (patchPtr, (float)u/(float)(nU), (float)v/(float)(nV));
	pos[0]=pos[0]*(node->GetObjectTM (t));
	pos[1]=patchPtr->patches[patch].interp (patchPtr, (float)u/(float)(nU), (float)(v+1)/(float)(nV));
	pos[1]=pos[1]*(node->GetObjectTM (t));
	pos[2]=patchPtr->patches[patch].interp (patchPtr, (float)(u+1)/(float)(nU), (float)(v+1)/(float)(nV));
	pos[2]=pos[2]*(node->GetObjectTM (t));
	pos[3]=patchPtr->patches[patch].interp (patchPtr, (float)(u+1)/(float)(nU), (float)v/(float)(nV));
	pos[3]=pos[3]*(node->GetObjectTM (t));

	// Symmetry ?
	if (vectMesh[Mesh].Symmetry)
	{
		Point3 tmp = pos[0];
		pos[0] = pos[3];
		pos[3] = tmp;
		tmp = pos[1];
		pos[1] = pos[2];
		pos[2] = tmp;
	}

	// Get the normal
	Point3 up = (pos[1]-pos[0]) ^ (pos[2]-pos[0]);
	topVector.x = up.x;
	topVector.y = up.y;
	topVector.z = up.z;
	topVector.normalize ();

	// Check first tri
	if (CheckTri (pos[0], pos[1], pos[3], ray, hit))
		return true;

	// Check second tri
	if (CheckTri (pos[1], pos[2], pos[3], ray, hit))
		return true;

	// No intersection
	return false;
}

/*-------------------------------------------------------------------*/

int EPM_PaintMouseProc::proc(
			HWND hwnd,
			int msg,
			int point,
			int flags,
			IPoint2 m)
{
#if MAX_VERSION_MAJOR >= 19
	ViewExp *vpt = &ip->GetViewExp(hwnd);
#else
	ViewExp *vpt = ip->GetViewport(hwnd);
#endif
	int res = TRUE;
	static PatchMesh *shape1 = NULL;
	static int poly1, tile1, tile2, mesh1, mesh2, seg1;
	static bool pressed=false;
	static IPoint2 anchor, lastPoint;

	switch (msg)
	{
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			if (point)
				PaintPatchMod::paintMode->DoPaint ();
			break;

		case MOUSE_MOVE:
			break;

		case MOUSE_FREEMOVE:
			break;

		case MOUSE_ABORT:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;
	}

#if MAX_VERSION_MAJOR < 19
	if (vpt)
		ip->ReleaseViewport(vpt);
#endif

	return res;
}

/*-------------------------------------------------------------------*/
