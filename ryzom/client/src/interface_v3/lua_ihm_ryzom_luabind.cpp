// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// to get rid of you_must_not_use_assert___use_nl_assert___read_debug_h_file messages
#include <cassert>
#ifdef assert
#undef assert
#endif

// Warning: cannot use namespace std,    when using luabind
#ifdef NL_OS_WINDOWS
#  ifndef NL_EXTENDED_FOR_SCOPE
#    undef for
#  endif
#endif

#ifdef NL_DEBUG
#	define assert(x) nlassert(x)
#else
#	define assert(x)
#endif

// Always use unique_ptr with ValyriaTear/luabind on Ubuntu 20,
// since the setting is not stored in build_information.hpp
#ifndef LUABIND_USE_CXX11
#define LUABIND_USE_CXX11
#endif

#include <luabind/luabind.hpp>
// in luabind > 0.6, LUABIND_MAX_ARITY is set to 10
#if LUABIND_MAX_ARITY == 10
#	include <luabind/operator.hpp>
// only luabind > 0.7 have version.hpp (file checked with build system)
#	ifdef HAVE_LUABIND_VERSION
#		include <luabind/version.hpp>
#	endif
#	ifndef LUABIND_VERSION
// luabind 0.7 doesn't define LUABIND_VERSION
#		define LUABIND_VERSION 700
#	endif
// luabind 0.6 doesn't define LUABIND_VERSION but LUABIND_MAX_ARITY is set to 5
#elif LUABIND_MAX_ARITY == 5
#	define LUABIND_VERSION 600
#else
#	pragma error("luabind version not recognized")
#endif

#include "lua_ihm_ryzom.h"
#include "nel/gui/lua_ihm_ucstring.h"

// The luabind part of CLuaIHMRyzom::RegisterRyzomFunctions, in a translation unit of its own: together
// with the rest of lua_ihm_ryzom.cpp and its headers, luabind overflows the internal structures of
// Visual C++ 6.0 (C1204)

using namespace NLMISC;
using namespace NLGUI;

#define LUABIND_FUNC(__func__) luabind::def(#__func__,    &__func__)

// ***************************************************************************
void CLuaIHMRyzom::registerLuabindFunctions(CLuaState &ls)
{
	lua_State *L = ls.getStatePointer();

	luabind::module(L)
	[
		LUABIND_FUNC(getDbProp),
		LUABIND_FUNC(getDbProp64),
		LUABIND_FUNC(setDbProp),
		LUABIND_FUNC(setDbProp64),
		LUABIND_FUNC(addDbProp),
		LUABIND_FUNC(delDbProp),
		LUABIND_FUNC(getDbRGBA),
		LUABIND_FUNC(setDbRGBA),
		LUABIND_FUNC(debugInfo),
		LUABIND_FUNC(rawDebugInfo),
		// through the plain function type: Visual C++ 6.0 keeps the default argument in the deduced type, and its
		// instantiation of def then clashes with that for sendMsgToServerUseItem (duplicate comdat)
		luabind::def("dumpCallStack", (void (*)(int)) &dumpCallStack),
		LUABIND_FUNC(getDefine),
		LUABIND_FUNC(setContextHelpText),
#ifdef RYZOM_LUA_UCSTRING
		luabind::def("messageBox", (void(*)(const CLuaUCString &)) &messageBox),
		luabind::def("messageBox", (void(*)(const CLuaUCString &, const std::string &)) &messageBox),
		luabind::def("messageBox", (void(*)(const CLuaUCString &, const std::string &, int caseMode)) &messageBox),
#endif
		luabind::def("messageBox", (void(*)(const std::string &)) &messageBox),
#ifdef RYZOM_LUA_UCSTRING
		luabind::def("messageBoxWithHelp", (void(*)(const CLuaUCString &)) &messageBoxWithHelp), // TODO: Lua UTF-8
		luabind::def("messageBoxWithHelp", (void(*)(const CLuaUCString &, const std::string &)) &messageBoxWithHelp), // TODO: Lua UTF-8
		luabind::def("messageBoxWithHelp", (void(*)(const CLuaUCString &, const std::string &, int caseMode)) &messageBoxWithHelp), // TODO: Lua UTF-8
#endif
		luabind::def("messageBoxWithHelp", (void(*)(const std::string &)) &messageBoxWithHelp),
		LUABIND_FUNC(replacePvpEffectParam),
		LUABIND_FUNC(secondsSince1970ToHour),
#ifdef RYZOM_BG_DOWNLOADER
		LUABIND_FUNC(pauseBGDownloader),
		LUABIND_FUNC(unpauseBGDownloader),
		LUABIND_FUNC(requestBGDownloaderPriority),
		LUABIND_FUNC(getBGDownloaderPriority),
#endif
		LUABIND_FUNC(loadBackground),
		LUABIND_FUNC(getPatchLastErrorMessage),
		LUABIND_FUNC(getPlayerSelectedSlot),
		LUABIND_FUNC(isInGame),
		LUABIND_FUNC(isPlayerSlotNewbieLand),
		LUABIND_FUNC(getSkillIdFromName),
		LUABIND_FUNC(getSkillLocalizedName),
		LUABIND_FUNC(getMaxSkillValue),
		LUABIND_FUNC(getBaseSkillValueMaxChildren),
		LUABIND_FUNC(getMagicResistChance),
		LUABIND_FUNC(getDodgeParryChance),
		LUABIND_FUNC(browseNpcWebPage),
		LUABIND_FUNC(clearHtmlUndoRedo),
		LUABIND_FUNC(getDynString),
		LUABIND_FUNC(isDynStringAvailable),
		LUABIND_FUNC(isFullyPatched),
		LUABIND_FUNC(getSheetType),
		LUABIND_FUNC(getSheetFamily),
		LUABIND_FUNC(getSheetName),
		LUABIND_FUNC(getFameIndex),
		LUABIND_FUNC(getFameName),
		LUABIND_FUNC(getFameDBIndex),
		LUABIND_FUNC(getFirstTribeFameIndex),
		LUABIND_FUNC(getNbTribeFameIndex),
		LUABIND_FUNC(getClientCfg),
		LUABIND_FUNC(sendMsgToServer),
		LUABIND_FUNC(sendMsgToServerPvpTag),
		LUABIND_FUNC(sendMsgToServerUseItem),
		LUABIND_FUNC(isGuildQuitAvailable),
		LUABIND_FUNC(sortGuildMembers),
		LUABIND_FUNC(getNbGuildMembers),
		LUABIND_FUNC(getGuildMemberName),
		LUABIND_FUNC(getGuildMemberGrade),
		LUABIND_FUNC(isR2Player),
		LUABIND_FUNC(getR2PlayerRace),
		LUABIND_FUNC(isR2PlayerMale),
		LUABIND_FUNC(getCharacterSheetSkel),
		LUABIND_FUNC(getSheetId),
		LUABIND_FUNC(getCharacterSheetRegionForce),
		LUABIND_FUNC(getCharacterSheetRegionLevel),
		LUABIND_FUNC(getRegionByAlias),
		LUABIND_FUNC(getGroundZ),
		LUABIND_FUNC(tell),
		LUABIND_FUNC(isRingAccessPointInReach),
		LUABIND_FUNC(updateTooltipCoords),
		LUABIND_FUNC(isCtrlKeyDown),
		LUABIND_FUNC(encodeURLUnicodeParam),
		LUABIND_FUNC(getPlayerLevel),
		LUABIND_FUNC(getPlayerVpa),
		LUABIND_FUNC(getPlayerVpb),
		LUABIND_FUNC(getPlayerVpc),
		LUABIND_FUNC(getTargetLevel),
		LUABIND_FUNC(getTargetForceRegion),
		LUABIND_FUNC(getTargetLevelForce),
		LUABIND_FUNC(getTargetSheet),
		LUABIND_FUNC(getTargetVpa),
		LUABIND_FUNC(getTargetVpb),
		LUABIND_FUNC(getTargetVpc),
		LUABIND_FUNC(isTargetNPC),
		LUABIND_FUNC(isTargetPlayer),
		LUABIND_FUNC(isTargetUser),
		LUABIND_FUNC(isPlayerInPVPMode),
		LUABIND_FUNC(isTargetInPVPMode)
	];
}

/* end of file */
