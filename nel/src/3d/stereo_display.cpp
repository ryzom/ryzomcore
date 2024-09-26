/**
 * \file stereo_display.cpp
 * \brief IStereoDisplay
 * \date 2013-06-27 16:29GMT
 * \author Jan Boon (Kaetemi)
 * IStereoDisplay
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Thibaut GIRKA (ThibG) <thib@sitedethib.com>
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

#include "std3d.h"
#include "nel/3d/stereo_display.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "nel/3d/stereo_ovr.h"
#include "nel/3d/stereo_ovr_04.h"
#include "nel/3d/stereo_libvr.h"
#include "nel/3d/stereo_debugger.h"

using namespace std;
// using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

IStereoDisplay::IStereoDisplay()
{
	
}

IStereoDisplay::~IStereoDisplay()
{
	
}

const char *IStereoDisplay::getLibraryName(CStereoDeviceInfo::TStereoDeviceLibrary library)
{
	static const char *nel3dName = "NeL 3D";
	static const char *ovrName = "Oculus SDK";
	static const char *libvrName = "LibVR";
	static const char *openhmdName = "OpenHMD";
	switch (library)
	{
	case CStereoDeviceInfo::NeL3D:
		return nel3dName;
	case CStereoDeviceInfo::OVR:
		return ovrName;
	case CStereoDeviceInfo::LibVR:
		return libvrName;
	case CStereoDeviceInfo::OpenHMD:
		return openhmdName;
	}
	nlerror("Invalid device library specified");
	return "<InvalidDeviceLibrary>";
}

void IStereoDisplay::listDevices(std::vector<CStereoDeviceInfo> &devicesOut)
{
#ifdef HAVE_LIBOVR
	CStereoOVR::listDevices(devicesOut);
#endif
#ifdef HAVE_LIBVR
	CStereoLibVR::listDevices(devicesOut);
#endif
#if !FINAL_VERSION
	CStereoDebugger::listDevices(devicesOut);
#endif
}

IStereoDisplay *IStereoDisplay::createDevice(const CStereoDeviceInfo &deviceInfo)
{
	return deviceInfo.Factory->createDevice();
}

void IStereoDisplay::releaseUnusedLibraries()
{
#ifdef HAVE_LIBOVR
	if (!CStereoOVR::isLibraryInUse())
		CStereoOVR::releaseLibrary();
#endif
}

void IStereoDisplay::releaseAllLibraries()
{
#ifdef HAVE_LIBOVR
	CStereoOVR::releaseLibrary();
#endif
}

} /* namespace NL3D */

/* end of file */
