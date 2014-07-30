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


#include "stdligo.h"

#include "nel/ligo/zone_bank.h"

#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"

#ifdef NL_OS_WINDOWS
#ifndef NL_COMP_MINGW
#define NOMINMAX
#endif
#include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;


namespace NLLIGO
{

// ***************************************************************************
// CZoneBankElement
// ***************************************************************************

string CZoneBankElement::_NoCatTypeFound = STRING_NO_CAT_TYPE;

// ---------------------------------------------------------------------------
CZoneBankElement::CZoneBankElement()
{
	_SizeX = _SizeY = 0;
}

// ---------------------------------------------------------------------------
void CZoneBankElement::addCategory (const std::string &CatType, const std::string &CatValue)
{
	_CategoriesMap.insert(pair<string,string>(CatType, CatValue));
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getName ()
{
	return getCategory ("zone");
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getSize ()
{
	return getCategory ("size");
}

// ---------------------------------------------------------------------------
const string& CZoneBankElement::getCategory (const string &CatType)
{
	map<string,string>::iterator it = _CategoriesMap.find (CatType);
	if (it == _CategoriesMap.end())
		return _NoCatTypeFound;
	else
		return it->second;
}

// ---------------------------------------------------------------------------
void CZoneBankElement::convertSize()
{
	const string &sizeString =  getSize();
	string sTmp;
	uint32 i;

	for (i = 0; i < sizeString.size(); ++i)
	{
		if (sizeString[i] == 'x')
			break;
		else
			sTmp += sizeString[i];
	}
	fromString(sTmp, _SizeX);

	++i; sTmp = "";
	for (; i < sizeString.size(); ++i)
	{
		sTmp += sizeString[i];
	}
	fromString(sTmp, _SizeY);
}

// ---------------------------------------------------------------------------
void CZoneBankElement::serial (NLMISC::IStream &f)
{
	f.xmlPush ("LIGOZONE");

	sint version = 1;
	f.serialVersion (version);
	string check = "LIGOZONE";
	f.serialCheck (check);

	f.xmlPush ("CATEGORIES");
		f.serialCont (_CategoriesMap);
	f.xmlPop ();

	f.xmlPush ("MASK");
		f.serialCont (_Mask);
	f.xmlPop ();

	f.xmlPop ();

	convertSize();
}


// ---------------------------------------------------------------------------
void CZoneBankElement::setMask (const std::vector<bool> &mask, uint8 sizeX, uint8 sizeY)
{
	_SizeX = sizeX;
	_SizeY = sizeY;
	_Mask = mask;
}

// ***************************************************************************
// CZoneBank
// ***************************************************************************

// ---------------------------------------------------------------------------
void CZoneBank::debugSaveInit (CZoneBankElement &zbeTmp, const string &fileName)
{
	try
	{
		COFile fileOut;
		fileOut.open (fileName);
		COXml output;
		output.init (&fileOut);
		zbeTmp.serial (output);
	}
	catch (const Exception& /*e*/)
	{
	}

}

// ---------------------------------------------------------------------------
void CZoneBank::debugInit(const std::string &sPath) // \ todo trap remove this
{
	CZoneBankElement zbeTmp;
	zbeTmp.addCategory ("zone", "Zone001");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("material", "titFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone001.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();


	zbeTmp.addCategory ("zone", "Zone002");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("material", "titFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone002.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("zone", "Zone003");
	zbeTmp.addCategory ("size", "2x2");
	zbeTmp.addCategory ("material", "titFleur");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone003.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("zone", "Zone004");
	zbeTmp.addCategory ("size", "2x2");
	zbeTmp.addCategory ("material", "grozFleur");
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone004.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("zone", "Zone005");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone005.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();

	zbeTmp.addCategory ("zone", "Zone006");
	zbeTmp.addCategory ("size", "4x2");
	zbeTmp.addCategory ("material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (false);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (false);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone006.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "Zone007");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("material", "grozFleur");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone007.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "Zone008");
	zbeTmp.addCategory ("size", "2x2");
	zbeTmp.addCategory ("material", "prairie");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone008.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "Zone009");
	zbeTmp.addCategory ("size", "2x2");
	zbeTmp.addCategory ("material", "prairie");
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone009.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "Zone010");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("material", "prairie");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "Zone010.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT0");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "0");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT0.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT1");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "1");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT1.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT2");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "2");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT2.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT3");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "3");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT3.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT4");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "4");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT4.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT5");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "5");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT5.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT6");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "6");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT6.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT7");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "7");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT7.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "WT8");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "grozFleur-prairie");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "8");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "WT8.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT0");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "0");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT0.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT1");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "1");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT1.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT2");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "Flat");
	zbeTmp.addCategory ("transnum", "2");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT2.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT3");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "3");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT3.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT4");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "4");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT4.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT5");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerA");
	zbeTmp.addCategory ("transnum", "5");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT5.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT6");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "6");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT6.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT7");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "7");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT7.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

	zbeTmp.addCategory ("zone", "ZT8");
	zbeTmp.addCategory ("size", "1x1");
	zbeTmp.addCategory ("transname", "titFleur-grozFleur");
	zbeTmp.addCategory ("transtype", "CornerB");
	zbeTmp.addCategory ("transnum", "8");
	zbeTmp._Mask.push_back (true);
	_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(), zbeTmp));
	debugSaveInit (zbeTmp, sPath + "ZT8.ligozone");
	zbeTmp._CategoriesMap.clear ();
	zbeTmp._Mask.clear ();
	_ElementsMap.clear ();

}

// ---------------------------------------------------------------------------
void CZoneBank::reset ()
{
	_ElementsMap.clear ();
	_Selection.clear ();
}

#ifdef NL_OS_WINDOWS
// ---------------------------------------------------------------------------
bool CZoneBank::initFromPath(const std::string &sPathName, std::string &error)
{
	char sDirBackup[512];
	GetCurrentDirectory (512, sDirBackup);
	SetCurrentDirectory (sPathName.c_str());
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	hFind = FindFirstFile ("*.ligozone", &findData);

	while (hFind != INVALID_HANDLE_VALUE)
	{
		// If the name of the file is not . or .. then its a valid entry in the DataBase
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			if (!addElement (findData.cFileName, error))
				return false;
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	SetCurrentDirectory (sDirBackup);
	return true;
}
#endif // NL_OS_WINDOWS

// ---------------------------------------------------------------------------
bool CZoneBank::addElement (const std::string &elementName, std::string &error)
{
	try
	{
		CZoneBankElement zbeTmp;
		CIFile fileIn;
		if (fileIn.open (elementName))
		{
			CIXml input;
			input.init (fileIn);
			zbeTmp.serial (input);
			_ElementsMap.insert (pair<string,CZoneBankElement>(zbeTmp.getName(),zbeTmp));
			return true;
		}
		else
		{
			error = "Can't open file " + elementName;
		}
	}
	catch (const Exception& e)
	{
		error = "Error while loading ligozone "+elementName+" : "+e.what();
	}
	return false;
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoriesType (std::vector<std::string> &CategoriesType)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.begin();

		while (it != rZBE._CategoriesMap.end())
		{
			bool bFound = false;
			for (uint32 k = 0; k < CategoriesType.size(); ++k)
				if (it->first == CategoriesType[k])
				{
					bFound = true;
					break;
				}
			if (!bFound)
				CategoriesType.push_back (it->first);

			++it;
		}
		++itElt;
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.find (CategoryType);

		if (it != rZBE._CategoriesMap.end())
		{
			bool bFound = false;
			for (uint32 k = 0; k < CategoryValues.size(); ++k )
				if (it->second == CategoryValues[k])
				{
					bFound = true;
					break;
				}
			if (!bFound)
				CategoryValues.push_back (it->second);
		}
		++itElt;
	}
}

// ---------------------------------------------------------------------------
CZoneBankElement *CZoneBank::getElementByZoneName (const std::string &ZoneName)
{
	map<string,CZoneBankElement>::iterator it = _ElementsMap.find (ZoneName);
	if (it != _ElementsMap.end())
	{
		return &(it->second);
	}
	return NULL;
}

// ---------------------------------------------------------------------------
void CZoneBank::resetSelection ()
{
	_Selection.clear ();
}

// ---------------------------------------------------------------------------
void CZoneBank::addOrSwitch (const std::string &CategoryType, const std::string &CategoryValue)
{
	map<string,CZoneBankElement>::iterator itElt = _ElementsMap.begin();

	while (itElt != _ElementsMap.end())
	{
		CZoneBankElement &rZBE = itElt->second;

		map<string,string>::iterator it = rZBE._CategoriesMap.find (CategoryType);

		if (it != rZBE._CategoriesMap.end())
		{
			if (it->second == CategoryValue)
			{
				// Check if the element is not already present in the selection
				bool bFound = false;
				for (uint32 k = 0; k < _Selection.size(); ++k )
					if (&rZBE == _Selection[k])
					{
						bFound = true;
						break;
					}
				if (!bFound)
					_Selection.push_back (&rZBE);
			}
		}
		++itElt;
	}
}

// ---------------------------------------------------------------------------
void CZoneBank::addAndSwitch (const std::string &CategoryType, const std::string &CategoryValue)
{
	uint32 i, j;
	// And the selection with some constraints
	// All elements of the selection must have a catType and catValue equal to those given in parameters
	for (i = 0; i < _Selection.size(); ++i)
	{
		CZoneBankElement *pZBE = _Selection[i];
		bool bFound = false;

		map<string,string>::iterator it = pZBE->_CategoriesMap.find (CategoryType);
		if (it != pZBE->_CategoriesMap.end())
		{
			if (it->second == CategoryValue)
				bFound = true;
		}
		if (!bFound)
		{
			_Selection[i] = NULL; // Mark this item to be removed
		}
	}
	// Remove all unused items
	for (i = 0, j = 0; i < _Selection.size(); ++i)
	{
		if (_Selection[i] != NULL)
		{
			_Selection[j] = _Selection[i];
			++j;
		}
	}
	_Selection.resize (j);
}

// ---------------------------------------------------------------------------
void CZoneBank::getSelection (std::vector<CZoneBankElement*> &SelectedElements)
{
	SelectedElements = _Selection;
}

// ***************************************************************************

} // namespace NLLIGO
