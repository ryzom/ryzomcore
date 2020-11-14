/**********************************************************************
 *<
	FILE: vertex_tree_paint.cpp

	DESCRIPTION:	Modifier implementation

	CREATED BY: Christer Janson, Nikolai Sander

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "vertex_tree_paint.h"
#include "meshdelta.h"

 // flags:
#define VP_DISP_END_RESULT 0x01

static WNDPROC colorSwatchOriginalWndProc;

static	HIMAGELIST hButtonImages = NULL;

static void LoadImages()
{
	if (hButtonImages) return;
	HBITMAP hBitmap, hMask;
	hButtonImages = ImageList_Create(15, 14, ILC_MASK, 2, 0);	// 17 is kluge to center square. -SA
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BUTTONS));
	hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BUTTON_MASK));
	ImageList_Add(hButtonImages, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

ClassDesc* GetVertexPaintDesc();


class VertexPaintClassDesc :public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new VertexPaint(); }
	const MCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return VERTEX_TREE_PAINT_CLASS_ID; }
	const MCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	void			ResetClassParams(BOOL fileReset) {}
};

static VertexPaintClassDesc VertexPaintDesc;
ClassDesc* GetVertexPaintDesc() { return &VertexPaintDesc; }

static INT_PTR CALLBACK VertexPaintDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int numPoints;
	VertexPaint *mod = (VertexPaint*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!mod && msg != WM_INITDIALOG) return FALSE;
	int		comboResult;


	// Manages Spinners.
	if (((msg == CC_SPINNER_BUTTONUP) && HIWORD(wParam)) ||
		((msg == CC_SPINNER_CHANGE)))
	{
		ISpinnerControl *spin;
		spin = (ISpinnerControl *)lParam;

		switch (LOWORD(wParam))
		{
		case IDC_TINT_SPIN:
			if ((msg == CC_SPINNER_CHANGE))
			{
				mod->fTint = spin->GetFVal() / 100;
			}
			break;
		case IDC_BEND_SPIN:
			if ((msg == CC_SPINNER_CHANGE))
			{
				mod->fGradientBend = spin->GetFVal() / 100;
			}
			break;
		}
	}

	switch (msg)
	{
	case WM_INITDIALOG:
		LoadImages();
		mod = (VertexPaint*)lParam;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
		mod->hParams = hWnd;
		mod->iPaintButton = GetICustButton(GetDlgItem(hWnd, IDC_PAINT));
		mod->iPaintButton->SetType(CBT_CHECK);
		mod->iPaintButton->SetHighlightColor(GREEN_WASH);
		mod->iPaintButton->SetCheck(mod->ip->GetCommandMode()->ID() == CID_PAINT &&
			!((PaintMouseProc *)mod->ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		mod->iPaintButton->SetImage(hButtonImages, 0, 0, 0, 0, 15, 14);
		mod->iPaintButton->SetTooltip(TRUE, GetString(IDS_PAINT));

		mod->iPickButton = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		mod->iPickButton->SetType(CBT_CHECK);
		mod->iPickButton->SetHighlightColor(GREEN_WASH);
		mod->iPickButton->SetCheck(mod->ip->GetCommandMode()->ID() == CID_PAINT &&
			((PaintMouseProc *)mod->ip->GetCommandMode()->MouseProc(&numPoints))->GetPickMode());
		mod->iPickButton->SetImage(hButtonImages, 1, 1, 1, 1, 15, 14);
		mod->iPickButton->SetTooltip(TRUE, GetString(IDS_PICK));


		mod->iColor = GetIColorSwatch(GetDlgItem(hWnd, IDC_COLOR));
		// change current Color according to editMode
		mod->reloadBkupColor();

		// Get interface For ZGradient, reload bkuped colors
		mod->iColorGradient[0] = GetIColorSwatch(GetDlgItem(hWnd, IDC_PALETTE_GRAD0));
		mod->iColorGradient[1] = GetIColorSwatch(GetDlgItem(hWnd, IDC_PALETTE_GRAD1));
		mod->iColorGradient[0]->SetColor(mod->lastGradientColor[0]);
		mod->iColorGradient[1]->SetColor(mod->lastGradientColor[1]);


		// Init comboBox
		SendDlgItemMessage(hWnd, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("Tree Weight"));
		SendDlgItemMessage(hWnd, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("Phase Level 1"));
		SendDlgItemMessage(hWnd, IDC_COMBO_TYPE, CB_ADDSTRING, 0, (LPARAM)_T("Phase Level 2"));
		SendDlgItemMessage(hWnd, IDC_COMBO_TYPE, CB_SETCURSEL, mod->getEditionType(), 0);

		// If paint mode at last edit.
		if (mod->_LastPaintMode)
		{
			// ActivatePaint / check button.
			mod->ActivatePaint(TRUE);
			mod->iPaintButton->SetCheck(TRUE);
		}

		break;

	case WM_POSTINIT:
		mod->InitPalettes();
		break;

	case CC_COLOR_CHANGE:
		if (LOWORD(wParam) == IDC_COLOR)
		{
			IColorSwatch* iCol = (IColorSwatch*)lParam;
			switch (mod->getEditionType())
			{
			case 0: mod->lastWeightColor = iCol->GetColor(); break;
			case 1:
			case 2:
				mod->lastPhaseColor = iCol->GetColor(); break;
			}
		}
		break;
	case WM_DESTROY:
		mod->SavePalettes();
		mod->iPaintButton = NULL;
		mod->iPickButton = NULL;
		mod->iColor = NULL;
		mod->iColorGradient[0] = NULL;
		mod->iColorGradient[1] = NULL;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PAINT:
			mod->ActivatePaint(mod->iPaintButton->IsChecked());
			break;
		case IDC_PICK:
			mod->ActivatePaint(mod->iPickButton->IsChecked(), TRUE);
			break;

		case IDC_VC_ON:
			mod->TurnVCOn(FALSE);
			break;
		case IDC_SHADED:
			mod->TurnVCOn(TRUE);
			break;
		case IDC_COMBO_TYPE:
			// Init default type.
			comboResult = SendDlgItemMessage(hWnd, IDC_COMBO_TYPE, CB_GETCURSEL, 0, 0);
			mod->setEditionType(comboResult);
			break;
		case IDC_BUTTON_FILL:
			mod->fillSelectionColor();
			break;
		case IDC_BUTTON_GRADIENT:
			mod->fillSelectionGradientColor();
			break;
		case IDC_BUTTON_GRAD0:
			mod->iColorGradient[0]->SetColor(RGB(0, 0, 0));
			mod->iColorGradient[1]->SetColor(RGB(85, 85, 85));
			break;
		case IDC_BUTTON_GRAD1:
			mod->iColorGradient[0]->SetColor(RGB(85, 85, 85));
			mod->iColorGradient[1]->SetColor(RGB(170, 170, 170));
			break;
		case IDC_BUTTON_GRAD2:
			mod->iColorGradient[0]->SetColor(RGB(170, 170, 170));
			mod->iColorGradient[1]->SetColor(RGB(255, 255, 255));
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

// Subclass procedure 
LRESULT APIENTRY colorSwatchSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	{
		HWND hPanel = GetParent(hwnd);
		LONG_PTR mod = GetWindowLongPtr(hPanel, GWLP_USERDATA);
		if (mod)
		{
			((VertexPaint*)mod)->PaletteButton(hwnd);
		}
	}
						   break;
	case WM_DESTROY:
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)colorSwatchOriginalWndProc);
		// Fallthrough...
	default:
		return CallWindowProc(colorSwatchOriginalWndProc, hwnd, uMsg, wParam, lParam);
		break;
	}
	return 0;
}


IObjParam *VertexPaint::ip = NULL;
HWND VertexPaint::hParams = NULL;
VertexPaint* VertexPaint::editMod = NULL;
ICustButton* VertexPaint::iPaintButton = NULL;
ICustButton* VertexPaint::iPickButton = NULL;
IColorSwatch* VertexPaint::iColor = NULL;
COLORREF VertexPaint::lastWeightColor = RGB(85, 85, 85);
COLORREF VertexPaint::lastPhaseColor = RGB(0, 0, 0);
COLORREF VertexPaint::palColors[] =
{
	//RGB(32,  32,  32),	RGB(  96,96,96),	RGB(  160,160,160),	RGB(224,224,224) };
	RGB(0,  0,  0),	RGB(85,85,85),	RGB(170,170,170),	RGB(255,255,255),
		 RGB(42,  42,  42), RGB(127, 127, 127), RGB(212, 212, 212) };


IColorSwatch* VertexPaint::iColorGradient[] = { NULL, NULL };
COLORREF VertexPaint::lastGradientColor[] = { RGB(0,  0,  0), RGB(85,  85,  85) };


//--- VertexPaint -------------------------------------------------------
VertexPaint::VertexPaint() : iTint(NULL), fTint(1.0f), iGradientBend(NULL), fGradientBend(0.0f)
{
	flags = 0x0;
	_EditType = 0;
	_LastPaintMode = false;
}

VertexPaint::~VertexPaint()
{
}

Interval VertexPaint::LocalValidity(TimeValue t)
{
	return FOREVER;
}

BOOL VertexPaint::DependOnTopology(ModContext &mc)
{
	return TRUE;
}

RefTargetHandle VertexPaint::Clone(RemapDir& remap)
{
	VertexPaint* newmod = new VertexPaint();
	return(newmod);
}

void VertexPaint::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if (!mc->localData) return;
	((VertexPaintData*)mc->localData)->FreeCache();

}

void VertexPaint::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	if (!os->obj->IsSubClassOf(triObjectClassID)) return;

	os->obj->ReadyChannelsForMod(GEOM_CHANNEL | TOPO_CHANNEL | VERTCOLOR_CHANNEL | TEXMAP_CHANNEL);

	TriObject *tobj = (TriObject*)os->obj;
	VertexPaintData *d = (VertexPaintData*)mc.localData;

	Mesh* mesh = &tobj->GetMesh();

	if (mesh)
	{
		// We don't have any VColors yet, so we allocate the vcfaces
		// and set all vcolors to black (index 0)

		if (!mesh->vcFace)
		{
			mesh->setNumVCFaces(mesh->getNumFaces());
			mesh->setNumVertCol(1);

			mesh->vertCol[0] = Color(0, 0, 0);

			for (int f = 0; f < mesh->getNumFaces(); f++)
			{
				mesh->vcFace[f].t[0] = 0;
				mesh->vcFace[f].t[1] = 0;
				mesh->vcFace[f].t[2] = 0;
			}
		}

		if (!d) mc.localData = d = new VertexPaintData(tobj->GetMesh());
		if (!d->GetMesh()) d->SetCache(*mesh);


		{
			MeshDelta md(*mesh);
			//MeshDelta mdc;
			//if(cache) mdc.InitToMesh(*cache);

			// If the incoming Mesh had no vertex colors, this will add a default map to start with.
			// The default map has the same topology as the Mesh (so one color per vertex),
			// with all colors set to white.
			if (!mesh->mapSupport(0)) md.AddVertexColors();
			//if (cache && !cache->mapSupport(0)) mdc.AddVertexColors ();

			// We used two routines -- VCreate to add new map vertices, and FRemap to make the
			// existing map faces use the new verts.  frFlags tell FRemap which vertices on a face
			// should be "remapped", and the ww array contains the new locations.			
			VertColor nvc;
			int j;
			for (int v = 0; v < d->GetNumColors(); v++)
			{
				ColorData cd = d->GetColorData(v);

				// Edition Mode ??
				if (editMod == this)
				{
					nvc = Color(cd.color);
					// change color to view only monochromatic info for this channel;
					switch (_EditType)
					{
					case 0: nvc.y = nvc.z = nvc.x;
						nvc.y *= 0.7f;
						nvc.z *= 0.7f;
						break;
					case 1: nvc.x = nvc.z = nvc.y;
						nvc.x *= 0.7f;
						nvc.z *= 0.7f;
						break;
					case 2: nvc.x = nvc.y = nvc.z;
						nvc.x *= 0.7f;
						nvc.y *= 0.7f;
						break;
					}
				}
				else
				{
					// replace the VertexColor of the outgoing mesh
					nvc = Color(cd.color);
				}

				DWORD ww[3], frFlags;

				md.map->VCreate(&nvc);

				// increase the number of vcol's and set the vcfaces as well	
				for (int i = 0; i < d->GetNVert(v).faces.Count(); i++)
				{
					j = d->GetNVert(v).whichVertex[i];
					frFlags = (1 << j);
					ww[j] = md.map->outVNum() - 1;
					md.map->FRemap(d->GetNVert(v).faces[i], frFlags, ww);

				}
			}

			md.Apply(*mesh);
		}


		NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
		os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t, t));
	}
}

static bool oldShowEnd;

void VertexPaint::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{

	this->ip = ip;
	editMod = this;
	if (!hParams)
	{
		hParams = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_PANEL), VertexPaintDlgProc, GetString(IDS_PARAMS), (LPARAM)this);

		// Subclass the palette controls
		hPaletteWnd[0] = GetDlgItem(hParams, IDC_PALETTE_1);
		hPaletteWnd[1] = GetDlgItem(hParams, IDC_PALETTE_2);
		hPaletteWnd[2] = GetDlgItem(hParams, IDC_PALETTE_3);
		hPaletteWnd[3] = GetDlgItem(hParams, IDC_PALETTE_4);
		hPaletteWnd[4] = GetDlgItem(hParams, IDC_PALETTE_5);
		hPaletteWnd[5] = GetDlgItem(hParams, IDC_PALETTE_6);
		hPaletteWnd[6] = GetDlgItem(hParams, IDC_PALETTE_7);

		int	i;
		for (i = 0; i < NUMPALETTES; i++)
		{
			colorSwatchOriginalWndProc = (WNDPROC)SetWindowLongPtr(hPaletteWnd[i], GWLP_WNDPROC, (LONG_PTR)colorSwatchSubclassWndProc);
		}

		SendMessage(hParams, WM_POSTINIT, 0, 0);
	}
	else
	{
		SetWindowLongPtr(hParams, GWLP_USERDATA, (LONG_PTR)this);
	}

	iTint = SetupIntSpinner(hParams, IDC_TINT_SPIN, IDC_TINT, 0, 100, (int)(fTint*100.0f));

	// Init Gradient Bend spinner
	iGradientBend = SetupIntSpinner(hParams, IDC_BEND_SPIN, IDC_BEND, 0, 100, (int)(fGradientBend*100.0f));


	// Set show end result.
	oldShowEnd = ip->GetShowEndResult() ? TRUE : FALSE;
	ip->SetShowEndResult(GetFlag(VP_DISP_END_RESULT));

	// Force an eval to update caches.
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
}

void VertexPaint::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	// Dsiable Painting.
	bool	lpm = _LastPaintMode;
	ActivatePaint(FALSE);
	// bkup lastPainMode
	_LastPaintMode = lpm;

	ReleaseISpinner(iTint);
	ReleaseISpinner(iGradientBend);

	ModContextList list;
	INodeTab nodes;
	ip->GetModContexts(list, nodes);
	for (int i = 0; i < list.Count(); i++)
	{
		VertexPaintData *vd = (VertexPaintData*)list[i]->localData;
		if (vd) vd->FreeCache();
	}
	nodes.DisposeTemporary();

	// Reset show end result
	SetFlag(VP_DISP_END_RESULT, ip->GetShowEndResult() ? TRUE : FALSE);
	ip->SetShowEndResult(oldShowEnd);


	// Exit editMod => draw true colored weights.
	editMod = NULL;
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);


	ip->DeleteRollupPage(hParams);
	hParams = NULL;
	iTint = NULL;
	iGradientBend = NULL;
	this->ip = NULL;
}


//From ReferenceMaker 
RefResult VertexPaint::NotifyRefChanged(NOTIFY_REF_PARAMS)
{
	return REF_SUCCEED;
}

int VertexPaint::NumRefs()
{
	return 0;
}

RefTargetHandle VertexPaint::GetReference(int i)
{
	return NULL;
}

void VertexPaint::SetReference(int i, RefTargetHandle rtarg)
{
}

int VertexPaint::NumSubs()
{
	return 0;
}

Animatable* VertexPaint::SubAnim(int i)
{
	return NULL;
}

TSTR VertexPaint::SubAnimName(int i)
{
	return _T("");
}


#define VERSION_CHUNKID			0x100
#define COLORLIST_CHUNKID		0x120

static int currentVersion = 1;

IOResult VertexPaint::Load(ILoad *iload)
{
	IOResult res;
	ULONG nb;
	int version = 1;
	Modifier::Load(iload);

	while (IO_OK == (res = iload->OpenChunk()))
	{
		switch (iload->CurChunkID())
		{
		case VERSION_CHUNKID:
			iload->Read(&version, sizeof(version), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}

	return IO_OK;
}

IOResult VertexPaint::Save(ISave *isave)
{
	IOResult res;
	ULONG nb;

	Modifier::Save(isave);

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write(&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult VertexPaint::SaveLocalData(ISave *isave, LocalModData *ld)
{
	VertexPaintData*	d = (VertexPaintData*)ld;
	IOResult	res;
	ULONG		nb;
	int			numColors;
	ColorData	col;

	isave->BeginChunk(VERSION_CHUNKID);
	res = isave->Write(&currentVersion, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(COLORLIST_CHUNKID);
	numColors = d->GetNumColors();
	res = isave->Write(&numColors, sizeof(int), &nb);
	for (int i = 0; i < numColors; i++)
	{
		col = d->GetColorData(i);
		isave->Write(&col.color, sizeof(col.color), &nb);
	}

	isave->EndChunk();
	return IO_OK;
}

IOResult VertexPaint::LoadLocalData(ILoad *iload, LocalModData **pld)
{
	VertexPaintData *d = new VertexPaintData;
	IOResult	res;
	ULONG		nb;
	int			version = 1;
	int			numColors;
	ColorData	col;

	*pld = d;

	while (IO_OK == (res = iload->OpenChunk()))
	{
		switch (iload->CurChunkID())
		{
		case VERSION_CHUNKID:
			iload->Read(&version, sizeof(version), &nb);
			break;
		case COLORLIST_CHUNKID:
		{
			iload->Read(&numColors, sizeof(int), &nb);
			d->AllocColorData(numColors);
			for (int i = 0; i < numColors; i++)
			{
				iload->Read(&col.color, sizeof(col.color), &nb);
				d->SetColor(i, col);
			}
		}
		break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}
	return IO_OK;
}

void VertexPaint::PaletteButton(HWND hWnd)
{
	IColorSwatch* iPal = GetIColorSwatch(hWnd);
	if (iPal && iColor)
	{
		iColor->SetColor(iPal->GetColor(), TRUE);
	}
}

void VertexPaint::InitPalettes()
{
	IColorSwatch* c;
	for (int i = 0; i < NUMPALETTES; i++)
	{
		c = GetIColorSwatch(hPaletteWnd[i]);
		c->SetColor(palColors[i]);
		ReleaseIColorSwatch(c);
	}
}

void VertexPaint::SavePalettes()
{
	IColorSwatch* c;
	for (int i = 0; i < NUMPALETTES; i++)
	{
		c = GetIColorSwatch(hPaletteWnd[i]);
		palColors[i] = c->GetColor();
		ReleaseIColorSwatch(c);
	}
	// Save Gradient Palettes.
	lastGradientColor[0] = iColorGradient[0]->GetColor();
	lastGradientColor[1] = iColorGradient[1]->GetColor();
}

void VertexPaint::TurnVCOn(BOOL shaded)
{
	ModContextList list;
	INodeTab NodeTab;

	// Only the selected nodes will be affected
	ip->GetModContexts(list, NodeTab);

	for (int i = 0; i < NodeTab.Count(); i++)
	{
		if (shaded)
			NodeTab[i]->SetShadeCVerts(!NodeTab[i]->GetShadeCVerts());
		else
			NodeTab[i]->SetCVertMode(!NodeTab[i]->GetCVertMode());

	}
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}


// *****************************************************************
void	VertexPaint::setEditionType(int editMode)
{
	if (editMode < 0)	editMode = 0;
	if (editMode > 2)	editMode = 2;

	// backup current Color according to editMode
	backupCurrentColor();

	_EditType = editMode;

	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());

	// Change Color Swatch according to editMode.
	IColorSwatch* c;
	for (int i = 0; i < NUMPALETTES; i++)
	{
		// Change palColors[i].
		int	val;
		if (editMode == 0)
			val = i * 255 / (4 - 1);		// 0, 85, 170, 255
		else
			val = (i * 256 + 128) / 4;		// 32, 96, 160, 224
		// Change Addditional Palette colors.
		if (i >= 4)
		{
			if (editMode == 0)
				val = 42 + (i - 4) * 255 / (4 - 1);	// 42, 127, 212
			else
				val = 0;		// Phase not used
		}
		// Setup Color
		palColors[i] = RGB(val, val, val);


		c = GetIColorSwatch(hPaletteWnd[i]);
		c->SetColor(palColors[i]);
		ReleaseIColorSwatch(c);
	}

	// change current Color according to editMode
	reloadBkupColor();
}

// *****************************************************************
void	VertexPaint::backupCurrentColor()
{
	switch (getEditionType())
	{
	case 0: lastWeightColor = iColor->GetColor(); break;
	case 1:
	case 2: lastPhaseColor = iColor->GetColor(); break;
	}
}
void	VertexPaint::reloadBkupColor()
{
	// Change current color according to editMode.
	switch (getEditionType())
	{
	case 0: iColor->SetColor(lastWeightColor); break;
	case 1:
	case 2: iColor->SetColor(lastPhaseColor); break;
	}
}


// *****************************************************************
void	VertexPaint::fillSelectionColor()
{
	int		mci;

	// Put Data in Undo/Redo List.
	if (!theHold.Holding())
		theHold.Begin();

	ModContextList	modContexts;
	INodeTab		nodeTab;

	GetCOREInterface()->GetModContexts(modContexts, nodeTab);

	for (mci = 0; mci < modContexts.Count(); mci++)
	{
		ModContext *mc = modContexts[mci];
		if (mc && mc->localData)
			theHold.Put(new VertexPaintRestore((VertexPaintData*)mc->localData, this));
	}

	theHold.Accept(GetString(IDS_RESTORE_FILL));


	// Which Component to change??
	VertexPaintData::TComponent	whichComponent;
	switch (getEditionType())
	{
	case 0: whichComponent = VertexPaintData::Red; break;
	case 1: whichComponent = VertexPaintData::Green; break;
	case 2: whichComponent = VertexPaintData::Blue; break;
	}


	// Modify all meshes.
	for (mci = 0; mci < modContexts.Count(); mci++)
	{
		ModContext *mc = modContexts[mci];
		if (mc && mc->localData)
		{
			VertexPaintData* d = (VertexPaintData*)mc->localData;
			Mesh*		mesh = d->GetMesh();
			if (mesh && mesh->vertCol)
			{
				// For all faces of the mesh
				for (int fi = 0; fi < mesh->getNumFaces(); fi++)
				{
					Face* f = &mesh->faces[fi];

					for (int i = 0; i < 3; i++)
					{
						// Skip painting because not selected??
						if (mesh->selLevel == MESH_VERTEX && !mesh->VertSel()[f->v[i]])
							continue;
						if (mesh->selLevel == MESH_FACE && !mesh->FaceSel()[fi])
							continue;
						// Also skip if face is hidden.
						if (f->Hidden())
							continue;

						// Apply painting
						d->SetColor(f->v[i], 1, GetActiveColor(), whichComponent);
					}
				}
			}
		}
	}

	// refresh
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// *****************************************************************
void	VertexPaint::fillSelectionGradientColor()
{
	int		mci;

	// Put Data in Undo/Redo List.
	if (!theHold.Holding())
		theHold.Begin();

	ModContextList	modContexts;
	INodeTab		nodeTab;

	GetCOREInterface()->GetModContexts(modContexts, nodeTab);

	for (mci = 0; mci < modContexts.Count(); mci++)
	{
		ModContext *mc = modContexts[mci];
		if (mc && mc->localData)
			theHold.Put(new VertexPaintRestore((VertexPaintData*)mc->localData, this));
	}

	theHold.Accept(GetString(IDS_RESTORE_GRADIENT));


	// Which Component to change??
	VertexPaintData::TComponent	whichComponent;
	switch (getEditionType())
	{
	case 0: whichComponent = VertexPaintData::Red; break;
	case 1: whichComponent = VertexPaintData::Green; break;
	case 2: whichComponent = VertexPaintData::Blue; break;
	}
	COLORREF	grad0 = iColorGradient[0]->GetColor();
	COLORREF	grad1 = iColorGradient[1]->GetColor();


	// Get Matrix to viewport.
	Matrix3		viewMat;
	{
#if MAX_VERSION_MAJOR >= 19
		ViewExp *ve = &GetCOREInterface()->GetActiveViewExp();
#else
		ViewExp *ve = GetCOREInterface()->GetActiveViewport();
#endif
		// The affine TM transforms from world coords to view coords
		ve->GetAffineTM(viewMat);

#if MAX_VERSION_MAJOR < 19
		GetCOREInterface()->ReleaseViewport(ve);
#endif
	}


	// Modify all meshes.
	for (mci = 0; mci < modContexts.Count(); mci++)
	{
		ModContext *mc = modContexts[mci];
		if (mc && mc->localData)
		{
			VertexPaintData* d = (VertexPaintData*)mc->localData;
			Mesh*		mesh = d->GetMesh();
			if (mesh && mesh->vertCol)
			{
				float	yMini = FLT_MAX;
				float	yMaxi = -FLT_MAX;

				// 1st, For all faces of the mesh, comute BBox of selection.
				int fi;
				for (fi = 0; fi < mesh->getNumFaces(); fi++)
				{
					Face* f = &mesh->faces[fi];

					for (int i = 0; i < 3; i++)
					{
						// Skip painting because not selected??
						if (mesh->selLevel == MESH_VERTEX && !mesh->VertSel()[f->v[i]])
							continue;
						if (mesh->selLevel == MESH_FACE && !mesh->FaceSel()[fi])
							continue;
						// Also skip if face is hidden.
						if (f->Hidden())
							continue;

						// Transform to viewSpace.
						Point3	p = viewMat*mesh->getVert(f->v[i]);
						// extend bbox.
						yMini = p.y < yMini ? p.y : yMini;
						yMaxi = p.y > yMaxi ? p.y : yMaxi;
					}
				}

				// 2nd, For all faces of the mesh, fill with gradient
				for (fi = 0; fi < mesh->getNumFaces(); fi++)
				{
					Face* f = &mesh->faces[fi];

					for (int i = 0; i < 3; i++)
					{
						// Skip painting because not selected??
						if (mesh->selLevel == MESH_VERTEX && !mesh->VertSel()[f->v[i]])
							continue;
						if (mesh->selLevel == MESH_FACE && !mesh->FaceSel()[fi])
							continue;
						// Also skip if face is hidden.
						if (f->Hidden())
							continue;

						// Compute gradientValue.
						float	gradValue;
						Point3	p = viewMat*mesh->getVert(f->v[i]);
						gradValue = (p.y - yMini) / (yMaxi - yMini);
						// Modifie with bendPower. 1->6.
						float	pow = 1 + fGradientBend * 5;
						gradValue = powf(gradValue, pow);

						// Apply painting
						// Reset To 0.
						d->SetColor(f->v[i], 1, grad0, whichComponent);
						// Blend with gradientValue.
						d->SetColor(f->v[i], gradValue, grad1, whichComponent);
					}
				}

			}
		}
	}

	// refresh
	NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}


// *****************************************************************
VertexPaintData::VertexPaintData(Mesh& m) : mesh(NULL), colordata(NULL), nverts(NULL),
nvcverts(NULL), numColors(0), numnverts(0), numnvcverts(0)
{
	SetCache(m);
}

VertexPaintData::VertexPaintData() : mesh(NULL), colordata(NULL), nverts(NULL),
nvcverts(NULL), numColors(0), numnverts(0), numnvcverts(0)
{

}

VertexPaintData::~VertexPaintData()
{
	FreeCache();

	if (colordata) delete[] colordata;
	if (nverts) delete[] nverts;
	if (nvcverts) delete[] nvcverts;

	nverts = NULL;
	nvcverts = NULL;
	colordata = NULL;

	numColors = 0;
	numnverts = 0;
	numnvcverts = 0;
}

void VertexPaintData::SetCache(Mesh& m)
{
	FreeCache();
	mesh = new Mesh(m);
	SynchVerts(m);
	AllocColorData(mesh->getNumVerts());

}

void VertexPaintData::FreeCache()
{
	if (mesh) delete mesh;
	if (nverts) delete[] nverts;
	if (nvcverts) delete[] nvcverts;

	mesh = NULL;
	nverts = NULL;
	nvcverts = NULL;
	numnverts = 0;
	numnvcverts = 0;
}

Mesh* VertexPaintData::GetMesh()
{
	return mesh;
}

NVert&  VertexPaintData::GetNVert(int i)
{
	static NVert nv;

	if (numnverts > i)
		return nverts[i];
	else
		return nv;
}

NVert& VertexPaintData::GetNVCVert(int i)
{
	static NVert nv;

	if (numnvcverts > i)
		return nvcverts[i];
	else
		return nv;
}

COLORREF& VertexPaintData::GetColor(int i)
{
	static COLORREF c = RGB(0, 0, 0);
	if (numColors > i)
		return colordata[i].color;
	else
		return c;
}

ColorData& VertexPaintData::GetColorData(int i)
{
	static ColorData c;

	if (numColors > i)
		return colordata[i];
	else
		return c;
}

void VertexPaintData::SetColor(int i, float bary, COLORREF c, TComponent whichComp)
{

	if (colordata && numColors > i)
	{
		// change color.
		COLORREF	oldColor = colordata[i].color;
		int			oldVal;
		int			editVal;
		int			newVal;

		// Mask good component.
		switch (whichComp)
		{
		case  Red:
			oldVal = GetRValue(colordata[i].color);
			editVal = GetRValue(c);
			break;
		case  Green:
			oldVal = GetGValue(colordata[i].color);
			editVal = GetGValue(c);
			break;
		case  Blue:
			oldVal = GetBValue(colordata[i].color);
			editVal = GetBValue(c);
			break;
		}

		// Blend Color component
		// This color was set before !
		float alpha = (1.0f - bary);

		// Compute new value
		newVal = (int)(alpha*oldVal + bary*editVal);

		// Mask good component.
		switch (whichComp)
		{
		case  Red:
			colordata[i].color = (RGB(newVal, 0, 0)) | (oldColor & RGB(0, 255, 255));
			break;
		case  Green:
			colordata[i].color = (RGB(0, newVal, 0)) | (oldColor & RGB(255, 0, 255));
			break;
		case  Blue:
			colordata[i].color = (RGB(0, 0, newVal)) | (oldColor & RGB(255, 255, 0));
			break;
		}

	}
}

void VertexPaintData::SetColor(int i, const ColorData &c)
{
	if (colordata && numColors > i)
	{
		colordata[i] = c;
	}
}

int VertexPaintData::GetNumColors()
{
	return numColors;
}

void VertexPaintData::AllocColorData(int numcols)
{
	ColorData* newColorData;

	// Colors already exist.
	if (numColors == numcols)
		return;

	if (numColors > 0)
	{
		// If the new number of colors is bigger than what we have in the colordata array
		if (numcols > numColors)
		{
			// Allocate a new color list and fill in as many as
			// we have from the previous set
			newColorData = new ColorData[numcols];

			for (int i = 0; i < numcols; i++)
			{
				if (i < numColors)
				{
					newColorData[i] = colordata[i];
				}
			}
			delete[] colordata;

			colordata = newColorData;

			numColors = numcols;

		}
		else
		{
			numColors = numcols;
		}
	}
	else
	{
		// Allocate a complete new set of colors
		numColors = numcols;
		colordata = new ColorData[numColors];
	}
}

LocalModData* VertexPaintData::Clone()
{
	VertexPaintData* d = new VertexPaintData();

	if (colordata)
	{
		d->colordata = new ColorData[numColors];
		d->numColors = numColors;
		for (int i = 0; i < numColors; i++)
		{
			d->colordata[i] = colordata[i];
		}
	}
	if (nverts)
	{
		d->nverts = new NVert[numnverts];
		for (int i = 0; i < numnverts; i++)
		{
			d->nverts[i] = nverts[i];
		}

	}
	if (nvcverts)
	{
		d->nvcverts = new NVert[numnvcverts];
		for (int i = 0; i < numnvcverts; i++)
		{
			d->nvcverts[i] = nvcverts[i];
		}

	}

	return d;
}

void VertexPaintData::SynchVerts(Mesh &m)
{
	if (mesh == NULL)
	{
		nlwarning("mesh == NULL");
		return;
	}

	if (nverts)
		delete[] nverts;

	numnverts = m.getNumVerts();

	nverts = new NVert[numnverts];

	if (nvcverts)
		delete[] nvcverts;

	numnvcverts = m.getNumVertCol();

	nvcverts = new NVert[numnvcverts];

	for (int i = 0; i < mesh->getNumFaces(); i++)
	{
		// for each vertex of each face
		for (int j = 0; j < 3; j++)
		{
			int iCur = nverts[mesh->faces[i].v[j]].faces.Count();

			// Tell the vertex, which to which face it belongs and which 
			// of the three face v-indices corresponds to the vertex

			nverts[mesh->faces[i].v[j]].faces.SetCount(iCur + 1);
			nverts[mesh->faces[i].v[j]].whichVertex.SetCount(iCur + 1);

			nverts[mesh->faces[i].v[j]].faces[iCur] = i;
			nverts[mesh->faces[i].v[j]].whichVertex[iCur] = j;


			if (mesh->vcFace)
			{
				// Do the same for texture vertices
				iCur = nvcverts[mesh->vcFace[i].t[j]].faces.Count();

				nvcverts[mesh->vcFace[i].t[j]].faces.SetCount(iCur + 1);
				nvcverts[mesh->vcFace[i].t[j]].whichVertex.SetCount(iCur + 1);

				nvcverts[mesh->vcFace[i].t[j]].faces[iCur] = i;
				nvcverts[mesh->vcFace[i].t[j]].whichVertex[iCur] = j;

			}
			else
				nlassert(0);
		}
	}
}


//***************************************************************************
//**
//** NVert
//**
//***************************************************************************



NVert::NVert()
{
	faces.SetCount(0);
	whichVertex.SetCount(0);
}

NVert& NVert::operator= (NVert &nvert)
{
	faces = nvert.faces;
	whichVertex = nvert.whichVertex;
	return *this;
}

//***************************************************************************
//**
//** ColorData 
//**
//***************************************************************************


ColorData::ColorData(DWORD col) : color(col)
{
}

ColorData::ColorData() : color(0)
{
}

//***************************************************************************
//**
//** VertexPaintRestore : public RestoreObj
//**
//***************************************************************************

VertexPaintRestore::VertexPaintRestore(VertexPaintData *pLocalData, VertexPaint *pVPaint)
	: pMod(pVPaint), pPaintData(pLocalData), redoColordata(NULL)
{
	colordata = new ColorData[pPaintData->numColors];
	for (int i = 0; i < pPaintData->numColors; i++)
	{
		colordata[i] = pPaintData->colordata[i];
	}
	numcolors = pPaintData->numColors;

}

VertexPaintRestore::~VertexPaintRestore()
{
	if (colordata)
		delete[] colordata;

	if (redoColordata)
		delete[] redoColordata;
}

void VertexPaintRestore::Restore(int isUndo)
{
	if (isUndo)
	{
		nlassert(pPaintData->colordata);

		redoColordata = pPaintData->colordata;
		redonumcolors = pPaintData->numColors;

		pPaintData->colordata = colordata;
		pPaintData->numColors = numcolors;

		colordata = NULL;

		pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
}

void VertexPaintRestore::Redo()
{
	nlassert(pPaintData->colordata);

	colordata = pPaintData->colordata;
	numcolors = pPaintData->numColors;

	pPaintData->colordata = redoColordata;
	pPaintData->numColors = redonumcolors;

	redoColordata = NULL;

	pMod->NotifyDependents(FOREVER, PART_VERTCOLOR, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

}

int  VertexPaintRestore::Size()
{
	int iSize = 0;

	if (colordata)
		iSize += sizeof(ColorData) * numcolors;

	if (redoColordata)
		iSize += sizeof(ColorData) * redonumcolors;

	return iSize;
}

