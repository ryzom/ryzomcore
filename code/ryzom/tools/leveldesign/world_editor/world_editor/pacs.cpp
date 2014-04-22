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

#include "pacs.h"
#include "display.h"
#include "world_editor.h"
#include "main_frm.h"

using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLPACS;

// ***************************************************************************

CPacsManager PacsManager;

// ***************************************************************************

CPacsManager::~CPacsManager()
{
	releasePacs();
}

// ***************************************************************************

void CPacsManager::loadPacs()
{
	// Set a current directory
	string path = _Dir;
	string currentPath = CPath::getCurrentPath();

	// "Path" can be relative to the doc path so we have to be first in the doc path
	string s2 = NLMISC::CFile::getPath ((LPCTSTR)getMainFrame()->getDocument()->GetPathName());
	CPath::setCurrentPath(s2.c_str());
	string ss = CPath::getFullPath(path);
	CPath::setCurrentPath (ss.c_str());

	// Load each rbank here...
	std::vector<std::string> files;
	CPath::getPathContent (ss, false, false, true, files);
	uint i;
	for (i=0; i<files.size(); i++)
	{
		// rbank ?
		string path = NLMISC::CFile::getPath(files[i]);
		string name = NLMISC::CFile::getFilenameWithoutExtension(files[i]);
		string ext = NLMISC::CFile::getExtension(files[i]);
		string grFile = path+name+".gr";
		if ((strlwr(ext) == "rbank") && (NLMISC::CFile::isExists(grFile)))
		{
			// Create the retriever bank
			CRetriever retriever;
			retriever.Bank = URetrieverBank::createRetrieverBank (files[i].c_str(), true);
			if (retriever.Bank)
			{
				// Create the global retriever
				retriever.Retriever = UGlobalRetriever::createGlobalRetriever (grFile.c_str(), retriever.Bank);
				if (retriever.Retriever)
				{
					_Retrievers.push_back (retriever);
				}
				else
				{
					URetrieverBank::deleteRetrieverBank(retriever.Bank);
					nlwarning ("Can't create PACS retriever : %s", grFile.c_str());
				}
			}
			else
			{
				nlwarning ("Can't create PACS bank : %s", files[i].c_str());
			}
		}
	}
	CPath::setCurrentPath (currentPath.c_str());
	_Loaded = true;
}

// ***************************************************************************

void CPacsManager::releasePacs()
{
	uint i;
	for (i=0; i<_Retrievers.size(); i++)
	{
		UGlobalRetriever::deleteGlobalRetriever (_Retrievers[i].Retriever);
		URetrieverBank::deleteRetrieverBank (_Retrievers[i].Bank);
	}
	_Retrievers.clear();
	_Loaded = false;
}

// ***************************************************************************

void CPacsManager::displayPacs(CDisplay &display)
{
	uint i;
	for (i=0; i<_Retrievers.size(); i++)
	{
		// Get the borders
		static std::vector<std::pair<NLMISC::CLine, uint8> > edges;
		CAABBox box;
		box.setCenter (display._CurViewMin);
		box.extend (display._CurViewMax);
		_Retrievers[i].Retriever->getBorders(box, edges);

		// Draw the borders
		uint j;
		for(j=0; j<edges.size(); ++j)
		{
			// Choose the color according to the edge type.
			CRGBA color;
			switch(edges[j].second)
			{
			// Block
			case 0:
				color = CRGBA::Red;
				break;
			// Surmountable
			case 1:
				color = CRGBA::Green;
				break;
			// Link
			case 2:
				color = CRGBA::Yellow;
				break;
			// Waterline
			case 3:
				color = CRGBA::Blue;
				break;
			// Unknown
			default:
				color = CRGBA::White;
				break;
			}

			// Draw the line.
			display.lineRenderProxy (color, edges[j].first.V0, edges[j].first.V1, 0);
		}
	}
}

// ***************************************************************************
