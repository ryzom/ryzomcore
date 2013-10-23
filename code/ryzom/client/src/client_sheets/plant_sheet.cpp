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




//////////////
// INCLUDES //
//////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// Application
#include "plant_sheet.h"
// Nel
#include "nel/georges/u_form_elm.h"
#include "nel/misc/common.h"


using namespace NLMISC;

//=======================================================
void CSeasonFXSheet::build(const NLGEORGES::UFormElm &item, NLMISC::CSheetId parentId, const std::string &prefix)
{
	bool ok = true;
 	ok &= item.getValueByName(FXName, (prefix + "FXName").c_str());
	if (FXName.empty()) return;
	uint32 fxMode = 0;
	ok &= item.getValueByName(fxMode, (prefix + "Mode").c_str());
	Mode = (TMode) fxMode;
	ok &= item.getValueByName(CycleDuration, (prefix + "CycleDuration").c_str());
	ok &= item.getValueByName(StartHourMin, (prefix + "StartHourMin").c_str());
	ok &= item.getValueByName(StartHourMax, (prefix + "StartHourMax").c_str());
	ok &= item.getValueByName(EndHourMin, (prefix + "EndHourMin").c_str());
	ok &= item.getValueByName(EndHourMax, (prefix + "EndHourMax").c_str());
	ok &= item.getValueByName(InheritScale, (prefix + "InheritScale").c_str());
	ok &= item.getValueByName(InheritRot, (prefix + "InheritRot").c_str());
	ok &= item.getValueByName(AngleMin, (prefix + "AngleMin").c_str());
	ok &= item.getValueByName(AngleMax, (prefix + "AngleMax").c_str());
	ok &= item.getValueByName(DontRotate, (prefix + "DontRotate").c_str());
	ok &= item.getValueByName(DontRotateAroundLocalZ, (prefix + "DontRotateAroundLocalZ").c_str());
	ok &= item.getValueByName(ScaleMin.x, (prefix + "ScaleMinX").c_str());
	ok &= item.getValueByName(ScaleMin.y, (prefix + "ScaleMinY").c_str());
	ok &= item.getValueByName(ScaleMin.z, (prefix + "ScaleMinZ").c_str());
	ok &= item.getValueByName(ScaleMax.x, (prefix + "ScaleMaxX").c_str());
	ok &= item.getValueByName(ScaleMax.y, (prefix + "ScaleMaxY").c_str());
	ok &= item.getValueByName(ScaleMax.z, (prefix + "ScaleMaxZ").c_str());
	ok &= item.getValueByName(UniformScale, (prefix + "UniformScale").c_str());
	ok &= item.getValueByName(WantScaling, (prefix + "WantScaling").c_str());
	ok &= item.getValueByName(AlignOnWater, (prefix + "AlignOnWater").c_str());
	ok &= item.getValueByName(ZOffset, (prefix + "ZOffset").c_str());

	// Anti Crash
	if(CycleDuration<=0)
	{
		nlwarning("FX: CPlantSheet '%s' has a season with a CycleDuration<=0: %f !!!!! setting it to 0.1.",
			parentId.toString().c_str(), CycleDuration);
		CycleDuration= 0.1f;
	}

	StartHourMin = fmodf(StartHourMin, CycleDuration);
	StartHourMax = fmodf(StartHourMax, CycleDuration);
	EndHourMin   = fmodf(EndHourMin, CycleDuration);
	EndHourMax   = fmodf(EndHourMax, CycleDuration);

	nlassert(isValidDouble(StartHourMin));
	nlassert(isValidDouble(StartHourMax));
	nlassert(isValidDouble(EndHourMin));
	nlassert(isValidDouble(EndHourMax));
	nlassert(isValidDouble(CycleDuration));

	for(uint k = 0; k < 4; ++k)
	{
		ok &= item.getValueByName(UserParamsMin[k], (prefix + NLMISC::toString("UserParam%dMin", (int) k)).c_str());
		ok &= item.getValueByName(UserParamsMax[k], (prefix + NLMISC::toString("UserParam%dMax", (int) k)).c_str());
	}
	ok &= item.getValueByName(MinDuration, (prefix + "MinDuration").c_str());
	ok &= item.getValueByName(MaxDuration, (prefix + "MaxDuration").c_str());
	// prevent from overlapping interval
	if (Mode != AlwaysStarted && Mode != UseDuration && Mode != Spawn)
	{
		float startHourMin = StartHourMin;
		float startHourMax = StartHourMax;
		if (startHourMax < startHourMin)
		{
			startHourMax += CycleDuration;
		}
		float endHourMin = EndHourMin;
		float endHourMax = EndHourMax;
		if (endHourMax < endHourMin)
		{
			endHourMax += CycleDuration;
		}
		if (!(startHourMax <= endHourMin || startHourMin >= endHourMax))
		{
			// intervals overlap -> bad
			nlwarning("Overlapping time intervals for fx spawn.");
			if (startHourMin <= endHourMin)
			{
				float inter = endHourMin;
				if (inter >= CycleDuration)
				{
					inter -= CycleDuration;
				}
				StartHourMax = inter;
			}
			else
			{
				float inter = startHourMin;
				if (inter >= CycleDuration)
				{
					inter -= CycleDuration;
				}
				EndHourMax = inter;
			}
		}
	}
	// compute duration of start interval
	float startHourMaxInterval;
	if (StartHourMin <= StartHourMax)
	{
		startHourMaxInterval = StartHourMax - StartHourMin;
	}
	else
	{
		startHourMaxInterval = CycleDuration - StartHourMin + StartHourMax;
	}
	NLMISC::clamp(MinDuration, 0.f, CycleDuration /*- startHourMaxInterval*/);
	NLMISC::clamp(MaxDuration, 0.f, CycleDuration /*- startHourMaxInterval*/);

	if (!ok)
	{
		nldebug("Key not found.");
	}
}

//=======================================================
void CSeasonFXSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(FXName);
	f.serialEnum(Mode);
	f.serial(CycleDuration);
	f.serial(StartHourMin);
	f.serial(StartHourMax);
	f.serial(EndHourMin);
	f.serial(EndHourMax);
	f.serial(MinDuration);
	f.serial(MaxDuration);
	for(uint k = 0; k < 4; ++k)
	{
		f.serial(UserParamsMin[k]);
		f.serial(UserParamsMax[k]);
	}
	f.serial(InheritScale);
	f.serial(InheritRot);
	f.serial(AngleMin);
	f.serial(AngleMax);
	f.serial(DontRotate);
	f.serial(DontRotateAroundLocalZ);
	f.serial(ScaleMin);
	f.serial(ScaleMax);
	f.serial(UniformScale);
	f.serial(WantScaling);
	f.serial(AlignOnWater);
	f.serial(ZOffset);


	// Anti Crash. you may recompute the plant.packed_sheets if those assert trigger.
	nlassert(isValidDouble(CycleDuration));
	nlassert(isValidDouble(StartHourMin));
	nlassert(isValidDouble(StartHourMax));
	nlassert(isValidDouble(EndHourMin));
	nlassert(isValidDouble(EndHourMax));
	nlassert(isValidDouble(MinDuration));
	nlassert(isValidDouble(MaxDuration));
	nlassert(isValidDouble(UserParamsMin[0]));
	nlassert(isValidDouble(UserParamsMax[0]));
	nlassert(isValidDouble(UserParamsMin[1]));
	nlassert(isValidDouble(UserParamsMax[1]));
	nlassert(isValidDouble(UserParamsMin[2]));
	nlassert(isValidDouble(UserParamsMax[2]));
	nlassert(isValidDouble(UserParamsMin[3]));
	nlassert(isValidDouble(UserParamsMax[3]));
	nlassert(isValidDouble(AngleMin));
	nlassert(isValidDouble(AngleMax));
	nlassert(isValidDouble(ScaleMin.x) && isValidDouble(ScaleMin.y) && isValidDouble(ScaleMin.z));
	nlassert(isValidDouble(ScaleMax.x) && isValidDouble(ScaleMax.y) && isValidDouble(ScaleMax.z));
	nlassert(isValidDouble(ZOffset));
}

//=======================================================
CPlantSheet::CPlantSheet() : _MaxDist(-1), _CoarseMeshDist(-1)
{
	Type = PLANT;

	_MaxDist = 0.0f;
	_CoarseMeshDist = 0.0f;
}

//=======================================================
void CPlantSheet::build(const NLGEORGES::UFormElm &item)
{
	if(!(item.getValueByName(_ShapeName, "3D.Shape") &&
		 item.getValueByName(_MaxDist, "3D.MaxDist") &&
		 item.getValueByName(_CoarseMeshDist, "3D.CoarseMeshDist")))
	{
		nldebug("Key not found.");
	}

	// serial fxs by season
	SeasonFX[EGSPD::CSeason::Spring].build(item, Id, "3D.SpringFX.");
	SeasonFX[EGSPD::CSeason::Summer].build(item, Id, "3D.SummerFX.");
	SeasonFX[EGSPD::CSeason::Autumn].build(item, Id, "3D.AutomnFX.");
	SeasonFX[EGSPD::CSeason::Winter].build(item, Id, "3D.WinterFX.");
}

//=======================================================
void CPlantSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(_ShapeName, _MaxDist, _CoarseMeshDist);
	for(uint k = 0; k < EGSPD::CSeason::Invalid; ++k)
	{
		f.serial(SeasonFX[k]);
	}
}










































