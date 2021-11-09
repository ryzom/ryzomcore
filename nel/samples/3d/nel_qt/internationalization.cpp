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

#include <nel/misc/types_nl.h>
#include "internationalization.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/i18n.h>
#include <nel/misc/config_file.h>

// Project includes
#include "nel_qt_config.h"
#include "configuration.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

namespace {

class CI18NLoadProxyBuildInfo : public CI18N::ILoadProxy
{
public:
	CI18NLoadProxyBuildInfo() { }
	virtual ~CI18NLoadProxyBuildInfo() { }
	virtual void loadStringFile(const std::string &filename, ucstring &text)
	{
		CI18N::readTextFile(filename, text);
		text += ucstring("\n"
#if defined (NL_DEBUG)
#	if defined (NL_DEBUG_FAST)
			"COMPILE_MODE [NL_DEBUG (NL_DEBUG_FAST)]\n"
#	elif defined (NL_DEBUG_INSTRUMENT)
			"COMPILE_MODE [NL_DEBUG (NL_DEBUG_INSTRUMENT)]\n"
#	else
			"COMPILE_MODE [NL_DEBUG]\n"
#	endif
#elif defined (NL_RELEASE_DEBUG)
			"COMPILE_MODE [NL_RELEASE_DEBUG]\n"
#elif defined (NL_RELEASE)
			"COMPILE_MODE [NL_RELEASE]\n"
#endif
#if FINAL_VERSION
			"RELEASE_TYPE [FINAL_VERSION]\n"
#else
			"RELEASE_TYPE [DEVELOPER_VERSION]\n"
#endif
			"VERSION_NUMBER [" NLQT_VERSION "]\n");
	}
};

CI18NLoadProxyBuildInfo a_I18NLoadProxy;

} /* anonymous namespace */

CInternationalization::CInternationalization()
{

}

CInternationalization::~CInternationalization()
{
	
}

void CInternationalization::init(CConfiguration *configuration)
{
	//H_AUTO2

	// copy parameters
	m_Configuration = configuration;
	
	// check stuff we need
	nlassert(m_Configuration);

	// set the load proxy
	nlassert(!m_Callbacks.size());
	CI18N::setLoadProxy(&a_I18NLoadProxy);

	// set the language code
	m_Configuration->setAndCallback("LanguageCode",  CConfigCallback(this, &CInternationalization::cfcbLanguageCode));	
}

void CInternationalization::release()
{
	//H_AUTO2

	// i18n itself cannot be released, but is 'ok' since static
	m_Configuration->dropCallback("LanguageCode");

	// drop the load proxy	
	CI18N::setLoadProxy(NULL);
	nlassert(!m_Callbacks.size());

	// reset parameters
	m_Configuration = NULL;
}

/// Sets the language code, but does not store to the config file
void CInternationalization::setLanguageCode(const std::string &language)
{
	cfcbLanguageCode(language);
}

/// Load the language code that was stored in the config file
void CInternationalization::loadLanguageCode()
{
	cfcbLanguageCode(m_Configuration->getConfigFile().getVar("LanguageCode"));
}

/// Sets the language code and stores it to the config file
void CInternationalization::saveLanguageCode(const std::string &language)
{
	CConfigFile::CVar &var = m_Configuration->getConfigFile().getVar("LanguageCode");
	var.setAsString(language);
	cfcbLanguageCode(language);
}

void CInternationalization::enableCallback(CEmptyCallback incb)
{
	m_Callbacks.push_back(incb);
}

void CInternationalization::disableCallback(CEmptyCallback incb)
{
	std::vector<CEmptyCallback>::iterator it(m_Callbacks.begin()), end(m_Callbacks.end());
	for (; it != end; ++it) if (*it == incb) { m_Callbacks.erase(it); return; }
	nlassert(false);
}

void CInternationalization::cfcbLanguageCode(CConfigFile::CVar &var)
{
	cfcbLanguageCode(var.asString());
}

void CInternationalization::cfcbLanguageCode(const std::string &language)
{
	CI18N::load(language);
	std::vector<CEmptyCallback>::iterator it(m_Callbacks.begin()), end(m_Callbacks.end());
	for (; it != end; ++it) (*it)();
}

} /* namespace NLQT */

/* end of file */
