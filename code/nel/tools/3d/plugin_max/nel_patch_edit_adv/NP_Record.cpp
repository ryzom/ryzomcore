#include "stdafx.h"
#include "editpat.h"

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


BOOL ClearPVertSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->vertSel;
	patch->vertSel.ClearAll();
	return TRUE;
}

#define CVSR_SEL_CHUNK 0x1000

IOResult ClearPVertSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case CVSR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------


BOOL SetPVertSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->vertSel;
	patch->vertSel.SetAll();
	return TRUE;
}

#define SVSR_SEL_CHUNK 0x1000

IOResult SetPVertSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case SVSR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL InvertPVertSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	patch->vertSel = ~patch->vertSel;
	return TRUE;
}

IOResult InvertPVertSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		//		switch(iload->CurChunkID())  {
		//			default:
		//				break;
		//			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL ClearPEdgeSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->edgeSel;
	patch->edgeSel.ClearAll();
	return TRUE;
}

#define CESR_SEL_CHUNK 0x1000

IOResult ClearPEdgeSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case CESR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL SetPEdgeSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->edgeSel;
	patch->edgeSel.SetAll();
	return TRUE;
}

#define SESR_SEL_CHUNK 0x1000

IOResult SetPEdgeSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case SESR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL InvertPEdgeSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	patch->edgeSel = ~patch->edgeSel;
	return TRUE;
}

IOResult InvertPEdgeSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		//		switch(iload->CurChunkID())  {
		//			default:
		//				break;
		//			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL ClearPatchSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->patchSel;
	patch->patchSel.ClearAll();
	return TRUE;
}

#define CPSR_SEL_CHUNK 0x1000

IOResult ClearPatchSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case CPSR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL SetPatchSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		sel = patch->patchSel;
	patch->patchSel.SetAll();
	return TRUE;
}

#define SPSR_SEL_CHUNK 0x1000

IOResult SetPatchSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case SPSR_SEL_CHUNK:
			res = sel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL InvertPatchSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	patch->patchSel = ~patch->patchSel;
	return TRUE;
}

IOResult InvertPatchSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		//		switch(iload->CurChunkID())  {
		//			default:
		//				break;
		//			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PVertSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (!IsCompatible(patch->vertSel, newSel))
		return FALSE;
	patch->vertSel = newSel;
	return TRUE;
}

#define VSR_OLDSEL_CHUNK 0x1000
#define VSR_NEWSEL_CHUNK 0x1010

IOResult PVertSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case VSR_OLDSEL_CHUNK:
			res = oldSel.Load(iload);
			break;
		case VSR_NEWSEL_CHUNK:
			res = newSel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PEdgeSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (!IsCompatible(patch->edgeSel, newSel))
		return FALSE;
	patch->edgeSel = newSel;
	return TRUE;
}

#define ESR_OLDSEL_CHUNK 0x1000
#define ESR_NEWSEL_CHUNK 0x1010

IOResult PEdgeSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case ESR_OLDSEL_CHUNK:
			res = oldSel.Load(iload);
			break;
		case ESR_NEWSEL_CHUNK:
			res = newSel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PatchSelRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (!IsCompatible(patch->patchSel, newSel))
		return FALSE;
	patch->patchSel = newSel;
	return TRUE;
}

#define PSR_OLDSEL_CHUNK 0x1000
#define PSR_NEWSEL_CHUNK 0x1010

IOResult PatchSelRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PSR_OLDSEL_CHUNK:
			res = oldSel.Load(iload);
			break;
		case PSR_NEWSEL_CHUNK:
			res = newSel.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define PDELR_PATCH_CHUNK		0x1060

IOResult PatchDeleteRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		//		switch(iload->CurChunkID())  {
		//			case PDELR_PATCH_CHUNK:
		//				res = oldPatch.Load(iload);
		//				break;
		//			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PVertMoveRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (!delta.IsCompatible(*patch))
		return FALSE;
	delta.Apply(*patch);
	return TRUE;
}

#define VMR_DELTA_CHUNK		0x1000

IOResult PVertMoveRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case VMR_DELTA_CHUNK:
			res = delta.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------


extern void DeleteSelVerts(PatchMesh *patch, RPatchMesh *rpatch);

BOOL PVertDeleteRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	DeleteSelVerts(patch, rpatch);
	return TRUE;
}

#define VDELR_PATCH_CHUNK		0x1060

IOResult PVertDeleteRecord::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		//		switch(iload->CurChunkID())  {
		//			case VDELR_PATCH_CHUNK:
		//				res = oldPatch.Load(iload);
		//				break;
		//			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PVertChangeRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	patch->ChangeVertType(index, type);
	return TRUE;
}

#define VCHG_GENERAL_CHUNK		0x1001
#define VCHG_PATCH_CHUNK		0x1010

IOResult PVertChangeRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case VCHG_GENERAL_CHUNK:
			res = iload->Read(&index, sizeof(int), &nb);
			res = iload->Read(&type, sizeof(int), &nb);
			break;
			//			case VCHG_PATCH_CHUNK:
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

BOOL PAttachRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
		oldPatchCount = patch->numPatches;
	patch->Attach(&attPatch, mtlOffset);
	return TRUE;
}

#define ATTR_GENERAL_CHUNK		0x1001
#define ATTR_ATTPATCH_CHUNK		0x1010
#define ATTR_MTLOFFSET_CHUNK	0x1020

IOResult PAttachRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case ATTR_GENERAL_CHUNK:
			res = iload->Read(&oldPatchCount, sizeof(int), &nb);
			break;
		case ATTR_ATTPATCH_CHUNK:
			res = attPatch.Load(iload);
			break;
		case ATTR_MTLOFFSET_CHUNK:
			res = iload->Read(&mtlOffset, sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PatchDetachRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord && !copy)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	if (!copy)
	{
		BitArray vdel(patch->numVerts);
		vdel.ClearAll();
		BitArray pdel = patch->patchSel;
		DeletePatchParts(patch, rpatch, vdel, pdel);
	}
	return TRUE;
}

#define PDETR_GENERAL_CHUNK		0x1000
#define PDETR_PATCH_CHUNK		0x1030

IOResult PatchDetachRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PDETR_GENERAL_CHUNK:
			res = iload->Read(&copy, sizeof(int), &nb);
			break;
			//			case PDETR_PATCH_CHUNK:
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

BOOL PatchMtlRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	for (int i = 0; i < patch->numPatches; ++i)
	{
		if (patch->patchSel[i])
			patch->patches[i].setMatID(index);
	}
	return TRUE;
}

#define PMTLR_GENERAL_CHUNK		0x1000
#define PMTLR_INDEX_CHUNK		0x1020

IOResult PatchMtlRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PMTLR_INDEX_CHUNK:
			res = iload->Read(&index, sizeof(MtlID), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
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

#define PADDR_TYPE_CHUNK		0x1000
#define PADDR_PATCH_CHUNK		0x1010
#define PADDR_POSTWELD_CHUNK	0x1020

IOResult PatchAddRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	postWeld = FALSE;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PADDR_TYPE_CHUNK:
			res = iload->Read(&type, sizeof(int), &nb);
			break;
			//			case PADDR_PATCH_CHUNK:
			//				res = oldPatch.Load(iload);
			//				break;
			// If the following chunk is present, it's a MAX 2.0 file and a post-addition
			// weld is to be performed
		case PADDR_POSTWELD_CHUNK:
			postWeld = TRUE;
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

// Compute midpoint division for patch vectors -- Provide patchmesh, patch number, 4 bez points
// returns 2 new vectors
/*
static Point3 InterpPoint(PatchMesh *patch, int index, float pct, int e1, int i1, int i2, int e2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	PatchVec *v = patch->vecs;
	Patch &p = patch->patches[index];
	Point3 pe1 = v[p.vec[e1]].p;
	Point3 pe2 = v[p.vec[e2]].p;
	Point3 pi1 = v[p.interior[i1]].p;
	Point3 pi2 = v[p.interior[i2]].p;
	Point3 e1i1 = pe1 +(pi1 - pe1) * pct;
	Point3 i1i2 = pi1 +(pi2 - pi1) * pct;
	Point3 i2e2 = pi2 +(pe2 - pi2) * pct;
	Point3 a = e1i1 +(i1i2 - e1i1) * pct;
	Point3 b = i1i2 +(i2e2 - i1i2) * pct;
	if (v1)
		*v1 = e1i1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = i2e2;
	return a +(b - a) * pct;
}

static Point3 InterpPoint(float pct, Point3 e1, Point3 i1, Point3 i2, Point3 e2, Point3 *v1 = NULL, Point3 *v2 = NULL, Point3 *v3 = NULL, Point3 *v4 = NULL) 
{
	Point3 e1i1 = e1 +(i1 - e1) * pct;
	Point3 i1i2 = i1 +(i2 - i1) * pct;
	Point3 i2e2 = i2 +(e2 - i2) * pct;
	Point3 a = e1i1 +(i1i2 - e1i1) * pct;
	Point3 b = i1i2 +(i2e2 - i1i2) * pct;
	if (v1)
		*v1 = e1i1;
	if (v2)
		*v2 = a;
	if (v3)
		*v3 = b;
	if (v4)
		*v4 = i2e2;
	return a +(b - a) * pct;
}

static Point3 InterpLinear(Point3 a, Point3 b, float interp) 
{
	return a +(a - b) * interp;
}

static Point3 InterpDegree2(Point3 a, Point3 b, Point3 c, float interp) 
{
	Point3 ab = a +(b - a) * interp;
	Point3 bc = b +(c - b) * interp;
	return ab +(bc - ab) * interp;
}

static Point3 InterpDegree3(Point3 a, Point3 b, Point3 c, Point3 d, float interp) 
{
	Point3 ab = a +(b - a) * interp;
	Point3 bc = b +(c - b) * interp;
	Point3 cd = c +(d - c) * interp;
	Point3 abbc = ab +(bc - ab) * interp;
	Point3 bccd = bc +(cd - bc) * interp;
	return abbc +(bccd - abbc) * interp;
}
*/
extern void SubdividePatch(int type, BOOL propagate, PatchMesh *patch, RPatchMesh *rpatch);

BOOL EdgeSubdivideRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	SubdividePatch(SUBDIV_EDGES, propagate, patch, rpatch);
	return TRUE;
}

#define ESUBR_PROPAGATE_CHUNK		0x1000
#define ESUBR_PATCH_CHUNK			0x1010

IOResult EdgeSubdivideRecord::Load(ILoad *iload) 
{
	IOResult res;
	propagate = FALSE;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case ESUBR_PROPAGATE_CHUNK:
			propagate = TRUE;
			break;
			//			case ESUBR_PATCH_CHUNK:
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

BOOL PatchSubdivideRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	SubdividePatch(SUBDIV_PATCHES, propagate, patch, rpatch);
	return TRUE;
}

#define PSUBR_PROPAGATE_CHUNK		0x1000
#define PSUBR_PATCH_CHUNK			0x1010

IOResult PatchSubdivideRecord::Load(ILoad *iload) 
{
	IOResult res;
	propagate = FALSE;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PSUBR_PROPAGATE_CHUNK:
			propagate = TRUE;
			break;
			//			case PSUBR_PATCH_CHUNK:
			//				res = oldPatch.Load(iload);
			//				break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

BOOL PVertWeldRecord::Redo(PatchMesh *patch, RPatchMesh *rpatch, int reRecord) 
{
	if (reRecord)
	{
		oldPatch = *patch;
		roldPatch = *rpatch;
	}
	patch->Weld(thresh);
	return TRUE;
}

#define WELDR_THRESH_CHUNK			0x1010
#define WELDR_PATCH_CHUNK			0x1000

IOResult PVertWeldRecord::Load(ILoad *iload) 
{
	IOResult res;
	ULONG nb;
	propagate = FALSE;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case WELDR_THRESH_CHUNK:
			res = iload->Read(&thresh, sizeof(float), &nb);
			break;
			//			case WELDR_PATCH_CHUNK:
			//				res = oldPatch.Load(iload);
			//				break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}
