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
#include <sstream>
#include <nel/3d/driver.h>
#include <nel/3d/dru.h>
#include <QtOpenGL/QGLWidget>

CSystem *CSystem::instance = NULL;

CSystem::CSystem()
{
	GatherSysInfo();
#ifdef WIN32
	GatherD3DInfo();
#endif
	GatherOpenGLInfo();
}

CSystem::~CSystem()
{
	instance = 0;
}


void CSystem::GatherSysInfo()
{
	std::string device;
	uint64 driver;

	if( NLMISC::CSystemInfo::getVideoInfo( device, driver ) )
	{
		sysInfo.videoDevice = device;

		//////////////////////////////////////////////////////////////
		// FIXME
		// This is taken from the original configuration tool, and
		// it generates the same *wrong* version number
		//////////////////////////////////////////////////////////////
		uint32 version = static_cast< uint32 >( driver & 0xffff );
		std::stringstream ss;

		ss << ( version / 1000 % 10 );
		ss << ".";
		ss << ( version / 100 % 10 );
		ss << ".";
		ss << ( version / 10 % 10 );
		ss << ".";
		ss << ( version % 10 );

		sysInfo.videoDriverVersion = ss.str();
		//////////////////////////////////////////////////////////////
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

#ifdef WIN32
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

			sint64 ver = adapter.DriverVersion;
			std::stringstream ss;
			ss << static_cast< uint16 >( ver >> 48 );
			ss << ".";
			ss << static_cast< uint16 >( ver >> 32 );
			ss << ".";
			ss << static_cast< uint16 >( ver >> 16 );
			ss << ".";
			ss << static_cast< uint16 >( ver & 0xFFFF );
			d3dInfo.driverVersion = ss.str();
		}

		GetVideoModes( d3dInfo.modes, driver );

		driver->release();
	}

	catch( NLMISC::Exception &e )
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

	NL3D::IDriver *driver = NULL;
	try
	{
		driver = NL3D::CDRU::createGlDriver();
		GetVideoModes( openglInfo.modes, driver );
		driver->release();
	}

	catch( NLMISC::Exception &e )
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
			mode.widht = itr->Width;
			mode.height = itr->Height;
			mode.frequency = itr->Frequency;

			dst.push_back( mode );
		}
	}
}