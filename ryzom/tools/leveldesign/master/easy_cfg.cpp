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

#include "stdafx.h"
#include "easy_cfg.h"

#include "nel/misc/config_file.h"

using namespace NLMISC;
using namespace std;

// ---------------------------------------------------------------------------
IEasyCFG::IEasyCFG()
{
	cf = NULL;
	f = NULL;
}

// ---------------------------------------------------------------------------
IEasyCFG::~IEasyCFG()
{
	if (cf != NULL)
		delete cf;
	cf = NULL;
	if (f != NULL)
		fclose(f);
	f = NULL;
}

// ---------------------------------------------------------------------------
bool IEasyCFG::openRead (const std::string &filename)
{
	try 
	{
		FILE *fTemp = fopen (filename.c_str(), "rt");
		if (fTemp == NULL)
			return false;
		else
			fclose(fTemp);

		cf = new CConfigFile;
		cf->load (filename);

	}
	catch (Exception &)
	{
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
bool IEasyCFG::openWrite (const std::string &filename)
{
	f = fopen (filename.c_str(), "wt");
	if (f == NULL)
		return false;
	return true;
}

// ---------------------------------------------------------------------------
void IEasyCFG::close ()
{
	if (cf != NULL)
		delete cf;
	cf = NULL;
	if (f != NULL)
		fclose (f);
	f = NULL;
}

// ---------------------------------------------------------------------------
sint32 IEasyCFG::getInt (const string &sVarName)
{
	if (cf == NULL)
		return 0;
	try
	{
		CConfigFile::CVar &cv = cf->getVar(sVarName);
		return cv.asInt();
	}
	catch (Exception &)
	{
		return 0;
	}
}

// ---------------------------------------------------------------------------
void IEasyCFG::putInt (const string &sVarName, sint32 sVarValue)
{
	if (f == NULL)
		return;
	fprintf (f, "%s = %d;\n", sVarName.c_str(), sVarValue);
}

// ---------------------------------------------------------------------------
string IEasyCFG::getStr (const string &sVarName)
{
	if (cf == NULL)
		return string("");
	try
	{
		CConfigFile::CVar &cv = cf->getVar(sVarName);
		return cv.asString();
	}
	catch (Exception &)
	{
		return string("");
	}
}

// ---------------------------------------------------------------------------
void IEasyCFG::putStr (const string &sVarName, const string &sVarValue)
{
	if (f == NULL)
		return;
	fprintf (f, "%s = \"%s\";\n", sVarName.c_str(), sVarValue.c_str());
}

// ---------------------------------------------------------------------------
bool IEasyCFG::getBool (const string &sVarName)
{
	if (cf == NULL)
		return false;
	return (getStr (sVarName) == "true" ? true : false);
}

// ---------------------------------------------------------------------------
void IEasyCFG::putBool (const string &sVarName, bool sVarValue)
{
	if (f == NULL)
		return;
	if (sVarValue)
		fprintf (f, "%s = \"true\";\n", sVarName.c_str());
	else
		fprintf (f, "%s = \"false\";\n", sVarName.c_str());
}

// ---------------------------------------------------------------------------
void IEasyCFG::putCommentLine (const std::string &sComments)
{
	if (f == NULL)
		return;
	fprintf (f, "// %s\n", sComments.c_str());
}
