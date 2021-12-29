#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern Point3 zeroPoint;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

EPVertMapper::~EPVertMapper() 
{
	if (vertMap)
	{
		delete[] vertMap;
		vertMap = NULL;
	}
	if (vecMap)
	{
		delete[] vecMap;
		vecMap = NULL;
	}
}

void EPVertMapper::Build(PatchMesh &patch) 
{
	verts = patch.numVerts;
	if (vertMap)
		delete[] vertMap;
	vertMap = new EPMapVert[verts];
	vecs = patch.numVecs;
	if (vecMap)
		delete[] vecMap;
	vecMap = new EPMapVert[vecs];
	int i;
	for (i = 0; i < verts; ++i)
		vertMap[i] = EPMapVert(i, patch.verts[i].p, zeroPoint);
	for (i = 0; i < vecs; ++i)
		vecMap[i] = EPMapVert(i, patch.vecs[i].p, zeroPoint);
}

void EPVertMapper::RecordTopologyTags(PatchMesh &patch) 
{
	int i;
	for (i = 0; i < verts; ++i)
	{
		// If it's still mapped, record it!
		if (vertMap[i].vert >= 0)
			patch.verts[vertMap[i].vert].aux1 = i;
	}
	for (i = 0; i < vecs; ++i)
	{
		// If it's still mapped, record it!
		if (vecMap[i].vert >= 0)
			patch.vecs[vecMap[i].vert].aux1 = i;
	}
}

void EPVertMapper::UpdateMapping(PatchMesh &patch) 
{
	// Flush existing mapping
	int i;
	for (i = 0; i < verts; ++i)
		vertMap[i].vert = -1;
	for (i = 0; i < vecs; ++i)
		vecMap[i].vert = -1;
	// Build the new mapping
	int verts = patch.numVerts;
	for (int vert = 0; vert < verts; ++vert)
	{
		int aux = patch.verts[vert].aux1;
		if (aux != 0xffffffff)
			vertMap[aux].vert = vert;
	}
	int vecs = patch.numVecs;
	for (int vec = 0; vec < vecs; ++vec)
	{
		int aux = patch.vecs[vec].aux1;
		if (aux != 0xffffffff)
			vecMap[aux].vert = vec;
	}
}

void EPVertMapper::RecomputeDeltas(PatchMesh &patch) 
{
	int i;
	for (i = 0; i < verts; ++i)
	{
		EPMapVert &map = vertMap[i];
		if (map.vert >= 0 && map.originalStored)
		{
			Point3 pnew = patch.verts[map.vert].p;
#ifdef VMAP_DEBUG
			Point3 oldDelta = map.delta;
#endif
			map.delta = pnew - map.original;
#ifdef VMAP_DEBUG
			if (map.delta != oldDelta)
				DebugPrint("Vert %d delta changed from %.2f %.2f %.2f to %.2 %.2f %.2f\n", i, oldDelta.x, oldDelta.y, oldDelta.z, map.delta.x, map.delta.y, map.delta.z);
#endif
		}
	}
	for (i = 0; i < vecs; ++i)
	{
		EPMapVert &map = vecMap[i];
		if (map.vert >= 0 && map.originalStored)
		{
			Point3 pnew = patch.vecs[map.vert].p;
#ifdef VMAP_DEBUG
			Point3 oldDelta = map.delta;
#endif
			map.delta = pnew - map.original;
#ifdef VMAP_DEBUG
			if (map.delta != oldDelta)
				DebugPrint("Vec %d delta changed from %.2f %.2f %.2f to %.2 %.2f %.2f\n", i, oldDelta.x, oldDelta.y, oldDelta.z, map.delta.x, map.delta.y, map.delta.z);
#endif
		}
	}
}

void EPVertMapper::UpdateAndApplyDeltas(PatchMesh &inPatch, PatchMesh &outPatch) 
{
	
	// watje 4-27-99 here to handle 0 patch situations
	if (inPatch.numPatches == 0)
	{
		//		outPatch.setNumVerts(0,TRUE); 
		return;
	}
	
	
	// Update the original point locations
	int i;
	for (i = 0; i < verts; ++i)
	{
		// If this table has more in it than we need, forget the rest
		// This can happen if the input object changes to fewer verts
		if (i >= inPatch.numVerts)
			break;
		// If it's still mapped, update it!
		if (vertMap[i].vert >= 0)
		{
			vertMap[i].original = inPatch.verts[i].p;
			vertMap[i].originalStored = TRUE;
		}
	}
	for (i = 0; i < vecs; ++i)
	{
		// If this table has more in it than we need, forget the rest
		// This can happen if the input object changes to fewer vecs
		if (i >= inPatch.numVecs)
			break;
		// If it's still mapped, update it!
		if (vecMap[i].vert >= 0)
		{
			vecMap[i].original = inPatch.vecs[i].p;
			vecMap[i].originalStored = TRUE;
		}
	}
	// Now apply to output
	for (i = 0; i < verts; ++i)
	{
		EPMapVert &pv = vertMap[i];
		if (pv.vert >= 0 && pv.originalStored)
		{
			//  nlassert(pv.vert >= 0 && pv.vert < outPatch.numVerts);
			// watje 4-27-99 instead just throwing an nlassert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.numVerts))
			{
				outPatch.setNumVerts(pv.vert + 1, TRUE); 
				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
					warning = GetString(IDS_PW_SURFACEERROR);
				
				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK | MB_APPLMODAL);
			}
			
			if (i >= inPatch.numVerts) 
				outPatch.verts[pv.vert].p = zeroPoint;
			else
				outPatch.verts[pv.vert].p = pv.original + pv.delta;
#ifdef VMAP_DEBUG
			if (pv.delta != zeroPoint)
				DebugPrint("Vert %d applied delta of %.2f %.2f %.2f\n", i, pv.delta.x, pv.delta.y, pv.delta.z);
#endif
		}
	}
	for (i = 0; i < vecs; ++i)
	{
		EPMapVert &pv = vecMap[i];
		if (pv.vert >= 0 && pv.originalStored)
		{
			// nlassert(pv.vert >= 0 && pv.vert < outPatch.numVecs);
			// watje 4-27-99 instead just throwing an nlassert it pops a message box up and troes to recover
			if (!(pv.vert >= 0 && pv.vert < outPatch.numVecs))
			{
				outPatch.setNumVecs(pv.vert + 1, TRUE); 
				
				TSTR title = GetString(IDS_TH_EDITPATCH_CLASS),
					warning = GetString(IDS_PW_SURFACEERROR);
				
				MessageBox(GetCOREInterface()->GetMAXHWnd(),
					warning, title, MB_OK | MB_APPLMODAL);
			}
			
			if (i >= inPatch.numVecs) 
				outPatch.vecs[pv.vert].p = zeroPoint;
			else
				outPatch.vecs[pv.vert].p = pv.original + pv.delta;
#ifdef VMAP_DEBUG
			if (pv.delta != zeroPoint)
				DebugPrint("Vec %d applied delta of %.2f %.2f %.2f\n", i, pv.delta.x, pv.delta.y, pv.delta.z);
#endif
		}
	}
}

EPVertMapper& EPVertMapper::operator=(EPVertMapper &from) 
{
	if (vertMap)
		delete[] vertMap;
	verts = from.verts;
	vertMap = new EPMapVert[verts];
	int i;
	for (i = 0; i < verts; ++i)
		vertMap[i] = from.vertMap[i];
	if (vecMap)
		delete[] vecMap;
	vecs = from.vecs;
	vecMap = new EPMapVert[vecs];
	for (i = 0; i < vecs; ++i)
		vecMap[i] = from.vecMap[i];
	return *this;
}

void EPVertMapper::RescaleWorldUnits(float f) 
{
	int i;
	for (i = 0; i < verts; ++i)
	{
		vertMap[i].delta *= f;
		if (vertMap[i].originalStored)
			vertMap[i].original *= f;
	}
	for (i = 0; i < vecs; ++i)
	{
		vecMap[i].delta *= f;
		if (vecMap[i].originalStored)
			vecMap[i].original *= f;
	}
}

#define EPVM_DATA_CHUNK 0x1000

IOResult EPVertMapper::Save(ISave *isave) 
{
	ULONG nb;
	isave->BeginChunk(EPVM_DATA_CHUNK);
	isave->Write(&verts, sizeof(int), &nb);
	isave->Write(vertMap, sizeof(EPMapVert) * verts, &nb);
	isave->Write(&vecs, sizeof(int), &nb);
	isave->Write(vecMap, sizeof(EPMapVert) * vecs, &nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult EPVertMapper::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	int index = 0;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case EPVM_DATA_CHUNK:
			res = iload->Read(&verts, sizeof(int), &nb);
			if (vertMap)
				delete[] vertMap;
			vertMap = new EPMapVert[verts];
			res = iload->Read(vertMap, sizeof(EPMapVert) * verts, &nb);
			res = iload->Read(&vecs, sizeof(int), &nb);
			if (vecMap)
				delete[] vecMap;
			vecMap = new EPMapVert[vecs];
			res = iload->Read(vecMap, sizeof(EPMapVert) * vecs, &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}


