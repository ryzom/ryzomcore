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
#include "system.h"

#include <nel/3d/driver.h>
#include <nel/3d/dru.h>
#include <QtOpenGL/QGLWidget>

CSystem::CSystem()
{
	GatherSysInfo();
#ifdef Q_OS_WIN32
	GatherD3DInfo();
#endif
	GatherOpenGLInfo();
}

CSystem::~CSystem()
{
}

bool CSystem::parseDriverVersion(const std::string &device, uint64 driver, std::string &version)
{
	// file version
	uint32 version1 = driver >> 48;
	uint32 version2 = (driver >> 32) & 0xffff;
	uint32 version3 = (driver >> 16) & 0xffff;
	uint32 version4 = driver & 0xffff;

	if (device.find("NVIDIA") != std::string::npos)
	{
		// nvidia should be something like 9.18.13.2018 and 9.18.13.1422
		// which respectively corresponds to drivers 320.18 and 314.22
		uint32 nvVersionMajor = (version3 % 10) * 100 + (version4 / 100);
		uint32 nvVersionMinor = version4 % 100;

		version = NLMISC::toString("%u.%u", nvVersionMajor, nvVersionMinor);
	}
	else
	{
		version = NLMISC::toString("%u.%u.%u.%u", version1, version2, version3, version4);
	}

	return true;
}

void CSystem::GatherSysInfo()
{
	std::string device;
	uint64 driver;

	if( NLMISC::CSystemInfo::getVideoInfo( device, driver ) )
	{
		sysInfo.videoDevice = device;

		CSystem::parseDriverVersion(device, driver, sysInfo.videoDriverVersion);
	}
	else
	{
		sysInfo.videoDevice = "video card";
		sysInfo.videoDriverVersion = "0.0.0.0";
	}

	sysInfo.osName   = NLMISC::CSystemInfo::getOS();
	sysInfo.cpuName  = NLMISC::CSystemInfo::getProc();
	sysInfo.totalRAM = NLMISC::CSystemInfo::totalPhysicalMemory();
	sysInfo.totalRAM /= ( 1024 * 1024 );
}

#ifdef Q_OS_WIN32
void CSystem::GatherD3DInfo()
{
	NL3D::IDriver *driver = NULL;
	try
	{
		driver = NL3D::CDRU::createD3DDriver();

		NL3D::IDriver::CAdapter adapter;

		if( driver->getAdapter( 0xffffffff, adapter ) )
		{
			d3dInfo.device        = adapter.Description;
			d3dInfo.driver        = adapter.Driver;

			CSystem::parseDriverVersion(d3dInfo.device, adapter.DriverVersion, d3dInfo.driverVersion);
		}

		GetVideoModes( d3dInfo.modes, driver );

		driver->release();
	}

	catch(const NLMISC::Exception &e)
	{
		nlwarning( e.what() );
	}
}
#endif

void CSystem::GatherOpenGLInfo()
{
	QGLWidget *gl = new QGLWidget();

	gl->makeCurrent();

	const char *s = NULL;
	s = reinterpret_cast< const char * >( glGetString( GL_VENDOR ) );
	if( s != NULL )
		openglInfo.vendor.assign( s );

	s = reinterpret_cast< const char * >( glGetString( GL_RENDERER ) );
	if( s != NULL )
		openglInfo.renderer.assign( s );

	s = reinterpret_cast< const char * >( glGetString( GL_VERSION ) );
	if( s != NULL )
		openglInfo.driverVersion.assign( s );

	s = reinterpret_cast< const char * >( glGetString( GL_EXTENSIONS ) );
	if( s != NULL )
	{
		openglInfo.extensions.assign( s );
		for( std::string::iterator itr = openglInfo.extensions.begin(); itr != openglInfo.extensions.end(); ++itr )
			if( *itr == ' ' )
				*itr = '\n';
	}

	delete gl;

	try
	{
		NL3D::IDriver *driver = NL3D::CDRU::createGlDriver();
		GetVideoModes( openglInfo.modes, driver );
		driver->release();
	}
	catch(const NLMISC::Exception &e)
	{
		nlwarning( e.what() );
	}
}

void CSystem::GetVideoModes( std::vector< CVideoMode > &dst, NL3D::IDriver *driver ) const
{
	std::vector< NL3D::GfxMode > modes;
	driver->getModes( modes );

	for( std::vector< NL3D::GfxMode >::iterator itr = modes.begin(); itr != modes.end(); ++itr )
	{
		if( ( itr->Width >= 800 ) && ( itr->Height >= 600 ) && ( itr->Depth == 32 ) && ( itr->Frequency >= 60 ) )
		{
			CVideoMode mode;
			mode.depth = itr->Depth;
			mode.width = itr->Width;
			mode.height = itr->Height;
			mode.frequency = itr->Frequency;

			dst.push_back( mode );
		}
	}
}
