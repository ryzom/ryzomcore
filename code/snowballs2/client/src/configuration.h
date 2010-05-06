/**
 * \file configuration.h
 * \brief CConfiguration
 * \date 2008-11-26 14:12GMT
 * \author Jan Boon (Kaetemi)
 * CConfiguration
 */

/* 
 * Copyright (C) 2008  by authors
 * 
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef SBCLIENT_CONFIGURATION_H
#define SBCLIENT_CONFIGURATION_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/misc/ucstring.h>

// Project includes

namespace SBCLIENT {

/**
 * \brief CConfiguration
 * \date 2008-11-06 16:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfiguration
 */
class CConfiguration
{
public:
	static bool init();
	static bool release();

	static void updateUtilities();

	static void setAndCallback(const std::string &varName, void (*cb)(NLMISC::CConfigFile::CVar &var));
	static void dropCallback(const std::string &varName);

	static float getValue(const std::string &varName, float defaultValue);
	static double getValue(const std::string &varName, double defaultValue);
	static int getValue(const std::string &varName, int defaultValue);
	static std::string getValue(const std::string &varName, const std::string &defaultValue);
	static ucstring getValue(const std::string &varName, const ucstring &defaultValue);
	static bool getValue(const std::string &varName, bool defaultValue);
	static NLMISC::CRGBA getValue(const std::string &varName, const NLMISC::CRGBA &defaultValue);
	static NLMISC::CRGBA getValue(const NLMISC::CConfigFile::CVar &var, const NLMISC::CRGBA &defaultValue);
	
}; /* class CConfiguration */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_CONFIGURATION_H */

/* end of file */
