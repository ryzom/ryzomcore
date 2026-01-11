// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdafx.h"
#include "database.h"
#include "resource.h"
#include "client_config.h"

using namespace NLMISC;
using namespace std;

// ***************************************************************************
// Bitmaps
// ***************************************************************************

const CBitmapId BitmapId[BitmapCount] =
{
	{ IDB_DISPLAY },
	{ IDB_DISPLAY_DETAILS },
	{ IDB_DISPLAY_ADVANCED },
	{ IDB_CARD	 },
	{ IDB_GENERAL },
	{ IDB_NETWORK },
	{ IDB_SOUND	 },
	{ IDB_UPDATE },
	{ IDB_TOP_RIGHT },
	{ IDB_WELCOME },
};

// ***************************************************************************
// Pages
// ***************************************************************************

CPage Root;

// ***************************************************************************

CPage Pages[PageCount];

// ***************************************************************************

void CreateDataBase ()
{
	Pages[PageGeneral].init (PageGeneral, "uiConfigGeneral", true, BitmapGeneral, IDD_GENERAL, &Root);
	Pages[PageDisplay].init (PageDisplay, "uiConfigDisplay", true, BitmapDisplay, IDD_DISPLAY, &Root);
		Pages[PageDisplayDetails].init (PageDisplayDetails, "uiConfigDisplayDetails", false, BitmapDisplayDetails, IDD_DISPLAY_DETAILS, &Pages[PageDisplay]);
		Pages[PageDisplayAdvanced].init (PageDisplayAdvanced, "uiConfigDisplayAdvanced", false, BitmapDisplayAdvanced, IDD_DISPLAY_ADVANCED, &Pages[PageDisplay]);
	Pages[PageSound].init (PageSound, "uiConfigSound", true, BitmapSound, IDD_SOUND, &Root);
	Pages[PageDisplaySysInfo].init (PageDisplaySysInfo, "uiConfigDisplaySysInfo", true, BitmapGeneral, IDD_SYSTEM_INFO, &Root);
		Pages[PageDisplayOpenGLInfo].init (PageDisplayOpenGLInfo, "uiConfigDisplayOpenGLInfo", false, BitmapCard, IDD_DISPLAY_INFO, &Pages[PageDisplaySysInfo]);
		Pages[PageDisplayOpenD3DInfo].init (PageDisplayOpenD3DInfo, "uiConfigDisplayD3DInfo", false, BitmapCard, IDD_DISPLAY_D3D, &Pages[PageDisplaySysInfo]);
};

// ***************************************************************************

CPage::CPage ()
{
	Parent = NULL;
	ChildId = 0xffffffff;
}

// ***************************************************************************

void CPage::init (uint id, const char *name, bool bold, uint icon, uint resid, CPage *parent)
{
	PageId = id;
	Parent = parent;
	Icon = icon;
	ResId = resid;
	ChildId = (uint)parent->Children.size ();
	if (parent)
		parent->Children.push_back (this);
	Name = name;
	Bold = bold;
}

// ***************************************************************************

void CPage::select ()
{
	
}

// ***************************************************************************
// Config File default
// ***************************************************************************

NLMISC::CConfigFile ConfigFileDefault;

void LoadConfigFileDefault ()
{
	try
	{
		ConfigFileDefault.load (CONFIG_DEFAULT_FILE_NAME);
	}
	catch (Exception &e)
	{
		theApp.error (CI18N::get ("uiConfigErrorReadingTheFile")+" "CONFIG_FILE_NAME" : "+string (e.what ()));
	}
}

// ***************************************************************************
// Merge method
// ***************************************************************************

struct CMergeDescriptor
{
	const char		*Name;
	TMergeMethod	Method;
};

// ***************************************************************************

static const CMergeDescriptor MergeDescriptor[] =
{
	{ "DivideTextureSizeBy2", PreferSuperior },
	{ "LandscapeThreshold", PreferInferior },
	{ "MicroVeget", PreferFalse },
	{ "HDEntityTexture", PreferFalse },
	{ "Shadows", PreferFalse },
	{ "DisableDXTC", PreferFalse },
	{ "DisableVtxProgram", PreferTrue },
	{ "DisableVtxAGP", PreferTrue },
	{ "DisableTextureShdr", PreferTrue },
	{ "SoundOn", PreferFalse },
	{ "UseEax", PreferFalse },
	{ "MaxTrack", PreferSuperior },
};

// ***************************************************************************

TMergeMethod GetMergeMethod (const char *varName)
{
	const uint count = sizeof (MergeDescriptor) / sizeof (CMergeDescriptor);
	uint i;
	for (i=0; i<count; i++)
	{
		if (strcmp (MergeDescriptor[i].Name, varName) == 0)
			return MergeDescriptor[i].Method;
	}
	return PreferInferior;
}

// ***************************************************************************

const float QualityToLandscapeThreshold[QUALITY_STEP] =
{
	100.0f,
	200.0f,
	1000.0f,
	2000.0f,
};

// ***************************************************************************

const float QualityToZFar[QUALITY_STEP] =
{
	200,
	300,
	500,
	1000,
};

// ***************************************************************************

const float QualityToLandscapeTileNear[QUALITY_STEP] =
{
	20,
	30,
	50,
	100,
};

// ***************************************************************************

const int QualityToSkinNbMaxPoly[QUALITY_STEP] =
{
	15000,
	30000,
	50000,
	100000,
};

// ***************************************************************************
const int QualityToNbMaxSkeletonNotCLod[QUALITY_STEP] =
{
	10,
	15,
	20,
	30,
};


// ***************************************************************************

const int QualityToFxNbMaxPoly[QUALITY_STEP] =
{
	2500,
	5000,
	10000,
	20000,
};

// ***************************************************************************

