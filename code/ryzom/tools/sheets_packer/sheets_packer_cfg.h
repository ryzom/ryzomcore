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



#ifndef TL_SHEETS_PACKER_CFG_H
#define TL_SHEETS_PACKER_CFG_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
// std.
#include <string>


//---------------------------------------------------
// CClientConfig :
// Struct to manage a config file for the client.
//---------------------------------------------------
struct CClientConfig
{
	// the config file must be always be available
	NLMISC::CConfigFile		ConfigFile;

	void init (const std::string &configFileName);

	/// Data Path.
	std::vector<std::string>			DataPath;

	/// World sheet name
	std::string			WorldSheet;
	/// Path where to find .primitive files
	std::string			PrimitivesPath;
	/// Path where to create lmconts.packed
	std::string			OutputDataPath;


	std::string		LigoPrimitiveClass;

public:
	/// Constructor.
	CClientConfig();

	friend void setValues ();

	/// Serialize CFG.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// End process
	void release ();
};// CClientConfig //


////////////
// GLOBAL //
////////////
extern CClientConfig AppCfg;
extern const std::string ConfigFileName;

#endif // TL_SHEETS_PACKER_CFG_H

/* End of sheets_packer_cfg.h */
