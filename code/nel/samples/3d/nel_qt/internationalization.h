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

#ifndef NLQT_INTERNATIONALIZATION_H
#define NLQT_INTERNATIONALIZATION_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/singleton.h>

// Project includes
#include "callback.h"
#include "configuration.h"

namespace NLQT {

/**
 * CInternationalization
 * \brief CInternationalization
 * \date 2010-02-05 17:29GMT
 * \author Jan Boon (Kaetemi)
 */
class CInternationalization : public NLMISC::CManualSingleton<CInternationalization> // singleton due to ci18n issues
{
public:
	CInternationalization();
	virtual ~CInternationalization();

	void init(CConfiguration *configuration);
	void release();
	
	/// Sets the language code, but does not store to the config file
	void setLanguageCode(const std::string &language);
	/// Load the language code that was stored in the config file
	void loadLanguageCode();
	/// Sets the language code and stores it to the config file
	void saveLanguageCode(const std::string &language);

	void enableCallback(CEmptyCallback incb);
	void disableCallback(CEmptyCallback incb);

private:
	void cfcbLanguageCode(NLMISC::CConfigFile::CVar &var);
	void cfcbLanguageCode(const std::string &language);

private:
	CConfiguration *m_Configuration;
	std::vector<CEmptyCallback> m_Callbacks;

private:
	CInternationalization(const CInternationalization &);
	CInternationalization &operator=(const CInternationalization &);
	
}; /* class CInternationalization */

} /* namespace NLQT */

#endif /* #ifndef NLQT_INTERNATIONALIZATION_H */

/* end of file */
