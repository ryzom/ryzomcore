#include "stdafx.h"
#include "nel_patch_paint.h"
#include "nel/misc/time_nl.h"

using namespace NLMISC;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

class FinalPatchRestore : public RestoreObj 
{
public:
	BOOL gotRedo;
	PatchMesh undo;
	PatchMesh redo;
	PatchMesh *patch;
	RPatchMesh *rundo;
	RPatchMesh *rredo;
	RPatchMesh *rpatch;
	
	FinalPatchRestore(PatchMesh *s, RPatchMesh *rs)
	{
		rundo = NULL;
		rredo = NULL;

		undo = *s;

		if (rs)
		{
			rundo=new RPatchMesh();
			*rundo = *rs;
		}

		patch = s;
		rpatch = rs;
		gotRedo = FALSE;
	}
	
	virtual ~FinalPatchRestore()
	{
		if (rundo)
			delete rundo;
		if (rredo)
			delete rredo;
	}

	void Restore(int isUndo) 
	{
		if (!gotRedo)
		{
			gotRedo = TRUE;
			redo = *patch;

			if (rpatch)
			{
				if (rredo==NULL)
					rredo=new RPatchMesh();

				*rredo = *rpatch;
			}
		}
		*patch = undo;

		if (rundo)
			*rpatch = *rundo;
	}
	
	void Redo() 
	{
		*patch = redo;

		if (rredo)
			*rpatch = *rredo;
	}
	
	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("FinalPatchRestore")); }
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------

PaintPatchData::PaintPatchData (PaintPatchMod *mod)
{
	flags = 0;
	tempData = NULL;
	preloadTiles = mod->preloadTiles;
}

PaintPatchData::PaintPatchData (PaintPatchData& emc)
{
	flags = emc.flags;
	tempData = NULL;
	finalPatch = emc.finalPatch;
	rfinalPatch = emc.rfinalPatch;
	preloadTiles = emc.preloadTiles;
}

void PaintPatchData::Apply(TimeValue t, RPO *patchOb, int selLevel)
{
	TTicks ticks=CTime::getPerformanceTime ();
	// Either just copy it from the existing cache or rebuild from previous level!
	if (!GetFlag(EPD_UPDATING_CACHE) && tempData 
		&& tempData->PatchCached(t))
	{
		RPatchMesh *rpatch;
		PatchMesh *patch=tempData->GetPatch(t, rpatch);
		patchOb->patch.DeepCopy( patch,
			PART_GEOM | SELECT_CHANNEL | PART_SUBSEL_TYPE|
			PART_DISPLAY | PART_TOPO | TEXMAP_CHANNEL);
		//rpatch->UpdateBinding (*patch, t);
		*patchOb->rpatch=*rpatch;
		patchOb->PointsWereChanged();
	}	
	else if (GetFlag(EPD_HASDATA))
	{
		int count = changes.Count();
		if (count)
		{
			finalPatch = patchOb->patch;
			rfinalPatch = *patchOb->rpatch;
			for (int i = 0; i < count; ++i)
			{
				PModRecord *rec = changes[i];
				// Record the topo flags
				RecordTopologyTags(&patchOb->patch);
				BOOL result = rec->Redo(&patchOb->patch, patchOb->rpatch, 0);
				UpdateChanges(&patchOb->patch, patchOb->rpatch);
				// If we hit one that didn't play back OK, we need to flush the remainder
				if (!result)
				{
					for (int j = i; j < count; ++j)
						delete changes[j];
					changes.Delete(i, count - i);
					break;
				}
			}
			// Nuke the changes table
			count = changes.Count();
			for (int k = 0; k < count; ++k)
				delete changes[k];
			changes.Delete(0, count);
			changes.Shrink();
			count = 0;
		}
		else 
		{
			// Apply deltas to incoming shape, placing into finalPatch
			patchOb->patch = finalPatch;
			*patchOb->rpatch = rfinalPatch;
		}
		patchOb->PointsWereChanged();
		// Kind of a waste when there's no animation...		
		patchOb->UpdateValidity(GEOM_CHAN_NUM, FOREVER);
		patchOb->UpdateValidity(TOPO_CHAN_NUM, FOREVER);
		patchOb->UpdateValidity(SELECT_CHAN_NUM, FOREVER);
		patchOb->UpdateValidity(SUBSEL_TYPE_CHAN_NUM, FOREVER);
		patchOb->UpdateValidity(DISP_ATTRIB_CHAN_NUM, FOREVER);
	}
	else 
	{
		finalPatch = patchOb->patch;
		rfinalPatch = *patchOb->rpatch;
	}

	patchOb->patch.dispFlags = 0;

	if (GetFlag(EPD_UPDATING_CACHE))
	{
		nlassert(tempData);
		tempData->UpdateCache(patchOb);
		SetFlag(EPD_UPDATING_CACHE, FALSE);
	}		
	ticks=CTime::getPerformanceTime ()-ticks;
	nldebug ("%f", CTime::ticksToSecond(ticks));
}

void PaintPatchData::Invalidate(PartID part, BOOL patchValid)
{
	if (tempData)
	{
		tempData->Invalidate(part, patchValid);
	}
}

void PaintPatchData::BeginEdit(TimeValue t)
{
	nlassert(tempData);
	if (!GetFlag(EPD_HASDATA))
		SetFlag(EPD_HASDATA, TRUE);
}

EPTempData *PaintPatchData::TempData(PaintPatchMod *mod)
{
	if (!tempData)
	{
		nlassert(mod->ip);
		tempData = new EPTempData(mod, this);
	}
	return tempData;
}

void PaintPatchData::RescaleWorldUnits(float f) 
{
	// Now rescale stuff inside our data structures
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	finalPatch.Transform(stm);
}

void PaintPatchData::RecordTopologyTags(PatchMesh *patch) 
{
	// First, stuff all -1's into aux fields
	int i;
	for (i = 0; i < patch->numVerts; ++i)
		patch->verts[i].aux1 = 0xffffffff;
	for (i = 0; i < patch->numVecs; ++i)
		patch->vecs[i].aux1 = 0xffffffff;
	for (i = 0; i < patch->numPatches; ++i)
		patch->patches[i].aux1 = 0xffffffff;
}

void PaintPatchData::UpdateChanges(PatchMesh *patch, RPatchMesh *rpatch, BOOL checkTopology) 
{
	if (theHold.Holding())
	{
		//theHold.Put(new FinalPatchRestore(&finalPatch, &rfinalPatch));
		if (rpatch)
			theHold.Put(new FinalPatchRestore(&finalPatch, &rfinalPatch));
		else
			theHold.Put(new FinalPatchRestore(&finalPatch, NULL));
	}
	// Store the final shape
	finalPatch = *patch;

	if (rpatch)
		rfinalPatch = *rpatch;
}

#define EPD_GENERAL_CHUNK		0x1000	// Obsolete as of 11/12/98 (r3)
#define CHANGE_CHUNK			0x1010 	// Obsolete as of 11/12/98 (r3)
#define EPD_R3_GENERAL_CHUNK	0x1015
#define MESH_ATTRIB_CHUNK		0x1020
#define DISP_PARTS_CHUNK		0x1030
#define VTESS_ATTRIB_CHUNK		0x1070
#define PTESS_ATTRIB_CHUNK		0x1080
#define DTESS_ATTRIB_CHUNK		0x1090
#define NORMAL_TESS_ATTRIB_CHUNK	0x1110
#define WELD_TESS_ATTRIB_CHUNK	0x1120
#define VERTMAP_CHUNK			0x1130
#define FINALPATCH_CHUNK		0x1140
#define RENDERSTEPS_CHUNK		0x1150
#define SHOWINTERIOR_CHUNK		0x1160

// Named sel set chunks
#define VSELSET_CHUNK		0x1040
#define ESELSET_CHUNK		0x1050
#define PSELSET_CHUNK		0x1060

#define RPO_MODE_TILE 0x4000
#define RFINALPATCH_CHUNK 0x4001
#define RPO_MODE_TILE_TRANSITION 0x4002
#define RPO_INCLUDE_MESHES 0x4003
#define RPO_PRELOAD_TILES 0x4010

IOResult PaintPatchData::Save(ISave *isave) 
{
	ULONG nb;
	isave->BeginChunk(EPD_R3_GENERAL_CHUNK);
	isave->Write(&flags, sizeof(DWORD), &nb);
	isave->EndChunk();

	isave->BeginChunk(FINALPATCH_CHUNK);
	finalPatch.Save(isave);
	isave->EndChunk();
	
	isave->BeginChunk(RFINALPATCH_CHUNK);
	rfinalPatch.Save(isave);
	isave->EndChunk();	

	isave->BeginChunk(RPO_INCLUDE_MESHES);
	isave->Write(&includeMeshes, sizeof(includeMeshes), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(RPO_PRELOAD_TILES);
	isave->Write(&preloadTiles, sizeof(preloadTiles), &nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult PaintPatchData::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	PModRecord *theChange;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
			// The following code is here to load pre-release 3 files.
		case EPD_GENERAL_CHUNK:
			iload->SetObsolete();
			iload->Read(&flags, sizeof(DWORD), &nb);
			break;
		case PATCHCHANGERECORD_CHUNK:
			theChange = new PatchChangeRecord;
			goto load_change;
load_change:
			//
			// The following code is used for post-release 3 files
			//
		case EPD_R3_GENERAL_CHUNK:
			res = iload->Read(&flags, sizeof(DWORD), &nb);
			break;
		case FINALPATCH_CHUNK:
			res = finalPatch.Load(iload);
			break;
		case RFINALPATCH_CHUNK:
			res = rfinalPatch.Load(iload);
			break;
			
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
