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
#include <nel/misc/i18n.h>

// Project includes
#include "snowballs_client.h"
#include "snowballs_config.h"
#include "configuration.h"

using namespace std;
using namespace NLMISC;

namespace SBCLIENT {

class _CI18NLoadProxy : public CI18N::ILoadProxy
{
public:
	_CI18NLoadProxy() { }
	virtual ~_CI18NLoadProxy() { }
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
#	if defined (NL_NO_DEBUG)
			"COMPILE_MODE [NL_RELEASE (NL_NO_DEBUG)]\n"
#	else
			"COMPILE_MODE [NL_RELEASE]\n"
#	endif
#endif
#if FINAL_VERSION
			"RELEASE_TYPE [FINAL_VERSION]\n"
#else
			"RELEASE_TYPE [DEVELOPER_VERSION]\n"
#endif
			"VERSION_NUMBER [" SBCLIENT_VERSION "]\n");
	}
};

static _CI18NLoadProxy _I18NLoadProxy;
static std::vector<void (*)()> _Callbacks;

void CInternationalization::enableCallback(void (*cb)())
{
	_Callbacks.push_back(cb);
}

void CInternationalization::disableCallback(void (*cb)())
{
	std::vector<void (*)()>::iterator it(_Callbacks.begin()), end(_Callbacks.end());
	for (; it != end; ++it) if (*it == cb) { _Callbacks.erase(it); return; }
	nlassert(false);
}

static void cbLanguageCode(CConfigFile::CVar &var)
{
	CI18N::load(var.asString());
	std::vector<void (*)()>::iterator it(_Callbacks.begin()), end(_Callbacks.end());
	for (; it != end; ++it) (*it)();
}

void CInternationalization::init()
{
	// check stuff we need
	nlassert(ConfigFile);

	// set the load proxy
	nlassert(!_Callbacks.size());
	CI18N::setLoadProxy(&_I18NLoadProxy);

	// set the language code
	CConfiguration::setAndCallback("LanguageCode", cbLanguageCode);	
}

void CInternationalization::release()
{
	// i18n itself cannot be released, but is 'ok' since static
	CConfiguration::dropCallback("LanguageCode");

	// drop the load proxy	
	CI18N::setLoadProxy(NULL);
	nlassert(!_Callbacks.size());
}

} /* namespace SBCLIENT */

/* end of file */
