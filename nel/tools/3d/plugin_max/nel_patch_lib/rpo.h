// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef __RYKOL_PATCH_OBJ_H
#define __RYKOL_PATCH_OBJ_H

#pragma warning (disable : 4786)
#include "nel/misc/debug.h"
#include <tchar.h>
#include <vector>
#include <set>
#include <string>
#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"
#include "nel_patch_mesh.h"

//#define USE_CACHE

typedef unsigned int uint;

#define RYKOLPATCHOBJ_CLASS_ID	Class_ID(0x368c679f, 0x711c22ee)

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define RPO_SERIALIZE_VERSION 0

#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_TILE		4

#define PO_TILE		4

#define PATCH_HIT_TILE (PATCH_HIT_INTERIOR+1)

#define MAX_TILE_IN_PATCH 16
#define NUM_TILE_SEL (MAX_TILE_IN_PATCH*MAX_TILE_IN_PATCH)

#pragma warning (disable : 4786)

// ------------------------------------------------------------------------------------------------------------------------------------------------

class RPO : public PatchObject
{
	public:
//		PatchObject			PO;
		RPatchMesh			*rpatch;			// User info for this RykolPatchObject

		// bug hack
		bool bBigHack;

		// Validity
		Interval topoValid;
		Interval geomValid;
		Interval selectValid;
		Interval texmapValid;
		/*Interval vcolorValid;
		Interval gfxdataValid;*/
		DWORD validBits; // for the remaining constant channels

		//Class vars
		static IObjParam *ip;			//Access to the interface
		BOOL suspendSnap;				//A flag for setting snapping on/off
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		//TODO: Return the name that will appear in the history browser (modifier stack)
		GET_OBJECT_NAME_CONST MCHAR *GetObjectName() { return _M("Rykol Patch Object");}
		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		//TODO: Return the default name of the node when it is created.
		void InitNodeName(TSTR& s) { s = _M("Rykol Patch Object"); }
		
		// From Object
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		void PointsWereChanged();
		
		//TODO: Evaluate the object and return the ObjectState
		ObjectState Eval(TimeValue t) 
		{
			return ObjectState(this); 
		};		
		
		//TODO: Return the validity interval of the object as a whole
		Interval ObjectValidity(TimeValue t) 
		{ 
			Interval iv;
			iv.SetInfinite();
			iv &= geomValid;
			//iv &= vcolorValid;
			iv &= topoValid;
			iv &= texmapValid;
			iv &= selectValid;
			//iv &= gfxdataValid;
			
			if (!(validBits&chMask[SUBSEL_TYPE_CHAN_NUM])) iv.SetEmpty();
			if (!(validBits&chMask[DISP_ATTRIB_CHAN_NUM])) iv.SetEmpty();
			return iv;
		}
		
		Interval ChannelValidity(TimeValue t, int nchan)
		{
			switch(nchan) 
			{
				case GEOM_CHAN_NUM: return geomValid; break;
				case TOPO_CHAN_NUM: return topoValid; break;
				case TEXMAP_CHAN_NUM: return texmapValid; break;
				case SELECT_CHAN_NUM: return selectValid; break;
				default:
					return((chMask[nchan]&validBits) ? FOREVER: NEVER);
			}
		}
		
		void SetChannelValidity(int nchan, Interval v)
		{
			switch(nchan) 
			{
				case GEOM_CHAN_NUM: geomValid = v; break;
				//case VERT_COLOR_CHAN_NUM: vcolorValid = v; break;
				case TOPO_CHAN_NUM: topoValid = v; break;
				case TEXMAP_CHAN_NUM: texmapValid = v; break;
				case SELECT_CHAN_NUM: selectValid = v; break;
				//case GFX_DATA_CHAN_NUM: gfxdataValid = v; break;
				default :
					//if (v.InInterval(0)) validBits|= chMask[nchan];
					if (!(v==NEVER)) validBits|= chMask[nchan];
					else validBits &= ~chMask[nchan];
					break;
			}	
		}
		
		Interval ConvertValidity(TimeValue t)
		{
			Interval iv = FOREVER;	
			if (geomValid.InInterval(t)) iv &= geomValid;
			//if (vcolorValid.InInterval(t)) iv &= vcolorValid;
			if (topoValid.InInterval(t)) iv &= topoValid;	
			if (texmapValid.InInterval(t)) iv &= texmapValid;	
			if (selectValid.InInterval(t)) iv &= selectValid;	
			return iv;
		}

#define copyFlags(dest, source, mask) dest =  ((dest&(~mask))|(source&mask))
		
		void CopyValidity(RPO *fromOb, ChannelMask channels) 
		{
			if (channels&GEOM_CHANNEL) geomValid = fromOb->geomValid;
			//if (channels&VERTCOLOR_CHANNEL) vcolorValid = fromOb->vcolorValid;
			if (channels&TOPO_CHANNEL) topoValid = fromOb->topoValid;
			if (channels&TEXMAP_CHANNEL) texmapValid = fromOb->texmapValid;
			if (channels&SELECT_CHANNEL) selectValid = fromOb->selectValid;
			//if (channels&GFX_DATA_CHANNEL) gfxdataValid = fromOb->gfxdataValid;
			copyFlags(validBits, fromOb->validBits,channels);
		}

		void InvalidateChannels(ChannelMask channels);

		// From Animatable
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() 
		{
			if (bBigHack)
				return Class_ID(PATCHOBJ_CLASS_ID,0);
			else
				return RYKOLPATCHOBJ_CLASS_ID;
		}
		BOOL IsSubClassOf(Class_ID classID) 
		{
			return classID == ClassID() 
				? true : PatchObject::IsSubClassOf(classID);
		}
		SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
		void GetClassName(TSTR& s) { s = _T("Rykol Patch Object");}
		
		RefTargetHandle Clone ( RemapDir &remap );
		RefResult NotifyRefChanged (NOTIFY_REF_PARAMS);

		int NumSubs() 
		{ 
			return PatchObject::NumSubs ();
		}
		//TSTR SubAnimName(int i) { return NULL; }
		Animatable* SubAnim(int i) 
		{ 
			return PatchObject::SubAnim(i);
			// return pblock; 
		}
		int NumRefs() 
		{ 
			return PatchObject::NumRefs();
		}
		RefTargetHandle GetReference(int i) 
		{ 
			return PatchObject::GetReference(i);
			//return pblock; 
		}
		void SetReference(int i, RefTargetHandle rtarg) 
		{ 
			PatchObject::SetReference(i, rtarg);
			//pblock=(IParamBlock2*)rtarg; 
		}

		/*int	NumParamBlocks() 
		{ 
			//return PatchObject::NumParamBlocks();
			return 1; 
		}					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) 
		{ 
			//return PatchObject::GetParamBlock(i);
			return pblock; 
		} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) 
		{ 
			return PatchObject::GetParamBlockByID(id);
			//return (pblock->ID() == id) ? pblock : NULL; 
		} // return id'd ParamBlock*/
		void DeleteThis() { delete this; }
		Object *MakeShallowCopy(ChannelMask channels);
		void ShallowCopy(Object* fromOb, ChannelMask channels);
		void NewAndCopyChannels(ChannelMask channels);
		void FreeChannels(ChannelMask channels);


		//Constructor/Destructor
		RPO();
		RPO(PatchObject& pPO);
		virtual ~RPO();

		void SetPoint(int i, const Point3& p)
		{
			PatchObject::SetPoint(i, p);
		}

		/**
		  * Return true if it is a zone.
		  *
		  * skeletonShape must be NULL if no bones.
		  */
		static bool						isZone (INode& node, TimeValue time);

#if MAX_RELEASE >= 4000
		// ace: These functions are needed in max 4
		// NS: New SubObjType API
		int NumSubObjTypes() { return 0; }
		ISubObjType *GetSubObjType(int i) { return NULL; }
#endif

};

#endif // __RYKOL_PATCH_OBJ_H
