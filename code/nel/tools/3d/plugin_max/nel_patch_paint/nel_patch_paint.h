#ifndef __EDITPATCH_H__
#define __EDITPATCH_H__

#include "resource.h"
#include <algorithm>
#include <maxversion.h>
#if MAX_VERSION_MAJOR >= 14
#	include <maxscript/maxscript.h>
#else
#	include <MaxScrpt/maxscrpt.h>
#endif
#include "namesel.h"
#include "nsclip.h"
#include "sbmtlapi.h"
#include "../nel_patch_lib/rpo.h"
#include "nel/3d/tile_bank.h"
#include "nel/3d/quad_tree.h"
#include "nel/misc/rgba.h"
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

#define COLOR_BRUSH_STEP 10
#define COLOR_BRUSH_MIN	(2.f)
#define COLOR_BRUSH_MAX	(32.f)

#define Alert(x) MessageBox(GetActiveWindow(),x,_T("Alert"),MB_OK);

#define EDITPAT_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL)
#define CID_EP_PAINT CID_EP_BEVEL+5

// These are values for selLevel.
#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_TILE		4

// Named selection set levels:
#define EP_NS_VERTEX 0
#define EP_NS_EDGE 1
#define EP_NS_PATCH 2

// Conversion from selLevel to named selection level:
static int namedSetLevel[] = { EP_NS_VERTEX, EP_NS_VERTEX, EP_NS_EDGE, EP_NS_PATCH };
static int namedClipLevel[] = { CLIP_P_VERT, CLIP_P_VERT, CLIP_P_EDGE, CLIP_P_PATCH };

#define MAX_MATID	0xffff

#define UNDEFINED	0xffffffff

class PaintPatchMod;
class CNelPatchChanger;

#define BRUSH_COUNT 3

inline int getScriptAppDataPatchMesh (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (sscanf ((const char*)ap->data, "%d", &value)==1)
		return value;
	else
		return def;
}

struct EPM_Mesh
{
	EPM_Mesh (PatchMesh *pmesh, RPatchMesh *rmesh, class PaintPatchData *patchData, INode* node, ModContext *mod, int mcListIndex, bool original)
	{
		PMesh=pmesh;
		RMesh=rmesh;
		PatchData=patchData;
		Node=node;
		Mod=mod;
		McListIndex=mcListIndex;
		Original = original;

		Rotate = getScriptAppDataPatchMesh (node, NEL3D_APPDATA_ZONE_ROTATE, 0);
		Symmetry = getScriptAppDataPatchMesh (node, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
	}
	PatchMesh	*PMesh;
	RPatchMesh	*RMesh;
	INode		*Node;
	ModContext	*Mod;
	uint		Rotate;
	bool		Symmetry;
	bool		Original;
	int			McListIndex;
	class PaintPatchData *PatchData;
};

struct EPM_PaintTile
{
	EPM_PaintTile ()
	{
		Mesh=-1;
		frozen = false;
		locked = 0;
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
	bool			intersect (const Ray& ray, std::vector<EPM_Mesh>& vectMesh, TimeValue t, NLMISC::CVector& hit, CVector& topVector);
	sint32			patch;
	sint32			tile;
	sint16			Mesh;
	uint8			u;
	uint8			v;
	bool			frozen;
	uint8			locked;
	EPM_PaintTile*	voisins[4];
	uint8			rotate[4];
	CVector			Center;
	float			Radius;
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
	friend class EPM_PaintCMode;
	friend  class MouseListener;
	friend DWORD WINAPI myThread (LPVOID vData);
	friend class CTileUndo;
	friend class CPaintColor;
	friend class CFillPatch;

private:
	PaintPatchMod *pobj;
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

	bool isLockedEx (PaintPatchMod *pobj, EPM_PaintTile* pTile, std::vector<EPM_Mesh>& vectMesh, CLandscape* land);

	inline static bool isLocked (PaintPatchMod *pobj, EPM_PaintTile* pTile, uint8 mask = 0xff);

	static bool isLocked256 (PaintPatchMod *pobj, EPM_PaintTile* pTile);

	// Get a tile
	void GetTile (int mesh, int tile, tileDesc& desc, std::vector<EPM_Mesh>& vectMesh, CLandscape* land);

	bool PutATile ( EPM_PaintTile* pTile, int tileSet, int curRotation, const NL3D::CTileBank& bank, 
								   bool selectCycle, std::set<EPM_PaintTile*>& visited, std::vector<EPM_Mesh>& vectMesh, 
								   NL3D::CLandscape* land, CNelPatchChanger& nelPatchChg, bool _256);

	bool ClearATile ( EPM_PaintTile* pTile, std::vector<EPM_Mesh>& vectMesh, NL3D::CLandscape* land, CNelPatchChanger& nelPatchChg, bool _256, bool _force128=false);

	void PutADisplacetile ( EPM_PaintTile* pTile, const CTileBank& bank, 
								   std::vector<EPM_Mesh>& vectMesh, 
								   CLandscape* land, CNelPatchChanger& nelPatchChg);

	BOOL PutDisplace (int tile, int mesh, const CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, CLandscape* land, 
								  int recurs, std::set<EPM_PaintTile*>& alreadyRecursed, CNelPatchChanger& nelPatchChg);

	int	selectTile (uint tileSet, bool selectCycle, bool _256, uint group, const CTileBank& bank);
	bool GetBorderDesc (EPM_PaintTile* tile, tileSetIndex *pVoisinCorner, NL3D::CTileSet::TFlagBorder pBorder[4][3],
						tileDesc *pVoisinIndex, const NL3D::CTileBank& bank, std::vector<EPM_Mesh>& vectMesh, CNelPatchChanger& nelPatchChg, 
						CLandscape *land);

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


	BOOL HitATile(ViewExp *vpt, IPoint2 *p, int *tile, int *mesh, TimeValue t, std::vector<EPM_Mesh>& vectMesh, NLMISC::CVector &hit, NLMISC::CVector &topVector);
	BOOL HitATile(const CViewport& viewport, const CCamera& camera, float x, float y, int *tile, int *mesh, TimeValue t, std::vector<EPM_Mesh>& vectMesh, NLMISC::CVector& hit, NLMISC::CVector &topVector);
	BOOL AnyHits( ViewExp *vpt ) 
	{ 
		return vpt->NumSubObjHits(); 
	}
	int getLayer (EPM_PaintTile* tile, int border, int tileSet, int rotate, std::vector<EPM_Mesh>& vectMesh, NL3D::CLandscape *land);

public:
	EPM_PaintMouseProc(PaintPatchMod* spl, IObjParam *i) 
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

/*-------------------------------------------------------------------*/

class EPM_PaintCMode : public CommandMode
{
	private:
		ChangeFGObject fgProc;
		EPM_PaintMouseProc eproc;
		PaintPatchMod* pobj;

	public:
		EPM_PaintCMode(PaintPatchMod* spl, IObjParam *i) :
			fgProc((ReferenceTarget*)spl), eproc(spl,i) 
		{
			pobj=spl;
		}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EP_PAINT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
		void DoPaint ();
//		void SetType(int type) { this->type = type; eproc.SetType(type); }
};

/*-------------------------------------------------------------------*/

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

class PaintPatchMod : public Modifier
{
	friend class EPTempData;
	friend class PaintPatchData;
	friend class PatchRestore;

	public:
		static HWND hOpsPanel;
		static IObjParam *ip;		
		static BOOL rsOps;	// rollup states (FALSE = rolled up)
		
		static EPM_PaintCMode *paintMode;
		static bool		ShowCurrentState;
		static uint		CurrentState;
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
		static bool		lockBorders;

		RefResult NotifyRefChanged(NOTIFY_REF_PARAMS) { return REF_SUCCEED; }

		bool includeMeshes;
		bool preloadTiles;
		
		// Remembered info
		PatchMesh *rememberedPatch;	// NULL if using all selected patches
		int rememberedIndex;
		int rememberedData;

		PaintPatchMod();
		~PaintPatchMod();

		Interval LocalValidity(TimeValue t);
		ChannelMask ChannelsUsed()  { return EDITPAT_CHANNELS; }
		ChannelMask ChannelsChanged() 	{ return channelModified; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);
		Class_ID InputType() { return RYKOLPATCHOBJ_CLASS_ID; }
		
		int CompMatrix(TimeValue t, ModContext& mc, Matrix3& tm, Interval& valid);

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(_T("NeLPatchPaintMod")); }
		Class_ID ClassID() { return Class_ID(0xc49560f, 0x3c3d68e7); }
		void* GetInterface(ULONG id);

		// From BaseObject
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);

 		BOOL DependOnTopology(ModContext &mc);

		BOOL SupportsNamedSubSels() {return FALSE;}

		void ClearPatchDataFlag(ModContextList& mcList,DWORD f);
		void DeletePatchDataTempData();		
		void CreatePatchDataTempData();

		int NumRefs() { return 0; }
		RefTargetHandle GetReference(int i) { return NULL; }
		void SetReference(int i, RefTargetHandle rtarg) {}

		void ChangeRememberedPatch(int type);
		int RememberPatchThere(HWND hWnd, IPoint2 m);
		void SetRememberedPatchType(int type);

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
		GET_OBJECT_NAME_CONST MCHAR *GetObjectName() { return _M("NeL Patch Painter"); }
		
		void RescaleWorldUnits(float f);

		DWORD GetSelLevel();
		void SetSelLevel(DWORD level);
		void LocalDataChanged();

		void	SetOpsDlgEnables();
};

class EditPatchClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE ) 
	{ 
		return new PaintPatchMod; 
	}
	const MCHAR *	ClassName() { return _M("NeL Painter"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0xc49560f, 0x3c3d68e7); }
	const MCHAR* 	Category() { return _M("NeL Tools");}
	void			ResetClassParams(BOOL fileReset);
	};

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

#define PATCHCHANGERECORD_CHUNK		0x2070
										 
class PatchChangeRecord : public PModRecord {
	public:
		PatchMesh oldPatch;		// How the patch mesh looked before the change
		RPatchMesh roldPatch;		// How the patch looked before the delete
		int index;
		int type;
		BOOL Redo(PatchMesh *patch,RPatchMesh *rpatch,int reRecord);
		IOResult Load(ILoad *iload);
	};

typedef Tab<MtlID> MtlIDTab;

/*-------------------------------------------------------------------*/

// PaintPatchData flags
#define EPD_BEENDONE			(1<<0)
#define EPD_UPDATING_CACHE		(1<<1)
#define EPD_HASDATA				(1<<2)
#define EMD_HELD				(1<<3) // equivalent to A_HELD

// This is the data that each mod app will have.
class PaintPatchData : public LocalModData {
	public:
		BOOL handleFlag;
		int handleVert;

		// Stuff we need to have for the patch's mesh conversion -- These are
		// Here because they're kind of a global change -- not undoable.

		bool includeMeshes;
		bool preloadTiles;

		DWORD flags;

		// This records the changes to the incoming object.
		ModRecordTab changes;

		// While an object is being edited, this exists.
		EPTempData *tempData;

		// The final edited patch
		PatchMesh finalPatch;
		RPatchMesh rfinalPatch;

		PaintPatchData(PaintPatchMod *mod);
		PaintPatchData(PaintPatchData& emc);

		// Applies modifications to a patchObject
		void Apply(TimeValue t,RPO *patchOb,int selLevel);

		// Invalidates any caches affected by the change.
		void Invalidate(PartID part,BOOL meshValid=TRUE);

		// If this is the first edit, then the delta arrays will be allocated
		void BeginEdit(TimeValue t);

		LocalModData *Clone() { return new PaintPatchData(*this); }

		void SetFlag(DWORD f,BOOL on) 
		{ 
			if ( on )
			{
				flags|=f;
			}
			else
			{
				flags&=~f; 
			}
		}
		DWORD GetFlag(DWORD f) { return flags&f; }

		EPTempData *TempData(PaintPatchMod *mod);

		// Change recording functions:
		void ClearHandleFlag() { handleFlag = FALSE; }
		void SetHandleFlag(int vert) { handleVert = vert; handleFlag = TRUE; }
		BOOL DoingHandles() { return handleFlag; }
		//void ApplyHandlesAndZero(PatchMesh &patch) { vdelta.ApplyHandlesAndZero(patch, handleVert); }
		void RescaleWorldUnits(float f);

		// MAXr3: New recording system
		void RecordTopologyTags(PatchMesh *patch);
		void UpdateChanges(PatchMesh *patch, RPatchMesh *rpatch, BOOL checkTopology=TRUE);

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
		PaintPatchData *epd;
		PaintPatchMod *mod;
		TSTR where;
		
		PatchRestore (PaintPatchData* pd, PaintPatchMod* mod, PatchMesh *patch, RPatchMesh *rpatch, TCHAR *id=_T(""));
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

/*-------------------------------------------------------------------*/

class EPTempData 
{
	private:
		PatchMesh		*patch;
		RPatchMesh		*rpatch;
		Interval 		patchValid;
		
		PaintPatchMod 	*mod;
		PaintPatchData 	*patchData;

	public:		
		
		~EPTempData();
		EPTempData(PaintPatchMod *m,PaintPatchData *md);
		void Invalidate(PartID part,BOOL meshValid=TRUE);
		
		PatchMesh		*GetPatch(TimeValue t, RPatchMesh *&rPatch);

		BOOL PatchCached(TimeValue t);
		void UpdateCache(RPO *patchOb);
		PaintPatchMod	*GetMod() { return mod; }
};

inline bool EPM_PaintMouseProc::isLocked (PaintPatchMod *pobj, EPM_PaintTile* pTile, uint8 mask)
{
	// Lock border ?
	if (pobj->lockBorders)
	{
		// Return the lock state
		return (pTile->locked & mask) != 0;
	}

	// Not locked
	return false;
}


#endif // __EDITPATCH_H__
