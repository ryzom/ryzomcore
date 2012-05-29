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

#ifndef RY_SKILLS_H
#define RY_SKILLS_H

#include "nel/misc/types_nl.h"

#include <string.h>

// NbSkills in enum : 225 Report this in database.xls
// BRIANCODE - adding an "any" enum AFTER NUM_SKILLS so I can check if "any" skill is level X or higher (for example)

namespace SKILLS
{
	enum ESkills
	{
		SC = 0,
		SCA,
		SCAH,
		SCAHB,
		SCAHBE,
		SCAHBEM,
		SCAHG,
		SCAHGE,
		SCAHGEM,
		SCAHH,
		SCAHHE,
		SCAHHEM,
		SCAHP,
		SCAHPE,
		SCAHPEM,
		SCAHS,
		SCAHSE,
		SCAHSEM,
		SCAHV,
		SCAHVE,
		SCAHVEM,
		SCAL,
		SCALB,
		SCALBE,
		SCALBEM,
		SCALG,
		SCALGE,
		SCALGEM,
		SCALP,
		SCALPE,
		SCALPEM,
		SCALS,
		SCALSE,
		SCALSEM,
		SCALV,
		SCALVE,
		SCALVEM,
		SCAM,
		SCAMB,
		SCAMBE,
		SCAMBEM,
		SCAMG,
		SCAMGE,
		SCAMGEM,
		SCAMP,
		SCAMPE,
		SCAMPEM,
		SCAMS,
		SCAMSE,
		SCAMSEM,
		SCAMV,
		SCAMVE,
		SCAMVEM,
		SCAS,
		SCASB,
		SCASBE,
		SCASBEM,
		SCASS,
		SCASSE,
		SCASSEM,
		SCJ,
		SCJA,
		SCJAA,
		SCJAAE,
		SCJAAEM,
		SCJB,
		SCJBA,
		SCJBAE,
		SCJBAEM,
		SCJD,
		SCJDA,
		SCJDAE,
		SCJDAEM,
		SCJE,
		SCJEA,
		SCJEAE,
		SCJEAEM,
		SCJP,
		SCJPA,
		SCJPAE,
		SCJPAEM,
		SCJR,
		SCJRA,
		SCJRAE,
		SCJRAEM,
		SCM,
		SCM1,
		SCM1A,
		SCM1AE,
		SCM1AEM,
		SCM1D,
		SCM1DE,
		SCM1DEM,
		SCM1M,
		SCM1ME,
		SCM1MEM,
		SCM1P,
		SCM1PE,
		SCM1PEM,
		SCM1S,
		SCM1SE,
		SCM1SEM,
		SCM1T,
		SCM1TE,
		SCM1TEM,
		SCM2,
		SCM2A,
		SCM2AE,
		SCM2AEM,
		SCM2M,
		SCM2ME,
		SCM2MEM,
		SCM2P,
		SCM2PE,
		SCM2PEM,
		SCM2S,
		SCM2SE,
		SCM2SEM,
		SCMC,
		SCMCA,
		SCMCAE,
		SCMCAEM,
		SCR,
		SCR1,
		SCR1P,
		SCR1PE,
		SCR1PEM,
		SCR2,
		SCR2A,
		SCR2AE,
		SCR2AEM,
		SCR2L,
		SCR2LE,
		SCR2LEM,
		SCR2R,
		SCR2RE,
		SCR2REM,
		SF,
		SFM,
		SFM1,
		SFM1B,
		SFM1BM,
		SFM1BMM,
		SFM1BS,
		SFM1BSM,
		SFM1P,
		SFM1PS,
		SFM1PSM,
		SFM1S,
		SFM1SA,
		SFM1SAM,
		SFM1SS,
		SFM1SSM,
		SFM2,
		SFM2B,
		SFM2BM,
		SFM2BMM,
		SFM2P,
		SFM2PP,
		SFM2PPM,
		SFM2S,
		SFM2SA,
		SFM2SAM,
		SFM2SS,
		SFM2SSM,
		SFMC,
		SFMCA,
		SFMCAD,
		SFMCADM,
		SFMCAH,
		SFMCAHM,
		SFR,
		SFR1,
		SFR1A,
		SFR1AP,
		SFR1APM,
		SFR2,
		SFR2A,
		SFR2AA,
		SFR2AAM,
		SFR2AL,
		SFR2ALM,
		SFR2AR,
		SFR2ARM,
		SH,
		SHF,
		SHFD,
		SHFDA,
		SHFDAE,
		SHFDAEM,
		SHFF,
		SHFFA,
		SHFFAE,
		SHFFAEM,
		SHFJ,
		SHFJA,
		SHFJAE,
		SHFJAEM,
		SHFL,
		SHFLA,
		SHFLAE,
		SHFLAEM,
		SHFP,
		SHFPA,
		SHFPAE,
		SHFPAEM,
		SM,
		SMD,
		SMDA,
		SMDAA,
		SMDAAE,
		SMDAAEM,
		SMDH,
		SMDHA,
		SMDHAE,
		SMDHAEM,
		SMO,
		SMOA,
		SMOAA,
		SMOAAE,
		SMOAAEM,
		SMOE,
		SMOEA,
		SMOEAE,
		SMOEAEM,

		NUM_SKILLS,
		unknown,
		any,
	};

	/**
	 * get the right skill enum from the input string
	 * \param str the input string
	 * \return the ESkills associated to this string (Unknown if the string cannot be interpreted)
	 */
	ESkills	toSkill ( const std::string &str );

	/**
	 * get the right skill string from the gived enum
	 * \param skill the skill to convert
	 * \return the string associated to this enum number (Unknown if the enum number not exist)
	 */
	const std::string& toString( uint16 skill );

	/**
	 * get the skill category name
	 * \param s is the enum number
	 * \return the string name of skill type (Unknown if the enum number not exist)
	 */
	const std::string& getSkillCategoryName( uint16 s );

}; // SKILLS

#endif // RY_SKILLS_H
/* End of skills.h */
