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



#include "stdpch.h"
#include "user_agent.h"
#include "client_cfg.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_X86_64)
#define RYZOM_ARCH "x64"
#elif defined(HAVE_X86)
#define RYZOM_ARCH "x86"
#elif defined(HAVE_ARM)
#define RYZOM_ARCH "arm"
#else
#define RYZOM_ARCH "unknown"
#endif
#if defined(NL_OS_WINDOWS)
#define RYZOM_SYSTEM "windows"
#elif defined(NL_OS_MAC)
#define RYZOM_SYSTEM "mac"
#elif defined(NL_OS_UNIX)
#define RYZOM_SYSTEM "unix"
#else
#define RYZOM_SYSTEM "unknown"
#endif

std::string getUserAgent()
{
	return getUserAgentName() + "/" + getUserAgentVersion();
}

std::string getUserAgentName()
{
	return "Ryzom";
}

std::string getUserAgentVersion()
{
	static std::string s_userAgent;

	if (s_userAgent.empty())
	{
		s_userAgent = NLMISC::toString("%s-%s-%s", RYZOM_VERSION, RYZOM_SYSTEM, RYZOM_ARCH);
	}

	return s_userAgent;
}

std::string getVersion()
{
	return RYZOM_VERSION;
}

std::string getDisplayVersion()
{
	static std::string s_version;

	if (s_version.empty())
	{
#if FINAL_VERSION
		s_version = "FV ";
#else
		s_version = "DEV ";
#endif
		if (ClientCfg.ExtendedCommands) s_version += "_E";

		s_version += getVersion();
	}

	return s_version;
}

std::string getDebugVersion()
{
	static std::string s_version;

	if (s_version.empty())
	{
		s_version = getDisplayVersion();
#ifdef BUILD_DATE
		s_version += NLMISC::toString(" (%s)", BUILD_DATE);
#else
		s_version += NLMISC::toString(" (%s %s)", __DATE__, __TIME__);
#endif
	}

	return s_version;
}

bool isStereoAvailable()
{
#ifdef NL_STEREO_AVAILABLE
	return true;
#else
	return false;
#endif
}
