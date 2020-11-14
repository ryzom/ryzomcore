// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "std_afx.h"
#include "nel_export.h"
#include "../nel_mesh_lib/export_appdata.h"
#include "../nel_mesh_lib/calc_lm.h"
#include "../nel_patch_lib/nel_patch_mesh.h"

using namespace NLMISC;

#define DIFFERENT_VALUE_STRING "<different values>"

// ***************************************************************************

#define TAB_COUNT	9
#define VP_COUNT	1
// Which dialog tab is the VerytexProgram one?
#define TAB_VP_ID	5

TCHAR *_EnvironmentNames[] =
{
	_T(""),
	_T("no fx"),
	_T("GENERIC"),
	_T("PADDEDCELL"),
	_T("ROOM"),
	_T("BATHROOM"),
	_T("LIVINGROOM"),
	_T("STONEROOM"),
	_T("AUDITORIUM"),
	_T("CONCERTHALL"),
	_T("CAVE"),
	_T("ARENA"),
	_T("HANGAR"),
	_T("CARPETEDHALLWAY"),
	_T("HALLWAY"),
	_T("STONECORRIDOR"),
	_T("ALLEY"),
	_T("FOREST"),
	_T("CITY"),
	_T("MOUNTAINS"),
	_T("QUARRY"),
	_T("PLAIN"),
	_T("PARKINGLOT"),
	_T("SEWERPIPE"),
	_T("UNDERWATER"),
	_T("DRUGGED"),
	_T("DIZZY"),
	_T("PSYCHOTIC"),
	NULL
};
TCHAR *_MaterialNames[] =
{
	_T(""),
	_T("no occlusion"),
	_T("SINGLEWINDOW"),
	_T("DOUBLEWINDOW"),
	_T("THINDOOR"),
	_T("THICKDOOR"),
	_T("WOODWALL"),
	_T("BRICKWALL"),
	_T("STONEWALL"),
	_T("CURTAIN"),
	NULL
};

std::set<std::string>	_KnownSoundGroups;
// ***************************************************************************

const std::set<INode*> *listNodeCallBack;

// ***************************************************************************

class addSubLodNodeHitCallBack : public HitByNameDlgCallback
{
public:
	INodeTab	NodeTab;
private:
	virtual GET_OBJECT_NAME_CONST MCHAR *dialogTitle()
	{
		return _M("Select sub lod objects to add");
	}
	virtual GET_OBJECT_NAME_CONST MCHAR *buttonText()
	{
		return _M("Add");
	}
	virtual BOOL singleSelect()
	{
		return FALSE;
	}
	virtual BOOL useFilter()
	{
		return TRUE;
	}
	virtual int filter(INode *node)
	{
		return (int)(listNodeCallBack->find (node)==listNodeCallBack->end());
	}
	virtual BOOL useProc()
	{
		return TRUE;
	}
	virtual void proc(INodeTab &nodeTab)
	{
		NodeTab=nodeTab;
	}
	virtual BOOL doCustomHilite()
	{
		return FALSE;
	}
	virtual BOOL showHiddenAndFrozen()
	{
		return TRUE;
	}
};

// ***************************************************************************
// An RGBA property which can represent <different values> due to multiple selection
class CRGBAProp : public CRGBA
{
public:
	// Flag which tells or not if represent different values
	bool			DifferentValues;
	// The color ctrl
	IColorSwatch	*Ctrl;

	CRGBAProp() : CRGBA(CRGBA::Black)
	{
		DifferentValues= false;
		Ctrl= NULL;
	}
	CRGBAProp(CRGBA col) : CRGBA(col)
	{
		DifferentValues= false;
		Ctrl= NULL;
	}

	CRGBAProp &operator=(CRGBA col)
	{
		((CRGBA*)this)->operator= (col);
		return *this;
	}

	void setDifferentValuesMode()
	{
		DifferentValues= true;
		// reconizable color
		set(255,0,128,255);
	}
};

// ***************************************************************************
class CLodDialogBoxParam
{
public:
	CLodDialogBoxParam ()
	{
		uint i;
		for (i=0; i<TAB_COUNT; i++)
			SubDlg[i] = NULL;
		for (i=0; i<VP_COUNT; i++)
			SubVPDlg[i] = NULL;
		InterfaceThreshold = 0.1f;
		GetInterfaceNormalsFromSceneObjects = 0;
		LMCEnabled= 0;
	}

	// Lod.
	bool					ListActived;
	std::list<std::string>	ListLodName;
	int						BlendIn;
	int						BlendOut;
	int						CoarseMesh;
	int						DynamicMesh;
	std::string				DistMax;
	std::string				BlendLength;
	int						MRM;
	int						SkinReduction;
	std::string				NbLod;
	std::string				Divisor;
	std::string				DistanceFinest;
	std::string				DistanceMiddle;
	std::string				DistanceCoarsest;
	int						ExportLodCharacter;

	const std::set<INode*> *ListNode;

	// Bone Lod.
	std::string				BoneLodDistance;

	// Accelerator
	int						AcceleratorType; // -1->undeterminate   0->Not  1->Portal  2->Cluster
	int						ParentVisible;
	int						VisibleFromParent;
	int						DynamicPortal;
	int						Clusterized;
	int						AudibleLikeVisible;
	int						FatherAudible;
	int						AudibleFromFather;

	std::string				OcclusionModel;
	std::string				OpenOcclusionModel;
	std::string				SoundGroup;
	std::string				EnvironmentFX;

	// Instance Group
	std::string				InstanceShape;
	std::string				InstanceName;
	std::string				InstanceGroupName;
	std::string				InterfaceFileName;
	int					    GetInterfaceNormalsFromSceneObjects;
	float					InterfaceThreshold;
	int						DontAddToScene;
	int						DontExport;

	// Lighting
	std::string				LumelSizeMul;
	std::string				SoftShadowRadius;
	std::string				SoftShadowConeLength;
	sint					LightGroup;

	// VertexProgram.
	int						VertexProgramId;
	bool					VertexProgramBypassed;
	// WindTree VertexProgram.
	CVPWindTreeAppData		VertexProgramWindTree;


	// Lighting
	int						ExportRealTimeLight;
	int						ExportLightMapLight;
	int						ExportAsSunLight;
	int						UseLightingLocalAttenuation;
	int						ExportLightMapAnimated;
	std::string				ExportLightMapName;
	int						LightDontCastShadowInterior;
	int						LightDontCastShadowExterior;
	int						RealTimeAmbientLightAddSun;

	// Lighting 2
	enum	{NumLightGroup= 3};
	// Compress this lightmapped object in 8 bit lightmap?
	int						LMCEnabled;
	// Compression term for 8Bits LightMap Compression
	CRGBAProp				LMCAmbient[NumLightGroup];
	CRGBAProp				LMCDiffuse[NumLightGroup];


	// Misc
	int						FloatingObject;
	int						SWT;
	std::string				SWTWeight;
	int						LigoSymmetry;
	std::string				LigoRotate;
	int						UseRemanence;
	int						RemanenceShiftingTexture;
	int						RemanenceSliceNumber;
	float					RemanenceSamplingPeriod;
	float                   RemanenceRollupRatio;
	int						CollisionMeshGeneration; // -1->undeterminate   0->Auto 1->NoCameraCol 2->ForceCameraCol 3->ForceCameraColAndHideInIG

	// Animation
	int						ExportNodeAnimation;
	int						PrefixeTracksNodeName;
	int						ExportNoteTrack;
	int						ExportSSSTrack;
	int						ExportAnimatedMaterials;
	int						AutomaticAnimation;

	// Vegetable
	int						Vegetable;
	int						VegetableAlphaBlend;
	int						VegetableAlphaBlendOnLighted;
	int						VegetableAlphaBlendOffLighted;
	int						VegetableAlphaBlendOffDoubleSided;
	int						VegetableForceBestSidedLighting;
	int						VegetableBendCenter;
	std::string				VegetableBendFactor;

	// Collision
	int						Collision;
	int						CollisionExterior;

	// Radial normals
	std::string				RadialNormals[NEL3D_RADIAL_NORMAL_COUNT];

	// Skeleton Scale
	int						ExportBoneScale;
	std::string				ExportBoneScaleNameExt;

	// Dialog
	HWND					SubDlg[TAB_COUNT];

	// Dialog
	HWND					SubVPDlg[VP_COUNT];
};

INT_PTR CALLBACK MRMDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AccelDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InstanceDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LightmapDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Lightmap2DialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VegetableDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VertexProgramDialogCallBack (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MiscDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AnimationDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

const TCHAR				*SubText[TAB_COUNT]	= { _T("LOD & MRM"), _T("Accelerator"), _T("Instance"), _T("Lighting"), _T("LMC"), _T("Vegetable"), _T("VertexProgram"), _T("Misc"), _T("Animation")};
const int				SubTab[TAB_COUNT]	= {IDD_LOD, IDD_ACCEL, IDD_INSTANCE, IDD_LIGHTMAP, IDD_LIGHTMAP2, IDD_VEGETABLE, IDD_VERTEX_PROGRAM, IDD_MISC, IDD_ANIM};
DLGPROC					SubProc[TAB_COUNT]	= {MRMDialogCallback, AccelDialogCallback, InstanceDialogCallback, LightmapDialogCallback, Lightmap2DialogCallback, VegetableDialogCallback, VertexProgramDialogCallBack, MiscDialogCallback, AnimationDialogCallback};

// VertexPrograms.
INT_PTR CALLBACK VPWindTreeCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
const int				SubVPTab[VP_COUNT]	= {IDD_VP_WINDTREE};
DLGPROC					SubVPProc[VP_COUNT]	= {VPWindTreeCallback};


// The last opened TAB.
static int				LastTabActivated= 0;


// ***************************************************************************

void MRMStateChanged (HWND hwndDlg)
{
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0)!=BST_UNCHECKED ) &&
		( SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED );
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MIN), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MAX), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_BEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_NB_LOD), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIVISOR), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_FINEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), enable);
}

// ***************************************************************************

void LightingStateChanged (HWND hwndDlg, CLodDialogBoxParam *currentParam)
{
	bool lightmapLight = (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	lightmapLight |= (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	lightmapLight |= (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), lightmapLight);
	EnableWindow(GetDlgItem (hwndDlg, IDC_USE_LIGHT_LOCAL_ATTENUATION), currentParam->VertexProgramBypassed ? FALSE : TRUE);

	// Enable AmbientAddSun for realTime Ambient only
	EnableWindow(GetDlgItem (hwndDlg, IDC_REALTIME_LIGHT_AMBIENT_ADD_SUN),
		SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
}

// ***************************************************************************

void CoarseStateChanged (HWND hwndDlg)
{
	// Like if MRM button was clicked
	MRMStateChanged (hwndDlg);

	// Bouton enabled ?
	bool enable = SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED;
	EnableWindow (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), enable);
}

// ***************************************************************************

void VegetableStateChanged (HWND hwndDlg)
{
	// Vegetable ?
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0)!=BST_UNCHECKED );

	// Enable alpha blend button
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_Z), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_NULL), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), enable);

	// Alpha blend ?
	bool alphaBlend = IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON)!=BST_UNCHECKED;

	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), enable && !alphaBlend);


	// Precomputed Liggting?
	bool	precomputed;
	if(alphaBlend)
		precomputed= IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED)!=BST_UNCHECKED;
	else
		precomputed= IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED)!=BST_UNCHECKED;

	// Force Best Sided Lighting only if precomputed
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_FORCE_BEST_SIDED_LIGHTING), enable && precomputed);

}

// ***************************************************************************

void AccelStateChanged (HWND hwndDlg)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	bool cluster = (currentParam->AcceleratorType&NEL3D_APPDATA_ACCEL_TYPE) == NEL3D_APPDATA_ACCEL_CLUSTER;
	bool portal = (currentParam->AcceleratorType&NEL3D_APPDATA_ACCEL_TYPE) == NEL3D_APPDATA_ACCEL_PORTAL;
	bool defined = currentParam->AcceleratorType != -1;
	bool accelerator = portal || cluster;
	bool dynamic_portal = IsDlgButtonChecked(hwndDlg, IDC_DYNAMIC_PORTAL) != BST_UNCHECKED;
	bool audibleLikeVisible = IsDlgButtonChecked(hwndDlg, IDC_AUDIBLE_LIKE_VISIBLE) == BST_CHECKED;

	EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), (accelerator && cluster) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), (accelerator && cluster) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SOUND_GROUP), (accelerator && cluster) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_ENV_FX), (accelerator && cluster) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), (accelerator && portal) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_OCC_MODEL), (accelerator && portal) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_OPEN_OCC_MODEL), (accelerator && portal && dynamic_portal) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), (!accelerator) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_AUDIBLE_LIKE_VISIBLE), (accelerator && cluster) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_AUDIBLE), (accelerator && cluster && !audibleLikeVisible) || !defined);
	EnableWindow (GetDlgItem (hwndDlg, IDC_AUDIBLE_FROM_FATHER), (accelerator && cluster && !audibleLikeVisible) || !defined);
}


// ***************************************************************************
void InstanceStateChanged (HWND hwndDlg)
{
	bool	colEnable= ( SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_GETCHECK, 0, 0)!=BST_UNCHECKED );

	EnableWindow (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), colEnable);
}

// ***************************************************************************

void exploreNode(INode *node)
{
	std::string soundGroup = CExportNel::getScriptAppData(node, NEL3D_APPDATA_SOUND_GROUP, "no sound");

	if (soundGroup != "no sound")
		_KnownSoundGroups.insert(soundGroup);

	for (int i= 0; i<node->NumChildren(); ++i)
	{
		exploreNode(node->GetChildNode(i));
	}
}


INT_PTR CALLBACK AccelDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// Fill the known sound groups by parsing the max node tree.
			{
				INode *root = theCNelExport._Ip->GetRootNode();

				for (int i= 0; i<root->NumChildren(); ++i)
				{
					exploreNode(root->GetChildNode(i));
				}
			}
			_KnownSoundGroups.insert(currentParam->SoundGroup);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			// Check accelerator check box
			SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_SETCHECK, currentParam->ParentVisible, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_SETCHECK, currentParam->VisibleFromParent, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_SETCHECK, currentParam->DynamicPortal, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_SETCHECK, currentParam->Clusterized, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_AUDIBLE_LIKE_VISIBLE), BM_SETCHECK, currentParam->AudibleLikeVisible, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_AUDIBLE), BM_SETCHECK, currentParam->FatherAudible, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_AUDIBLE_FROM_FATHER), BM_SETCHECK, currentParam->AudibleFromFather, 0);

			// fill the combo box
			{
				for (uint i=0; _MaterialNames[i] != 0; ++i)
				{
					SendMessage (GetDlgItem (hwndDlg, IDC_OCC_MODEL), CB_ADDSTRING, 0, (LPARAM)_MaterialNames[i]);
					SendMessage (GetDlgItem (hwndDlg, IDC_OPEN_OCC_MODEL), CB_ADDSTRING, 0, (LPARAM)_MaterialNames[i]);
				}
			}
			{
				for (uint i =0; _EnvironmentNames[i] != 0; ++i)
				{
					SendMessage (GetDlgItem (hwndDlg, IDC_ENV_FX), CB_ADDSTRING, 0, (LPARAM)_EnvironmentNames[i]);
				}
			}
			{
				std::set<std::string>::iterator first(_KnownSoundGroups.begin()), last(_KnownSoundGroups.end());
				for (; first != last; ++first)
				{
					SendMessage (GetDlgItem (hwndDlg, IDC_SOUND_GROUP), CB_ADDSTRING, 0, (LPARAM)MaxTStrFromUtf8(*first).data());
				}
			}
			// set the combo and edit box
			if (SendMessage (GetDlgItem (hwndDlg, IDC_OCC_MODEL), CB_SELECTSTRING, -1, (LPARAM)MaxTStrFromUtf8(currentParam->OcclusionModel).data()) == CB_ERR)
			{
//				nlassert(false);
			}
			if (SendMessage (GetDlgItem (hwndDlg, IDC_OPEN_OCC_MODEL), CB_SELECTSTRING, -1, (LPARAM)MaxTStrFromUtf8(currentParam->OpenOcclusionModel).data()) == CB_ERR)
			{
//				nlassert(false);
			}
			if (SendMessage (GetDlgItem (hwndDlg, IDC_ENV_FX), CB_SELECTSTRING, -1, (LPARAM)MaxTStrFromUtf8(currentParam->EnvironmentFX).data()) == CB_ERR)
			{
//				nlassert(false);
			}
			if (SendMessage (GetDlgItem (hwndDlg, IDC_SOUND_GROUP), CB_SELECTSTRING, -1, (LPARAM)MaxTStrFromUtf8(currentParam->SoundGroup).data()) == CB_ERR)
			{
//				nlassert(false);
			}
//			SendMessage(GetDlgItem(hwndDlg, IDC_SOUND_GROUP), WM_SETTEXT, 0, (LPARAM)MaxTStrFromUtf8(currentParam->SoundGroup).data());

			bool accelerator = (currentParam->AcceleratorType != -1);
			CheckRadioButton (hwndDlg, IDC_RADIOACCELNO, IDC_RADIOACCELCLUSTER, accelerator?(IDC_RADIOACCELNO+(currentParam->AcceleratorType&NEL3D_APPDATA_ACCEL_TYPE)):0);
			AccelStateChanged (hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Get the acceleration type
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
								currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_NOT_ACCELERATOR;
							else if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
								currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_PORTAL;
							else if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
								currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_CLUSTER;
							else
								currentParam->AcceleratorType = -1;

							// Get the flags
							currentParam->ParentVisible=SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_GETCHECK, 0, 0);
							currentParam->VisibleFromParent=SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_GETCHECK, 0, 0);
							currentParam->DynamicPortal=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_GETCHECK, 0, 0);
							currentParam->Clusterized=SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_GETCHECK, 0, 0);
							currentParam->AudibleLikeVisible=SendMessage (GetDlgItem (hwndDlg, IDC_AUDIBLE_LIKE_VISIBLE), BM_GETCHECK, 0, 0);
							currentParam->FatherAudible=SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_AUDIBLE), BM_GETCHECK, 0, 0);
							currentParam->AudibleFromFather=SendMessage (GetDlgItem (hwndDlg, IDC_AUDIBLE_FROM_FATHER), BM_GETCHECK, 0, 0);

							// get the strings params
							TCHAR tmp[256];
							SendMessage (GetDlgItem(hwndDlg, IDC_OCC_MODEL), WM_GETTEXT, 256, (LPARAM)tmp);
							currentParam->OcclusionModel = MCharStrToUtf8(tmp);
							SendMessage (GetDlgItem(hwndDlg, IDC_OPEN_OCC_MODEL), WM_GETTEXT, 256, (LPARAM)tmp);
							currentParam->OpenOcclusionModel = MCharStrToUtf8(tmp);
							SendMessage (GetDlgItem(hwndDlg, IDC_SOUND_GROUP), WM_GETTEXT, 256, (LPARAM)tmp);
							currentParam->SoundGroup = MCharStrToUtf8(tmp);
							_KnownSoundGroups.insert(currentParam->SoundGroup);
							SendMessage (GetDlgItem(hwndDlg, IDC_ENV_FX), WM_GETTEXT, 256, (LPARAM)tmp);
							currentParam->EnvironmentFX = MCharStrToUtf8(tmp);

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_RADIOACCELNO:
					case IDC_RADIOACCELPORTAL:
					case IDC_RADIOACCELCLUSTER:
						// Get the acceleration type
						if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
							currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_NOT_ACCELERATOR;
						else if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
							currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_PORTAL;
						else if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
							currentParam->AcceleratorType = NEL3D_APPDATA_ACCEL_CLUSTER;
						else
							currentParam->AcceleratorType = -1;
						AccelStateChanged (hwndDlg);
						break;
					case IDC_DYNAMIC_PORTAL:
					case IDC_FATHER_VISIBLE:
					case IDC_VISIBLE_FROM_FATHER:
					case IDC_CLUSTERIZE:
					case IDC_AUDIBLE_LIKE_VISIBLE:
					case IDC_AUDIBLE_FROM_FATHER:
					case IDC_FATHER_AUDIBLE:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							AccelStateChanged (hwndDlg);
						}
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************
INT_PTR CALLBACK MRMDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// Window text
			std::string winName= MCharStrToUtf8((*(currentParam->ListNode->begin()))->GetName());
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, MaxTStrFromUtf8(winName).data());

			// Set default state
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_SETCHECK, currentParam->BlendIn, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_SETCHECK, currentParam->BlendOut, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_SETCHECK, currentParam->CoarseMesh, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_SETCHECK, currentParam->DynamicMesh, 0);

			EnableWindow (GetDlgItem (hwndDlg, IDC_LIST1), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_ADD), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_REMOVE), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_UP), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_DOWN), currentParam->ListActived);

			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), MaxTStrFromUtf8(currentParam->DistMax).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), MaxTStrFromUtf8(currentParam->BlendLength).data());

			SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_SETCHECK, currentParam->MRM, 0);
			CoarseStateChanged (hwndDlg);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			SetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), MaxTStrFromUtf8(currentParam->NbLod).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), MaxTStrFromUtf8(currentParam->Divisor).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), MaxTStrFromUtf8(currentParam->DistanceFinest).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), MaxTStrFromUtf8(currentParam->DistanceMiddle).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), MaxTStrFromUtf8(currentParam->DistanceCoarsest).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BONE_LOD_DISTANCE), MaxTStrFromUtf8(currentParam->BoneLodDistance).data());

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}

			// default LodCharacter
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_CLOD), BM_SETCHECK, currentParam->ExportLodCharacter, 0);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							currentParam->BlendIn=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_GETCHECK, 0, 0);
							currentParam->BlendOut=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_GETCHECK, 0, 0);
							currentParam->CoarseMesh=SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0);
							currentParam->DynamicMesh=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_GETCHECK, 0, 0);

							TCHAR tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), tmp, 512);
							currentParam->DistMax = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), tmp, 512);
							currentParam->BlendLength = MCharStrToUtf8(tmp);

							currentParam->MRM=SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0);

							currentParam->SkinReduction=-1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MIN)==BST_CHECKED)
								currentParam->SkinReduction=0;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MAX)==BST_CHECKED)
								currentParam->SkinReduction=1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_BEST)==BST_CHECKED)
								currentParam->SkinReduction=2;

							GetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), tmp, 512);
							currentParam->NbLod = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), tmp, 512);
							currentParam->Divisor = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), tmp, 512);
							currentParam->DistanceFinest = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), tmp, 512);
							currentParam->DistanceMiddle = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), tmp, 512);
							currentParam->DistanceCoarsest = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_BONE_LOD_DISTANCE), tmp, 512);
							currentParam->BoneLodDistance = MCharStrToUtf8(tmp);

							// Iterate list
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
							currentParam->ListLodName.clear ();

							// Insert item in the list
							uint itemCount=SendMessage (hwndList, LB_GETCOUNT, 0, 0);
							for (uint item=0; item<itemCount; item++)
							{
								// Get the string
								SendMessage (hwndList, LB_GETTEXT, item, (LPARAM) tmp);

								// Push it back
								currentParam->ListLodName.push_back (MCharStrToUtf8(tmp));
							}

							// default LodCharacter
							currentParam->ExportLodCharacter= SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_CLOD), BM_GETCHECK, 0, 0);
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
							listNodeCallBack=currentParam->ListNode;
							if (theCNelExport._Ip->DoHitByNameDialog(&callBack))
							{
								// Add the selected object in the list
								HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
								for (uint i=0; i<(uint)callBack.NodeTab.Count(); i++)
									SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) callBack.NodeTab[i]->GetName());
							}
						}
						break;
					case IDC_REMOVE:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if (sel!=LB_ERR)
							{
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								if (sel==SendMessage (hwndList, LB_GETCOUNT, 0, 0))
									sel--;
								if (sel>=0)
									SendMessage (hwndList, LB_SETCURSEL, sel, 0);
							}
						}
					break;
					case IDC_UP:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ((sel!=LB_ERR)&&(sel>0))
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move up the item
								SendMessage (hwndList, LB_INSERTSTRING, sel-1, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel+1, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel-1, 0);
							}
						}
					break;
					case IDC_DOWN:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ( (sel!=LB_ERR) && (sel<SendMessage (hwndList, LB_GETCOUNT, 0, 0)-1 ) )
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move down the item
								SendMessage (hwndList, LB_INSERTSTRING, sel+2, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel+1, 0);
							}
						}
					break;
					// 3 states management
					case IDC_COARSE_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							CoarseStateChanged (hwndDlg);
						}
						break;
					case IDC_ACTIVE_MRM:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							MRMStateChanged (hwndDlg);
						}
						break;
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_DYNAMIC_MESH:
					case IDC_EXPORT_CLOD:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					TCHAR name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport._Ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}




// ***************************************************************************
INT_PTR CALLBACK InstanceDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_SHAPE), MaxTStrFromUtf8(currentParam->InstanceShape).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), MaxTStrFromUtf8(currentParam->InstanceName).data());

			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_SETCHECK, currentParam->DontAddToScene, 0);

			EnableWindow (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), true);
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_SETCHECK, currentParam->DontExport, 0);

			SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_SETCHECK, currentParam->Collision, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), BM_SETCHECK, currentParam->CollisionExterior, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), MaxTStrFromUtf8(currentParam->InstanceGroupName).data());

			bool colOk = currentParam->CollisionMeshGeneration>=0 && currentParam->CollisionMeshGeneration<4;
			CheckRadioButton (hwndDlg, IDC_CAMERA_COL_RADIO1, IDC_CAMERA_COL_RADIO4, colOk?(IDC_CAMERA_COL_RADIO1+(currentParam->CollisionMeshGeneration)):0);

			InstanceStateChanged(hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							TCHAR tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_SHAPE), tmp, 512);
							currentParam->InstanceShape = MCharStrToUtf8(tmp);

							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), tmp, 512);
							currentParam->InstanceName = MCharStrToUtf8(tmp);

							currentParam->DontAddToScene=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), tmp, 512);
							currentParam->InstanceGroupName = MCharStrToUtf8(tmp);

							currentParam->DontExport=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_GETCHECK, 0, 0);

							currentParam->Collision= SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_GETCHECK, 0, 0);
							currentParam->CollisionExterior= SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), BM_GETCHECK, 0, 0);

							// Get the CollisionMeshGeneration
							if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO1) == BST_CHECKED)
								currentParam->CollisionMeshGeneration = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO2) == BST_CHECKED)
								currentParam->CollisionMeshGeneration = 1;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO3) == BST_CHECKED)
								currentParam->CollisionMeshGeneration = 2;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO4) == BST_CHECKED)
								currentParam->CollisionMeshGeneration = 3;
							else
								currentParam->CollisionMeshGeneration = -1;
						}
					break;
					case IDC_DONT_ADD_TO_SCENE:
					case IDC_DONT_EXPORT:
					case IDC_CHECK_COLLISION:
					case IDC_CHECK_COLLISION_EXTERIOR:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						// if change collision state
						if ( LOWORD(wParam) == IDC_CHECK_COLLISION )
							InstanceStateChanged(hwndDlg);
						break;
					case IDC_CAMERA_COL_RADIO1:
					case IDC_CAMERA_COL_RADIO2:
					case IDC_CAMERA_COL_RADIO3:
					case IDC_CAMERA_COL_RADIO4:
						// Get the acceleration type
						if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO1) == BST_CHECKED)
							currentParam->CollisionMeshGeneration = 0;
						else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO2) == BST_CHECKED)
							currentParam->CollisionMeshGeneration = 1;
						else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO3) == BST_CHECKED)
							currentParam->CollisionMeshGeneration = 2;
						else if (IsDlgButtonChecked (hwndDlg, IDC_CAMERA_COL_RADIO4) == BST_CHECKED)
							currentParam->CollisionMeshGeneration = 3;
						else
							currentParam->CollisionMeshGeneration = -1;
						break;

				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}



// ***************************************************************************
INT_PTR CALLBACK LightmapDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), MaxTStrFromUtf8(currentParam->LumelSizeMul).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), MaxTStrFromUtf8(currentParam->SoftShadowRadius).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), MaxTStrFromUtf8(currentParam->SoftShadowConeLength).data());

			// Lighting
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_SETCHECK, currentParam->ExportRealTimeLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_SETCHECK, currentParam->ExportLightMapLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_SETCHECK, currentParam->ExportLightMapAnimated, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_AS_SUN_LIGHT), BM_SETCHECK, currentParam->ExportAsSunLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_USE_LIGHT_LOCAL_ATTENUATION), BM_SETCHECK, currentParam->UseLightingLocalAttenuation, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LIGHT_DONT_CAST_SHADOW_INTERIOR), BM_SETCHECK, currentParam->LightDontCastShadowInterior, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LIGHT_DONT_CAST_SHADOW_EXTERIOR), BM_SETCHECK, currentParam->LightDontCastShadowExterior, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), MaxTStrFromUtf8(currentParam->ExportLightMapName).data());
			SendMessage (GetDlgItem (hwndDlg, IDC_REALTIME_LIGHT_AMBIENT_ADD_SUN), BM_SETCHECK, currentParam->RealTimeAmbientLightAddSun, 0);

			// Set enable disable
			LightingStateChanged (hwndDlg, currentParam);

			CheckRadioButton (hwndDlg, IDC_LIGHT_GROUP_ALWAYS, IDC_LIGHT_GROUP_LANDSCAPE_AMBIENT, IDC_LIGHT_GROUP_ALWAYS+(currentParam->LightGroup%5));
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDC_RESET_NAME:
						SetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), _T("GlobalLight"));
					break;
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							TCHAR tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), tmp, 512);
							currentParam->LumelSizeMul = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), tmp, 512);
							currentParam->SoftShadowRadius = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), tmp, 512);
							currentParam->SoftShadowConeLength = MCharStrToUtf8(tmp);

							// RealTime light
							currentParam->ExportRealTimeLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->ExportLightMapLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->ExportAsSunLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_AS_SUN_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->UseLightingLocalAttenuation = SendMessage (GetDlgItem (hwndDlg, IDC_USE_LIGHT_LOCAL_ATTENUATION), BM_GETCHECK, 0, 0);
							currentParam->LightDontCastShadowInterior = SendMessage (GetDlgItem (hwndDlg, IDC_LIGHT_DONT_CAST_SHADOW_INTERIOR), BM_GETCHECK, 0, 0);
							currentParam->LightDontCastShadowExterior = SendMessage (GetDlgItem (hwndDlg, IDC_LIGHT_DONT_CAST_SHADOW_EXTERIOR), BM_GETCHECK, 0, 0);
							currentParam->ExportLightMapAnimated = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), tmp, 512);
							currentParam->ExportLightMapName = MCharStrToUtf8(tmp);
							currentParam->RealTimeAmbientLightAddSun= SendMessage (GetDlgItem (hwndDlg, IDC_REALTIME_LIGHT_AMBIENT_ADD_SUN), BM_GETCHECK, 0, 0);

							// Get the acceleration type
							if (IsDlgButtonChecked (hwndDlg, IDC_LIGHT_GROUP_ALWAYS) == BST_CHECKED)
								currentParam->LightGroup = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_LIGHT_GROUP_LANDSCAPE_DIFFUSE) == BST_CHECKED)
								currentParam->LightGroup = 1;
							else if (IsDlgButtonChecked (hwndDlg, IDC_LIGHT_GROUP_NIGHT_CYCLE) == BST_CHECKED)
								currentParam->LightGroup = 2;
							else if (IsDlgButtonChecked (hwndDlg, IDC_LIGHT_GROUP_DAY_CYCLE) == BST_CHECKED)
								currentParam->LightGroup = 3;
							else if (IsDlgButtonChecked (hwndDlg, IDC_LIGHT_GROUP_LANDSCAPE_AMBIENT) == BST_CHECKED)
								currentParam->LightGroup = 4;
							else
								currentParam->LightGroup = -1;
						}
					break;
					case IDC_REALTIME_LIGHT_AMBIENT_ADD_SUN:
					case IDC_EXPORT_REALTIME_LIGHT:
					case IDC_USE_LIGHT_LOCAL_ATTENUATION:
					case IDC_EXPORT_LIGHTMAP_LIGHT:
					case IDC_EXPORT_LIGHTMAP_ANIMATED:
					case IDC_EXPORT_AS_SUN_LIGHT:
					case IDC_LIGHT_DONT_CAST_SHADOW_INTERIOR:
					case IDC_LIGHT_DONT_CAST_SHADOW_EXTERIOR:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);

						// Set enable disable
						LightingStateChanged (hwndDlg, currentParam);

						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					TCHAR name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport._Ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:
		break;


		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************
void	Lightmap2StateChanged (HWND hwndDlg, CLodDialogBoxParam *currentParam)
{
	bool enabled = (SendMessage (GetDlgItem (hwndDlg, IDC_LM_COMPRESS_8BIT), BM_GETCHECK, 0, 0)==BST_CHECKED);
	nlctassert(CLodDialogBoxParam::NumLightGroup==3);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_ALWAYS_AMBIENT), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_DAY_AMBIENT), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_NIGHT_AMBIENT), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_ALWAYS_DIFFUSE), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_DAY_DIFFUSE), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LM_NIGHT_DIFFUSE), enabled);

	EnableWindow (GetDlgItem(hwndDlg, IDC_LMC_AUTO_SETUP), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LMC_AUTO_SETUP_VISIBLEONLY), enabled);
	EnableWindow (GetDlgItem(hwndDlg, IDC_LMC_COPY_FROM), enabled);

	// MAX enable/disable
	uint i;
	for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
	{
		currentParam->LMCAmbient[i].Ctrl->Enable(enabled);
		currentParam->LMCDiffuse[i].Ctrl->Enable(enabled);
	}

	// Static Enable / Disable
	uint	staticItems[]= {IDC_LMC_STATIC0, IDC_LMC_STATIC1, IDC_LMC_STATIC2, IDC_LMC_STATIC3,
		IDC_LMC_STATIC4, IDC_LMC_STATIC5, IDC_LMC_STATIC6, IDC_LMC_STATIC7, IDC_LMC_STATIC8, };
	uint	numStaticItems= sizeof(staticItems) / sizeof(staticItems[0]);
	for(i=0;i<numStaticItems;i++)
	{
		EnableWindow (GetDlgItem(hwndDlg, staticItems[i]), enabled);
	}

}

// ***************************************************************************
void	lmcAutoSetup(CLodDialogBoxParam *currentParam, bool visibleOnly)
{
	uint	i;

	// get all lightmap lights
	std::vector<SLightBuild>		lights;
	getLightmapLightBuilds(lights, theCNelExport._Ip->GetTime(), *theCNelExport._Ip, visibleOnly);

	// mean all light, by lightgroup
	CRGBAF		sumAmbient[CLodDialogBoxParam::NumLightGroup];
	CRGBAF		sumDiffuse[CLodDialogBoxParam::NumLightGroup];
	uint		countAmbient[CLodDialogBoxParam::NumLightGroup];
	uint		countDiffuse[CLodDialogBoxParam::NumLightGroup];
	for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
	{
		sumAmbient[i].set(0,0,0,0);
		sumDiffuse[i].set(0,0,0,0);
		countAmbient[i]= 0;
		countDiffuse[i]= 0;
	}
	// stats
	for(i=0;i<lights.size();i++)
	{
		SLightBuild		&light= lights[i];
		uint			lg= light.LightGroup;
		if(lg<CLodDialogBoxParam::NumLightGroup)
		{
			if( light.Type==SLightBuild::LightAmbient || light.bAmbientOnly)
			{
				sumAmbient[lg]+= CRGBAF(light.Ambient) * light.rMult;
				countAmbient[lg]++;
			}
			else
			{
				sumDiffuse[lg]+= CRGBAF(light.Diffuse) * light.rMult;
				countDiffuse[lg]++;
			}
		}
	}

	// mean, by type
	for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
	{
		// defaults if no lights
		CRGBA	amb(0,0,0,0);
		CRGBA	diff(255,255,255,255);
		// mean the sums
		if(countAmbient[i])
		{
			sumAmbient[i]/= float(countAmbient[i]);
			amb= sumAmbient[i];
		}
		if(countDiffuse[i])
		{
			sumDiffuse[i]/= float(countDiffuse[i]);
			diff= sumDiffuse[i];
		}

		// change the control and value
		currentParam->LMCAmbient[i]= amb;
		currentParam->LMCAmbient[i].Ctrl->SetColor(RGB(amb.R, amb.G, amb.B));
		currentParam->LMCDiffuse[i]= diff;
		currentParam->LMCDiffuse[i].Ctrl->SetColor(RGB(diff.R, diff.G, diff.B));
	}
}


// ***************************************************************************
struct CLMCParamFrom
{
	bool		AmbFilter[CLodDialogBoxParam::NumLightGroup];
	bool		DiffFilter[CLodDialogBoxParam::NumLightGroup];
	CRGBAProp	AmbValue[CLodDialogBoxParam::NumLightGroup];
	CRGBAProp	DiffValue[CLodDialogBoxParam::NumLightGroup];
	std::vector<INode*>		Nodes;
	// true if the user has clicked at least one item
	bool		SelectionDone;

	CLMCParamFrom()
	{
		SelectionDone= false;
		for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
		{
			AmbFilter[i]= true;
			DiffFilter[i]= true;
		}
	}

	// to call before each call to the dialog box
	void	reset()
	{
		SelectionDone= false;
		for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
		{
			AmbValue[i].setDifferentValuesMode();
			DiffValue[i].setDifferentValuesMode();
		}
	}
};

// ***************************************************************************
INT_PTR CALLBACK LMCCopyFromDialogCallback(
	HWND hwndDlg,  // handle to dialog box
	UINT uMsg,     // message
	WPARAM wParam, // first message parameter
	LPARAM lParam  // second message parameter
	)
{
	CLMCParamFrom *lmcParam=(CLMCParamFrom *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	uint	i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			lmcParam=(CLMCParamFrom *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// init the colors
			nlctassert(CLodDialogBoxParam::NumLightGroup==3);
			nlverify(lmcParam->AmbValue[0].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_ALWAYS_AMBIENT)));
			nlverify(lmcParam->AmbValue[1].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_SUN_AMBIENT)));
			nlverify(lmcParam->AmbValue[2].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_NIGHT_AMBIENT)));
			nlverify(lmcParam->DiffValue[0].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_ALWAYS_DIFFUSE)));
			nlverify(lmcParam->DiffValue[1].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_SUN_DIFFUSE)));
			nlverify(lmcParam->DiffValue[2].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LMC_COPY_NIGHT_DIFFUSE)));
			// set color, and color state
			for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
			{
				CRGBA	a= lmcParam->AmbValue[i];
				CRGBA	d= lmcParam->DiffValue[i];
				lmcParam->AmbValue[i].Ctrl->SetColor(RGB(a.R, a.G, a.B));
				lmcParam->DiffValue[i].Ctrl->SetColor(RGB(d.R, d.G, d.B));
			}

			// init the filters.
			nlctassert(CLodDialogBoxParam::NumLightGroup==3);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_AMBIENT_FILTER), BM_SETCHECK, lmcParam->AmbFilter[0]?BST_CHECKED:BST_UNCHECKED, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_AMBIENT_FILTER), BM_SETCHECK, lmcParam->AmbFilter[1]?BST_CHECKED:BST_UNCHECKED, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_AMBIENT_FILTER), BM_SETCHECK, lmcParam->AmbFilter[2]?BST_CHECKED:BST_UNCHECKED, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_DIFFUSE_FILTER), BM_SETCHECK, lmcParam->DiffFilter[0]?BST_CHECKED:BST_UNCHECKED, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_DIFFUSE_FILTER), BM_SETCHECK, lmcParam->DiffFilter[1]?BST_CHECKED:BST_UNCHECKED, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_DIFFUSE_FILTER), BM_SETCHECK, lmcParam->DiffFilter[2]?BST_CHECKED:BST_UNCHECKED, 0);

			// init the list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LMC_COPY_LIST);
			for(i=0;i<lmcParam->Nodes.size();i++)
			{
				// get the node name
				std::string	str= CExportNel::getName(*lmcParam->Nodes[i]);
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) str.c_str());
			}

			// gray the OK button
			EnableWindow( GetDlgItem(hwndDlg, IDOK), FALSE);
		}
		break;

	case WM_COMMAND:
		if( HIWORD(wParam) == BN_CLICKED )
		{
			HWND hwndButton = (HWND) lParam;
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			case IDOK:
				{
					// get filter states
					nlctassert(CLodDialogBoxParam::NumLightGroup==3);
					lmcParam->AmbFilter[0]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_AMBIENT_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;
					lmcParam->AmbFilter[1]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_AMBIENT_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;
					lmcParam->AmbFilter[2]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_AMBIENT_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;
					lmcParam->DiffFilter[0]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_DIFFUSE_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;
					lmcParam->DiffFilter[1]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_DIFFUSE_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;
					lmcParam->DiffFilter[2]= SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_DIFFUSE_FILTER), BM_GETCHECK, 0, 0) == BST_CHECKED;

					// get colors
					for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
					{
						COLORREF a = lmcParam->AmbValue[i].Ctrl->GetColor();
						COLORREF d = lmcParam->DiffValue[i].Ctrl->GetColor();
						lmcParam->AmbValue[i].R = (uint8)((a & 0xFF)); // GetRValue(a); // runtime check failure :(
						lmcParam->AmbValue[i].G = (uint8)((a & 0xFF00) >> 8); // GetGValue(a);
						lmcParam->AmbValue[i].B = (uint8)((a & 0xFF0000) >> 16); // GetBValue(a);
						lmcParam->DiffValue[i].R = (uint8)((d & 0xFF)); // GetRValue(d);
						lmcParam->DiffValue[i].G = (uint8)((d & 0xFF00) >> 8); // GetGValue(d);
						lmcParam->DiffValue[i].B = (uint8)((d & 0xFF0000) >> 16); // GetBValue(d);
					}

					EndDialog(hwndDlg, IDOK);
				}
				break;
			case IDC_LMC_COPY_CLEAR:
				{
					// init the filters.
					nlctassert(CLodDialogBoxParam::NumLightGroup==3);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_AMBIENT_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_AMBIENT_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_AMBIENT_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_DIFFUSE_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_DIFFUSE_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_DIFFUSE_FILTER), BM_SETCHECK, BST_UNCHECKED, 0);
				}
				break;
			case IDC_LMC_COPY_GET_ALL:
				{
					// init the filters.
					nlctassert(CLodDialogBoxParam::NumLightGroup==3);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_AMBIENT_FILTER), BM_SETCHECK, BST_CHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_AMBIENT_FILTER), BM_SETCHECK, BST_CHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_AMBIENT_FILTER), BM_SETCHECK, BST_CHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_ALWAYS_DIFFUSE_FILTER), BM_SETCHECK, BST_CHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_SUN_DIFFUSE_FILTER), BM_SETCHECK, BST_CHECKED, 0);
					SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_NIGHT_DIFFUSE_FILTER), BM_SETCHECK, BST_CHECKED, 0);
				}
				break;
			}
		}
		else if( HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam)== IDC_LMC_COPY_LIST )
		{
			// List item clicked
			uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_LIST), LB_GETCURSEL, 0, 0);
			if (wID!=LB_ERR)
			{
				lmcParam->SelectionDone= true;

				// Get the node name
				TCHAR name[512];
				SendMessage (GetDlgItem (hwndDlg, IDC_LMC_COPY_LIST), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

				// Find the node
				INode *nodeClk=theCNelExport._Ip->GetINodeByName(name);
				if (nodeClk)
				{
					// Get the color of this node, and assign to the color swatch
					for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
					{
						CRGBA	a= CExportNel::getScriptAppData (nodeClk, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START+i, CRGBA::Black);
						CRGBA	d= CExportNel::getScriptAppData (nodeClk, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START+i, CRGBA::White);
						lmcParam->AmbValue[i].Ctrl->SetColor(RGB(a.R, a.G, a.B));
						lmcParam->DiffValue[i].Ctrl->SetColor(RGB(d.R, d.G, d.B));
						lmcParam->AmbValue[i].DifferentValues= false;
						lmcParam->DiffValue[i].DifferentValues= false;
					}
				}

				// ungray the OK button
				EnableWindow( GetDlgItem(hwndDlg, IDOK), TRUE);
			}

		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg,IDCANCEL);
		break;

	case WM_DESTROY:
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// ***************************************************************************
void	lmcCopyFrom(CLodDialogBoxParam *currentParam, HWND parentDlg)
{
	uint	i;

	// static to save filter between calls
	static CLMCParamFrom		paramLMCFrom;

	// **** get all nodes
	// get all nodes
	std::vector<INode*>	nodes;
	theCNelExport._ExportNel->getObjectNodes(nodes, theCNelExport._Ip->GetTime());
	paramLMCFrom.Nodes.clear();
	// retrieve only ones that have a lmc 8 bits setup....
	for(i=0;i<nodes.size();i++)
	{
		if(CExportNel::getScriptAppData (nodes[i], NEL3D_APPDATA_EXPORT_LMC_ENABLED, BST_UNCHECKED)==BST_CHECKED)
		{
			paramLMCFrom.Nodes.push_back(nodes[i]);
		}
	}


	// **** launch the choosing dialog
	paramLMCFrom.reset();
	if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_LMC_CHOOSE_FROM), parentDlg, LMCCopyFromDialogCallback, (LPARAM)&paramLMCFrom)==IDOK
		&& paramLMCFrom.SelectionDone)
	{
		// **** Apply to the current setup
		for(i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
		{
			CRGBA	amb= paramLMCFrom.AmbValue[i];
			CRGBA	diff= paramLMCFrom.DiffValue[i];
			// change the control and value
			if(paramLMCFrom.AmbFilter[i])
			{
				currentParam->LMCAmbient[i]= amb;
				currentParam->LMCAmbient[i].Ctrl->SetColor(RGB(amb.R, amb.G, amb.B));
				currentParam->LMCAmbient[i].DifferentValues= false;
			}
			if(paramLMCFrom.DiffFilter[i])
			{
				currentParam->LMCDiffuse[i]= diff;
				currentParam->LMCDiffuse[i].Ctrl->SetColor(RGB(diff.R, diff.G, diff.B));
				currentParam->LMCDiffuse[i].DifferentValues= false;
			}
		}
	}
}

// ***************************************************************************
INT_PTR CALLBACK Lightmap2DialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// retrieve the color choosing Ctrl
			nlctassert(CLodDialogBoxParam::NumLightGroup==3);
			nlverify(currentParam->LMCAmbient[0].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_ALWAYS_AMBIENT)));
			nlverify(currentParam->LMCAmbient[1].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_DAY_AMBIENT)));
			nlverify(currentParam->LMCAmbient[2].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_NIGHT_AMBIENT)));
			nlverify(currentParam->LMCDiffuse[0].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_ALWAYS_DIFFUSE)));
			nlverify(currentParam->LMCDiffuse[1].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_DAY_DIFFUSE)));
			nlverify(currentParam->LMCDiffuse[2].Ctrl= GetIColorSwatch(GetDlgItem(hwndDlg, IDC_LM_NIGHT_DIFFUSE)));

			// set color, and color state
			for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
			{
				CRGBA	a= currentParam->LMCAmbient[i];
				CRGBA	d= currentParam->LMCDiffuse[i];
				currentParam->LMCAmbient[i].Ctrl->SetColor(RGB(a.R, a.G, a.B));
				currentParam->LMCDiffuse[i].Ctrl->SetColor(RGB(d.R, d.G, d.B));
			}

			// the enable button
			SendMessage (GetDlgItem (hwndDlg, IDC_LM_COMPRESS_8BIT), BM_SETCHECK, currentParam->LMCEnabled, 0);

			// Set enable disable
			Lightmap2StateChanged (hwndDlg, currentParam);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDC_LM_COMPRESS_8BIT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						Lightmap2StateChanged(hwndDlg, currentParam);
					break;
					case IDOK:
						{
							// get state
							currentParam->LMCEnabled= SendMessage (GetDlgItem (hwndDlg, IDC_LM_COMPRESS_8BIT), BM_GETCHECK, 0, 0);

							// get color
							for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
							{
								COLORREF a = currentParam->LMCAmbient[i].Ctrl->GetColor();
								COLORREF d = currentParam->LMCDiffuse[i].Ctrl->GetColor();
								currentParam->LMCAmbient[i].R = (uint8)((a & 0xFF)); // GetRValue(a); // runtime check failure :(
								currentParam->LMCAmbient[i].G = (uint8)((a & 0xFF00) >> 8); // GetGValue(a);
								currentParam->LMCAmbient[i].B = (uint8)((a & 0xFF0000) >> 16); // GetBValue(a);
								currentParam->LMCDiffuse[i].R = (uint8)((d & 0xFF)); // GetRValue(d);
								currentParam->LMCDiffuse[i].G = (uint8)((d & 0xFF00) >> 8); // GetGValue(d);
								currentParam->LMCDiffuse[i].B = (uint8)((d & 0xFF0000) >> 16); // GetBValue(d);
							}
						}
					break;
					case IDC_LMC_AUTO_SETUP:
						lmcAutoSetup(currentParam, false);
					break;
					case IDC_LMC_AUTO_SETUP_VISIBLEONLY:
						lmcAutoSetup(currentParam, true);
					break;
					case IDC_LMC_COPY_FROM:
						lmcCopyFrom(currentParam, hwndDlg);
					break;
				}
			}
		break;

		case CC_COLOR_CHANGE:
			{
				nlctassert(CLodDialogBoxParam::NumLightGroup==3);
				CRGBAProp		*propEdited;
				switch(LOWORD(wParam))
				{
				case IDC_LM_ALWAYS_AMBIENT:	propEdited= &currentParam->LMCAmbient[0]; break;
				case IDC_LM_DAY_AMBIENT:	propEdited= &currentParam->LMCAmbient[1]; break;
				case IDC_LM_NIGHT_AMBIENT:	propEdited= &currentParam->LMCAmbient[2]; break;
				case IDC_LM_ALWAYS_DIFFUSE:	propEdited= &currentParam->LMCDiffuse[0]; break;
				case IDC_LM_DAY_DIFFUSE:	propEdited= &currentParam->LMCDiffuse[1]; break;
				case IDC_LM_NIGHT_DIFFUSE:	propEdited= &currentParam->LMCDiffuse[2]; break;
				default: nlstop;
				};
				// no more true
				propEdited->DifferentValues= false;
			}
		break;

		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************
INT_PTR CALLBACK VegetableDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_SETCHECK, currentParam->Vegetable, 0);

			// Yoyo: enable like this. Crappy CheckRadioButton method...
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON), BM_SETCHECK, currentParam->VegetableAlphaBlend==0, 0 );
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF), BM_SETCHECK, currentParam->VegetableAlphaBlend==1, 0 );

			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_ON_UNLIGHTED, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOnLighted);

			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_OFF_UNLIGHTED, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOffLighted);

			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_SETCHECK, currentParam->VegetableAlphaBlendOffDoubleSided, 0);

			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_FORCE_BEST_SIDED_LIGHTING), BM_SETCHECK, currentParam->VegetableForceBestSidedLighting, 0);

			CheckRadioButton(hwndDlg, IDC_CENTER_NULL, IDC_CENTER_Z, IDC_CENTER_NULL+currentParam->VegetableBendCenter);

			SetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), MaxTStrFromUtf8(currentParam->VegetableBendFactor).data());

			VegetableStateChanged (hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->Vegetable=SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 1;
							else
								currentParam->VegetableAlphaBlend = -1;

							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 1;
							else
								currentParam->VegetableAlphaBlendOnLighted = -1;

							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 1;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 2;
							else
								currentParam->VegetableAlphaBlendOffLighted = -1;

							currentParam->VegetableAlphaBlendOffDoubleSided = SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_GETCHECK, 0, 0);

							currentParam->VegetableForceBestSidedLighting= SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_FORCE_BEST_SIDED_LIGHTING), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_NULL) == BST_CHECKED)
								currentParam->VegetableBendCenter = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_Z) == BST_CHECKED)
								currentParam->VegetableBendCenter = 1;
							else
								currentParam->VegetableBendCenter = -1;

							TCHAR tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), tmp, 512);
							currentParam->VegetableBendFactor = MCharStrToUtf8(tmp);
						}
					break;
					case IDC_VEGETABLE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
					case IDC_VEGETABLE_ALPHA_BLEND_ON:
					case IDC_VEGETABLE_ALPHA_BLEND_OFF:
					case IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED:
					case IDC_VEGETABLE_AB_ON_UNLIGHTED:
					case IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED:
					case IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC:
					case IDC_VEGETABLE_AB_OFF_UNLIGHTED:
						VegetableStateChanged (hwndDlg);
						break;

					case IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED:
					case IDC_VEGETABLE_FORCE_BEST_SIDED_LIGHTING:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}

// ***************************************************************************
INT_PTR CALLBACK VertexProgramDialogCallBack (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// test wether v.p are bypassed for that object (this may happen when a v.p is needed by a material of this mesh)
			if (!currentParam->VertexProgramBypassed)
			{
				if(currentParam->VertexProgramId>=0)
				{
					// Init DropList.
					SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_ADDSTRING, 0, (LPARAM)_T("Disable"));
					SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_ADDSTRING, 0, (LPARAM)_T("Wind Tree"));
					SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_SETCURSEL, currentParam->VertexProgramId, 0);
					EnableWindow( GetDlgItem(hwndDlg, IDC_COMBO_VP), TRUE);
				}
				else
				{
					SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_SETCURSEL, -1, 0);
					EnableWindow( GetDlgItem(hwndDlg, IDC_COMBO_VP), FALSE);
				}
				ShowWindow( GetDlgItem(hwndDlg, IDC_BYPASS_VP), SW_HIDE);
			}
			else // no vertex program available
			{
				EnableWindow( GetDlgItem(hwndDlg, IDC_COMBO_VP), FALSE);
				EnableWindow( GetDlgItem(hwndDlg, IDC_VP_TEXT), FALSE);
				ShowWindow( GetDlgItem(hwndDlg, IDC_BYPASS_VP), SW_SHOW);
			}

			// Get the tab client rect in screen
			RECT tabRect;
			GetClientRect (hwndDlg, &tabRect);
			// Remove VP choose combo box
			tabRect.top += 25;
			tabRect.left += 2;
			tabRect.right -= 2;
			tabRect.bottom -= 2;
			ClientToScreen (hwndDlg, (POINT*)&tabRect.left);
			ClientToScreen (hwndDlg, (POINT*)&tabRect.right);

			// For each VP Dlg to init.
			for (uint vpId=0; vpId<VP_COUNT; vpId++)
			{
				// Create the dialog
				currentParam->SubVPDlg[vpId] = CreateDialogParam (hInstance, MAKEINTRESOURCE(SubVPTab[vpId]), hwndDlg, SubVPProc[vpId], lParam);

				// To client coord
				RECT client = tabRect;
				ScreenToClient (currentParam->SubVPDlg[vpId], (POINT*)&client.left);
				ScreenToClient (currentParam->SubVPDlg[vpId], (POINT*)&client.right);

				// Resize and pos it
				SetWindowPos (currentParam->SubVPDlg[vpId], NULL, client.left, client.top, client.right-client.left, client.bottom-client.top, SWP_NOOWNERZORDER|SWP_NOZORDER);
			}

			// Show the prop window
			if(currentParam->VertexProgramId>0)
			{
				int	vpWind= currentParam->VertexProgramId-1;
				ShowWindow(currentParam->SubVPDlg[vpWind], SW_SHOW);
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Validate chosen VertexProgram.
							currentParam->VertexProgramId= SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_GETCURSEL, 0, 0);
							if(currentParam->VertexProgramId==CB_ERR)
								currentParam->VertexProgramId= -1;

							// Advice VP windows
							for (uint vpId=0; vpId<VP_COUNT; vpId++)
							{
								// Send back an ok message
								SendMessage (currentParam->SubVPDlg[vpId], uMsg, wParam, lParam);
							}

						}
					break;
				}
			}
			else if( HIWORD(wParam) == CBN_SELCHANGE )
			{
				if(LOWORD(wParam) == IDC_COMBO_VP)
				{
					// Validate chosen VertexProgram.
					currentParam->VertexProgramId= SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_GETCURSEL, 0, 0);
					if(currentParam->VertexProgramId==CB_ERR)
						currentParam->VertexProgramId= -1;

					// Show the appropriate window (if VP enabled)
					for(int vpId=0; vpId<VP_COUNT;vpId++)
					{
						int		vpWindow= currentParam->VertexProgramId - 1;
						if(vpId==vpWindow)
							ShowWindow (currentParam->SubVPDlg[vpId], SW_SHOW);
						else
							ShowWindow (currentParam->SubVPDlg[vpId], SW_HIDE);
					}

				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************


// Slider Control Ids.
static	int	VPWTFreqSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_FREQ_L0, IDC_SLIDER_VPWT_FREQ_L1, IDC_SLIDER_VPWT_FREQ_L2};
static	int	VPWTFreqWDSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_FREQWD_L0, IDC_SLIDER_VPWT_FREQWD_L1, IDC_SLIDER_VPWT_FREQWD_L2};
static	int	VPWTDistXYSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_DISTXY_L0, IDC_SLIDER_VPWT_DISTXY_L1, IDC_SLIDER_VPWT_DISTXY_L2};
static	int	VPWTDistZSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_DISTZ_L0, IDC_SLIDER_VPWT_DISTZ_L1, IDC_SLIDER_VPWT_DISTZ_L2};
static	int	VPWTBiasSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_BIAS_L0, IDC_SLIDER_VPWT_BIAS_L1, IDC_SLIDER_VPWT_BIAS_L2};
// Static Control Ids.
static	int	VPWTFreqStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_FREQ_L0, IDC_STATIC_VPWT_FREQ_L1, IDC_STATIC_VPWT_FREQ_L2};
static	int	VPWTFreqWDStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_FREQWD_L0, IDC_STATIC_VPWT_FREQWD_L1, IDC_STATIC_VPWT_FREQWD_L2};
static	int	VPWTDistXYStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_DISTXY_L0, IDC_STATIC_VPWT_DISTXY_L1, IDC_STATIC_VPWT_DISTXY_L2};
static	int	VPWTDistZStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_DISTZ_L0, IDC_STATIC_VPWT_DISTZ_L1, IDC_STATIC_VPWT_DISTZ_L2};
static	int	VPWTBiasStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_BIAS_L0, IDC_STATIC_VPWT_BIAS_L1, IDC_STATIC_VPWT_BIAS_L2};


void	updateVPWTStatic(HWND hwndDlg, uint type, uint depth, const CVPWindTreeAppData &vpwt)
{
	int		sliderValue;
	TCHAR	stmp[256];
	float	nticks= CVPWindTreeAppData::NumTicks;
	float	scale;

	// which scale??
	switch(type)
	{
	case 0:
	case 1:
		scale= vpwt.FreqScale;
		break;
	case 2:
	case 3:
		scale= vpwt.DistScale;
		break;
	// case 4: special code from -2 to 2 ...
	}

	// update static according to type.
	switch(type)
	{
	case 0:
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[depth], TBM_GETPOS, 0, 0);
		_stprintf(stmp, _T("%.2f"), scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTFreqStaticId[depth]), stmp );
		break;
	case 1:
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[depth], TBM_GETPOS, 0, 0);
		_stprintf(stmp, _T("%.2f"), scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTFreqWDStaticId[depth]), stmp );
		break;
	case 2:
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[depth], TBM_GETPOS, 0, 0);
		_stprintf(stmp, _T("%.2f"), scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTDistXYStaticId[depth]), stmp );
		break;
	case 3:
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[depth], TBM_GETPOS, 0, 0);
		_stprintf(stmp, _T("%.2f"), scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTDistZStaticId[depth]), stmp );
		break;
	case 4:
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[depth], TBM_GETPOS, 0, 0);
		// expand to -2 to 2.
		float	biasVal= 4 * float(sliderValue)/nticks - 2;
		_stprintf(stmp, _T("%.2f"), biasVal);
		SetWindowText( GetDlgItem(hwndDlg, VPWTBiasStaticId[depth]), stmp );
		break;
	}
}


static	void concatEdit2Lines(HWND edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	TCHAR	tmp0[2*lineLen];
	TCHAR	tmp1[lineLen];
	*(WORD*)tmp0= lineLen;
	*(WORD*)tmp1= lineLen;
	n= SendMessage(edit, EM_GETLINE, 0, (LPARAM)tmp0); tmp0[n]= 0;
	n= SendMessage(edit, EM_GETLINE, 1, (LPARAM)tmp1); tmp1[n]= 0;
	// concat and update the CEdit.
	SetWindowText(edit, _tcscat(tmp0, tmp1));
}


static	void updateVPWTStaticForControl(HWND hwndDlg, HWND ctrlWnd, CVPWindTreeAppData &vpwt, int sliderValue )
{
	// What ctrlWnd is modified??
	int		sliderType=-1;
	int		depth;
	for(depth =0;depth<CVPWindTreeAppData::HrcDepth;depth ++)
	{
		if(ctrlWnd== GetDlgItem(hwndDlg, VPWTFreqSliderId[depth]))
		{sliderType= 0; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTFreqWDSliderId[depth]))
		{sliderType= 1; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTDistXYSliderId[depth]))
		{sliderType= 2; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTDistZSliderId[depth]))
		{sliderType= 3; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTBiasSliderId[depth]))
		{sliderType= 4; break;}
	}

	// Set to value, and update static.
	if(sliderType>=0)
	{
		// update value.
		if(sliderType==0)	vpwt.Frequency[depth]= sliderValue;
		if(sliderType==1)	vpwt.FrequencyWindFactor[depth]= sliderValue;
		if(sliderType==2)	vpwt.DistXY[depth]= sliderValue;
		if(sliderType==3)	vpwt.DistZ[depth]= sliderValue;
		if(sliderType==4)	vpwt.Bias[depth]= sliderValue;
		// update text.
		updateVPWTStatic(hwndDlg, sliderType, depth, vpwt);
	}
}


INT_PTR CALLBACK VPWindTreeCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// Init controls
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int	nticks= CVPWindTreeAppData::NumTicks;


			// Init Global. editBox
			TCHAR		stmp[256];
			_stprintf(stmp, _T("%.2f"), vpwt.FreqScale);
			SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp );
			_stprintf(stmp, _T("%.2f"), vpwt.DistScale);
			SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp );
			SendDlgItemMessage(hwndDlg, IDC_CHECK_VP_SPECLIGHT, BM_SETCHECK, vpwt.SpecularLighting, 0);


			// Init sliders for each level.
			nlassert(CVPWindTreeAppData::HrcDepth==3);
			for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
			{
				// Init ranges.
				SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));

				// Clamp values to range.
				clamp(vpwt.Frequency[i], 0, nticks);
				clamp(vpwt.FrequencyWindFactor[i], 0, nticks);
				clamp(vpwt.DistXY[i], 0, nticks);
				clamp(vpwt.DistZ[i], 0, nticks);
				clamp(vpwt.Bias[i], 0, nticks);

				// Init current selection.
				SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_SETPOS, TRUE, vpwt.Frequency[i]);
				SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_SETPOS, TRUE, vpwt.FrequencyWindFactor[i]);
				SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_SETPOS, TRUE, vpwt.DistXY[i]);
				SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_SETPOS, TRUE, vpwt.DistZ[i]);
				SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_SETPOS, TRUE, vpwt.Bias[i]);

				// Init current Static selection.
				updateVPWTStatic(hwndDlg, 0, i, vpwt);	// FreqStatic
				updateVPWTStatic(hwndDlg, 1, i, vpwt);	// FreqWDStatic
				updateVPWTStatic(hwndDlg, 2, i, vpwt);	// DistXYStatic
				updateVPWTStatic(hwndDlg, 3, i, vpwt);	// DistZStatic
				updateVPWTStatic(hwndDlg, 4, i, vpwt);	// BiasStatic
			}
		}
		break;

		case WM_COMMAND:
		{
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int	nticks= CVPWindTreeAppData::NumTicks;
			TCHAR		stmp[256];
			float		val;

			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
					{
						// Read FreqScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp, 256 );
						val= toFloatMax(stmp);
						if(val>0)
							vpwt.FreqScale= val;
						// Read DistScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp, 256 );
						val= toFloatMax(stmp);
						if(val>0)
							vpwt.DistScale= val;
						// Read SpecularLighting.
						vpwt.SpecularLighting= SendDlgItemMessage(hwndDlg, IDC_CHECK_VP_SPECLIGHT, BM_GETCHECK, 0, 0);

						// Read slider for each level.
						nlassert(CVPWindTreeAppData::HrcDepth==3);
						for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
						{
							// Read current selection.
							vpwt.Frequency[i]= SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.FrequencyWindFactor[i]= SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.DistXY[i]= SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.DistZ[i]= SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.Bias[i]= SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_GETPOS, 0, 0);

							// Clamp values to range.
							clamp(vpwt.Frequency[i], 0, nticks);
							clamp(vpwt.FrequencyWindFactor[i], 0, nticks);
							clamp(vpwt.DistXY[i], 0, nticks);
							clamp(vpwt.DistZ[i], 0, nticks);
							clamp(vpwt.Bias[i], 0, nticks);
						}
					}
					break;
				}
			}
			// Aware EditBox: selectAll on setFocus
			if( HIWORD(wParam) == EN_SETFOCUS )
			{
				// Select All.
				PostMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), EM_SETSEL, 0, -1);
				InvalidateRect(GetDlgItem(hwndDlg, LOWORD(wParam)), NULL, TRUE);
			}

			// Aware EditBox: Update and killFocus on enter!!
			bool	EnChangeReturn= false;
			if( HIWORD(wParam) == EN_CHANGE )
			{
				// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
				// user has press enter.
				if( SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), EM_GETLINECOUNT, 0, 0) > 1)
				{
					// Concat the 2 lines.
					concatEdit2Lines( GetDlgItem(hwndDlg, LOWORD(wParam)) );
					// Must update value next.
					EnChangeReturn= true;
				}
			}

			// EditBox change: ...
			if( HIWORD(wParam) == EN_KILLFOCUS || EnChangeReturn)
			{
				switch (LOWORD(wParam))
				{
					case IDC_EDIT_VPWT_FREQ_SCALE:
					{
						// Read FreqScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp, 256 );
						val= toFloatMax(stmp);
						if(val>0)
						{
							// update
							vpwt.FreqScale= val;
							// update All Statics
							for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
							{
								updateVPWTStatic(hwndDlg, 0, i, vpwt);	// FreqStatic
								updateVPWTStatic(hwndDlg, 1, i, vpwt);	// FreqWDStatic
							}
						}
						// Update Scale Edit text.
						_stprintf(stmp, _T("%.2f"), vpwt.FreqScale);
						SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp );
					}
					break;
					case IDC_EDIT_VPWT_DIST_SCALE:
					{
						// Read DistScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp, 256 );
						val= toFloatMax(stmp);
						if(val>0)
						{
							// update
							vpwt.DistScale= val;
							// update All Statics
							for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
							{
								updateVPWTStatic(hwndDlg, 2, i, vpwt);	// DistXYStatic
								updateVPWTStatic(hwndDlg, 3, i, vpwt);	// DistZStatic
							}
						}
						// Update Scale Edit text.
						_stprintf(stmp, _T("%.2f"), vpwt.DistScale);
						SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp );
					}
					break;
				}
			}
		}
		break;

		// Handle dynamic scroll updating static
		case WM_HSCROLL:
		{
			HWND	ctrlWnd= (HWND)lParam;
			UINT	nSBCode= LOWORD(wParam);
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int		nticks= CVPWindTreeAppData::NumTicks;

			if( nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
			{
				int		sliderValue= HIWORD(wParam);
				clamp(sliderValue, 0, nticks);

				updateVPWTStaticForControl( hwndDlg, ctrlWnd, vpwt, sliderValue );
			}
		}
		break;

		// update static on release
		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			HWND	ctrlWnd= pnmh->hwndFrom;
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int		nticks= CVPWindTreeAppData::NumTicks;

			if( pnmh->code == NM_RELEASEDCAPTURE )
			{
				int sliderValue= SendMessage (ctrlWnd, TBM_GETPOS, 0, 0);
				clamp(sliderValue, 0, nticks);

				updateVPWTStaticForControl( hwndDlg, ctrlWnd, vpwt, sliderValue );
			}
		}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}



// ***************************************************************************
INT_PTR CALLBACK MiscDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_SETCHECK, currentParam->FloatingObject, 0);

			// Ligoscape
			SendMessage (GetDlgItem (hwndDlg, IDC_LIGO_SYMMETRY), BM_SETCHECK, currentParam->LigoSymmetry, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_LIGO_ROTATE), MaxTStrFromUtf8(currentParam->LigoRotate).data());

			// SWT
			SendMessage (GetDlgItem (hwndDlg, IDC_SWT), BM_SETCHECK, currentParam->SWT, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_SWT_WEIGHT), MaxTStrFromUtf8(currentParam->SWTWeight).data());

			// Radial normals
			for (uint smoothGroup=0; smoothGroup<NEL3D_RADIAL_NORMAL_COUNT; smoothGroup++)
				SetWindowText (GetDlgItem (hwndDlg, IDC_RADIAL_NORMAL_29+smoothGroup), MaxTStrFromUtf8(currentParam->RadialNormals[smoothGroup]).data());

			// Mesh interfaces
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INTERFACE_FILE), MaxTStrFromUtf8(currentParam->InterfaceFileName).data());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INTERFACE_THRESHOLD),
							currentParam->InterfaceThreshold != -1.f ? MaxTStrFromUtf8(toStringMax(currentParam->InterfaceThreshold)).data()
																	 : _T("")
						  );
			SendMessage(GetDlgItem(hwndDlg, IDC_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS), BM_SETCHECK, currentParam->GetInterfaceNormalsFromSceneObjects, 0);

			// Skeleton Scale
			SendMessage( GetDlgItem(hwndDlg, IDC_EXPORT_BONE_SCALE), BM_SETCHECK, currentParam->ExportBoneScale, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_BONE_SCALE_NAME_EXT), MaxTStrFromUtf8(currentParam->ExportBoneScaleNameExt).data());

			// Remanence
			SendMessage (GetDlgItem (hwndDlg, IDC_USE_REMANENCE), BM_SETCHECK, currentParam->UseRemanence, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_REMANENCE_SHIFTING_TEXTURE), BM_SETCHECK, currentParam->RemanenceShiftingTexture, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_SLICE_NUMBER), currentParam->RemanenceSliceNumber != - 1 ? MaxTStrFromUtf8(toStringMax(currentParam->RemanenceSliceNumber)).data() : _T(""));
			SetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_SAMPLING_PERIOD), currentParam->RemanenceSamplingPeriod != -1 ? MaxTStrFromUtf8(toStringMax(currentParam->RemanenceSamplingPeriod)).data() : _T(""));
			SetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_ROLLUP_RATIO), currentParam->RemanenceRollupRatio != -1 ? MaxTStrFromUtf8(toStringMax(currentParam->RemanenceRollupRatio)).data() : _T(""));
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->FloatingObject = SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_GETCHECK, 0, 0);

							// Ligoscape
							currentParam->LigoSymmetry = SendMessage (GetDlgItem (hwndDlg, IDC_LIGO_SYMMETRY), BM_GETCHECK, 0, 0);
							TCHAR tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_LIGO_ROTATE), tmp, 512);
							currentParam->LigoRotate = MCharStrToUtf8(tmp);

							// SWT
							currentParam->SWT = SendMessage (GetDlgItem (hwndDlg, IDC_SWT), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_SWT_WEIGHT), tmp, 512);
							currentParam->SWTWeight = MCharStrToUtf8(tmp);

							// Radial normals
							for (uint smoothGroup=0; smoothGroup<NEL3D_RADIAL_NORMAL_COUNT; smoothGroup++)
							{
								HWND edit = GetDlgItem (hwndDlg, IDC_RADIAL_NORMAL_29+smoothGroup);
								GetWindowText (edit, tmp, 512);
								currentParam->RadialNormals[smoothGroup] = MCharStrToUtf8(tmp);
							}

							// mesh interfaces
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INTERFACE_FILE), tmp, 512);
							currentParam->InterfaceFileName = MCharStrToUtf8(tmp);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INTERFACE_THRESHOLD), tmp, 512);
							if (_tcslen(tmp) != 0)
								currentParam->InterfaceThreshold = toFloatMax(tmp);
							currentParam->GetInterfaceNormalsFromSceneObjects = SendMessage (GetDlgItem (hwndDlg, IDC_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS), BM_GETCHECK, 0, 0);


							// Skeleton Scale
							currentParam->ExportBoneScale= SendMessage( GetDlgItem(hwndDlg, IDC_EXPORT_BONE_SCALE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_BONE_SCALE_NAME_EXT), tmp, 512);
							currentParam->ExportBoneScaleNameExt = MCharStrToUtf8(tmp);

							// remanence
							currentParam->UseRemanence = SendMessage (GetDlgItem (hwndDlg, IDC_USE_REMANENCE), BM_GETCHECK, 0, 0);
							currentParam->RemanenceShiftingTexture = SendMessage (GetDlgItem (hwndDlg, IDC_REMANENCE_SHIFTING_TEXTURE), BM_GETCHECK, 0, 0);

							GetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_SLICE_NUMBER), tmp, 512);

							uint rsn;
							if (NLMISC::fromString(MCharStrToUtf8(tmp), rsn))
							{
								currentParam->RemanenceSliceNumber = rsn;
							}

							GetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_SAMPLING_PERIOD), tmp, 512);
							toFloatMax(tmp, currentParam->RemanenceSamplingPeriod);
							GetWindowText (GetDlgItem (hwndDlg, IDC_REMANENCE_ROLLUP_RATIO), tmp, 512);
							toFloatMax(tmp, currentParam->RemanenceRollupRatio);
						}
					break;
					case IDC_FLOATING_OBJECT:
					case IDC_SWT:
					case IDC_EXPORT_BONE_SCALE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}




// ***************************************************************************
INT_PTR CALLBACK AnimationDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_SETCHECK, currentParam->ExportNoteTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_SSS_TRACK), BM_SETCHECK, currentParam->ExportSSSTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_SETCHECK, currentParam->ExportAnimatedMaterials, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NODE_ANIMATION), BM_SETCHECK, currentParam->ExportNodeAnimation, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATION_PREFIXE_NAME), BM_SETCHECK, currentParam->PrefixeTracksNodeName, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_AUTOMATIC_ANIM), BM_SETCHECK, currentParam->AutomaticAnimation, 0);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->ExportNoteTrack = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_GETCHECK, 0, 0);
							currentParam->ExportSSSTrack = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_SSS_TRACK), BM_GETCHECK, 0, 0);
							currentParam->ExportAnimatedMaterials = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_GETCHECK, 0, 0);
							currentParam->ExportNodeAnimation = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NODE_ANIMATION), BM_GETCHECK, 0, 0);
							currentParam->PrefixeTracksNodeName = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATION_PREFIXE_NAME), BM_GETCHECK, 0, 0);
							currentParam->AutomaticAnimation = SendMessage (GetDlgItem (hwndDlg, IDC_AUTOMATIC_ANIM), BM_GETCHECK, 0, 0);
						}
					break;
					case IDC_EXPORT_NOTE_TRACK:
					case IDC_EXPORT_SSS_TRACK:
					case IDC_EXPORT_ANIMATED_MATERIALS:
					case IDC_EXPORT_ANIMATION_PREFIXE_NAME:
					case IDC_EXPORT_NODE_ANIMATION:
					case IDC_AUTOMATIC_ANIM:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}






// ***************************************************************************
INT_PTR CALLBACK LodDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG_PTR res = SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			currentParam = (CLodDialogBoxParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// Window text
			TSTR winName = (*(currentParam->ListNode->begin()))->GetName();
			winName = TSTR(_M("Node properties (")) + winName + ((currentParam->ListNode->size() > 1) ? _M(", ...)") : _M(")"));
			SetWindowText(hwndDlg, winName.data());

			// Move dialog
			RECT windowRect, desktopRect;
			GetWindowRect (hwndDlg, &windowRect);
			HWND desktop=GetDesktopWindow ();
			GetClientRect (desktop, &desktopRect);
			SetWindowPos (hwndDlg, NULL, (desktopRect.right-desktopRect.left-(windowRect.right-windowRect.left))/2,
				(desktopRect.bottom-desktopRect.top-(windowRect.bottom-windowRect.top))/2, 0, 0, SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);

			// List of windows to create

			// Get the tab client rect in screen
			RECT tabRect;
			GetClientRect (GetDlgItem (hwndDlg, IDC_TAB), &tabRect);
			tabRect.top += 30;
			tabRect.left += 5;
			tabRect.right -= 5;
			tabRect.bottom -= 5;
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.left);
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.right);

			// For each tab
			for (uint tab=0; tab<TAB_COUNT; tab++)
			{
				// Insert a tab
				TCITEM tabItem;
				tabItem.mask = TCIF_TEXT;
				tabItem.pszText = (LPTSTR)SubText[tab];
				SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_INSERTITEM, SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_GETITEMCOUNT, 0, 0), (LPARAM)&tabItem);

				// Create the dialog
				currentParam->SubDlg[tab] = CreateDialogParam (hInstance, MAKEINTRESOURCE(SubTab[tab]), GetDlgItem (hwndDlg, IDC_TAB), SubProc[tab], lParam);

				// To client coord
				RECT client = tabRect;
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.left);
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.right);

				// Resize and pos it
				SetWindowPos (currentParam->SubDlg[tab], NULL, client.left, client.top, client.right-client.left, client.bottom-client.top, SWP_NOOWNERZORDER|SWP_NOZORDER);
			}

			// Activate the last activated TAB
			SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_SETCURSEL, LastTabActivated, 0);
			// Show the last activated TAB
			ShowWindow (currentParam->SubDlg[LastTabActivated], SW_SHOW);
		}
		break;

		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch (pnmh->code)
				{
				case TCN_SELCHANGE:
					{

						uint curSel=SendMessage (pnmh->hwndFrom, TCM_GETCURSEL, 0, 0);
						for (uint tab=0; tab<TAB_COUNT; tab++)
						{
							ShowWindow (currentParam->SubDlg[tab], (tab == curSel)?SW_SHOW:SW_HIDE);
						}
						LastTabActivated= curSel;
						clamp(LastTabActivated, 0, TAB_COUNT-1);
						break;
					}
				}
				break;
			}
		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Ok in each other window
							for (uint tab=0; tab<TAB_COUNT; tab++)
							{
								// Send back an ok message
								SendMessage (currentParam->SubDlg[tab], uMsg, wParam, lParam);
							}

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:
		break;

		default:
		return FALSE;
	}
	return TRUE;
}

// ***************************************************************************

void CNelExport::OnNodeProperties (const std::set<INode*> &listNode)
{

	// Get
	uint nNumSelNode=(uint)listNode.size();

	if (nNumSelNode)
	{
		// Get the selected node
		INode* node=*listNode.begin();

		// Dialog box param
		CLodDialogBoxParam param;

		// Value of the properties
		param.ListNode=&listNode;
		param.ListActived=true;
		param.BlendIn=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED);
		param.BlendOut=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED);
		param.CoarseMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED);
		param.DynamicMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED);
		float floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
		param.DistMax=toStringMax (floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
		param.BlendLength=toStringMax (floatTmp);
		param.MRM=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED);
		param.SkinReduction=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT);

		int intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT);
		param.NbLod=toStringMax (intTmp);
		intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT);
		param.Divisor=toStringMax(intTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT);
		param.DistanceFinest=toStringMax(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT);
		param.DistanceMiddle=toStringMax(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT);
		param.DistanceCoarsest=toStringMax(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.f);
		param.BoneLodDistance=toStringMax(floatTmp);

		// Lod names
		int nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
		int name;
		for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
		{
			// Get a string
			std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
			if (!nameLod.empty())
			{
				param.ListLodName.push_back (nameLod);
			}
		}

		int accelFlag = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
		param.AcceleratorType = accelFlag&NEL3D_APPDATA_ACCEL_TYPE;
		param.ParentVisible = (accelFlag&NEL3D_APPDATA_ACCEL_FATHER_VISIBLE) ? BST_CHECKED : BST_UNCHECKED;
		param.VisibleFromParent = (accelFlag&NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER) ? BST_CHECKED : BST_UNCHECKED;
		param.DynamicPortal = (accelFlag&NEL3D_APPDATA_ACCEL_DYNAMIC_PORTAL) ? BST_CHECKED : BST_UNCHECKED;
		param.Clusterized = (accelFlag&NEL3D_APPDATA_ACCEL_CLUSTERIZED) ? BST_CHECKED : BST_UNCHECKED;
		param.AudibleLikeVisible = (accelFlag&NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE) ? BST_UNCHECKED : BST_CHECKED;
		param.FatherAudible = (accelFlag&NEL3D_APPDATA_ACCEL_FATHER_AUDIBLE) ? BST_CHECKED : BST_UNCHECKED;
		param.AudibleFromFather = (accelFlag&NEL3D_APPDATA_ACCEL_AUDIBLE_FROM_FATHER) ? BST_CHECKED : BST_UNCHECKED;
		param.OcclusionModel = CExportNel::getScriptAppData(node, NEL3D_APPDATA_OCC_MODEL, "no occlusion");
		param.OpenOcclusionModel = CExportNel::getScriptAppData(node, NEL3D_APPDATA_OPEN_OCC_MODEL, "no occlusion");
		param.SoundGroup = CExportNel::getScriptAppData(node, NEL3D_APPDATA_SOUND_GROUP, "no sound");
		param.EnvironmentFX = CExportNel::getScriptAppData(node, NEL3D_APPDATA_ENV_FX, "no fx");

		param.InstanceShape=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_SHAPE, "");
		param.InstanceName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "");
		param.DontAddToScene=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED);
		param.AutomaticAnimation=CExportNel::getScriptAppData (node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, BST_UNCHECKED);
		param.InstanceGroupName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "");
		param.InterfaceFileName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INTERFACE_FILE, "");
		param.InterfaceThreshold=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INTERFACE_THRESHOLD, 0.1f);
		param.GetInterfaceNormalsFromSceneObjects = CExportNel::getScriptAppData (node, NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS, 0);
		param.DontExport=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED);
		param.ExportNoteTrack=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED);
		param.ExportSSSTrack=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SSS_TRACK, BST_UNCHECKED);
		param.ExportAnimatedMaterials=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED);
		param.ExportNodeAnimation=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NODE_ANIMATION, BST_UNCHECKED);
		param.PrefixeTracksNodeName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME, BST_UNCHECKED);
		param.FloatingObject=CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED);
		param.LumelSizeMul=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0");
		param.SoftShadowRadius=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toStringMax(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT));
		param.SoftShadowConeLength=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toStringMax(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT));
		param.UseRemanence=CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_REMANENCE, BST_UNCHECKED);
		param.RemanenceShiftingTexture=CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE, BST_CHECKED);
		param.RemanenceSliceNumber=CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SLICE_NUMBER, NEL3D_APPDATA_ACCEL_DYNAMIC_PORTAL);
		param.RemanenceSamplingPeriod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD, 0.02f);
		param.RemanenceRollupRatio=CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO, 1.f);

		// Radial normals
		for (uint smoothGroup=0; smoothGroup<NEL3D_RADIAL_NORMAL_COUNT; smoothGroup++)
			param.RadialNormals[smoothGroup] = CExportNel::getScriptAppData (node, NEL3D_APPDATA_RADIAL_NORMAL_SM+smoothGroup, "");

		// Vegetable
		param.Vegetable = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED);
		param.VegetableAlphaBlend = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0);
		param.VegetableAlphaBlendOnLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffDoubleSided = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED);
		param.VegetableBendCenter = CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, 0);
		param.VegetableBendFactor = toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT));
		param.VegetableForceBestSidedLighting= CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING, BST_UNCHECKED);

		param.ExportLightMapName = CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED_LIGHT, NEL3D_APPDATA_LM_ANIMATED_LIGHT_DEFAULT);

		// Ligoscape
		param.LigoSymmetry = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, BST_UNCHECKED);
		param.LigoRotate = toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, 0));

		// Ligoscape
		param.SWT = CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, BST_UNCHECKED);
		param.SWTWeight = toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, 0.f));

		// RealTimeLigt.
		param.ExportRealTimeLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED);

		// LightmapLigt. (true by default)
		param.ExportLightMapLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, BST_CHECKED);

		// LightmapLigt. (true by default)
		param.ExportLightMapAnimated= CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, BST_UNCHECKED);

		param.LightGroup = CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_LIGHT_GROUP, 0);

		// ExportAsSunLight.
		param.ExportAsSunLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);

		// UseLightingLocalAttenuation
		param.UseLightingLocalAttenuation= CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, BST_UNCHECKED);

		// LightDontCastShadowInterior
		param.LightDontCastShadowInterior= CExportNel::getScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR, BST_UNCHECKED);

		// LightDontCastShadowExterior
		param.LightDontCastShadowExterior= CExportNel::getScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR, BST_UNCHECKED);

		// VertexProgram
		param.VertexProgramId= CExportNel::getScriptAppData (node, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);

		// Test wether Vertex program not bypass by material v.p
		NL3D::CMaterial::TShader dummyShader;
		param.VertexProgramBypassed = CExportNel::hasMaterialWithShaderForVP(*node, 0, dummyShader);
		if (param.VertexProgramBypassed)
		{
			param.UseLightingLocalAttenuation = false;
		}

		// VertexProgramWindTree
		CExportNel::getScriptAppDataVPWT(node, param.VertexProgramWindTree);


		// Collision
		param.Collision= CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		param.CollisionExterior= CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, BST_UNCHECKED);


		// ExportLodCharacter
		param.ExportLodCharacter= CExportNel::getScriptAppData (node, NEL3D_APPDATA_CHARACTER_LOD, BST_UNCHECKED);

		// Bone Scale
		param.ExportBoneScale= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE, BST_UNCHECKED);
		param.ExportBoneScaleNameExt= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE_NAME_EXT, "");


		// LightMap2
		param.LMCEnabled= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_ENABLED, BST_UNCHECKED);
		// must change the Size of APPDATA size, keeping old IDs.....
		nlctassert(CLodDialogBoxParam::NumLightGroup<NEL3D_APPDATA_EXPORT_LMC_MAX_LIGHT_GROUP);
		for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
		{
			param.LMCAmbient[i]= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START+i, CRGBA::Black);
			param.LMCDiffuse[i]= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START+i, CRGBA::White);
		}

		// CollisionMeshGeneration
		param.CollisionMeshGeneration=CExportNel::getScriptAppData (node, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0);

		// RealTimeAmbientLightAddSun
		param.RealTimeAmbientLightAddSun= CExportNel::getScriptAppData (node, NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN, BST_UNCHECKED);

		// Something selected ?
		std::set<INode*>::const_iterator ite=listNode.begin();
		ite++;
		while (ite!=listNode.end())
		{
			// Get the selected node
			node=*ite;

			// Get the properties
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED)!=param.BlendIn)
				param.BlendIn=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED)!=param.BlendOut)
				param.BlendOut=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED)!=param.CoarseMesh)
				param.CoarseMesh=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED)!=param.DynamicMesh)
				param.DynamicMesh=BST_INDETERMINATE;
			if (param.DistMax!=toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT)))
				param.DistMax=DIFFERENT_VALUE_STRING;
			if (param.BlendLength!=toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT)))
				param.BlendLength=DIFFERENT_VALUE_STRING;

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED)!=param.MRM)
				param.MRM=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT)!=param.SkinReduction)
				param.SkinReduction=-1;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT))!=param.NbLod)
				param.NbLod=DIFFERENT_VALUE_STRING;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT))!=param.Divisor)
				param.Divisor=DIFFERENT_VALUE_STRING;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT))!=param.DistanceFinest)
				param.DistanceFinest=DIFFERENT_VALUE_STRING;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT))!=param.DistanceMiddle)
				param.DistanceMiddle=DIFFERENT_VALUE_STRING;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT))!=param.DistanceCoarsest)
				param.DistanceCoarsest=DIFFERENT_VALUE_STRING;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.f))!=param.BoneLodDistance)
				param.BoneLodDistance=DIFFERENT_VALUE_STRING;

			int accelFlag = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
			if ( param.AcceleratorType != (accelFlag&NEL3D_APPDATA_ACCEL_TYPE) )
				param.AcceleratorType = -1;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_FATHER_VISIBLE) ? BST_CHECKED : BST_UNCHECKED) != param.ParentVisible)
				param.ParentVisible = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER) ? BST_CHECKED : BST_UNCHECKED) != param.VisibleFromParent)
				param.VisibleFromParent = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_DYNAMIC_PORTAL) ? BST_CHECKED : BST_UNCHECKED) != param.DynamicPortal)
				param.DynamicPortal = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_CLUSTERIZED) ? BST_CHECKED : BST_UNCHECKED) != param.Clusterized)
				param.Clusterized = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE) ? BST_UNCHECKED : BST_CHECKED) != param.AudibleLikeVisible)
				param.AudibleLikeVisible = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_FATHER_AUDIBLE) ? BST_CHECKED : BST_UNCHECKED) != param.FatherAudible)
				param.FatherAudible = BST_INDETERMINATE;
			if ( ((accelFlag&NEL3D_APPDATA_ACCEL_AUDIBLE_FROM_FATHER) ? BST_CHECKED : BST_UNCHECKED) != param.AudibleFromFather)
				param.AudibleFromFather = BST_INDETERMINATE;

			if (CExportNel::getScriptAppData(node, NEL3D_APPDATA_OCC_MODEL, "no occlusion") != param.OcclusionModel)
			{
				param.OcclusionModel = DIFFERENT_VALUE_STRING;
			}
			if ( CExportNel::getScriptAppData(node, NEL3D_APPDATA_OPEN_OCC_MODEL, "no occlusion") != param.OpenOcclusionModel)
			{
				param.OpenOcclusionModel = DIFFERENT_VALUE_STRING;
			}
			if (CExportNel::getScriptAppData(node, NEL3D_APPDATA_SOUND_GROUP, "no sound") != param.SoundGroup)
				param.SoundGroup = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData(node, NEL3D_APPDATA_ENV_FX, "no fx") != param.EnvironmentFX)
			{
				param.EnvironmentFX.clear();
			}

			if ( param.LightGroup != CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_LIGHT_GROUP, 0) )
				param.LightGroup = -1;

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_SHAPE, "")!=param.InstanceShape)
				param.InstanceShape = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "")!=param.InstanceName)
				param.InstanceName = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED)!=param.DontAddToScene)
				param.DontAddToScene = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, BST_UNCHECKED)!=param.AutomaticAnimation)
				param.AutomaticAnimation = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "")!=param.InstanceGroupName)
				param.InstanceGroupName = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INTERFACE_FILE, "")!=param.InterfaceFileName)
				param.InterfaceFileName.clear();
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INTERFACE_THRESHOLD, 0.1f)!=param.InterfaceThreshold)
				param.InterfaceThreshold = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS, 0)!=param.GetInterfaceNormalsFromSceneObjects)
				param.GetInterfaceNormalsFromSceneObjects = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED)!=param.DontExport)
				param.DontExport= BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED)!=param.ExportNoteTrack)
				param.ExportNoteTrack = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SSS_TRACK, BST_UNCHECKED)!=param.ExportSSSTrack)
				param.ExportSSSTrack = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED)!=param.ExportAnimatedMaterials)
				param.ExportAnimatedMaterials = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NODE_ANIMATION, BST_UNCHECKED)!=param.ExportNodeAnimation)
				param.ExportNodeAnimation = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME, BST_UNCHECKED)!=param.PrefixeTracksNodeName)
				param.PrefixeTracksNodeName = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED)!=param.FloatingObject)
				param.FloatingObject = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_REMANENCE, BST_UNCHECKED)!=param.UseRemanence)
				param.UseRemanence = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE, BST_UNCHECKED)!=param.RemanenceShiftingTexture)
				param.RemanenceShiftingTexture = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD, 0.01f)!=param.RemanenceSamplingPeriod)
				param.RemanenceSamplingPeriod = -1.f;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO, 1.f)!=param.RemanenceRollupRatio)
				param.RemanenceRollupRatio = -1.f;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_REMANENCE_SLICE_NUMBER, 64)!=param.RemanenceSliceNumber)
				param.RemanenceSliceNumber = -1;


			// Radial normals
			for (uint smoothGroup=0; smoothGroup<NEL3D_RADIAL_NORMAL_COUNT; smoothGroup++)
			{
				if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_RADIAL_NORMAL_SM+smoothGroup, "")!=param.RadialNormals[smoothGroup])
					param.RadialNormals[smoothGroup] = DIFFERENT_VALUE_STRING;
			}

			// Vegetable
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED)!=param.Vegetable)
				param.Vegetable = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0)!=param.VegetableAlphaBlend)
				param.VegetableAlphaBlend = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0)!=param.VegetableAlphaBlendOnLighted)
				param.VegetableAlphaBlendOnLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0)!=param.VegetableAlphaBlendOffLighted)
				param.VegetableAlphaBlendOffLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED)!=param.VegetableAlphaBlendOffDoubleSided)
				param.VegetableAlphaBlendOffDoubleSided = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, BST_UNCHECKED)!=param.VegetableBendCenter)
				param.VegetableBendCenter = -1;
			if (toStringMax(CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT))!=param.VegetableBendFactor)
				param.VegetableBendFactor = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING, BST_UNCHECKED)!=param.VegetableForceBestSidedLighting)
				param.VegetableForceBestSidedLighting = BST_INDETERMINATE;

			// Lightmap
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0")!=param.LumelSizeMul)
				param.LumelSizeMul = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toStringMax(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT))!=param.SoftShadowRadius)
				param.SoftShadowRadius = DIFFERENT_VALUE_STRING;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toStringMax(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT))!=param.SoftShadowConeLength)
				param.SoftShadowConeLength = DIFFERENT_VALUE_STRING;

			// Get name count for this node
			std::list<std::string> tmplist;
			nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
			for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
			{
				// Get a string
				std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
				if (!nameLod.empty())
				{
					tmplist.push_back (nameLod);
				}
			}

			// Compare with original list
			if (tmplist!=param.ListLodName)
			{
				// Not the same, so clear
				param.ListLodName.clear();
				param.ListActived=false;
			}

			// Ligoscape
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, BST_UNCHECKED) != param.LigoSymmetry)
				param.LigoSymmetry = BST_INDETERMINATE;
			if (toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, 0)) != param.LigoRotate)
				param.LigoRotate = DIFFERENT_VALUE_STRING;

			// SWT
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, BST_UNCHECKED) != param.SWT)
				param.SWT = BST_INDETERMINATE;
			if (toStringMax (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, 0.f)) != param.SWTWeight)
				param.SWTWeight = DIFFERENT_VALUE_STRING;

			// RealTimeLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED) != param.ExportRealTimeLight)
				param.ExportRealTimeLight= BST_INDETERMINATE;

			// ExportLightMapLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, BST_CHECKED) != param.ExportLightMapLight)
				param.ExportLightMapLight= BST_INDETERMINATE;

			// ExportLightMapAnimated
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, BST_UNCHECKED) != param.ExportLightMapAnimated)
				param.ExportLightMapAnimated= BST_INDETERMINATE;

			// ExportAsSunLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED) != param.ExportAsSunLight)
				param.ExportAsSunLight= BST_INDETERMINATE;

			// Lightmap name
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED_LIGHT, NEL3D_APPDATA_LM_ANIMATED_LIGHT_DEFAULT)!=param.ExportLightMapName)
			{
				param.ExportLightMapName = DIFFERENT_VALUE_STRING;
			}

			// UseLightingLocalAttenuation
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, BST_UNCHECKED) != param.UseLightingLocalAttenuation)
				param.UseLightingLocalAttenuation= BST_INDETERMINATE;

			// LightDontCastShadowInterior
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR, BST_UNCHECKED) != param.LightDontCastShadowInterior)
				param.LightDontCastShadowInterior= BST_INDETERMINATE;

			// LightDontCastShadowExterior
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR, BST_UNCHECKED) != param.LightDontCastShadowExterior)
				param.LightDontCastShadowExterior= BST_INDETERMINATE;


			// VertexProgram
			// simply disable VertexProgram edition of multiple selection...
			param.VertexProgramId= -1;

			// Collision
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED) != param.Collision)
				param.Collision= BST_INDETERMINATE;
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, BST_UNCHECKED) != param.CollisionExterior)
				param.CollisionExterior= BST_INDETERMINATE;

			// ExportLodCharacter
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_CHARACTER_LOD, BST_UNCHECKED) != param.ExportLodCharacter)
				param.ExportLodCharacter= BST_INDETERMINATE;

			// Bone Scale
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE, BST_UNCHECKED) != param.ExportBoneScale)
				param.ExportBoneScale= BST_INDETERMINATE;
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE_NAME_EXT, "") != param.ExportBoneScaleNameExt)
				param.ExportBoneScaleNameExt= DIFFERENT_VALUE_STRING;


			// LightMap2
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_ENABLED, BST_UNCHECKED) != param.LMCEnabled)
				param.LMCEnabled= BST_INDETERMINATE;
			// if not same RGBA, enter in DifferentValues mode
			for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
			{
				if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START+i, CRGBA::Black) != param.LMCAmbient[i])
					param.LMCAmbient[i].setDifferentValuesMode();
				if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START+i, CRGBA::White) != param.LMCDiffuse[i])
					param.LMCDiffuse[i].setDifferentValuesMode();
			}

			// CollisionMeshGeneration
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, 0)!=param.CollisionMeshGeneration)
				param.CollisionMeshGeneration = -1;

			// RealTimeAmbientLightAddSun
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN, BST_UNCHECKED) != param.RealTimeAmbientLightAddSun)
				param.RealTimeAmbientLightAddSun= BST_INDETERMINATE;

			// Next sel
			ite++;
		}

		if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NODE_PROPERTIES), _Ip->GetMAXHWnd(), LodDialogCallback, (LPARAM)&param)==IDOK)
		{
			// Next node
			ite=listNode.begin();
			while (ite!=listNode.end())
			{
				// Get the selected node
				node=*ite;

				// Ok pushed, copy returned params
				if (param.BlendIn!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, param.BlendIn);
				if (param.BlendOut!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, param.BlendOut);
				if (param.CoarseMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, param.CoarseMesh);
				if (param.DynamicMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, param.DynamicMesh);

				if (param.DistMax!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, param.DistMax);
				if (param.BlendLength!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, param.BlendLength);

				if (param.MRM!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_MRM, param.MRM);
				if (param.SkinReduction!=-1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, param.SkinReduction);
				if (param.NbLod!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, param.NbLod);
				if (param.Divisor!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, param.Divisor);
				if (param.DistanceFinest!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, param.DistanceFinest);
				if (param.DistanceMiddle!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, param.DistanceMiddle);
				if (param.DistanceCoarsest!=DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, param.DistanceCoarsest);
				if (param.BoneLodDistance!=DIFFERENT_VALUE_STRING)
				{
					float	f= toFloatMax(param.BoneLodDistance.c_str());
					f= std::max(0.f, f);
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BONE_LOD_DISTANCE, f);
				}

				int accelType = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, NEL3D_APPDATA_ACCEL_DEFAULT);
				if (param.AcceleratorType != -1)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_TYPE;
					accelType |= param.AcceleratorType&NEL3D_APPDATA_ACCEL_TYPE;
				}
				if (param.ParentVisible != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_FATHER_VISIBLE;
					accelType |= (param.ParentVisible == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_FATHER_VISIBLE : 0;
				}
				if (param.VisibleFromParent != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER;
					accelType |= (param.VisibleFromParent == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_VISIBLE_FROM_FATHER : 0;
				}
				if (param.DynamicPortal != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_DYNAMIC_PORTAL;
					accelType |= (param.DynamicPortal == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_DYNAMIC_PORTAL : 0;
				}
				if (param.Clusterized != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_CLUSTERIZED;
					accelType |= (param.Clusterized == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_CLUSTERIZED : 0;
				}
				if (param.AudibleLikeVisible != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE;
					accelType |= (param.AudibleLikeVisible == BST_CHECKED) ? 0 : NEL3D_APPDATA_ACCEL_AUDIBLE_NOT_LIKE_VISIBLE;
				}
				if (param.FatherAudible != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_FATHER_AUDIBLE;
					accelType |= (param.FatherAudible == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_FATHER_AUDIBLE : 0;
				}
				if (param.AudibleFromFather != BST_INDETERMINATE)
				{
					accelType &= ~NEL3D_APPDATA_ACCEL_AUDIBLE_FROM_FATHER;
					accelType |= (param.AudibleFromFather == BST_CHECKED) ? NEL3D_APPDATA_ACCEL_AUDIBLE_FROM_FATHER : 0;
				}
				CExportNel::setScriptAppData (node, NEL3D_APPDATA_ACCEL, accelType);

				if (param.OcclusionModel != DIFFERENT_VALUE_STRING)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_OCC_MODEL, param.OcclusionModel);
				}
				if (param.OpenOcclusionModel != DIFFERENT_VALUE_STRING)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_OPEN_OCC_MODEL, param.OpenOcclusionModel);
				}
				if (param.SoundGroup != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOUND_GROUP, param.SoundGroup);
				if (param.EnvironmentFX != "")
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ENV_FX, param.EnvironmentFX);
				}

				if ( (param.InstanceShape != DIFFERENT_VALUE_STRING) || (listNode.size()==1))
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INSTANCE_SHAPE, param.InstanceShape);
				if (param.InstanceName != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, param.InstanceName);
				if (param.DontAddToScene != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, param.DontAddToScene);
				if (param.AutomaticAnimation != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, param.AutomaticAnimation);
				if (param.InstanceGroupName != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_IGNAME, param.InstanceGroupName);
				if (param.InterfaceFileName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INTERFACE_FILE, param.InterfaceFileName);
				if (param.InterfaceThreshold != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INTERFACE_THRESHOLD, param.InterfaceThreshold);
				if (param.GetInterfaceNormalsFromSceneObjects != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS, param.GetInterfaceNormalsFromSceneObjects);

				if (param.LightGroup != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LM_LIGHT_GROUP, param.LightGroup);

				{
					if (param.DontExport != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, param.DontExport);
					if (param.ExportNoteTrack != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, param.ExportNoteTrack);
					if (param.ExportSSSTrack != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_SSS_TRACK, param.ExportSSSTrack);
					if (param.ExportAnimatedMaterials != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, param.ExportAnimatedMaterials);
					if (param.ExportNodeAnimation != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_NODE_ANIMATION, param.ExportNodeAnimation);
					if (param.PrefixeTracksNodeName != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME, param.PrefixeTracksNodeName);
				}

				// Radial normals
				for (uint smoothGroup=0; smoothGroup<NEL3D_RADIAL_NORMAL_COUNT; smoothGroup++)
				{
					if ( (param.RadialNormals[smoothGroup] != DIFFERENT_VALUE_STRING)  || (listNode.size()==1))
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_RADIAL_NORMAL_SM+smoothGroup, param.RadialNormals[smoothGroup]);
				}


				if (param.LumelSizeMul != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, param.LumelSizeMul);
				if (param.SoftShadowRadius != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, param.SoftShadowRadius);
				if (param.SoftShadowConeLength != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, param.SoftShadowConeLength);
				if (param.FloatingObject != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, param.FloatingObject);

				// Vegetable
				if (param.Vegetable != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE, param.Vegetable);
				if (param.VegetableAlphaBlend != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, param.VegetableAlphaBlend);
				if (param.VegetableAlphaBlendOnLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, param.VegetableAlphaBlendOnLighted);
				if (param.VegetableAlphaBlendOffLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, param.VegetableAlphaBlendOffLighted);
				if (param.VegetableAlphaBlendOffDoubleSided != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, param.VegetableAlphaBlendOffDoubleSided);
				if (param.VegetableBendCenter != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, param.VegetableBendCenter);
				if (param.VegetableBendFactor != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, param.VegetableBendFactor);
				if (param.VegetableForceBestSidedLighting != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_FORCE_BEST_SIDED_LIGHTING, param.VegetableForceBestSidedLighting);

				if (param.ListActived)
				{
					// Write size of the list
					uint sizeList=std::min ((uint)param.ListLodName.size(), (uint)NEL3D_APPDATA_LOD_NAME_COUNT_MAX);
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, (int)sizeList);

					// Write the strings
					uint stringIndex=0;
					std::list<std::string>::iterator ite=param.ListLodName.begin();
					while (ite!=param.ListLodName.end())
					{
						// Write the string
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME+stringIndex, *ite);
						stringIndex++;
						ite++;
					}
				}

				// Ligoscape
				if (param.LigoSymmetry != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, param.LigoSymmetry);
				if (param.LigoRotate != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, param.LigoRotate);

				// SWT
				if (param.SWT != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, param.SWT);
				if (param.SWTWeight != DIFFERENT_VALUE_STRING)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, param.SWTWeight);

				// RealTime Light.
				if (param.ExportRealTimeLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, param.ExportRealTimeLight);

				// ExportLightMapLight.
				if (param.ExportLightMapLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, param.ExportLightMapLight);

				// ExportLightMapAnimated.
				if (param.ExportLightMapAnimated != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, param.ExportLightMapAnimated);

				// ExportAsSunLight.
				if (param.ExportAsSunLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, param.ExportAsSunLight);

				// ExportLightMapName
				if ( (param.ExportLightMapName != DIFFERENT_VALUE_STRING) || (listNode.size()==1))
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED_LIGHT, param.ExportLightMapName);

				// UseLightingLocalAttenuation
				if (param.UseLightingLocalAttenuation != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, param.UseLightingLocalAttenuation);

				// LightDontCastShadowInterior
				if (param.LightDontCastShadowInterior != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_INTERIOR, param.LightDontCastShadowInterior);

				// LightDontCastShadowExterior
				if (param.LightDontCastShadowExterior != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LIGHT_DONT_CAST_SHADOW_EXTERIOR, param.LightDontCastShadowExterior);

				// VertexProgram
				if (param.VertexProgramId!=-1)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VERTEXPROGRAM_ID, param.VertexProgramId);
					// according to VertexProgram, change setup
					if(param.VertexProgramId==1)
					{
						CExportNel::setScriptAppDataVPWT(node, param.VertexProgramWindTree);
					}
				}

				// Collision
				if (param.Collision != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_COLLISION, param.Collision);
				if (param.CollisionExterior != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, param.CollisionExterior);

				// ExportLodCharacter
				if(param.ExportLodCharacter!= BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_CHARACTER_LOD, param.ExportLodCharacter);

				// Bone Scale
				if(param.ExportBoneScale!= BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE, param.ExportBoneScale);
				if ( (param.ExportBoneScaleNameExt != DIFFERENT_VALUE_STRING) || (listNode.size()==1) )
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_BONE_SCALE_NAME_EXT, param.ExportBoneScaleNameExt);

				// Remanence
				if (param.UseRemanence != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_USE_REMANENCE, param.UseRemanence);
				if (param.RemanenceShiftingTexture != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_REMANENCE_SHIFTING_TEXTURE, param.RemanenceShiftingTexture);
				if (param.RemanenceSliceNumber != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_REMANENCE_SLICE_NUMBER, param.RemanenceSliceNumber);
				if (param.RemanenceSamplingPeriod != -1.f)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_REMANENCE_SAMPLING_PERIOD, param.RemanenceSamplingPeriod);
				if (param.RemanenceRollupRatio != -1.f)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_REMANENCE_ROLLUP_RATIO, param.RemanenceRollupRatio);


				// LightMap2
				if(param.LMCEnabled!= BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_ENABLED, param.LMCEnabled);
				// for all lightmap compress terms
				for(uint i=0;i<CLodDialogBoxParam::NumLightGroup;i++)
				{
					if(!param.LMCAmbient[i].DifferentValues)
					{
						// force 255 to avoid dummy differences
						CRGBA	col= param.LMCAmbient[i];
						col.A= 255;
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_AMBIENT_START+i, col);
					}
					if(!param.LMCDiffuse[i].DifferentValues)
					{
						// force 255 to avoid dummy differences
						CRGBA	col= param.LMCDiffuse[i];
						col.A= 255;
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_LMC_DIFFUSE_START+i, col);
					}
				}

				// CollisionMeshGeneration
				if (param.CollisionMeshGeneration != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_CAMERA_COLLISION_MESH_GENERATION, param.CollisionMeshGeneration);

				// RealTimeAmbientLightAddSun.
				if (param.RealTimeAmbientLightAddSun != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_REALTIME_AMBIENT_ADD_SUN, param.RealTimeAmbientLightAddSun);

				// Next node
				ite++;
			}
		}
	}
}

// ***************************************************************************
