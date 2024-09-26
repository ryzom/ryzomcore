#include "vertex_tree_paint.h"

static	PaintCommandMode thePaintCommandMode;
static	HCURSOR hPaintCursor = NULL;	// Paint cursor
static	HCURSOR hDropperCursor = NULL;	// Paint cursor
static	HCURSOR hNoPaintCursor = NULL;	// NoPaint cursor

TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

void NukePaintCommandMode()
{
	CommandMode* cmdMode = GetCOREInterface()->GetCommandMode();
	if (!cmdMode || cmdMode->ID() == thePaintCommandMode.ID()) {
		// Our command mode is at the top of the stack.
		// Set selection mode and remove our mode
		// We need to make sure some kind of command mode is set, so
		// we set selection mode to be the default.
		GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
		GetCOREInterface()->DeleteMode(&thePaintCommandMode);
	}
	else {
		// Our command mode is not at the top of the stack,
		// so we can safely remove our mode.
		GetCOREInterface()->DeleteMode(&thePaintCommandMode);
	}
}

BOOL VertexPaint::ActivatePaint(BOOL bOnOff, BOOL bPick)
	{

	// Remind PaintMode between change of modifier.
	if(bOnOff && !bPick)
		_LastPaintMode= true;
	else
		_LastPaintMode= false;
	
	if (bOnOff) {
		
		thePaintCommandMode.mouseProc.SetPickMode(bPick);
		
		if (!hPaintCursor) {
			hPaintCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_PAINTCURSOR));
		}
		if (!hNoPaintCursor) {
			hNoPaintCursor = LoadCursor(NULL,IDC_CROSS);
		}
		if (!hDropperCursor) {
			hDropperCursor = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_DROPPER_CURSOR));
		}

		thePaintCommandMode.SetInterface(GetCOREInterface());
		thePaintCommandMode.SetModifier(this);
		ip->SetCommandMode(&thePaintCommandMode);
		}
	else {
		NukePaintCommandMode();
		}

	return TRUE;
	}

ModContext* VertexPaint::ModContextFromNode(INode* node)
	{
	Object* obj = node->GetObjectRef();

	if (!obj)	return FALSE;

    while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
		IDerivedObject* dobj = (IDerivedObject*)obj;
		int m;
		int numMods = dobj->NumModifiers();

		for (m=0; m<numMods; m++) {
			ModContext* mc = dobj->GetModContext(m);
			for (int i=0; i<modContexts.Count(); i++) {
				if (mc == modContexts[i]) {
					return mc;
					}
				}
			}

		obj = dobj->GetObjRef();
		}

	return NULL;
	}

void PaintMouseProc::DoPainting(HWND hWnd, IPoint2 m)
	{
	INode*		node = GetCOREInterface()->PickNode(hWnd,m,NULL);
	TimeValue	t = GetCOREInterface()->GetTime();
	Ray			ray;

	if (pModifier->IsValidNode(node)) {
		SetCursor(hPaintCursor);
		ModContext* mc = pModifier->ModContextFromNode(node);
		
		// If we got an instanced modifier but the second node is not selected
		// it mght be valid, but it will return NULL for the ModContext
		if(!mc || !mc->localData)
		{
			return;
		}

		VertexPaintData* d = (VertexPaintData*)mc->localData;
#if MAX_VERSION_MAJOR >= 19
		ViewExp*	pView = &GetCOREInterface()->GetViewExp(hWnd);
#else
		ViewExp*	pView = GetCOREInterface()->GetViewport(hWnd);
#endif
		Mesh*		mesh = 	d->GetMesh();

		if (mesh) {
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	

			float at;
			Point3 norm;
			DWORD fi;
			Point3 bary;
			float opacity;

			if (mesh->IntersectRay(ray, at, norm, fi, bary)) 
			{
				if (mesh->vertCol) 
				{
					TVFace* tvf = &mesh->vcFace[fi];
					Face* f = &mesh->faces[fi];

					VertexPaintData::TComponent	whichComponent;
					switch(pModifier->getEditionType())
					{
					case 0: whichComponent= VertexPaintData::Red; break;
					case 1: whichComponent= VertexPaintData::Green; break;
					case 2: whichComponent= VertexPaintData::Blue; break;
					}

					for (int i=0; i<3; i++) 
					{
						opacity = bary[i]*pModifier->fTint;
						
						if(opacity > 1)
							opacity = 1;
						if(opacity < 0)
							opacity = 0;

						// Skip painting because not selected??
						if(mesh->selLevel == MESH_VERTEX && !mesh->VertSel()[f->v[i]] )
							continue;
						if(mesh->selLevel == MESH_FACE && !mesh->FaceSel()[fi])
							continue;

						// Apply painting
						d->SetColor(f->v[i],  opacity ,pModifier->GetActiveColor(), whichComponent);
					}
					pModifier->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
					//pModifier->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(t);
				}
			}

#if MAX_VERSION_MAJOR < 19
			GetCOREInterface()->ReleaseViewport(pView);
#endif
			}
		else {
			SetCursor(hNoPaintCursor);
			}
		}
	return;
	}

void PaintMouseProc::DoPickColor(HWND hWnd, IPoint2 m)
	{
	INode*		node = GetCOREInterface()->PickNode(hWnd,m,NULL);
	TimeValue	t = GetCOREInterface()->GetTime();
	Ray			ray;

	if(!node) 
	{
		SetCursor(hNoPaintCursor);
		return;
	}

	ObjectState os = node->EvalWorldState(t);
	
	if(os.obj->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID,0))
	{
		SetCursor(hDropperCursor);
		TriObject *pTri = (TriObject *) os.obj;

#if MAX_VERSION_MAJOR >= 19
		ViewExp*	pView = &GetCOREInterface()->GetViewExp(hWnd);
#else
		ViewExp*	pView = GetCOREInterface()->GetViewport(hWnd);
#endif
		Mesh*		mesh = &pTri->mesh;

		if (mesh) {
			pView->MapScreenToWorldRay((float)m.x, (float)m.y, ray);
			Matrix3 obtm  = node->GetObjTMAfterWSM(t);
			Matrix3 iobtm = Inverse(obtm);
			ray.p   = iobtm * ray.p;
			ray.dir = VectorTransform(iobtm, ray.dir);	

			float at;
			Point3 norm;
			DWORD fi;
			Point3 bary;
			float opacity = 0;

			if (mesh->IntersectRay(ray, at, norm, fi, bary)) 
			{
				if (mesh->vertCol) 
				{
					TVFace* tvf = &mesh->vcFace[fi];
					Face* f = &mesh->faces[fi];
					Color PickCol(0,0,0);

					for (int i=0; i<3; i++) 
					{
						PickCol += (bary[i])*mesh->vertCol[tvf->t[i]];
					}
					
#if MAX_RELEASE <= 3100
					// Yoyo: grayScale: get max value.
					int	maxVal= GetRValue((DWORD)PickCol);
					if(GetGValue((DWORD)PickCol) > maxVal)
						maxVal= GetGValue((DWORD)PickCol);
					if(GetBValue((DWORD)PickCol) > maxVal)
						maxVal= GetBValue((DWORD)PickCol);
#else
					// Yoyo: grayScale: get max value.
					int	maxVal= GetRValue((DWORD)PickCol.toRGB());
					if(GetGValue((DWORD)PickCol.toRGB()) > maxVal)
						maxVal= GetGValue((DWORD)PickCol.toRGB());
					if(GetBValue((DWORD)PickCol.toRGB()) > maxVal)
						maxVal= GetBValue((DWORD)PickCol.toRGB());
#endif
					pModifier->iColor->SetColor(RGB(maxVal, maxVal, maxVal));
				}
			}

#if MAX_VERSION_MAJOR < 19
			GetCOREInterface()->ReleaseViewport(pView);
#endif
			}
		}
	else {
			pModifier->iColor->SetColor(node->GetWireColor());
			}
		
	return;
	}

int PaintMouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	int			nRetval = FALSE;

	BOOL		bButtonDown = flags & MOUSE_LBUTTON;

	switch (msg) {
		case MOUSE_POINT:
			
			if( flags & MOUSE_CTRL || bPickMode )
				DoPickColor(hWnd, m);
			else
			{
				if(bButtonDown)
					MaybeStartHold();
				
				DoPainting(hWnd, m);
				
				if(!bButtonDown)
					MaybeEndHold();
			}

			break;
		case MOUSE_MOVE:
			if(flags & MOUSE_CTRL || bPickMode)
				DoPickColor(hWnd, m);
			else
				DoPainting(hWnd, m);
			break;
		case MOUSE_ABORT:
			if(wasHolding)
				theHold.Cancel();
			
			GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
			break;
		case MOUSE_PROPCLICK:
			GetCOREInterface()->SetStdCommandMode(CID_OBJSELECT);
			break;
		case MOUSE_FREEMOVE:
			{				
				INode* node = GetCOREInterface()->PickNode(hWnd,m,NULL);

				if(flags & MOUSE_CTRL || bPickMode)
				{
					if(node)
						SetCursor(hDropperCursor);
					else
						SetCursor(hNoPaintCursor);
				}
				else
				{
					if (pModifier->IsValidNode(node)) {
						SetCursor(hPaintCursor);
					}
					else {
						SetCursor(hNoPaintCursor);
					}
				}
			}
			break;
	}
		
	return TRUE;
}

void PaintMouseProc::MaybeStartHold()
{
	if(!theHold.Holding())
	{
		theHold.Begin();
		wasHolding = TRUE;
	}
	else
		wasHolding = FALSE;
	
	ModContextList	modContexts;
	INodeTab		nodeTab;
	
	GetCOREInterface()->GetModContexts(modContexts, nodeTab);
	
	for (int i=0; i<modContexts.Count(); i++) 
	{
		ModContext *mc = modContexts[i];
		if(mc && mc->localData)
			theHold.Put(new VertexPaintRestore((VertexPaintData*)mc->localData,pModifier));
	}
}

void PaintMouseProc::MaybeEndHold()
{
	if(wasHolding)
		theHold.Accept(GetString(IDS_RESTORE));

}

//=====================================

PaintCommandMode::PaintCommandMode()
{
	iInterface	= NULL;
}

int PaintCommandMode::Class()
{
	return PICK_COMMAND;
}

int PaintCommandMode::ID()
{
	return CID_PAINT;
}

MouseCallBack* PaintCommandMode::MouseProc(int *numPoints)
{
	*numPoints = 2; return &mouseProc;
}

ChangeForegroundCallback* PaintCommandMode::ChangeFGProc()
{
	return CHANGE_FG_SELECTED;
}

BOOL PaintCommandMode::ChangeFG( CommandMode *oldMode )
{
	return FALSE;
}

void PaintCommandMode::EnterMode() 
{
	iInterface->PushPrompt(GetString(IDS_PAINTPROMPT));
	pModifier->EnterMode();

}

void PaintCommandMode::ExitMode() 
{
	iInterface->PopPrompt();
	pModifier->ExitMode();

}

void VertexPaint::EnterMode()
	{
		int numPoints;
		iPaintButton->SetCheck(ip->GetCommandMode()->ID() == CID_PAINT && 
				!((PaintMouseProc *)ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		iPickButton->SetCheck(ip->GetCommandMode()->ID() == CID_PAINT && 
				((PaintMouseProc *)ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		ip->GetModContexts(modContexts, nodeTab);
	}

void VertexPaint::ExitMode()
	{
		iPaintButton->SetCheck(FALSE);
		iPickButton->SetCheck(FALSE);

		nodeTab.DisposeTemporary();
		modContexts.ZeroCount();
		modContexts.Shrink();
	}

BOOL VertexPaint::IsValidNode(INode* node)
	{
	if (!node) return FALSE;

	for (int i=0; i<nodeTab.Count(); i++) {
		if (nodeTab[i] == node) {
			return TRUE;
			}
		}
	return FALSE;
	}

COLORREF VertexPaint::GetActiveColor()
	{
	return iColor->GetColor();
	}

// ------------- Convert object to TriObject -----------------------

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(0, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object

		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
		}
	else {
		return NULL;
		}
	}
