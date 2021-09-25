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

#ifndef _BUILDER_CONFIG_H
#define _BUILDER_CONFIG_H


namespace NLMISC
{
	class CConfigFile;
}

class CBuilderConfig
{
public:
	std::string					CachePath;
	std::string					OutputPath;
	std::vector<std::string>	SearchPaths;
	std::vector<std::string>	CWMapList;
	std::string					CWMapCachePath;
	float						CamSpeed;
	bool						Fly;
	bool						HeightMapsAsTga;
	float						PixelPerMeter;
	float						RefineThreshold;
public:
	CBuilderConfig();
	void build(NLMISC::CConfigFile &cf);
};


#endif
