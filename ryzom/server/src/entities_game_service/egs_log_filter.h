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

#ifndef RY_EGS_LOG_FILTER_H
#define RY_EGS_LOG_FILTER_H

#include "nel/net/cvar_log_filter.h"

// Filter to allow control of log by the config file
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_nminfo,  NameManagerLogEnabled,          true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_giinfo,  GameItemLogEnabled,             true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_ecinfo,  EntityCallbacksLogEnabled,      true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_eminfo,  EntityManagerLogEnabled,        true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_gminfo,  GuildManagerLogEnabled,         true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_feinfo,  ForageExtractionLogEnabled,     true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_pminfo,  PhraseManagerLogEnabled,        true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_chinfo,  CharacterLogEnabled,            true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_plinfo,  PlayerLogEnabled,               true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_shinfo,  ShoppingLogEnabled,             true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_pvpinfo, PVPLogEnabled,                  true)
NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_ppdinfo, PersistentPlayerDataLogEnabled, true)




#endif
