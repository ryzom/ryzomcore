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




#ifndef RY_AI_EVENT_REPORT_H
#define RY_AI_EVENT_REPORT_H

#include "game_share/action_nature.h"
#include "game_share/scores.h"
#include "game_share/mirror.h"


namespace AI_EVENT_REPORT
{
	enum TAIReportStat
	{		
		HitPoints = 0,
		Stamina,
		Sap,
		Unknown,		
	};

	/**
	  * get the right string from the given enum value
	  * \param jop the TAIReportStat value to convert
	  * \return the string associated to this enum number (Unknown if the enum number not exist)
	  */
	const std::string& toString( TAIReportStat stat );

	/**
	  * get the right TAIReportStat from its string
	  * \param str the input string
	  * \return the TAIReportStat associated to this string (unknown if the string cannot be interpreted)
	  */
	TAIReportStat toStat( const std::string& str );

	/**
	  * get the right TAIReportStat from a score
	  * \param score the score to convert
	  * \return the TAIReportStat associated to this score (unknown if the score is unknown)
	  */
	TAIReportStat scoreToStat( SCORES::TScores score );


} // AI_EVENT_REPORT


struct CAiEventReport
{
	CAiEventReport()
	{
//		AggroMul = 1.0;
		AggroAdd = 0.0f;
		Type = ACTNATURE::UNKNOWN;
	}

	inline void init()
	{
//		AggroMul = 1.0;
		AggroAdd = 0.0f;
		Type = ACTNATURE::UNKNOWN;
		AffectedStats.clear();
		DeltaValue.clear();
		Originator = TDataSetRow();
		Target = TDataSetRow();
	}

	inline void addDelta(AI_EVENT_REPORT::TAIReportStat stat, sint32 delta)
	{
		const uint size = (uint)AffectedStats.size();
		for (uint i = 0; i < size;  ++i)
		{
			if (AffectedStats[i] == stat)
			{
				DeltaValue[i] += delta;
				return;
			}
		}

		AffectedStats.push_back(stat);
		DeltaValue.push_back(delta);
	}

	/// Originator
	TDataSetRow					Originator;
	/// Target
	TDataSetRow					Target;
	/// the affected stats (Hp, sap...)
	std::vector<AI_EVENT_REPORT::TAIReportStat>	AffectedStats;
	/// the delta values
	std::vector<sint32>							DeltaValue;
	/// multiplier on target aggro
//	float						AggroMul;
	/// value to add to target creature aggro
	float						AggroAdd;
	/// type of action (beneficial, neutral, offensive)
	ACTNATURE::TActionNature	Type;
};



#endif // RY_AI_EVENT_REPORT_H //

