#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------


void PatchVertexDelta::SetSize(PatchMesh& patch, BOOL load)
{
	dtab.MakeCompatible(patch, FALSE);
	
	// Load it if necessary
	if (load)
	{
		int verts = patch.numVerts;
		int vecs = patch.numVecs;
		int i;
		for (i = 0; i < verts; ++i)
		{
			dtab.ptab[i] = patch.verts[i].p;
			dtab.pttab[i] = patch.verts[i].flags & PVERT_COPLANAR;
		}
		for (i = 0; i < vecs; ++i)
			dtab.vtab[i] = patch.vecs[i].p;
	}
}

void PatchVertexDelta::Apply(PatchMesh &patch)
{
	// DebugPrint(_T("PVD:Applying\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);
	
	// Apply the deltas
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	int i;
	for (i = 0; i < verts; ++i)
	{
		patch.verts[i].p += dtab.ptab[i];
		patch.verts[i].flags ^= dtab.pttab[i];
	}
	for (i = 0; i < vecs; ++i)
	{
		patch.vecs[i].p += dtab.vtab[i];
	}
	patch.computeInteriors();
}

void PatchVertexDelta::UnApply(PatchMesh &patch)
{
	// DebugPrint(_T("PVD:UnApplying\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);
	
	// Apply the deltas
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	int i;
	for (i = 0; i < verts; ++i)
	{
		patch.verts[i].p -= dtab.ptab[i];
		patch.verts[i].flags ^= dtab.pttab[i];
	}
	for (i = 0; i < vecs; ++i)
	{
		patch.vecs[i].p -= dtab.vtab[i];
	}
	patch.computeInteriors();
}

// This function applies the current changes to slave handles and their knots, and zeroes everything else
void PatchVertexDelta::ApplyHandlesAndZero(PatchMesh &patch, int handleVert) 
{
	// DebugPrint(_T("PVD:ApplyAndZero\n"));
	// This does nothing if the number of verts hasn't changed in the mesh.
	SetSize(patch, FALSE);
	
	Point3 zeroPt(0.0f, 0.0f, 0.0f);
	
	// Apply the deltas	to just the slave handles
	int verts = patch.numVerts;
	int vecs = patch.numVecs;
	Point3Tab& delta = dtab.vtab;
	IntTab& kdelta = dtab.pttab;
	int i;
	for (i = 0; i < vecs; ++i)
	{
		if (!(delta[i] == zeroPt))
		{
			if (i != handleVert)
				patch.vecs[i].p += delta[i];
			else
				delta[i] = zeroPt;
		}
	}
	
	for (i = 0; i < verts; ++i)
	{
		if (kdelta[i])
			patch.verts[i].flags ^= kdelta[i];
	}
}


#define PVD_POINTTAB_CHUNK		0x1000

IOResult PatchVertexDelta::Save(ISave *isave) 
{
	isave->BeginChunk(PVD_POINTTAB_CHUNK);
	dtab.Save(isave);
	isave->	EndChunk();
	return IO_OK;
}

IOResult PatchVertexDelta::Load(ILoad *iload) 
{
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PVD_POINTTAB_CHUNK:
			res = dtab.Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}

