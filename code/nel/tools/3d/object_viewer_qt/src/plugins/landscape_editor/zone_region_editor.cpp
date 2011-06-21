// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

// Project includes
#include "zone_region_editor.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/o_xml.h>

// Qt includes
#include <QtGui/QMessageBox>

namespace LandscapeEditor
{

ZoneRegionEditor::ZoneRegionEditor()
{
	m_fileName = "";
}

ZoneRegionEditor::~ZoneRegionEditor()
{
}

bool ZoneRegionEditor::load(const std::string &fileName)
{
	bool result = true;
	try
	{
		// Open it
		NLMISC::CIFile fileIn;
		if (fileIn.open(fileName))
		{
			NLMISC::CIXml xml(true);
			xml.init(fileIn);
			m_zoneRegion.serial(xml);
		}
		else
		{
			nlwarning("Can't open file %s for reading", fileName.c_str());
			result = false;
		}
	}
	catch (NLMISC::Exception& e)
	{
		nlwarning("Error reading file %s : %s", fileName.c_str(), e.what ());
		result = false;
	}
	if (result)
		m_fileName = fileName;
	return result;
}

bool ZoneRegionEditor::save()
{
	if (m_fileName.empty())
		return false;

	bool result = true;
	// Save the landscape
	try
	{

		// Open file for writing
		NLMISC::COFile fileOut;
		if (fileOut.open(m_fileName, false, false, true))
		{
			// Be careful with the flushing of the COXml object
			{
				NLMISC::COXml xmlOut;
				xmlOut.init(&fileOut);
				m_zoneRegion.serial(xmlOut);
				// Done
				m_modified = false;
			}
			fileOut.close();
		}
		else
		{
			nlwarning("Can't open file %s for writing.", m_fileName.c_str());
			result = false;
		}
	}
	catch (NLMISC::Exception& e)
	{
		nlwarning("Error writing file %s : %s", m_fileName.c_str(), e.what());
		result = false;
	}
	return result;
}

void ZoneRegionEditor::setFileName(const std::string &fileName)
{
	m_fileName = fileName;
}

NLLIGO::CZoneRegion &ZoneRegionEditor::zoneRegion()
{
	return m_zoneRegion;
}

} /* namespace LandscapeEditor */
