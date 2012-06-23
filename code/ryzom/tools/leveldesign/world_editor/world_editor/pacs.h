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

// pacs.g : implementation file
//

#ifndef NL_PACS_H
#define NL_PACS_H

class CPacsManager
{
public:
	// Destructor
	~CPacsManager();

	// Load pacs data
	void loadPacs();

	// Release pacs data
	void releasePacs();

	// Display pacs data
	void displayPacs(class CDisplay &display);

	// Are PACS loaded ?
	bool areLoaded() { return _Loaded; }

	// Set PACS directory
	void setDir(const std::string &path) { _Dir = path; }

private:
	class CRetriever
	{
	public:
		class NLPACS::URetrieverBank	*Bank;
		class NLPACS::UGlobalRetriever	*Retriever;
	};
	std::vector<CRetriever>				_Retrievers;
	bool								_Loaded;
	std::string							_Dir;
};

extern CPacsManager PacsManager;

#endif // NL_PACS_H
