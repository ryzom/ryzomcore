#include "stdafx.h"
#include "nel_patch_paint.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define RPO_INCLUDE_MESHES 0x2002
#define RPO_PRELOAD_TILES 0x2003

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult PaintPatchMod::Save(ISave *isave) 
{
	Modifier::Save(isave);
	Interval valid;
	ULONG nb;

	// Tile mode
	isave->BeginChunk(RPO_INCLUDE_MESHES);
	isave->Write(&includeMeshes, sizeof(includeMeshes), &nb);
	isave->	EndChunk();

	isave->BeginChunk(RPO_PRELOAD_TILES);
	isave->Write(&preloadTiles, sizeof(preloadTiles), &nb);
	isave->	EndChunk();

	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult PaintPatchMod::LoadNamedSelChunk(ILoad *iload, int level)
	{	
	IOResult res;
	
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
		}
	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult PaintPatchMod::Load(ILoad *iload) 
{
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
			case RPO_INCLUDE_MESHES:
				res = iload->Read(&includeMeshes, sizeof(includeMeshes), &nb);
				break;
			case RPO_PRELOAD_TILES:
				res = iload->Read(&preloadTiles, sizeof(preloadTiles), &nb);
				break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define EDITPATCHDATA_CHUNK 0x1000

IOResult PaintPatchMod::SaveLocalData(ISave *isave, LocalModData *ld) 
{
	PaintPatchData *ep =(PaintPatchData *)ld;

	isave->BeginChunk(EDITPATCHDATA_CHUNK);
	ep->Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult PaintPatchMod::LoadLocalData(ILoad *iload, LocalModData **pld) 
{
	IOResult res;
	PaintPatchData *ep;
	if (*pld == NULL)
	{
		*pld =(LocalModData *) new PaintPatchData(this);
		}
	ep =(PaintPatchData *)*pld;

	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
			case EDITPATCHDATA_CHUNK:
				res = ep->Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
		}
	return IO_OK;
	}
