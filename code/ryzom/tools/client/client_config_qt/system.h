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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <nel/misc/system_info.h>
#include "config.h"

namespace NL3D
{
class IDriver;
}

struct CVideoMode
{
	unsigned int widht;
	unsigned int height;
	unsigned int depth;
	unsigned int frequency;

	CVideoMode()
	{
		widht = 0;
		height = 0;
		depth = 0;
		frequency = 0;
	}

	bool operator==( CVideoMode &o )
	{
		if( ( o.widht == widht ) && ( o.height == height ) && ( o.depth == depth ) && ( o.frequency == frequency ) )
			return true;
		else
			return false;
	}
};

/**
 @brief Singleton class that holds the system information, configs, etc
*/
class CSystem
{
public:
	CSystem();
	~CSystem();

	static CSystem &GetInstance()
	{
		if( instance == 0 )
		{
			instance = new CSystem;
		}
		return *instance;
	}

	struct CSysInfo
	{
		std::string videoDevice;
		std::string videoDriverVersion;
		std::string osName;
		std::string cpuName;
		uint64 totalRAM;
	} sysInfo;

#ifdef WIN32
	struct CD3DInfo
	{
		std::string device;
		std::string driver;
		std::string driverVersion;
		std::vector< CVideoMode > modes;
	} d3dInfo;
#endif

	struct COpenGLInfo
	{
		std::string vendor;
		std::string renderer;
		std::string driverVersion;
		std::string extensions;
		std::vector< CVideoMode > modes;
	} openglInfo;

	CConfig config;

private:
	void GatherSysInfo();
#ifdef WIN32
	void GatherD3DInfo();
#endif
	void GatherOpenGLInfo();

	void GetVideoModes( std::vector< CVideoMode > &dst, NL3D::IDriver *driver ) const;

	static CSystem *instance;
};

#endif // SYSTEM_H
