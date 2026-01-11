/**
 * \file NeLResourceProvider.h
 * \date January 2005
 * \author Matt Raykowski
 * \author Henri Kuuste
 */

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


/************************************************************************
	purpose:	Interface for main Nevrax Engine GUI renderer class

	For use with GUI Library:
	Crazy Eddie's GUI System (http://crayzedsgui.sourceforge.net)
    Copyright (C)2004 Paul D Turner (crayzed@users.sourceforge.net)

	This file contains code that is specific to NeL (http://www.nevrax.org)
*************************************************************************/

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif

// standard includes
#include <set>
#include <list>

// CEGUI includes
#include <nel/cegui/nelresourceprovider.h>
#include "CEGUIExceptions.h"
#include "CEGUILogger.h"
#include <memory.h>

// NeL includes
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/mem_stream.h> 

// Start of CEGUI namespace section
namespace CEGUI
{
	NeLResourceProvider::NeLResourceProvider()
	{
		; // nothing to be done here.
	}

	NeLResourceProvider::~NeLResourceProvider()
	{
		; // nothing to be done here.
	}

	void NeLResourceProvider::loadRawDataContainer(const String &filename, RawDataContainer &output, const String& resourceGroup)
	{
		if(!NLMISC::CPath::exists(filename.c_str()))
		{
			nlinfo("Scheme::Scheme - Filename supplied for Scheme loading must be valid [%s]",filename.c_str());
			String sMsg=(utf8*)"Scheme::Scheme - Filename supplied for Scheme loading must be valid";
			sMsg+=(utf8*)" ["+filename+(utf8*)"]";
			throw InvalidRequestException(sMsg);
		}

		uint32 input_size;
		std::string fname=NLMISC::CPath::lookup(filename.c_str());
		NLMISC::CIFile f(NLMISC::CPath::lookup(filename.c_str()));

		// get the size of the file and create a temp container.
		input_size=f.getFileSize();
		uint8 *input=new uint8[input_size+1];
		input[input_size]=0;
		f.serialBuffer(input,input_size);
		f.close();

		output.setData(input);
		output.setSize(input_size);
	}
}; // end namespace CEGUI
