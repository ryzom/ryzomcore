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


#define EXPORT_GET_ALLOCATOR

#include "PO2RPO.h"
#include "..\nel_patch_lib\rpo.h"

#define PBLOCK_REF	0


static RPO*			pRPO;
static ObjectState*	pOS;


PO2RPOClassDesc PO2RPODesc;
ClassDesc2* GetPO2RPODesc() {return &PO2RPODesc;}

enum { po2rpo_params };

//TODO: Add enums for various parameters
enum {	pb_spin,};

static ParamBlockDesc2 po2rpo_param_blk ( po2rpo_params, _T("params"),  0, &PO2RPODesc,
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF,
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_spin, 			_T("spin"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN,
		p_default, 		0.1f,
		p_range, 		0.0f,1000.0f,
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_EDIT,	IDC_SPIN, 0.01f,
		nl_p_end,
	nl_p_end
	);

IObjParam *PO2RPO::ip			= NULL;

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

PO2RPO::PO2RPO() : pblock(NULL)
{
	PO2RPODesc.MakeAutoParamBlocks(this);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

PO2RPO::~PO2RPO()
{
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

Interval PO2RPO::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
	{
		return NEVER;
	}
	//TODO: Return the validity interval of the modifier
	return NEVER;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

RefTargetHandle PO2RPO::Clone(RemapDir& remap)
{
	PO2RPO* newmod = new PO2RPO();
	//TODO: Add the cloning code here
	newmod->ReplaceReference(0,pblock->Clone(remap));
	return(newmod);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

static int done=0;
void PO2RPO::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
/*
	if (!done)
	{
		done=1;
	}
	else
	{
		return;
	}
*/
	// Note the default implementation: a class is considered to also be a subclass of itself.
	if (os->obj->IsSubClassOf(RYKOLPATCHOBJ_CLASS_ID))
	{
		pRPO = (RPO*)os->obj;
	}
	else
	{
		// Is our source is a MAX Patch Object ?
		nlassert(os->obj->IsSubClassOf(patchObjectClassID));

		// Create the RykolPatchObject
		pRPO=new RPO( *((PatchObject*)os->obj) );
		os->obj=pRPO;
		pRPO->rpatch->UpdateBinding (pRPO->patch, t);
	}
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------
extern HINSTANCE hInstance;
INT_PTR CALLBACK DlgProc_Panel(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// -----
		case WM_INITDIALOG:
		{
			// Get the module path
			HMODULE hModule = hInstance;
			if (hModule)
			{
				// Get module file name
				TCHAR moduldeFileName[512];
				if (GetModuleFileName (hModule, moduldeFileName, 512))
				{
					// Get version info size
					DWORD doomy;
					uint versionInfoSize=GetFileVersionInfoSize (moduldeFileName, &doomy);
					if (versionInfoSize)
					{
						// Alloc the buffer (size in bytes)
						uint8 *buffer = new uint8[versionInfoSize];

						// Find the verion resource
						if (GetFileVersionInfo(moduldeFileName, 0, versionInfoSize, buffer))
						{
							uint *versionTab;
							uint versionSize;
							if (VerQueryValue (buffer, _T("\\"), (void**)&versionTab,  &versionSize))
							{
								// Get the pointer on the structure
								VS_FIXEDFILEINFO *info=(VS_FIXEDFILEINFO*)versionTab;
								if (info)
								{
 									// Setup version number
									TCHAR version[512];
									_stprintf (version, _T("Version %d.%d.%d.%d"),
										info->dwFileVersionMS>>16,
										info->dwFileVersionMS&0xffff,
										info->dwFileVersionLS>>16,
										info->dwFileVersionLS&0xffff);
									SetWindowText (GetDlgItem (hWnd, IDC_VERSION), version);
								}
								else
									SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("VS_FIXEDFILEINFO * is NULL"));
							}
							else
								SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("VerQueryValue failed"));
						}
						else
							SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("GetFileVersionInfo failed"));

						// Free the buffer
						delete[] buffer;
					}
					else
						SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("GetFileVersionInfoSize failed"));
				}
				else
					SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("GetModuleFileName failed"));
			}
			else
				SetWindowText (GetDlgItem (hWnd, IDC_VERSION), _T("hInstance NULL"));
		}

		// -----
		case WM_COMMAND:
			/*switch( LOWORD(wParam) )
			{
			// ---
			default:
				break;
			}*/
			break;
		// -----
		default:
			return FALSE;
	};
	return TRUE;
}


void PO2RPO::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	hRollup = ip->AddRollupPage(hInstance,MAKEINTRESOURCE(IDD_PANEL),DlgProc_Panel,_T("PO2RPO"),(LPARAM)this);

	//PO2RPODesc.BeginEditParams(ip, this, flags, prev);
}

void PO2RPO::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	//PO2RPODesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(hRollup);
	this->ip = NULL;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

//From ReferenceMaker
RefResult PO2RPO::NotifyRefChanged(NOTIFY_REF_PARAMS)
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

//From Object
BOOL PO2RPO::HasUVW()
{
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

void PO2RPO::SetGenUVW(BOOL sw)
{
	if (sw==HasUVW())
	{
		return;
	}
	//TODO: Set the plugin internal value to sw
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------------

//#define CHK_TEST	0x0001
//static int counter;

IOResult PO2RPO::Save(ISave *isave)
{
/*
	char	string[64];
	ULONG	n,m;
	//TODO: Add code to allow plugin to save its data
	counter++;
	sprintf(string,"PO2RPO String Test %03d",counter);
	MessageBox(NULL,"save chunk","",MB_OK);
	isave->BeginChunk(CHK_TEST);
	n=strlen(string)+1;
	isave->Write(&n,4,&m);
	isave->Write(string,strlen(string)+1,&m);
	isave->EndChunk();
*/
	return IO_OK;
}

IOResult PO2RPO::Load(ILoad *iload)
{
/*
	char	string[64];
	ULONG	n,m;
	//TODO: Add code to allow plugin to load its data
	while( iload->OpenChunk()!=IO_END )
	{
		switch( iload->CurChunkID() )
		{
		case 0x0001:
			MessageBox(NULL,"this is a ryzom patch mesh !","",MB_OK);
			iload->Read(&n,4,&m);
			iload->Read(string,n,&m);
			break;
		default:
			break;
		}
		iload->CloseChunk();
	}
*/
	return IO_OK;
}
