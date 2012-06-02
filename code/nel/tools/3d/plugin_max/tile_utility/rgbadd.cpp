// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "tile_utility.h"

extern HINSTANCE hInstance;
TCHAR *GetString(int id);

#define NSUBTEX 2    // number of texture map slots
#define NCOLS 2

static int subTexId[NSUBTEX] = { IDC_MULT_TEX1, IDC_MULT_TEX2 };

#define ALPHA_FROM_1 0
#define ALPHA_FROM_2 1
#define ALPHA_FROM_MULT 2

//--------------------------------------------------------------
// RGBAdd: A Composite texture map
//--------------------------------------------------------------
class RGBAdd: public Texmap { 
	public:
	BOOL Param1;
	IParamBlock2 *pblock;   // ref #0
	Texmap* subTex[NSUBTEX];  // refs 1,2;
	BOOL mapOn[NSUBTEX];
	Color col[NCOLS];
	Interval ivalid;
	int alphaFrom;
	BOOL rollScroll;
	BOOL loadingOld;
		RGBAdd();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void NotifyChanged();

		void SetColor(int i, Color c, TimeValue t);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return RGBAddClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= "RGB Additive"; }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return NSUBTEX+1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return NSUBTEX+1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int RemapRefOnLoad(int iref); 

		RefTargetHandle Clone(RemapDir &remap = DefaultRemapDir());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


	};

class RGBAddClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new RGBAdd; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_RGBMULT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return RGBAddClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COMP;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("RGBAdd"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static RGBAddClassDesc maskCD;

ClassDesc* GetRGBAddDesc() { return &maskCD;  }

enum { RGBAdd_params };  // pblock ID
// RGBAdd_params param IDs
enum 
{ 
	RGBAdd_color1, RGBAdd_color2,
	RGBAdd_map1, RGBAdd_map2,		
	RGBAdd_map1_on, RGBAdd_map2_on, // main grad params 
	RGBAdd_type,
};


static ParamBlockDesc2 RGBAdd_param_blk ( RGBAdd_params, _T("parameters"),  0, &maskCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_RGBMULT, "RGB Additif Parameters", 0, 0, NULL, 
	// params
	RGBAdd_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_MULT_COL1, 
		end,
	RGBAdd_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_MULT_COL2, 
		end,
	RGBAdd_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		1,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MULT_TEX1,
		end,
	RGBAdd_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		2,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MULT_TEX2,
		end,
	RGBAdd_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	RGBAdd_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	RGBAdd_type, _T("alphaFrom"), TYPE_INT,				0,				IDS_PW_ALPHAFROM,
		p_default,		2,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_MULT_ALPHA1, IDC_MULT_ALPHA2, IDC_MULT_ALPHA3,
		end,

	end
);


//-----------------------------------------------------------------------------
//  RGBAdd
//-----------------------------------------------------------------------------
#define NPARAMS 2

static int name_id[NPARAMS] = {IDS_DS_COLOR1, IDS_DS_COLOR2};

#define RGBAdd_VERSION 2
static ParamBlockDescID pbdesc[NPARAMS] = {
	{ TYPE_RGBA, NULL, TRUE,RGBAdd_color1 },  // col1
	{ TYPE_RGBA, NULL, TRUE,RGBAdd_color2 }   // col2
	};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc,2,1),	// Version 1 params
};


void RGBAdd::Init() {
	ivalid.SetEmpty();
	alphaFrom = ALPHA_FROM_MULT;
	SetColor(0, Color(0.0f,0.0f,0.0f), TimeValue(0));
	SetColor(1, Color(0.0f,0.0f,0.0f), TimeValue(0));
	mapOn[0] = mapOn[1] = 1;
	}

void RGBAdd::Reset() {
	maskCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(1);	// get rid of maps
	DeleteReference(2);	// get rid of maps
	Init();
	}

void RGBAdd::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

RGBAdd::RGBAdd() 
{
	for (int i=0; i<NSUBTEX; i++) subTex[i] = NULL;
	pblock = NULL;
	maskCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}


static AColor white(1.0f,1.0f,1.0f,1.0f);

AColor RGBAdd::EvalColor(ShadeContext& sc) 
{
	if (gbufID) sc.SetGBufferID(gbufID);
	AColor c0 = (subTex[0]&&mapOn[0])? subTex[0]->EvalColor(sc): col[0];
	AColor c1 = (subTex[1]&&mapOn[1])? subTex[1]->EvalColor(sc): col[1];
	AColor c;
	c.r = c0.r+c1.r;
	c.r = c.r > 1.f ? 1.f : c.r;
	c.g = c0.g+c1.g;
	c.g = c.g > 1.f ? 1.f : c.g;
	c.b = c0.b+c1.b;
	c.b = c.b > 1.f ? 1.f : c.b;
	switch(alphaFrom) {
		case ALPHA_FROM_1: c.a = c0.a; break;
		case ALPHA_FROM_2: c.a = c1.a; break;
		case ALPHA_FROM_MULT: c.a = c0.a*c1.a; break;
		}
	return c;
}

float RGBAdd::EvalMono(ShadeContext& sc) 
{
	if (gbufID) sc.SetGBufferID(gbufID);
	float m = (subTex[1]&&mapOn[1])? subTex[1]->EvalMono(sc): Intens(col[0]);
	float c0 = (subTex[0]&&mapOn[0])? subTex[0]->EvalMono(sc): Intens(col[1]);
	return (m+c0) > 1.f ? 1.f : m+c0;
}

Point3 RGBAdd::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	Point3 p0  = subTex[0]&&mapOn[0]? subTex[0]->EvalNormalPerturb(sc): Point3(0.0f,0.0f,0.0f);
	Point3 p1  = subTex[1]&&mapOn[1]? subTex[1]->EvalNormalPerturb(sc): Point3(0.0f,0.0f,0.0f);
	return p0+p1;
	}

RefTargetHandle RGBAdd::Clone(RemapDir &remap) {
	RGBAdd *mnew = new RGBAdd();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+1,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	mnew->alphaFrom = alphaFrom;
	return (RefTargetHandle)mnew;
	}

ParamDlg* RGBAdd::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = maskCD.CreateParamDlgs(hwMtlEdit, imp, this);
	return masterDlg;

	}

void RGBAdd::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( RGBAdd_map1_on, 0, mapOn[0]);
		pblock->SetValue( RGBAdd_map2_on, 0, mapOn[1]);
		pblock->SetValue( RGBAdd_type, 0, alphaFrom);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( RGBAdd_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( RGBAdd_color2, t, col[1], ivalid );

		pblock->GetValue( RGBAdd_map1_on, t, mapOn[0], ivalid );
		pblock->GetValue( RGBAdd_map2_on, t, mapOn[1], ivalid );
		pblock->GetValue( RGBAdd_type, t, alphaFrom, ivalid );


		col[1].ClampMinMax();
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

void RGBAdd::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	int id = i==0?RGBAdd_color1:RGBAdd_color2;
	pblock->SetValue( id, t, c);
	}

RefTargetHandle RGBAdd::GetReference(int i) {
	if (i==0) return pblock;
	else return subTex[i-1];
	}

int RGBAdd::RemapRefOnLoad(int iref) { 
	if (loadingOld)  
		return iref+1;
	else return iref;
	}

void RGBAdd::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-1] = (Texmap *)rtarg; break;
		} 
	}

void RGBAdd::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+1,m);
	if (i==0)
		{
		RGBAdd_param_blk.InvalidateUI(RGBAdd_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		RGBAdd_param_blk.InvalidateUI(RGBAdd_map2);
		ivalid.SetEmpty();
		}	

	}

TSTR RGBAdd::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* RGBAdd::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return subTex[i-1]; 
		}
	}

TSTR RGBAdd::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-1);
		}
	}

RefResult RGBAdd::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock) 
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				RGBAdd_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define INVERT_RGBMULT_CHUNK 0x2000
#define ALPHA_FROM_CHUNK 0x2010
#define RGBAdd_VERSION_CHUNK  0x2020
#define PARAM2_CHUNK  0x2030

IOResult RGBAdd::Save(ISave *isave) { 
	IOResult res;
	ULONG nb;

	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	int vers = RGBAdd_VERSION;
	isave->BeginChunk(RGBAdd_VERSION_CHUNK);
	isave->Write(&vers,sizeof(vers),&nb);			
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	return IO_OK;
	}	
	  
class RGBAddPostLoad : public PostLoadCallback {
	public:
		RGBAdd *tm;
		RGBAddPostLoad(RGBAdd *b) {tm=b;}
		void proc(ILoad *iload) {  tm->loadingOld = FALSE; delete this; } 
	};

class RGBAdd2PostLoad : public PostLoadCallback {
	public:
		RGBAdd *n;
		RGBAdd2PostLoad(RGBAdd *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( RGBAdd_map1_on, 0, n->mapOn[0]);
				n->pblock->SetValue( RGBAdd_map2_on, 0, n->mapOn[1]);
				n->pblock->SetValue( RGBAdd_type, 0, n->alphaFrom);
				}
			delete this; 


			} 
	};



IOResult RGBAdd::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	int id;
	loadingOld = TRUE;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case PARAM2_CHUNK:
				Param1= FALSE;
				break;
			case RGBAdd_VERSION_CHUNK:
				loadingOld = FALSE;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			case ALPHA_FROM_CHUNK:
				res = iload->Read(&alphaFrom,sizeof(alphaFrom), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	if (loadingOld) 
		iload->RegisterPostLoadCallback(new RGBAddPostLoad(this));

	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &RGBAdd_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

//	iload->RegisterPostLoadCallback(new RGBAdd2PostLoad(this));



	return IO_OK;
	}


