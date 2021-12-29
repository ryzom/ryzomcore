#include "stdafx.h"
#include "nel_patch_paint.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

extern void DeletePatchParts(PatchMesh *patch, RPatchMesh *rpatch, BitArray &delVerts, BitArray &delPatches);

// ------------------------------------------------------------------------------------------------------------------------------------------------------

static BOOL IsCompatible(BitArray &a, BitArray &b) 
{
	return (a.GetSize() == b.GetSize()) ? TRUE : FALSE;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------

void ChangePatchType(PatchMesh *patch, int index, int type) 
{
	// If positive vertex number, do it to just one vertex
	if (index >= 0)
	{
		patch->patches[index].flags = type;
		patch->computeInteriors();
		return;
	}
	
	// Otherwise, do it to all selected vertices!
	int patches = patch->numPatches;
	BitArray &psel = patch->patchSel;
	for (int i = 0; i < patches; ++i)
	{
		if (psel[i])
			patch->patches[i].flags = type;
	}
	patch->computeInteriors();
}



BOOL PatchChangeRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (index >= 0 && index >= patch->numPatches)
		return FALSE;
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	ChangePatchType(patch, index, type);
	return TRUE;
}

#define PCHG_GENERAL_CHUNK		0x1001
#define PCHG_PATCH_CHUNK		0x1010

IOResult PatchChangeRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PCHG_GENERAL_CHUNK:
			res = iload->Read(&index, sizeof(int), &nb);
			res = iload->Read(&type, sizeof(int), &nb);
			break;
			//			case PCHG_PATCH_CHUNK:
			//				res = oldPatch.Load(iload);
			//				break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------
