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



#ifndef NL_DATABASE_H
#define NL_DATABASE_H

// ***************************************************************************
// Bitmaps
// ***************************************************************************

enum
{	
	BitmapDisplay = 0,
	BitmapDisplayDetails,
	BitmapDisplayAdvanced,
	BitmapCard,
	BitmapGeneral,
	BitmapNetwork,
	BitmapSound,
	BitmapUpdate,
	BitmapTopRight,
	BitmapWelcome,
	BitmapCount,
};

// ***************************************************************************

struct CBitmapId
{
public:
	uint		ResId;
};

// ***************************************************************************

extern const CBitmapId BitmapId[BitmapCount];

// ***************************************************************************
// Pages
// ***************************************************************************

enum
{
	PageGeneral=0,
	PageDisplay,
	PageDisplayDetails,
	PageDisplayAdvanced,
	PageSound,
	PageDisplaySysInfo,
	PageDisplayOpenGLInfo,
	PageDisplayOpenD3DInfo,
	PageCount
};

// ***************************************************************************

/**
 * Data page
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2003
 */
class CPage
{
public:

	/// Constructor
	CPage ();
	void init (uint id, const char *name, bool bold, uint icon, uint resid, CPage *parent);
	void select ();

	bool	Bold;
	uint	ChildId;
	uint	ResId;

	uint				PageId;
	std::string			Name;
	uint				Icon;
	CPage				*Parent;
	std::vector<CPage*>	Children;
};

// ***************************************************************************

extern CPage Pages[PageCount];

// ***************************************************************************

extern CPage Root;

// ***************************************************************************

void CreateDataBase ();

// ***************************************************************************
// Config File default
// ***************************************************************************

void LoadConfigFileDefault ();

extern NLMISC::CConfigFile ConfigFileDefault;

// ***************************************************************************
// Merge method
// ***************************************************************************

enum TMergeMethod
{
	PreferInferior = 0,	// Default
	PreferSuperior,
	PreferTrue,
	PreferFalse,
};

TMergeMethod GetMergeMethod (const char *varName);

// ***************************************************************************
// Quality
// ***************************************************************************

#define QUALITY_STEP 4
#define QUALITY_TEXTURE_STEP 3

extern const float QualityToLandscapeThreshold[QUALITY_STEP];
extern const float QualityToZFar[QUALITY_STEP];
extern const float QualityToLandscapeTileNear[QUALITY_STEP];
extern const int QualityToSkinNbMaxPoly[QUALITY_STEP];
extern const int QualityToNbMaxSkeletonNotCLod[QUALITY_STEP];
extern const int QualityToFxNbMaxPoly[QUALITY_STEP];

// ***************************************************************************

template<class T>
int GetQuality (const T *table, T value)
{
	if (table[0] < table[QUALITY_STEP-1])
	{
		uint i=0;
		while ((i<QUALITY_STEP) && (table[i]<value))
			i++;
		return i;
	}
	else
	{
		uint i=0;
		while ((i<QUALITY_STEP) && (table[i]>value))
			i++;
		return i;
	}
}

// ***************************************************************************

#endif // NL_DATABASE_H

/* End of database.h */
