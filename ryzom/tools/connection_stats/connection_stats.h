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


#ifndef CONNECTION_STATS_H
#define CONNECTION_STATS_H

#include <nel/misc/types_nl.h>
#include <vector>
#include <string>


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CONNECTION_STATS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CONNECTION_STATS_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef LOG_ANALYSER_PLUGIN_EXPORTS
#define LOG_ANALYSER_PLUGIN_API __declspec(dllexport)
#else
#define LOG_ANALYSER_PLUGIN_API __declspec(dllimport)
#endif


LOG_ANALYSER_PLUGIN_API std::string getInfoString();
LOG_ANALYSER_PLUGIN_API bool doAnalyse( const std::vector<const char *>& vec, std::string& res, std::string& log );

#endif