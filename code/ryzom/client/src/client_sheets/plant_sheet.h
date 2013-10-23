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



#ifndef CL_PLANT_SHEET_H
#define CL_PLANT_SHEET_H

#include "game_share/season.h"
#include "entity_sheet.h"

/**
 * Class to manage sheet of per season fxs
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CSeasonFXSheet
{
public:
	enum TMode { AlwaysStarted = 0, UseEndHour, UseDuration, Spawn };
public:
	TMode		Mode;
	std::string FXName;
	float		CycleDuration;
	float		StartHourMin;
	float		StartHourMax;
	float		EndHourMin;
	float		EndHourMax;
	float		MinDuration;
	float		MaxDuration;
	float		UserParamsMin[4];
	float		UserParamsMax[4];
	bool		InheritScale;
	bool		InheritRot;
	// for Micro-life primitives only : gives min & max angle of landscape normal toward K for which fx can be spawned
	float				AngleMin;
	float				AngleMax;
	bool				DontRotate;
	bool				DontRotateAroundLocalZ;
	NLMISC::CVector		ScaleMin;
	NLMISC::CVector		ScaleMax;
	bool				UniformScale;
	bool				WantScaling;
	bool				AlignOnWater;
	float				ZOffset;
public:
	CSeasonFXSheet()
	{
		Mode = AlwaysStarted;
		CycleDuration = 24.f;
		StartHourMin = 0.f;
		StartHourMax = 0.f;
		EndHourMin = 0.f;
		EndHourMax = 0.f;
		std::fill(UserParamsMin, UserParamsMin + 4, 0.f);
		std::fill(UserParamsMax, UserParamsMax + 4, 0.f);
		MinDuration = 0.f;
		MaxDuration = 0.f;
		InheritScale = false;
		InheritRot = false;
		AngleMin = 0.f;
		AngleMax = 70.f;
		DontRotate = false;
		DontRotateAroundLocalZ =false;
		ScaleMin.set(1.f, 1.f, 1.f);
		ScaleMax.set(1.f, 1.f, 1.f);
		UniformScale = true;
		WantScaling = false;
		AlignOnWater = false;
		ZOffset	= 0.f;
	}
	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item, NLMISC::CSheetId parentId, const std::string &prefix);
	/// Serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

/**
 * Class to manage plant sheets
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CPlantSheet : public CEntitySheet
{
public:
	///\name Object
	//@{
		/// ctor
		CPlantSheet();
		/// Build the sheet from an external script.
		virtual void build(const NLGEORGES::UFormElm &item);
		/// Serialize plant sheet into binary data file.
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}

	// Get the shape name of this plant
	const std::string	&getShapeName() const { return _ShapeName; }
	float				 getMaxDist() const { return _MaxDist; }
	float				 getCoarseMeshDist() const { return _CoarseMeshDist; }
	// get fx infos for the given season
	const CSeasonFXSheet &getFXSheet(EGSPD::CSeason::TSeason season) const
	{
		nlassert(season < EGSPD::CSeason::Invalid);
		return SeasonFX[season];
	}
private:
	std::string  _ShapeName;
	float        _MaxDist;
	float        _CoarseMeshDist;
	// fx spawned on a given date and season
	CSeasonFXSheet SeasonFX[EGSPD::CSeason::Invalid];
};



#endif
