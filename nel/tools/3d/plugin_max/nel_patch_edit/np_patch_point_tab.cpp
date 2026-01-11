#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern Point3 zeroPoint;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

PatchPointTab::PatchPointTab() 
{
}

PatchPointTab::~PatchPointTab() 
{
}

void PatchPointTab::Empty() 
{
	ptab.Delete(0, ptab.Count());
	vtab.Delete(0, vtab.Count());
	pttab.Delete(0, pttab.Count());
}

void PatchPointTab::Zero() 
{
	// DebugPrint("Zeroing\n");
	int points = ptab.Count();
	int vectors = vtab.Count();
	Point3 zero(0, 0, 0);
	
	int i;
	for (i = 0; i < points; ++i)
	{
		ptab[i] = zero;
		pttab[i] = 0;
	}
	for (i = 0; i < vectors; ++i)
		vtab[i] = zero;
}

void PatchPointTab::MakeCompatible(PatchMesh& patch, int clear) 
{
	int izero = 0;
	if (clear)
	{
		ptab.Delete(0, ptab.Count());
		pttab.Delete(0, pttab.Count());
		vtab.Delete(0, vtab.Count());
	}
	// First, the verts
	int size = patch.numVerts;
	if (ptab.Count() > size)
	{
		int diff = ptab.Count() - size;
		ptab.Delete(ptab.Count() - diff, diff);
		pttab.Delete(pttab.Count() - diff, diff);
	}
	if (ptab.Count() < size)
	{
		int diff = size - ptab.Count();
		ptab.Resize(size);
		pttab.Resize(size);
		for (int j = 0; j < diff; j++)
		{
			ptab.Append(1, &zeroPoint);
			pttab.Append(1, &izero);
		}
	}
	// Now, the vectors
	size = patch.numVecs;
	if (vtab.Count() > size)
	{
		int diff = vtab.Count() - size;
		vtab.Delete(vtab.Count() - diff, diff);
	}
	if (vtab.Count() < size)
	{
		int diff = size - vtab.Count();
		vtab.Resize(size);
		for (int j = 0; j < diff; j++)
			vtab.Append(1, &zeroPoint);
	}
}

PatchPointTab& PatchPointTab::operator=(PatchPointTab& from) 
{
	ptab = from.ptab;
	vtab = from.vtab;
	pttab = from.pttab;
	return *this;
}

BOOL PatchPointTab::IsCompatible(PatchMesh &patch) 
{
	if (ptab.Count() != patch.numVerts)
		return FALSE;
	if (pttab.Count() != patch.numVerts)
		return FALSE;
	if (vtab.Count() != patch.numVecs)
		return FALSE;
	return TRUE;
}

void PatchPointTab::RescaleWorldUnits(float f) 
{
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	int points = ptab.Count();
	int vectors = vtab.Count();
	
	int i;
	for (i = 0; i < points; ++i)
		ptab[i] = ptab[i] * stm;
	for (i = 0; i < vectors; ++i)
		vtab[i] = vtab[i] * stm;
}

#define PPT_VERT_CHUNK		0x1000
#define PPT_VEC_CHUNK		0x1010
#define PPT_VERTTYPE_CHUNK	0x1020

IOResult PatchPointTab::Save(ISave *isave) 
{	
	int i;
	ULONG nb;
	isave->BeginChunk(PPT_VERT_CHUNK);
	int count = ptab.Count();
	isave->Write(&count, sizeof(int), &nb);
	for (i = 0; i < count; ++i)
		isave->Write(&ptab[i], sizeof(Point3), &nb);
	isave->EndChunk();
	isave->BeginChunk(PPT_VERTTYPE_CHUNK);
	count = pttab.Count();
	isave->Write(&count, sizeof(int), &nb);
	for (i = 0; i < count; ++i)
		isave->Write(&pttab[i], sizeof(int), &nb);
	isave->EndChunk();
	isave->BeginChunk(PPT_VEC_CHUNK);
	count = vtab.Count();
	isave->Write(&count, sizeof(int), &nb);
	for (i = 0; i < count; ++i)
		isave->Write(&vtab[i], sizeof(Point3), &nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult PatchPointTab::Load(ILoad *iload) 
{	
	int i, count;
	Point3 workpt;
	int workint;
	IOResult res;
	ULONG nb;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
		case PPT_VERT_CHUNK:
			ptab.Delete(0, ptab.Count());
			iload->Read(&count, sizeof(int), &nb);
			for (i = 0; i < count; ++i)
			{
				iload->Read(&workpt, sizeof(Point3), &nb);
				ptab.Append(1, &workpt);
			}
			break;
		case PPT_VERTTYPE_CHUNK:
			pttab.Delete(0, pttab.Count());
			iload->Read(&count, sizeof(int), &nb);
			for (i = 0; i < count; ++i)
			{
				iload->Read(&workint, sizeof(int), &nb);
				pttab.Append(1, &workint);
			}
			break;
		case PPT_VEC_CHUNK:
			vtab.Delete(0, vtab.Count());
			iload->Read(&count, sizeof(int), &nb);
			for (i = 0; i < count; ++i)
			{
				iload->Read(&workpt, sizeof(Point3), &nb);
				vtab.Append(1, &workpt);
			}
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}
	return IO_OK;
}
