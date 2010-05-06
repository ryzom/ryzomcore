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

	/// Save the cfg file when exit the client ?
	bool			SaveConfig;
	/// Window position in windowed mode
	sint			PositionX;
	sint			PositionY;
	
	/// Window frequency
	uint			Frequency;
	/// Application start in a window or in fullscreen.
	bool			Windowed;
	/// Width for the Application.
	uint16			Width;
	/// Height for the Application.
	uint16			Height;
	/// Bit Per Pixel (only used in Fullscreen mode).
	uint16			Depth;
	/// Monitor Constrast [-1 ~ 1], default 0
	float			Contrast;
	/// Monitor Luminosity [-1 ~ 1], default 0
	float			Luminosity;
	/// Monitor Gamma [-1 ~ 1], default 0
	float			Gamma;

	/// Pre Data Path.
	std::vector<std::string>			PreDataPath;
	/// Data Path.
	std::vector<std::string>			DataPath;
	/// True if we want the packed sheet to be updated if needed
	bool			UpdatePackedSheet;
	/// Name of the scene to play.
	std::string			SceneName;
	/// Path for the Id file.
	std::string			IdFilePath;
	// Enable/disable Floating Point Exceptions
	bool			FPExceptions;


	std::string		LigoPrimitiveClass;
// TODO
	std::string		LanguageCode;

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
