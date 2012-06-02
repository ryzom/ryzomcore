
/**********************************************************************
 *<
	FILE: editpat.h

	DESCRIPTION:  Edit Patch OSM

	CREATED BY: Tom Hudson, Dan Silva & Rolf Berteig

	HISTORY: created 23 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/


#ifndef __EDITPATCH_H__
#define __EDITPATCH_H__

#include "mods.h"
#include "modsres.h"
#include "nel/misc/rgba.h"
#include "nel/3d/quad_tree.h"
#include <algorithm>
#include "../nel_patch_lib/rpo.h"
#include "nel/3d/tile_bank.h"
#include <list>
#include "../nel_patch_lib/vertex_neighborhood.h"

// For MAX_RELEASE
#include <plugapi.h>

namespace NL3D
{
class CCamera;
class CViewport;
class CLandscape;
}

using namespace NL3D;
using namespace NLMISC;

#define Alert(x) MessageBox(GetActiveWindow(),x,_T("Alert"),MB_OK);

#define EDITPAT_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL)
#define CID_EP_PAINT CID_EP_BEVEL+5

// These are values for selLevel.
#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_TILE		4

#define COLOR_BRUSH_STEP 10
#define COLOR_BRUSH_MIN	(2.f)
#define COLOR_BRUSH_MAX	(32.f)

// Named selection set levels:
#define EP_NS_VERTEX 0
#define EP_NS_EDGE 1
#define EP_NS_PATCH 2
// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { EP_NS_VERTEX, EP_NS_VERTEX, EP_NS_EDGE, EP_NS_PATCH };
static int namedClipLevel[] = { CLIP_P_VERT, CLIP_P_VERT, CLIP_P_EDGE, CLIP_P_PATCH };

#define MAX_MATID	0xffff

#define UNDEFINED	0xffffffff

#define CID_EPM_BIND	CID_USER + 203
#define CID_EPM_EXTRUDE	CID_USER + 204
#define CID_EPM_BEVEL	CID_USER + 205

class EditPatchMod;
class CNelPatchChanger;

#define BRUSH_COUNT 3

// Advanced TessApprox settings...
class AdvParams 
{
public:
	TessSubdivStyle mStyle;
	int mMin, mMax;
	int mTris;
};

class PatchRightMenu : public RightClickMenu 
{
private:
	EditPatchMod *ep;
public:
	void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
	void Selected(UINT id);
	void SetMod(EditPatchMod *ep) { this->ep = ep; }
};

class EPM_BindMouseProc : public MouseCallBack 
{
	friend  class MouseListener;
	private:
		EditPatchMod *pobj;
		IObjParam *ip;
		IPoint2 om;
		BitArray knotList;
		PatchMesh *pMesh;
		RPatchMesh *rpMesh;
		CVertexNeighborhood	tab;
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert);
		BOOL HitASegment(ViewExp *vpt, IPoint2 *p, int *Seg);

		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		EPM_BindMouseProc(EditPatchMod* spl, IObjParam *i) { pobj=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
};

struct EPM_Mesh
{
	EPM_Mesh (PatchMesh *pmesh, RPatchMesh *rmesh, class EditPatchData *patchData, INode* node, ModContext *mod, int mcListIndex)
	{
		PMesh=pmesh;
		RMesh=rmesh;
		PatchData=patchData;
		Node=node;
		Mod=mod;
		McListIndex=mcListIndex;
	}
	PatchMesh *PMesh;
	RPatchMesh *RMesh;
	INode *Node;
	ModContext *Mod;
	int		McListIndex;
	class EditPatchData *PatchData;
};

struct EPM_PaintVertex
{
	EPM_PaintVertex (sint32 mesh, sint32 vert)
	{
		Mesh=mesh;
		vertex=vert;
	}
	sint32		Mesh;
	sint32		vertex;
};

struct EPM_PaintTile
{
	EPM_PaintTile ()
	{
		Mesh=-1;
	}
	EPM_PaintTile* get2Voisin (int i)
	{
		if (voisins[i])
		{
			return voisins[i]->voisins[(i+rotate[i])&3];
		}
		return NULL;
	}
	int get2VoisinRotate (int i)
	{
		if (voisins[i])
		{
			return rotate[i]+voisins[i]->rotate[(i+rotate[i])&3];
		}
		return NULL;
	}
	EPM_PaintTile* getRight256 (int rot, int& _rotate)
	{
		_rotate=rotate[(2+rot)&3];
		return voisins[(2+rot)&3];
	}
	EPM_PaintTile* getBottom256 (int rot, int& _rotate)
	{
		_rotate=rotate[(1+rot)&3];
		return voisins[(1+rot)&3];
	}
	EPM_PaintTile* getRightBottom256 (int rot, int& _rotate)
	{
		int rightRot;
		EPM_PaintTile* right=getRight256 (rot, rightRot);
		if (right)
			return right->getBottom256 ((rot-rightRot)&3, _rotate);
		else
			return NULL;
	}
	EPM_PaintTile* getBottomRight256 (int rot, int& _rotate)
	{
		int bottomRot;
		EPM_PaintTile* bottom=getBottom256 (rot, bottomRot);
		if (bottom)
			return bottom->getRight256 ((rot-bottomRot)&3, _rotate);
		else
			return NULL;
	}
	bool validFor256 (int rot)
	{
		int _rotate;
		if (!getRight256 (rot, _rotate))
			return false;
		if (!getBottom256 (rot, _rotate))
			return false;
		if (!getRightBottom256 (rot, _rotate))
			return false;
		if (getRightBottom256 (rot, _rotate)!=getBottomRight256 (rot, _rotate))
			return false;
		return true;
	}
	void set256 (int rotate)
	{
	}
	bool intersect (const Ray& ray, std::vector<EPM_Mesh>& vectMesh, TimeValue t, NLMISC::CVector& hit);
	sint32		patch;
	sint32		tile;
	sint16		Mesh;
	uint8		u;
	uint8		v;
	EPM_PaintTile*	voisins[4];
	uint8			rotate[4];
	CVector			Center;
	float			Radius;
};

struct EPM_PaintPatch
{
	enum TBorder {left=0, bottom, right, top, count};
	EPM_PaintPatch ()
	{
		Mesh=-1;
	}
	EPM_PaintPatch (sint32 mesh, sint32 p)
	{
		Mesh=mesh;
		patch=p;
	}
	sint32		Mesh;
	sint32		patch;
};

/*-------------------------------------------------------------------*/

class tileSetIndex
{
public:
	int TileSet;
	int Rotate;
	bool operator< (const tileSetIndex& other) const
	{
		if (TileSet<other.TileSet)
			return true;
		if (TileSet>other.TileSet)
			return false;
		int delta=(other.Rotate-Rotate)&3;
		if (delta==1)
			return true;
		if (delta==3)
			return false;
		if (delta==0)
			return false;
		nlassert (0);	// no!
		return false;
	}
	bool operator!= (const tileSetIndex& other) const
	{
		return ((TileSet!=other.TileSet)||(Rotate!=other.Rotate));
	}
	bool operator== (const tileSetIndex& other) const
	{
		return ((TileSet==other.TileSet)&&(Rotate==other.Rotate));
	}
};

/*-------------------------------------------------------------------*/

class CBackupValue
{
public:
	// Ctor
	CBackupValue () {}
	CBackupValue (const tileDesc& desc, uint mesh, uint tile) : Desc (desc), Mesh (mesh), Tile (tile)
	{}

	// Public data
	tileDesc		Desc;
	uint			Mesh;
	uint			Tile;
};

/*-------------------------------------------------------------------*/

class EPM_PaintMouseProc : public MouseCallBack 
{
	friend  class MouseListener;
	friend DWORD WINAPI myThread (LPVOID vData);
	friend class CTileUndo;
	friend class CPaintColor;
	friend class CFillPatch;

private:
	EditPatchMod *pobj;
	IObjParam *ip;
	IPoint2 om;
	//PatchMesh *pMesh;
	//sint32 Mesh;
	//std::map<PatchMesh* ,sint32 > metaMeshIndex;
	std::vector<std::vector<EPM_PaintTile> > metaTile;
	std::vector<BitArray> bitArray;
	int Rotation;
	int TileIndex;
	CQuadTree<EPM_PaintTile*> quadTreeSelect;

protected:

	// Paint algorithm
	void SetTile (int mesh, int tile, const tileDesc& desc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, CNelPatchChanger& nelPatchChg,
			std::vector<CBackupValue>* backupStack, bool undo=true, bool updateDisplace=false);

	// Get a tile
	void GetTile (int mesh, int tile, tileDesc& desc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land);

	bool PutATile ( EPM_PaintTile* pTile, int tileSet, int curRotation, const NL3D::CTileBank& bank, 
								   bool selectCycle, std::set<EPM_PaintTile*>& visited, std::vector<EPM_Mesh>& vectMesh, 
								   NL3D::CLandscape* land, CNelPatchChanger& nelPatchChg, bool _256);

	void PutADisplacetile ( EPM_PaintTile* pTile, const CTileBank& bank, 
								   std::vector<EPM_Mesh>& vectMesh, 
								   CLandscape* land, CNelPatchChanger& nelPatchChg);

	BOOL PutDisplace (int tile, int mesh, const CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, 
								  int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, CNelPatchChanger& nelPatchChg);

	int	selectTile (uint tileSet, bool selectCycle, bool _256, uint group, const CTileBank& bank);
	bool GetBorderDesc (EPM_PaintTile* tile, tileSetIndex *pVoisinCorner, NL3D::CTileSet::TFlagBorder pBorder[4][3],
						tileDesc *pVoisinIndex, const NL3D::CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, CNelPatchChanger& nelPatchChg);

	const NL3D::CTileSetTransition* FindTransition (int nTileSet, int nRotate, const NL3D::CTileSet::TFlagBorder *border, 
		const NL3D::CTileBank& bank);
	bool PropagateBorder (EPM_PaintTile* tile, int curRotation, int curTileSet, std::set<EPM_PaintTile*>& visited, 
		const NL3D::CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, NL3D::CLandscape* land, CNelPatchChanger& nelPatchChg, 
		std::vector<CBackupValue>& backupStack, bool recurseNoDiff=true);

	// Calc rotate path
	uint8 CalcRotPath (EPM_PaintTile* from, EPM_PaintTile* to, int depth, int rotate, int& deltaX, int& deltaY, int& cost);

	// Just put a tile
	BOOL PutTile (int tile, int mesh, bool first, const NL3D::CTileBank& bank, int tileSet, std::vector<EPM_Mesh>& vectMesh, NL3D::CLandscape* land,
		int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, CNelPatchChanger& nelPatchChg, bool _256);

	void RecursTile (EPM_PaintTile* pTile, const CTileBank& bank, int tileSet, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, int recurs, 
		std::set<EPM_PaintTile*>& alreadyRecursed, bool first, int rotation, CNelPatchChanger& nelPatchChg, bool _256);


	BOOL HitATile(ViewExp *vpt, IPoint2 *p, int *tile, int *mesh, TimeValue t, std::vector<EPM_Mesh>& vectMesh, NLMISC::CVector& hit);
	BOOL HitATile(const CViewport& viewport, const CCamera& camera, float x, float y, int *tile, int *mesh, TimeValue t, std::vector<EPM_Mesh>& vectMesh, NLMISC::CVector& hit);
	BOOL AnyHits( ViewExp *vpt ) 
	{ 
		return vpt->NumSubObjHits(); 
	}

public:
	EPM_PaintMouseProc(EditPatchMod* spl, IObjParam *i) 
	{ 
		pobj=spl; 
		ip=i; 
		TileIndex=0;		

		// Init plane matrix XY
		NLMISC::CMatrix		tmp;
		NLMISC::CVector		I(1,0,0);
		NLMISC::CVector		J(0,0,-1);
		NLMISC::CVector		K(0,1,0);
		tmp.identity();
		tmp.setRot(I,J,K, true);
		quadTreeSelect.changeBase (tmp);
	}
	int proc( 
		HWND hwnd, 
		int msg, 
		int point, 
		int flags, 
		IPoint2 m );
};


class EPM_BindCMode : public CommandMode 
{
	private:
		ChangeFGObject fgProc;
		EPM_BindMouseProc eproc;
		EditPatchMod* pobj;
//		int type; // See above

	public:
		EPM_BindCMode(EditPatchMod* spl, IObjParam *i) :
			fgProc((ReferenceTarget*)spl), eproc(spl,i) {pobj=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EP_BIND; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
//		void SetType(int type) { this->type = type; eproc.SetType(type); }
};

class EPM_ExtrudeMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditPatchMod *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EPM_ExtrudeMouseProc(EditPatchMod* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EPM_ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EPM_ExtrudeSelectionProcessor(EPM_ExtrudeMouseProc *mc, EditPatchMod *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EPM_ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EPM_ExtrudeSelectionProcessor mouseProc;
	EPM_ExtrudeMouseProc eproc;
	EditPatchMod* po;

public:
	EPM_ExtrudeCMode(EditPatchMod* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EPM_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};




class EPM_BevelMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	EditPatchMod *po;
	Interface *ip;
	IPoint2 om;
	
public:
	EPM_BevelMouseProc(EditPatchMod* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EPM_BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EPM_BevelSelectionProcessor(EPM_BevelMouseProc *mc, EditPatchMod *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EPM_BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EPM_BevelSelectionProcessor mouseProc;
	EPM_BevelMouseProc eproc;
	EditPatchMod* po;

public:
	EPM_BevelCMode(EditPatchMod* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EPM_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class VertInsertRecord;
class PickPatchAttach;

class EditPatchMod : public Modifier, IPatchOps, IPatchSelect, ISubMtlAPI, AttachMatDlgUser 
{
	friend class EPTempData;
	friend class EditPatchData;
	friend class XFormProc;
	friend class PatchRestore;
	friend class PVertexRightMenu;
	friend class PatchRightMenu;
	friend class PickPatchAttach;

	public:
		static HWND hSelectPanel, hOpsPanel, hSurfPanel, hTilePanel, hEdgePanel;
		static BOOL rsSel, rsOps, rsSurf, rsTile, rsEdge;	// rollup states (FALSE = rolled up)
		static IObjParam *ip;		
		
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;
		static ISpinnerControl *weldSpin;
		static ISpinnerControl *stepsSpin;
		static ISpinnerControl *tileSpin;
		static ISpinnerControl *transitionSpin;
//3-18-99 to suport render steps and removal of the mental tesselator
		static ISpinnerControl *stepsRenderSpin;
		static PickPatchAttach pickCB;
		static BOOL patchUIValid;
		static BOOL tileUIValid;
		static BOOL edgeUIValid;

//watje command mode for the extrude and beevl		
		static EPM_ExtrudeCMode *extrudeMode;
		static EPM_BevelCMode *bevelMode;
		static EPM_BindCMode *bindMode;

		// for the tessellation controls
		static BOOL settingViewportTess;  // are we doing viewport or renderer
		static BOOL settingDisp;          // if we're doign renderer is it mesh or displacmenent
		static ISpinnerControl *uSpin;
		static ISpinnerControl *vSpin;
		static ISpinnerControl *edgeSpin;
		static ISpinnerControl *angSpin;
		static ISpinnerControl *distSpin;
		static ISpinnerControl *mergeSpin;
		static ISpinnerControl *matSpin;
		static ISpinnerControl *tessUSpin;
		static ISpinnerControl *tessVSpin;
		static ISpinnerControl *tileNum;
		static ISpinnerControl *tileRot;
		static int		attachMat;
		static BOOL		condenseMat;
		static int		CurrentTileSet;
		static int		brushSize;
		static int		ColorBushSize;
		static int		tileSize;
		static int		TileGroup;		// Active group of tiles. 0: no gourp, 1-4: group 0 to 3
		static int		DisplaceTile;	// Active displace tile
		static int 		DisplaceTileSet;	// Active tileset in displace
		static uint		TileFillRotation;	// Rotation used in fill
		static bool		TileTrick;			// Trick
		static int		tileSetSet;
		static int		channelModified;
		static bool		additiveTile;
		static bool		automaticLighting;

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message ) { return REF_SUCCEED; }
		
		int selLevel;

		// RB:named sel sets
		BOOL namedSelNeedsFixup;	// TRUE for pre-r3 files
		Tab<TSTR*> namedSel[5];
		int FindSet(TSTR &setName,int level);
		void AddSet(TSTR &setName,int level);
		void RemoveSet(TSTR &setName,int level);
		void RemoveAllSets();
		void ClearSetNames();

		// Remembered info
		PatchMesh *rememberedPatch;	// NULL if using all selected patches
		int rememberedIndex;
		int rememberedData;

		BOOL displaySurface;
		BOOL displayLattice;
		int meshSteps;
//3-18-99 to suport render steps and removal of the mental tesselator
		int meshStepsRender;
		BOOL showInterior;
		int tileLevel;
		bool tileMode;
		bool includeMeshes;
		bool keepMapping;
		int transitionType;

		BOOL meshAdaptive;	// Future use (Not used now)
		TessApprox viewTess; // for GAP tessellation
		TessApprox prodTess;
		TessApprox dispTess;

		BOOL mViewTessNormals;	// use normals from the tesselator
		BOOL mProdTessNormals;	// use normals from the tesselator
		BOOL mViewTessWeld;	// Weld the mesh after tessellation
		BOOL mProdTessWeld;	// Weld the mesh after tessellation
		BOOL propagate;

		BOOL inExtrude;
		BOOL inBevel;


		EditPatchMod();
		~EditPatchMod();

		Interval LocalValidity(TimeValue t);
		ChannelMask ChannelsUsed()  { return EDITPAT_CHANNELS; }
		ChannelMask ChannelsChanged() 	{ return channelModified; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
		Class_ID InputType() { return RYKOLPATCHOBJ_CLASS_ID; }
		
		int CompMatrix(TimeValue t, ModContext& mc, Matrix3& tm, Interval& valid);

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(_T("EditPatchRyzomMod")); }
		Class_ID ClassID() { return Class_ID(0x4dd14a3c, 0x4ac23c0c); }
		void* GetInterface(ULONG id);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);

		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		int SubObjectIndex(HitRecord *hitRec);

 		BOOL DependOnTopology(ModContext &mc);

		// Generic xform procedure.
		void XFormVerts( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis );

		// Specialized xform for bezier handles
		void XFormHandles( XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis, int object, int handleIndex );

		// Affine transform methods		
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		// The following is called before the first Move(), Rotate() or Scale() call
		void TransformStart(TimeValue t);

		// The following is called after the user has completed the Move, Rotate or Scale operation and
		// the undo object has been accepted.
		void TransformFinish(TimeValue t);		

		// The following is called when the transform operation is cancelled by a right-click and the undo
		// has been cancelled.
		void TransformCancel(TimeValue t);		

		BOOL SupportsNamedSubSels() {return TRUE;}
		void ActivateSubSelSet(TSTR &setName);
		void NewSetFromCurSel(TSTR &setName);
		void RemoveSubSelSet(TSTR &setName);
		void SetupNamedSelDropDown();
		int NumNamedSelSets();
		TSTR GetNamedSelSetName(int i);
		void SetNamedSelSetName(int i,TSTR &newName);
		void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
		BOOL GetUniqueSetName(TSTR &name);
		int SelectNamedSet();
		void NSCopy();
		void NSPaste();
		void MaybeFixupNamedSels();

//watje 12-10-98
		void DoHide(int type); 
		void DoUnHide(); 
		void DoPatchHide(); 
		void DoVertHide(); 
		void DoEdgeHide(); 

		void DoAddHook(PatchMesh *pMesh, int vert0, int vert1, int vert2, int seg, int config);
		void DoRemoveHook(); 

//watje bevel and extrusion stuff
		void DoExtrude() ;
		void BeginExtrude(TimeValue t); 	
		void EndExtrude (TimeValue t, BOOL accept=TRUE);		
		void Extrude( TimeValue t, float amount, BOOL useLocalNorm );

		
		void DoBevel() ;
		void BeginBevel(TimeValue t); 	
		void EndBevel (TimeValue t, BOOL accept=TRUE);		
		void Bevel( TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd );




		void DoDeleteSelected();
		void DoVertDelete();
		void DoEdgeDelete();
		void DoPatchDelete();
		void DoPatchAdd(int type);
		void DoSubdivide(int type);
		void DoEdgeSubdivide();
		void DoPatchSubdivide();
		void DoVertWeld();
		void DoVertReset();
		void DoPatchDetach(int copy, int reorient);
		void DoPatchTurn(bool ccw);

		void ClearPatchDataFlag(ModContextList& mcList,DWORD f);
		void DeletePatchDataTempData();		
		void CreatePatchDataTempData();

		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {}

		void ChangeRememberedPatch(int type);
		void ChangeSelPatches(int type);
		int RememberPatchThere(HWND hWnd, IPoint2 m);
		void SetRememberedPatchType(int type);
		void ChangeRememberedVert(int type);
		void ChangeSelVerts(int type);
		int RememberVertThere(HWND hWnd, IPoint2 m);
		void SetRememberedVertType(int type);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		IOResult SaveLocalData(ISave *isave, LocalModData *ld);
		IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
		IOResult LoadNamedSelChunk(ILoad *iload,int level);

		CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next );
		RefTargetHandle Clone(RemapDir& remap = DefaultRemapDir());
		TCHAR *GetObjectName() { return GetString(IDS_TH_EDITPATCH); }
		void ActivateSubobjSel(int level, XFormModes& modes );
		int NeedUseSubselButton() { return 0; }
		void SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert );
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);
		
		void SetDisplaySurface(BOOL sw);
		void SetDisplayLattice(BOOL sw);
		void SetPropagate(BOOL sw);		
		BOOL GetPropagate() {return propagate;}		
		void SetMeshSteps(int count);
		int GetMeshSteps() { return meshSteps; }
//3-18-99 to suport render steps and removal of the mental tesselator
		void SetMeshStepsRender(int count);
		int GetMeshStepsRender() { return meshStepsRender; }
		void SetShowInterior(BOOL si);
		BOOL GetShowInterior() { return showInterior; }
		void SetTileSteps(int steps);
		int GetTileLevel() { return tileLevel; }
		void SetTileMode (bool bTile);
		bool GetTileMode() { return tileMode; }
		bool GetIncludeMeshes() { return includeMeshes; }
		void SetKeepMapping (bool bKeep);
		bool GetKeepMapping() { return keepMapping; }
		int GetTransitionLevel() { return transitionType; }
		void SetTransitionLevel(int transition);

// Future use (Not used now)
//		void SetMeshAdaptive(BOOL sw);
		void SetViewTess(TessApprox &tess);
		TessApprox GetViewTess() { return viewTess; }
		void SetProdTess(TessApprox &tess);
		TessApprox GetProdTess() { return prodTess; }
		void SetDispTess(TessApprox &tess);
		TessApprox GetDispTess() { return dispTess; }
		void SetTessUI(HWND hDlg, TessApprox *tess);
		BOOL GetViewTessNormals() { return mViewTessNormals; }
		void SetViewTessNormals(BOOL use);
		BOOL GetProdTessNormals() { return mProdTessNormals; }
		void SetProdTessNormals(BOOL use);
		BOOL GetViewTessWeld() { return mViewTessWeld; }
		void SetViewTessWeld(BOOL weld);
		BOOL GetProdTessWeld() { return mProdTessWeld; }
		void SetProdTessWeld(BOOL weld);

		// Get the commonality of material index for the selection (-1 indicates no commonality)
		int GetSelMatIndex();
		void SetSelMatIndex(int index);
		void SelectByMat(int index,BOOL clear);

		// Tile tess
		int GetSelTessU ();
		int GetSelTessV ();
		void SetSelTess (int nU, int nV);
		void BalanceSelPatch (int patch, int size, bool balanceU, std::set<int>& visitedU, std::set<int>& visitedV, RPatchMesh* rpatch, PatchMesh *);
		void BalanceSelPatch ();

		// *** Smooth flags

		/**
		  * Return 0 if no edge are selected or no selected edges have the flag set.
		  * Return 1 if all the selected edge have the flag set.
		  * Return 2 if some of the selected edge of the flag set but not all of them.
		  */
		int getSmoothFlags ();

		/// Set the smmoth flag for the selected edges.
		void setSmoothFlags (bool smooth);

		/*void SetTileNum (ULONG nU);
		ULONG GetTileNum ();
		void SetTileRot (int nU);
		int GetTileRot ();*/

		// Smoothing
		DWORD GetSelSmoothBits(DWORD &invalid);
		DWORD GetUsedSmoothBits();
		void SelectBySmoothGroup(DWORD bits,BOOL clear);
		void SetSelSmoothBits(DWORD bits,DWORD which);

		void PatchSelChanged();

		// from AttachMatDlgUser
		int GetAttachMat() { return attachMat; }
		void SetAttachMat(int value) { attachMat = value; }
		BOOL GetCondenseMat() { return condenseMat; }
		void SetCondenseMat(BOOL sw) { condenseMat = sw; }

		int DoAttach(INode *node, PatchMesh *attPatch, RPatchMesh *rattPatch, bool & canUndo);

		// Store current topology in the PatchObject
		void RecordTopologyTags();

		// Re-match named selection sets, etc. with changed topology (Call RecordTopologyTags
		// before making the changes to the shape, then call this)
		void ResolveTopoChanges();

		void RescaleWorldUnits(float f);

		int GetSubobjectLevel();
		void SetSubobjectLevel(int level);
		void RefreshSelType();
		void UpdateSelectDisplay();
		void SetSelDlgEnables();
		void SetOpsDlgEnables();
		void SetSurfDlgEnables();
		void SetTileDlgEnables();
		void SetEdgeDlgEnables();		
		void SelectionChanged();
		void InvalidateSurfaceUI();
		void InvalidateTileUI();
		void InvalidateEdgeUI();
		BitArray *GetLevelSelectionSet(PatchMesh *patch, RPatchMesh *rpatch);

		// patch select and operations interfaces, JBW 2/2/99
		void StartCommandMode(patchCommandMode mode);
		void ButtonOp(patchButtonOp opcode);

		DWORD GetSelLevel();
		void SetSelLevel(DWORD level);
		void LocalDataChanged();
	
		// ISubMtlAPI methods:
		MtlID	GetNextAvailMtlID(ModContext* mc);
		BOOL	HasFaceSelection(ModContext* mc);
		void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
		int		GetSelFaceUniqueMtlID(ModContext* mc);
		int		GetSelFaceAnyMtlID(ModContext* mc);
		int		GetMaxMtlID(ModContext* mc);
};

class PickPatchAttach : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		EditPatchMod *ep;
		
		PickPatchAttach() {ep=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		HCURSOR GetHitCursor(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
	};

// Table to convert selLevel values to patch selLevel flags.
const int patchLevel[] = {PATCH_OBJECT,PATCH_VERTEX,PATCH_EDGE,PATCH_PATCH,PATCH_OBJECT};

// Get display flags based on selLevel.
const DWORD patchLevelDispFlags[] = {0,DISP_VERTTICKS|DISP_SELVERTS,DISP_SELEDGES,DISP_SELPATCHES,0};

// For hit testing...
static int patchHitLevel[] = {0,SUBHIT_PATCH_VERTS | SUBHIT_PATCH_VECS,SUBHIT_PATCH_EDGES,SUBHIT_PATCH_PATCHES, 0};

class EditPatchClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE ) 
	{ 
		return new EditPatchMod; 
	}
	const TCHAR *	ClassName() { return "NeL Edit Advanced"; }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0x4dd14a3c, 0x4ac23c0c); }
	const TCHAR* 	Category() { return "NeL Tools";}
	void			ResetClassParams(BOOL fileReset);
	};

typedef Tab<Point3> Point3Tab;

class XFormProc {
	public:
		virtual Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat)=0;
		virtual void SetMat( Matrix3& mat ) {}
	};

class MoveXForm : public XFormProc {
	private:
		Point3 delta, tdelta;		
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) 
			{ return p + tdelta; }
		void SetMat( Matrix3& mat ) 
			{ tdelta = VectorTransform(Inverse(mat),delta); }
		MoveXForm(Point3 d) { delta = d; }
	};

class RotateXForm : public XFormProc {
	private:
		Matrix3 rot, trot;
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) 
			{ return (trot*p)*imat; }
		void SetMat( Matrix3& mat ) 
			{ trot = mat * rot; }
		RotateXForm(Quat q) { q.MakeMatrix(rot); }
	};

class ScaleXForm : public XFormProc {
	private:
		Matrix3 scale, tscale;
	public:
		Point3 proc(Point3& p, Matrix3 &mat, Matrix3 &imat) 
			{ return (p*tscale)*imat; }
		void SetMat( Matrix3& mat ) 
			{ tscale = mat*scale; }
		ScaleXForm(Point3 s) { scale = ScaleMatrix(s); }
	};

typedef Tab<int> IntTab;

// General-purpose patch point table -- Maintains point table for each of n polygons
class PatchPointTab {
	public:
		Point3Tab ptab;	// Patch mesh points
		Point3Tab vtab;	// Patch mesh vectors
		IntTab pttab;	// Patch point types
		PatchPointTab();
		~PatchPointTab();
		void Empty();
		void Zero();
		void MakeCompatible(PatchMesh& patch, BOOL clear=TRUE);
		PatchPointTab& operator=(PatchPointTab& from);
		BOOL IsCompatible(PatchMesh &patch);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class PatchVertexDelta {
	public:
		PatchPointTab dtab;

		void SetSize(PatchMesh &patch, BOOL load=TRUE);
		void Empty() { dtab.Empty(); }
		void Zero() { dtab.Zero(); }
		void SetVert(int i, const Point3& p) { dtab.ptab[i] = p; }
		void SetVertType(int i, int k) { dtab.pttab[i] = k; }
		void SetVec(int i, const Point3& p) { dtab.vtab[i] = p; }
		void MoveVert(int i, const Point3& p) { dtab.ptab[i] += p; }
		void MoveVec(int i, const Point3& p) { dtab.vtab[i] += p; }
		void Apply(PatchMesh& patch);
		void UnApply(PatchMesh& patch);
		PatchVertexDelta& operator=(PatchVertexDelta& from) { dtab = from.dtab; return *this; }
		void ApplyHandlesAndZero(PatchMesh &patch, int handleVert);
		BOOL IsCompatible(PatchMesh &patch) { return dtab.IsCompatible(patch); }
		void RescaleWorldUnits(float f) { dtab.RescaleWorldUnits(f); }
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

class AdjEdgeList;
class EPTempData;

/*-------------------------------------------------------------------*/

// Class for recording changes -- This is used to reconstruct an object from the original whenever
// the modifier is re-entered or whenever the system needs to reconstruct an object's cache.  This may be
// slow if a lot of changes have been recorded, but it's about the only way to properly reconstruct an object
// because the output of one operation becomes the input of the next.

// These are used as follows:
// When a user makes a modification to an object, a StartChangeGroup call needs to be made to the EditPatchData
// object.  Then a change record needs to be added for each sub-operation that makes up the modification.  These
// records are owned by the EditPatchData object, but they should also be referenced by the undo object for that
// operation.  If an undo is done, ownership of the modification record transfers to the undo/redo object and the
// record is REMOVED (NOT DELETED) from the EditPatchData object.  This keeps the record around for a redo operation
// but removes it from the list of records for the modifier.  If the undo is redone, ownership transfers back to
// the modifier, when it is re-added to the modification record list.

// Note that this class contains load and save methods, necessary because the modifier needs to be able to save
// and load them.  When you subclass off of this, be sure your load and save methods call the base class's first!

class PatchRestore;

class PModRecord {
	public:
		virtual BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord)=0;
		virtual IOResult Load(ILoad *iload)=0;
	};

typedef PModRecord* PPModRecord;
typedef Tab<PPModRecord> ModRecordTab;

/*-------------------------------------------------------------------*/

// Here are the types of modification records we use!

#define CLEARVERTSELRECORD_CHUNK	0x2000
#define SETVERTSELRECORD_CHUNK		0x2001
#define INVERTVERTSELRECORD_CHUNK	0x2002
#define CLEAREDGESELRECORD_CHUNK	0x2005
#define SETEDGESELRECORD_CHUNK		0x2006
#define INVERTEDGESELRECORD_CHUNK	0x2007
#define CLEARPATCHSELRECORD_CHUNK	0x2010
#define SETPATCHSELRECORD_CHUNK		0x2011
#define INVERTPATCHSELRECORD_CHUNK	0x2012
#define VERTSELRECORD_CHUNK			0x2020
#define EDGESELRECORD_CHUNK			0x2025
#define PATCHSELRECORD_CHUNK		0x2030
#define VERTMOVERECORD_CHUNK		0x2040
#define PATCHDELETERECORD_CHUNK		0x2050
#define VERTDELETERECORD_CHUNK		0x2060
#define PATCHCHANGERECORD_CHUNK		0x2070
#define VERTCHANGERECORD_CHUNK		0x2080
#define PATCHADDRECORD_CHUNK		0x2090
#define EDGESUBDIVIDERECORD_CHUNK	0x20A0
#define PATCHSUBDIVIDERECORD_CHUNK	0x20B0
#define VERTWELDRECORD_CHUNK		0x20C0
#define PATTACHRECORD_CHUNK			0x20D0
#define PATCHDETACHRECORD_CHUNK		0x20E0
#define PATCHMTLRECORD_CHUNK		0x20F0
										 
class ClearPVertSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPVertSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPVertSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPEdgeSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPEdgeSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPEdgeSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class ClearPatchSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class SetPatchSelRecord : public PModRecord {
	public:
		BitArray sel;	// Old state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class InvertPatchSelRecord : public PModRecord {
	public:
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PEdgeSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchSelRecord : public PModRecord {
	public:
		BitArray oldSel;	// Old state
		BitArray newSel;	// New state
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertMoveRecord : public PModRecord {
	public:
		PatchVertexDelta delta;	// Position changes for each vertex (Wasteful!  Change later?)
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchDeleteRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the spline looked before the delete
		RPatchMesh roldPatch;		// How the spline looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertDeleteRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch looked before the delete
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchChangeRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch mesh looked before the change
		RPatchMesh roldPatch;		// How the patch looked before the delete
		int index;
		int type;
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertChangeRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch mesh looked before the change
		RPatchMesh roldPatch;		// How the patch looked before the delete
		int index;
		int type;
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchAddRecord : public PModRecord {
	public:
		BOOL postWeld;			// Present in MAX 2.0 and up
		int type;				// 3 or 4 sides!
		PatchMesh oldPatch;		// How the patch looked before the addition
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class EdgeSubdivideRecord : public PModRecord {
	public:
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchSubdivideRecord : public PModRecord {
	public:
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PVertWeldRecord : public PModRecord {
	public:
		float thresh;			// Weld threshold
		BOOL propagate;			// Carry around entire patch mesh?
		PatchMesh oldPatch;		// How the patch looked before the addition
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PAttachRecord : public PModRecord {
	public:
		PatchMesh attPatch;			// The patch we're attaching
		int oldPatchCount;		// The number of splines present before attaching
		int mtlOffset;
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

class PatchDetachRecord : public PModRecord {
	public:
		int copy;
		PatchMesh oldPatch;
		RPatchMesh roldPatch;		// How the patch looked before the delete
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

typedef Tab<MtlID> MtlIDTab;

class PatchMtlRecord : public PModRecord {
	public:
		MtlIDTab materials;		// Materials from selected patches
		MtlID index;				// New material index assigned
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// Vertex Mapping class -- Gives mapping from vert in original patch to
// vert in modified patch

class EPMapVert {
	public:
		BOOL originalStored;
		int vert;
		Point3 original;	// Original point location
		Point3 delta;		// The delta we've applied
		EPMapVert() { originalStored = FALSE; vert = 0; original = Point3(0,0,0); delta = Point3(0,0,0); }
		EPMapVert(int v, Point3 &o, Point3 &d) { vert = v; original = o; delta = d; originalStored = TRUE; }
	};

class EPVertMapper {
	public:
		int verts;
		EPMapVert *vertMap;
		int vecs;
		EPMapVert *vecMap;
		EPVertMapper() { verts = vecs = 0; vertMap = vecMap = NULL; }
		~EPVertMapper();
		// Set up remap data structures.
		void Build(PatchMesh &patch);
		// Update the deltas we have stored, if necessary and apply to output patch mesh.
		// This is in response to the original shape changing
		void UpdateAndApplyDeltas(PatchMesh &inPatch, PatchMesh &outPatch);
		// Recompute the deltas we have stored
		// This is done after the modifier's user interaction changes the shape
		void RecomputeDeltas(PatchMesh &patch);
		// Record the topology tags in the specified shape
		void RecordTopologyTags(PatchMesh &patch);
		// Update the topology tag mapping
		void UpdateMapping(PatchMesh &patch);
		EPVertMapper& operator=(EPVertMapper &from);
		void RescaleWorldUnits(float f);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

/*-------------------------------------------------------------------*/

// EditPatchData flags
#define EPD_BEENDONE			(1<<0)
#define EPD_UPDATING_CACHE		(1<<1)
#define EPD_HASDATA				(1<<2)
#define EMD_HELD				(1<<3) // equivalent to A_HELD

// This is the data that each mod app will have.
class EditPatchData : public LocalModData {
	public:
		BOOL handleFlag;
		int handleVert;

		// Stuff we need to have for the patch's mesh conversion -- These are
		// Here because they're kind of a global change -- not undoable.
		int meshSteps;
//3-18-99 to suport render steps and removal of the mental tesselator
		int meshStepsRender;
		BOOL showInterior;
		int tileLevel;
		bool tileMode;
		bool includeMeshes;
		bool keepMapping;
		int transitionType;

		BOOL meshAdaptive;	// Future use (Not used now)
		TessApprox viewTess;
		TessApprox prodTess;
		TessApprox dispTess;
		BOOL mViewTessNormals;	// use normals from the tesselator
		BOOL mProdTessNormals;	// use normals from the tesselator
		BOOL mViewTessWeld;	// Weld the mesh after tessellation
		BOOL mProdTessWeld;	// Weld the mesh after tessellation
		BOOL displaySurface;
		BOOL displayLattice;

		DWORD flags;

		// This records the changes to the incoming object.
		ModRecordTab changes;

		// A pointer to the change record's vertex delta object
		PatchVertexDelta vdelta;

		// RB: Named selection set lists
		GenericNamedSelSetList vselSet;  // Vertex
		GenericNamedSelSetList eselSet;  // Edge
		GenericNamedSelSetList pselSet;  // Patch

		// While an object is being edited, this exists.
		EPTempData *tempData;

		// The knot mapping for the edited patch
		EPVertMapper vertMap; 

		// The final edited patch
		PatchMesh finalPatch;
		RPatchMesh rfinalPatch;

		EditPatchData(EditPatchMod *mod);
		EditPatchData(EditPatchData& emc);

		// Applies modifications to a patchObject
		void Apply(TimeValue t,RPO *patchOb,int selLevel);

		// Invalidates any caches affected by the change.
		void Invalidate(PartID part,BOOL meshValid=TRUE);

		// If this is the first edit, then the delta arrays will be allocated
		void BeginEdit(TimeValue t);

		LocalModData *Clone() { return new EditPatchData(*this); }

		void SetFlag(DWORD f,BOOL on) 
			{ 
			if ( on ) {
				flags|=f;
			} else {
				flags&=~f; 
				}
			}
		DWORD GetFlag(DWORD f) { return flags&f; }

		EPTempData *TempData(EditPatchMod *mod);

		// Change recording functions:
		void ClearHandleFlag() { handleFlag = FALSE; }
		void SetHandleFlag(int vert) { handleVert = vert; handleFlag = TRUE; }
		BOOL DoingHandles() { return handleFlag; }
		void ApplyHandlesAndZero(PatchMesh &patch) { vdelta.ApplyHandlesAndZero(patch, handleVert); }
		void RescaleWorldUnits(float f);

		// MAXr3: New recording system
		void RecordTopologyTags(PatchMesh *patch);
		void UpdateChanges(PatchMesh *patch, RPatchMesh *rpatch, BOOL checkTopology=TRUE);

		// Named selection set access
		GenericNamedSelSetList &GetSelSet(EditPatchMod *mod);	// Get the one for the current subobject selection level
		GenericNamedSelSetList &GetSelSet(int level);	// Get the one for the specified subobject selection level

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

// My generic restore class

class PatchRestore : public RestoreObj 
{
	public:
		bool	RPatchModified;
		PatchMesh oldPatch, newPatch;
		RPatchMesh *roldPatch, *rnewPatch;		// How the patch looked before the delete
		BOOL gotRedo;
		TimeValue t;
		EditPatchData *epd;
		EditPatchMod *mod;
		TSTR where;
		
		PatchRestore (EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch, RPatchMesh *rpatch, TCHAR *id=_T(""));
		virtual ~PatchRestore ();

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {mod->ClearAFlag(A_HELD);}
		TSTR Description() 
		{
			TSTR string;
			string.printf(_T("Generic patch restore [%s]"),where);
			return string;
		}
};

// Patch selection restore class

class PatchSelRestore : public RestoreObj {
	public:
		BitArray oldVSel, newVSel;
		BitArray oldESel, newESel;
		BitArray oldPSel, newPSel;
		BOOL gotRedo;
		TimeValue t;
		EditPatchData *epd;
		EditPatchMod *mod;

		PatchSelRestore(EditPatchData* pd, EditPatchMod* mod, PatchMesh *patch);

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {mod->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("Patch Select restore")); }
	};

/*-------------------------------------------------------------------*/

class EPTempData {
	private:
		PatchMesh		*patch;
		RPatchMesh		*rpatch;
		Interval 		patchValid;
		
		EditPatchMod 	*mod;
		EditPatchData 	*patchData;

	public:		
		
		~EPTempData();
		EPTempData(EditPatchMod *m,EditPatchData *md);
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		PatchMesh		*GetPatch(TimeValue t, RPatchMesh *&rPatch);

		BOOL PatchCached(TimeValue t);
		void UpdateCache(RPO *patchOb);
		EditPatchMod	*GetMod() { return mod; }
	};


class PatchDeleteUser : public EventUser 
{
private:
	EditPatchMod *ep;
public:
	void Notify() { ep->DoDeleteSelected(); }
	void SetMod(EditPatchMod *ep) { this->ep = ep; }
};


// Patch hit override functions

extern void SetPatchHitOverride(int value);
extern void ClearPatchHitOverride();

#endif // __EDITPATCH_H__
