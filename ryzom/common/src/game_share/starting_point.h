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



#ifndef RY_STARTING_POINT_H
#define RY_STARTING_POINT_H


// strating point for new created character
namespace RYZOM_STARTING_POINT
{
	enum TStartPoint
	{
		// Matis start village
		matis_start,
			stalli = matis_start,
			borea,
			nistia,
			rosilio,
			miani,

		// zorai start village
		zorai_start,
			qai_lo = zorai_start,
			sheng_wo,
			men_xing,
			koi_zun,
			yin_piang,

		// fyros start village
		fyros_start,
			aegus = fyros_start,
			kaemon,
			sekovix,
			phyxon,
			galemus,

		// tryker start village
		tryker_start,
			aubermouth = tryker_start,
			barkdell,
			hobwelly,
			waverton,
			dingleton,

		// New NewbieLand start village
		starting_city,


		NB_START_POINTS	,
		Unknown = NB_START_POINTS
	};

	/**
	  * get the right string from the given enum value
	  * \param faber_type the TStartPoint value to convert
	  * \return the string associated to this enum number (UNKNOWN if the enum number not exist)
	  */
	const std::string& toString( TStartPoint start_point );

	/**
	  * get the right TStartPoint from its string
	  * \param str the input string
	  * \return the TStartPoint associated to this string (UNKNOWN if the string cannot be interpreted)
	  */
	TStartPoint toStartPoint( const std::string& str );
}

#endif // RY_STARTING_POINT_H
/* End of starting_point.h */
