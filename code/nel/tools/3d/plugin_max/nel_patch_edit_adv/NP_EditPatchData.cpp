#include "stdafx.h"
#include "editpat.h"
#include "nel/misc/time_nl.h"

using namespace NLMISC;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

class EPVertMapRestore : public RestoreObj 
{
public:
	BOOL gotRedo;
	EPVertMapper undo;
	EPVertMapper redo;
	EditPatchData *epd;
	
	EPVertMapRestore(EditPatchData *d) 
	{
		undo = d->vertMap;
		epd = d;
		gotRedo = FALSE;
	}
	
	void Restore(int isUndo) 
	{
		if (!gotRedo)
		{
			gotRedo = TRUE;
			redo = epd->vertMap;
		}
		epd->vertMap = undo;
	}
	
	void Redo() 
	{
		epd->vertMap = redo;
	}
	
	int Size() { return 1; }
	void EndHold() { }
	TSTR Description() { return TSTR(_T("EPVertMapRestore")); }
};

// --------------------------------------------------------------------------------------

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

EditPatchData::EditPatchData(EditPatchMod *mod)
{
	meshSteps = mod->meshSteps;
	// 3-18-99 to suport render steps and removal of the mental tesselator
	meshStepsRender = mod->meshStepsRender;
	showInterior = mod->showInterior;
	
	//	meshAdaptive = mod->meshAdaptive;	// Future use (Not used now)
	viewTess = mod->viewTess;
	prodTess = mod->prodTess;
	dispTess = mod->dispTess;
	tileLevel = mod->tileLevel;
	tileMode = mod->tileMode;
	includeMeshes = mod->includeMeshes;
	transitionType = mod->transitionType;
	keepMapping = mod->keepMapping;
	mViewTessNormals = mod->mViewTessNormals;
	mProdTessNormals = mod->mProdTessNormals;
	mViewTessWeld = mod->mViewTessWeld;
	mProdTessWeld = mod->mProdTessWeld;
	displayLattice = mod->displayLattice;
	displaySurface = mod->displaySurface;
	flags = 0;
	tempData = NULL;
}

EditPatchData::EditPatchData(EditPatchData& emc)
{
	meshSteps = emc.meshSteps;
	// 3-18-99 to suport render steps and removal of the mental tesselator
	meshStepsRender = emc.meshStepsRender;
	showInterior = emc.showInterior;
	
	//	meshAdaptive = emc.meshAdaptive;	// Future use (Not used now)
	viewTess = emc.viewTess;
	prodTess = emc.prodTess;
	dispTess = emc.dispTess;
	tileLevel = emc.tileLevel;
	transitionType = emc.transitionType;
	tileMode = emc.tileMode;
	includeMeshes = emc.includeMeshes;
	keepMapping = emc.keepMapping;
	mViewTessNormals = emc.mViewTessNormals;
	mProdTessNormals = emc.mProdTessNormals;
	mViewTessWeld = emc.mViewTessWeld;
	mProdTessWeld = emc.mProdTessWeld;
	displayLattice = emc.displayLattice;
	displaySurface = emc.displaySurface;
	flags = emc.flags;
	tempData = NULL;
	vertMap = emc.vertMap;
	finalPatch = emc.finalPatch;
	rfinalPatch = emc.rfinalPatch;
}

void EditPatchData::Apply(TimeValue t, RPO *patchOb, int selLevel)
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
		// For old files, which contain exhaustive data to reconstruct the editing process
		// of patches, we'll have data in the 'changes' table.  If it's there, go ahead and
		// replay the edits, then store the alterations in our new delta format and discard
		// the change table!
		int count = changes.Count();
		if (count)
		{
			// DebugPrint("*** Applying old style (%d) ***\n", count);
			// Store the topology for future reference
			vertMap.Build(patchOb->patch);
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
			vertMap.UpdateAndApplyDeltas(patchOb->patch, finalPatch);
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
	{	// No data yet -- Store initial required data
		// DebugPrint("<<<Storing Initial Data>>>\n");
		vertMap.Build(patchOb->patch);
		finalPatch = patchOb->patch;
		rfinalPatch = *patchOb->rpatch;
	}

	// Hand it its mesh interpolation info
	patchOb->SetMeshSteps(meshSteps);
	// 3-18-99 to suport render steps and removal of the mental tesselator
	patchOb->SetMeshStepsRender(meshStepsRender);
	patchOb->SetShowInterior(showInterior);

	//	patchOb->SetAdaptive(meshAdaptive);	// Future use (Not used now)
	patchOb->SetViewTess(viewTess);
	patchOb->SetProdTess(prodTess);
	patchOb->SetDispTess(dispTess);
	patchOb->rpatch->rTess.ModeTile=tileMode;
	patchOb->rpatch->rTess.TileTesselLevel=tileLevel;
	patchOb->rpatch->rTess.TransitionType=transitionType;
	patchOb->rpatch->rTess.KeepMapping=keepMapping;
	patchOb->SetViewTessNormals(mViewTessNormals);
	patchOb->SetProdTessNormals(mProdTessNormals);
	patchOb->SetViewTessWeld(mViewTessWeld);
	patchOb->SetProdTessWeld(mProdTessWeld);

	patchOb->showMesh = displaySurface;
	patchOb->SetShowLattice(displayLattice);
	patchOb->patch.dispFlags = 0;	// TH 3/3/99
	switch (selLevel)
	{
	case EP_PATCH:
		patchOb->patch.SetDispFlag(DISP_SELPATCHES);
		break;
	case EP_EDGE:
		patchOb->patch.SetDispFlag(DISP_SELEDGES);
		break;
	case EP_VERTEX:
		patchOb->patch.SetDispFlag(DISP_VERTTICKS | DISP_SELVERTS | DISP_VERTS);
		break;
	case EP_TILE:
		//patchOb->patch.SetDispFlag(DISP_VERTTICKS | DISP_SELVERTS | DISP_VERTS);
		break;
	}
	patchOb->patch.selLevel = patchLevel[selLevel];
	patchOb->rpatch->SetSelLevel (selLevel);

	/*rfinalPatch.UpdateBinding (finalPatch, t);
	patchOb->rpatch->UpdateBinding (patchOb->patch, t);*/

	if (GetFlag(EPD_UPDATING_CACHE))
	{
		nlassert(tempData);
		tempData->UpdateCache(patchOb);
		SetFlag(EPD_UPDATING_CACHE, FALSE);
	}		
	ticks=CTime::getPerformanceTime ()-ticks;
	nldebug ("%f", CTime::ticksToSecond(ticks));
}

void EditPatchData::Invalidate(PartID part, BOOL patchValid)
{
	if (tempData)
	{
		tempData->Invalidate(part, patchValid);
	}
}

void EditPatchData::BeginEdit(TimeValue t)
{
	nlassert(tempData);
	if (!GetFlag(EPD_HASDATA))
		SetFlag(EPD_HASDATA, TRUE);
}

EPTempData *EditPatchData::TempData(EditPatchMod *mod)
{
	if (!tempData)
	{
		nlassert(mod->ip);
		tempData = new EPTempData(mod, this);
	}
	return tempData;
}

void EditPatchData::RescaleWorldUnits(float f) 
{
	// Scale the deltas inside the vertex map
	vertMap.RescaleWorldUnits(f);
	// Now rescale stuff inside our data structures
	Matrix3 stm = ScaleMatrix(Point3(f, f, f));
	finalPatch.Transform(stm);
}

void EditPatchData::RecordTopologyTags(PatchMesh *patch) 
{
	// First, stuff all -1's into aux fields
	int i;
	for (i = 0; i < patch->numVerts; ++i)
		patch->verts[i].aux1 = 0xffffffff;
	for (i = 0; i < patch->numVecs; ++i)
		patch->vecs[i].aux1 = 0xffffffff;
	for (i = 0; i < patch->numPatches; ++i)
		patch->patches[i].aux1 = 0xffffffff;
	// Now put in our tags
	vertMap.RecordTopologyTags(*patch);
}

GenericNamedSelSetList &EditPatchData::GetSelSet(EditPatchMod *mod) 
{
	switch (mod->GetSubobjectLevel())
	{
	case EP_VERTEX:
		return vselSet;
	case EP_EDGE:
		return eselSet;
	case EP_PATCH:
	case EP_TILE:
	default:
		return pselSet;
	}
}

GenericNamedSelSetList &EditPatchData::GetSelSet(int level) 
{
	switch (level + EP_VERTEX)
	{
	case EP_VERTEX:
		return vselSet;
	case EP_EDGE:
		return eselSet;
	case EP_PATCH:
	case EP_TILE:
	default:
		return pselSet;
	}
}


void EditPatchData::UpdateChanges(PatchMesh *patch, RPatchMesh *rpatch, BOOL checkTopology) 
{
	if (theHold.Holding())
	{
		theHold.Put(new EPVertMapRestore(this));
		//theHold.Put(new FinalPatchRestore(&finalPatch, &rfinalPatch));
		if (rpatch)
			theHold.Put(new FinalPatchRestore(&finalPatch, &rfinalPatch));
		else
			theHold.Put(new FinalPatchRestore(&finalPatch, NULL));
	}
	// Update the mapper's indices
	if (checkTopology)
		vertMap.UpdateMapping(*patch);
	// Update mapper's XYZ deltas
	vertMap.RecomputeDeltas(*patch);
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

IOResult EditPatchData::Save(ISave *isave) 
{
	ULONG nb;
	isave->BeginChunk(EPD_R3_GENERAL_CHUNK);
	isave->Write(&flags, sizeof(DWORD), &nb);
	isave->EndChunk();
	isave->BeginChunk(MESH_ATTRIB_CHUNK);
	isave->Write(&meshSteps, sizeof(int), &nb);
	// Future use (Not used now)
	BOOL fakeAdaptive = FALSE;	
	isave->Write(&fakeAdaptive, sizeof(BOOL), &nb);
	//	isave->Write(&meshAdaptive,sizeof(BOOL),&nb);	// Future use (Not used now)
	isave->EndChunk();
	
	// 3-18-99 to suport render steps and removal of the mental tesselator
	isave->BeginChunk(RENDERSTEPS_CHUNK);
	if ((meshStepsRender < 0) ||(meshStepsRender > 100))
	{
		meshStepsRender = 5;
		nlassert(0);
	}
	isave->Write(&meshStepsRender, sizeof(int), &nb);
	isave->EndChunk();
	isave->BeginChunk(SHOWINTERIOR_CHUNK);
	isave->Write(&showInterior, sizeof(BOOL), &nb);
	isave->EndChunk();
	
	
	isave->BeginChunk(VTESS_ATTRIB_CHUNK);
	viewTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(PTESS_ATTRIB_CHUNK);
	prodTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(DTESS_ATTRIB_CHUNK);
	dispTess.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(DISP_PARTS_CHUNK);
	isave->Write(&displaySurface, sizeof(BOOL), &nb);
	isave->Write(&displayLattice, sizeof(BOOL), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(NORMAL_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessNormals, sizeof(BOOL), &nb);
	isave->Write(&mProdTessNormals, sizeof(BOOL), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(WELD_TESS_ATTRIB_CHUNK);
	isave->Write(&mViewTessWeld, sizeof(BOOL), &nb);
	isave->Write(&mProdTessWeld, sizeof(BOOL), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(RPO_MODE_TILE);
	isave->Write(&tileMode, sizeof(tileMode), &nb);
	isave->Write(&tileLevel, sizeof(tileLevel), &nb);
	isave->Write(&keepMapping, sizeof(keepMapping), &nb);
	isave->EndChunk();
	
	isave->BeginChunk(RPO_INCLUDE_MESHES);
	isave->Write(&includeMeshes, sizeof(includeMeshes), &nb);
	isave->EndChunk();

	isave->BeginChunk(RPO_MODE_TILE_TRANSITION);
	isave->Write(&transitionType, sizeof(transitionType), &nb);
	isave->EndChunk();
	
	// Save named sel sets
	if (vselSet.Count())
	{
		isave->BeginChunk(VSELSET_CHUNK);
		vselSet.Save(isave);
		isave->EndChunk();
	}
	if (eselSet.Count())
	{
		isave->BeginChunk(ESELSET_CHUNK);
		eselSet.Save(isave);
		isave->EndChunk();
	}
	if (pselSet.Count())
	{
		isave->BeginChunk(PSELSET_CHUNK);
		pselSet.Save(isave);
		isave->EndChunk();
	}
	
	isave->BeginChunk(VERTMAP_CHUNK);
	vertMap.Save(isave);
	isave->EndChunk();
	isave->BeginChunk(FINALPATCH_CHUNK);
	finalPatch.Save(isave);
	isave->EndChunk();
	
	isave->BeginChunk(RFINALPATCH_CHUNK);
	rfinalPatch.Save(isave);
	isave->EndChunk();	
	
	return IO_OK;
}

IOResult EditPatchData::Load(ILoad *iload) 
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
	case CLEARVERTSELRECORD_CHUNK:
		theChange = new ClearPVertSelRecord;
		goto load_change;
	case SETVERTSELRECORD_CHUNK:
		theChange = new SetPVertSelRecord;
		goto load_change;
	case INVERTVERTSELRECORD_CHUNK:
		theChange = new InvertPVertSelRecord;
		goto load_change;
	case CLEAREDGESELRECORD_CHUNK:
		theChange = new ClearPEdgeSelRecord;
		goto load_change;
	case SETEDGESELRECORD_CHUNK:
		theChange = new SetPEdgeSelRecord;
		goto load_change;
	case INVERTEDGESELRECORD_CHUNK:
		theChange = new InvertPEdgeSelRecord;
		goto load_change;
	case CLEARPATCHSELRECORD_CHUNK:
		theChange = new ClearPatchSelRecord;
		goto load_change;
	case SETPATCHSELRECORD_CHUNK:
		theChange = new SetPatchSelRecord;
		goto load_change;
	case INVERTPATCHSELRECORD_CHUNK:
		theChange = new InvertPatchSelRecord;
		goto load_change;
	case VERTSELRECORD_CHUNK:
		theChange = new PVertSelRecord;
		goto load_change;
	case EDGESELRECORD_CHUNK:
		theChange = new PEdgeSelRecord;
		goto load_change;
	case PATCHSELRECORD_CHUNK:
		theChange = new PatchSelRecord;
		goto load_change;
	case PATCHDELETERECORD_CHUNK:
		theChange = new PatchDeleteRecord;
		goto load_change;
	case VERTMOVERECORD_CHUNK:
		theChange = new PVertMoveRecord;
		goto load_change;
	case PATCHCHANGERECORD_CHUNK:
		theChange = new PatchChangeRecord;
		goto load_change;
	case VERTCHANGERECORD_CHUNK:
		theChange = new PVertChangeRecord;
		goto load_change;
	case PATCHADDRECORD_CHUNK:
		theChange = new PatchAddRecord;
		goto load_change;
	case EDGESUBDIVIDERECORD_CHUNK:
		theChange = new EdgeSubdivideRecord;
		goto load_change;
	case PATCHSUBDIVIDERECORD_CHUNK:
		theChange = new PatchSubdivideRecord;
		goto load_change;
	case PATTACHRECORD_CHUNK:
		theChange = new PAttachRecord;
		goto load_change;
	case PATCHDETACHRECORD_CHUNK:
		theChange = new PatchDetachRecord;
		goto load_change;
	case PATCHMTLRECORD_CHUNK:
		theChange = new PatchMtlRecord;
		goto load_change;
	case VERTWELDRECORD_CHUNK:
		theChange = new PVertWeldRecord;
		goto load_change;
	case VERTDELETERECORD_CHUNK:
		theChange = new PVertDeleteRecord;
		// Intentional fall-thru!
load_change:
		changes.Append(1, &theChange);
		changes[changes.Count() - 1]->Load(iload);
		break;
		//
		// The following code is used for post-release 3 files
		//
	case EPD_R3_GENERAL_CHUNK:
		res = iload->Read(&flags, sizeof(DWORD), &nb);
		break;
	case VERTMAP_CHUNK:
		res = vertMap.Load(iload);
		break;
	case FINALPATCH_CHUNK:
		res = finalPatch.Load(iload);
		break;
	case RFINALPATCH_CHUNK:
		res = rfinalPatch.Load(iload);
		break;
		//
		// The following code is common to all versions' files
		//
	case MESH_ATTRIB_CHUNK:
		iload->Read(&meshSteps, sizeof(int), &nb);
		res = iload->Read(&meshAdaptive, sizeof(BOOL), &nb);	// Future use (Not used now)
		break;
		// 3-18-99 to suport render steps and removal of the mental tesselator
	case RENDERSTEPS_CHUNK:
		iload->Read(&meshStepsRender, sizeof(int), &nb);
		if ((meshStepsRender < 0) ||(meshStepsRender > 100))
		{
			meshStepsRender = 5;
			nlassert(0);
		}
		
		break;
	case SHOWINTERIOR_CHUNK:
		iload->Read(&showInterior, sizeof(BOOL), &nb);
		break;
		
	case VTESS_ATTRIB_CHUNK:
		viewTess.Load(iload);
		break;
	case PTESS_ATTRIB_CHUNK:
		prodTess.Load(iload);
		break;
	case DTESS_ATTRIB_CHUNK:
		dispTess.Load(iload);
		break;
	case NORMAL_TESS_ATTRIB_CHUNK:
		iload->Read(&mViewTessNormals, sizeof(BOOL), &nb);
		res = iload->Read(&mProdTessNormals, sizeof(BOOL), &nb);
		break;
	case WELD_TESS_ATTRIB_CHUNK:
		iload->Read(&mViewTessWeld, sizeof(BOOL), &nb);
		res = iload->Read(&mProdTessWeld, sizeof(BOOL), &nb);
		break;
	case DISP_PARTS_CHUNK:
		iload->Read(&displaySurface, sizeof(BOOL), &nb);
		res = iload->Read(&displayLattice, sizeof(BOOL), &nb);
		break;
		// Load named selection sets
	case VSELSET_CHUNK:
		res = vselSet.Load(iload);
		break;
	case PSELSET_CHUNK:
		res = pselSet.Load(iload);
		break;
	case ESELSET_CHUNK:
		res = eselSet.Load(iload);
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
