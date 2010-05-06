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

/* Copyright, 2002 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef LD_CONTINENTCFG_H
#define LD_CONTINENTCFG_H

// ---------------------------------------------------------------------------

#include <string>
#include "easy_cfg.h"

// ---------------------------------------------------------------------------

struct SContinentCfg : public IEasyCFG
{
	std::string LandFile;		// The .land file
	std::string LandDir;		// Directory where the zoneLigos and zoneBitmaps are (used for the .land)
	std::string DfnDir;			// Directory where to get georges dfn
	std::string GameElemDir;	// Directory where to get georges file (pipoti.plant)

	std::string LandBankFile;		// .smallbank
	std::string LandFarBankFile;	// .farbank
	std::string LandTileNoiseDir;	// displace
	std::string LandZoneWDir;		// .zoneW
	std::string OutIGDir;			// Where to put ig

	// =======================================================================
	
	SContinentCfg ();

	bool load (const std::string &filename);
	bool save (const std::string &filename);
	
};

#endif // LD_CONTINENTCFG_H