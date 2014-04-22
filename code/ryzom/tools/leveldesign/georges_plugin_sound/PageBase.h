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

#ifndef BAGE_PAGE_H
#define BAGE_PAGE_H

#include <afxdlgs.h>
#include <set>
#include "resource.h"

namespace  NLGEORGES
{
	class CSoundDialog;
}

/** Base class for all property page
*/
class CPageBase : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageBase)

	static	std::set<CPageBase*>	_AllPages;

protected:
	CPageBase(NLGEORGES::CSoundDialog *soundDialog, UINT nIDTemplate, UINT nIDCaption = 0)
		:	CPropertyPage(nIDTemplate, nIDCaption),
			SoundDialog(soundDialog)
	{
		_AllPages.insert(this);
	}

	CPageBase()
		: SoundDialog(0)
	{
		_AllPages.insert(this);
	}

	~CPageBase()
	{
		_AllPages.erase(this);
	}

	/// The dialog pane call this method when the content of the doc has changed
//	void docChanged();


	NLGEORGES::CSoundDialog		*SoundDialog;

public:
	/// The master dialog call this method when the document is changed/updated
	static void docChanged();
	/// The docChanged static method call this method on each page.
	virtual void onDocChanged() {}
};


#endif