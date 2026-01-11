// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NLQT_CONFIGURATION_H
#define NLQT_CONFIGURATION_H
#include <nel/misc/types_nl.h>

// STL includes
#include <map>

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/misc/ucstring.h>
#include <nel/misc/singleton.h>

// Project includes
#include "callback.h"

namespace NLQT {

typedef CCallback<void, NLMISC::CConfigFile::CVar &> CConfigCallback;

/**
 * CConfiguration
 * \brief CConfiguration
 * \date 2010-02-05 15:44GMT
 * \author Jan Boon (Kaetemi)
 */
class CConfiguration : public NLMISC::CManualSingleton<CConfiguration> // singleton due to cconfigfile issues
{
public:
	CConfiguration();
	virtual ~CConfiguration();

	void init();
	void release();
	
	void updateUtilities();

	void setAndCallback(const std::string &varName, CConfigCallback configCallback);
	void setCallback(const std::string &varName, CConfigCallback configCallback);
	void dropCallback(const std::string &varName);
	
	float getValue(const std::string &varName, float defaultValue);
	double getValue(const std::string &varName, double defaultValue);
	int getValue(const std::string &varName, int defaultValue);
	std::string getValue(const std::string &varName, const std::string &defaultValue);
	ucstring getValue(const std::string &varName, const ucstring &defaultValue);
	bool getValue(const std::string &varName, bool defaultValue);
	NLMISC::CRGBA getValue(const std::string &varName, const NLMISC::CRGBA &defaultValue);
	NLMISC::CRGBA getValue(const NLMISC::CConfigFile::CVar &var, const NLMISC::CRGBA &defaultValue);
	
	inline NLMISC::CConfigFile &getConfigFile() { return m_ConfigFile; }

private:
	static void cbConfigCallback(NLMISC::CConfigFile::CVar &var);
	void cfcbLogFilter(NLMISC::CConfigFile::CVar &var);

private:
	CConfiguration(const CConfiguration &);
	CConfiguration &operator=(const CConfiguration &);

private:
	NLMISC::CConfigFile m_ConfigFile;
	std::map<std::string, CConfigCallback> m_ConfigCallbacks;
	
}; /* class CConfiguration */

} /* namespace NLQT */

#endif /* #ifndef NLQT_CONFIGURATION_H */

/* end of file */
