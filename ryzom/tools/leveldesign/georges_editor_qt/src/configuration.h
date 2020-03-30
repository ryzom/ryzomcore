/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <nel/misc/types_nl.h>

// STL includes
#include <map>

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/misc/ucstring.h>

// Project includes

#define NLQT_CONFIG_FILE "georges_editor.cfg"

namespace NLMISC {
	class IProgressCallback;
}

namespace NLQT {

	/**
	* CConfiguration
	* \brief CConfiguration
	* \date 2010-02-05 15:44GMT
	* \author Jan Boon (Kaetemi)
	*/
	class CConfiguration
	{
	public:
		CConfiguration();
		virtual ~CConfiguration();

		void init();
		void release();

		void updateUtilities();
		void configRemapExtensions();
		void addSearchPaths(std::vector<std::string>* list = 0);
		void addLeveldesignPath();

		void setProgressCallback(NLMISC::IProgressCallback *_progressCB);

		float getValue(const std::string &varName, float defaultValue);
		double getValue(const std::string &varName, double defaultValue);
		int getValue(const std::string &varName, int defaultValue);
		std::string getValue(const std::string &varName, const std::string &defaultValue);
		ucstring getValue(const std::string &varName, const ucstring &defaultValue);
		bool getValue(const std::string &varName, bool defaultValue);
		NLMISC::CRGBA getValue(const std::string &varName, const NLMISC::CRGBA &defaultValue);
		NLMISC::CRGBA getValue(const NLMISC::CConfigFile::CVar &var, const NLMISC::CRGBA &defaultValue);

		inline NLMISC::CConfigFile &getConfigFile() { return ConfigFile; }

	private:
		CConfiguration(const CConfiguration &);
		CConfiguration &operator=(const CConfiguration &);

		NLMISC::CConfigFile ConfigFile;

		NLMISC::IProgressCallback *_progressCB;

	};/* class CConfiguration */

} /* namespace NLQT */

#endif // CONFIGURATION_H
