#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define OLD_SEL_LEVEL_CHUNK 0x1000	// Original backwards ordering
#define SEL_LEVEL_CHUNK 0x1001
#define DISP_LATTICE_CHUNK 0x1010
#define DISP_SURFACE_CHUNK 0x1020
#define DISP_VERTS_CHUNK 0x1030
#define EPM_MESH_ATTRIB_CHUNK	0x1040
#define EPM_VTESS_ATTRIB_CHUNK	0x1090
#define EPM_PTESS_ATTRIB_CHUNK	0x10a0
#define EPM_DTESS_ATTRIB_CHUNK	0x10b0
#define EPM_NORMAL_TESS_ATTRIB_CHUNK	0x10c0
#define EPM_WELD_TESS_ATTRIB_CHUNK	0x10d0
#define EPM_RENDERSTEPS_CHUNK		0x10e0
#define EPM_SHOWINTERIOR_CHUNK		0x10f0
// The following chunk is written on r3 and later files
// If not present, named selection data structures need fixup
#define EPM_SEL_NAMES_OK 0x1100	

// Names of named selection sets
#define NAMEDVSEL_NAMES_CHUNK	0x1050
#define NAMEDESEL_NAMES_CHUNK	0x1060
#define NAMEDPSEL_NAMES_CHUNK	0x1070
#define NAMEDSEL_STRING_CHUNK	0x1080

#define RPO_MODE_TILE 0x2000
#define RPO_MODE_TILE_TRANSITION 0x2001
#define RPO_INCLUDE_MESHES 0x2002

static int namedSelID[] = 
{
	NAMEDVSEL_NAMES_CHUNK,
	NAMEDESEL_NAMES_CHUNK,
	NAMEDPSEL_NAMES_CHUNK
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult EditPatchMod::Save(ISave *isave) 
{
	Modifier::Save(isave);
	Interval valid;
	ULONG nb;
	// In r3 and later, if the named sel names are OK, write this chunk
	if (!namedSelNeedsFixup)
	{
		isave->BeginChunk(EPM_SEL_NAMES_OK);
		isave->EndChunk();
		}
	isave->BeginChunk(SEL_LEVEL_CHUNK);
	isave->Write(&selLevel, sizeof(int), &nb);
	isave->	EndChunk();
	isave->BeginChunk(DISP_LATTICE_CHUNK);
	isave->Write(&displayLattice, sizeof(BOOL), &nb);
	isave->	EndChunk();
	isave->BeginChunk(DISP_SURFACE_CHUNK);
	isave->Write(&displaySurface, sizeof(BOOL), &nb);
	isave->	EndChunk();
	isave->BeginChunk(EPM_MESH_ATTRIB_CHUNK);
	isave->Write(&meshSteps, sizeof(int), &nb);
// Future use (Not used now)
	BOOL fakeAdaptive = FALSE;
	isave->Write(&fakeAdaptive, sizeof(BOOL), &nb);
//	isave->Write(&meshAdaptive,sizeof(BOOL),&nb);	// Future use (Not used now)
	isave->	EndChunk();

// 3-18-99 to suport render steps and removal of the mental tesselator
	isave->BeginChunk(EPM_RENDERSTEPS_CHUNK);
	if ((meshStepsRender < 0) ||(meshStepsRender > 100))
		{
		meshStepsRender = 5;
		nlassert(0);
		}
	isave->Write(&meshStepsRender, sizeof(int), &nb);
	isave->	EndChunk();
	isave->BeginChunk(EPM_SHOWINTERIOR_CHUNK);
	isave->Write(&showInterior, sizeof(BOOL), &nb);
	isave->	EndChunk();

	isave->BeginChunk(EPM_VTESS_ATTRIB_CHUNK);
	viewTess.Save(isave);
	isave->	EndChunk();
	isave->BeginChunk(EPM_PTESS_ATTRIB_CHUNK);
	prodTess.Save(isave);
	isave->	EndChunk();
	isave->BeginChunk(EPM_DTESS_ATTRIB_CHUNK);
	dispTess.Save(isave);
	isave->	EndChunk();

	// Tile mode
	isave->BeginChunk(RPO_MODE_TILE);
	isave->Write(&tileMode, sizeof(tileMode), &nb);
	isave->Write(&tileLevel, sizeof(tileLevel), &nb);
	isave->Write(&keepMapping, sizeof(keepMapping), &nb);
	isave->	EndChunk();

	// Tile mode
	isave->BeginChunk(RPO_INCLUDE_MESHES);
	isave->Write(&includeMeshes, sizeof(includeMeshes), &nb);
	isave->	EndChunk();

	// Tile mode
	isave->BeginChunk(RPO_MODE_TILE_TRANSITION);
	isave->Write(&transitionType, sizeof(transitionType), &nb);
	isave->	EndChunk();

	isave->BeginChunk(EPM_NORMAL_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessNormals, sizeof(BOOL), &nb);
	isave->Write(&mProdTessNormals, sizeof(BOOL), &nb);
	isave->	EndChunk();
	isave->BeginChunk(EPM_WELD_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessWeld, sizeof(BOOL), &nb);
	isave->Write(&mProdTessWeld, sizeof(BOOL), &nb);
	isave->	EndChunk();
	
	// Save names of named selection sets
	for (int j = 0; j < 3; j++)
	{
		if (namedSel[j].Count())
		{
			isave->BeginChunk(namedSelID[j]);			
			for (int i = 0; i < namedSel[j].Count(); i++)
			{
				isave->BeginChunk(NAMEDSEL_STRING_CHUNK);
				isave->WriteWString(*namedSel[j][i]);
				isave->EndChunk();
				}
			isave->EndChunk();
			}
		}
	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult EditPatchMod::LoadNamedSelChunk(ILoad *iload, int level)
	{	
	IOResult res;
	
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
			case NAMEDSEL_STRING_CHUNK: 
				{
				TCHAR *name;
				res = iload->ReadWStringChunk(&name);
				// Set the name in the modifier
				AddSet(TSTR(name), level + 1);
				break;
				}
			}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
		}
	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult EditPatchMod::Load(ILoad *iload) 
{
	Modifier::Load(iload);
	IOResult res;
	ULONG nb;
	namedSelNeedsFixup = TRUE;	// Pre-r3 default
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())
		{
			case EPM_SEL_NAMES_OK:
				namedSelNeedsFixup = FALSE;
				break;
			case NAMEDVSEL_NAMES_CHUNK: 
				{				
				res = LoadNamedSelChunk(iload, 0);
				break;
				}
			case NAMEDESEL_NAMES_CHUNK: 
				{
				res = LoadNamedSelChunk(iload, 1);
				break;
				}
			case NAMEDPSEL_NAMES_CHUNK: 
				{
				res = LoadNamedSelChunk(iload, 2);
				break;
				}

			case OLD_SEL_LEVEL_CHUNK:	// Correct backwards ordering
				{
				short sl;
				res = iload->Read(&sl, sizeof(short), &nb);
				selLevel = sl;
				switch (selLevel)
				{
					case 1:
						selLevel = EP_PATCH;
						break;
					case 3:
						selLevel = EP_VERTEX;
						break;
					}
				}
				break;
			case SEL_LEVEL_CHUNK:
				res = iload->Read(&selLevel, sizeof(int), &nb);
				break;
			case DISP_LATTICE_CHUNK:
				res = iload->Read(&displayLattice, sizeof(BOOL), &nb);
				break;
			case DISP_SURFACE_CHUNK:
				res = iload->Read(&displaySurface, sizeof(BOOL), &nb);
				break;
			case DISP_VERTS_CHUNK:
				iload->SetObsolete();
				break;
			case EPM_MESH_ATTRIB_CHUNK:
				res = iload->Read(&meshSteps, sizeof(int), &nb);
				res = iload->Read(&meshAdaptive, sizeof(BOOL), &nb);
				break;
// 3-18-99 to suport render steps and removal of the mental tesselator
			case EPM_RENDERSTEPS_CHUNK:
				res = iload->Read(&meshStepsRender, sizeof(int), &nb);
				if ((meshStepsRender < 0) ||(meshStepsRender > 100))
					{
					meshStepsRender = 5;
					nlassert(0);
					}
				break;
			case EPM_SHOWINTERIOR_CHUNK:
				res = iload->Read(&showInterior, sizeof(BOOL), &nb);
				break;

			case EPM_VTESS_ATTRIB_CHUNK:
				viewTess.Load(iload);
				break;
			case EPM_PTESS_ATTRIB_CHUNK:
				prodTess.Load(iload);
				break;
			case EPM_DTESS_ATTRIB_CHUNK:
				dispTess.Load(iload);
				break;
			case EPM_NORMAL_TESS_ATTRIB_CHUNK:
				res = iload->Read(&mViewTessNormals, sizeof(BOOL), &nb);
				res = iload->Read(&mProdTessNormals, sizeof(BOOL), &nb);
				break;
			case EPM_WELD_TESS_ATTRIB_CHUNK:
				res = iload->Read(&mViewTessWeld, sizeof(BOOL), &nb);
				res = iload->Read(&mProdTessWeld, sizeof(BOOL), &nb);
				break;

			case RPO_MODE_TILE:
				res = iload->Read(&tileMode, sizeof(tileMode), &nb);
				res = iload->Read(&tileLevel, sizeof(tileLevel), &nb);
				res = iload->Read(&keepMapping, sizeof(keepMapping), &nb);
				break;

			case RPO_INCLUDE_MESHES:
				res = iload->Read(&includeMeshes, sizeof(includeMeshes), &nb);
				break;

			case RPO_MODE_TILE_TRANSITION:
				res = iload->Read(&transitionType, sizeof(transitionType), &nb);
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

IOResult EditPatchMod::SaveLocalData(ISave *isave, LocalModData *ld) 
{
	EditPatchData *ep =(EditPatchData *)ld;

	isave->BeginChunk(EDITPATCHDATA_CHUNK);
	ep->Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

IOResult EditPatchMod::LoadLocalData(ILoad *iload, LocalModData **pld) 
{
	IOResult res;
	EditPatchData *ep;
	if (*pld == NULL)
	{
		*pld =(LocalModData *) new EditPatchData(this);
		}
	ep =(EditPatchData *)*pld;

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
