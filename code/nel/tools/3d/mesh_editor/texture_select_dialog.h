// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2016  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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

#ifndef NL_TEXTURE_SELECT_DIALOG_H
#define NL_TEXTURE_SELECT_DIALOG_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QDialog>

// NeL includes
// ...

// Project includes
#include "texture_browser.h"

/**
 * CTextureSelectDialog
 * \brief CTextureSelectDialog
 * \date 2016-02-18 14:06GMT
 * \author Jan Boon <jan.boon@kaetemi.be>
 */
class CTextureSelectDialog : public QDialog
{
	Q_OBJECT

public:
	CTextureSelectDialog(QWidget *parent = NULL);
	virtual ~CTextureSelectDialog();

	inline std::string getSelectedTextureFile() const { return m_TextureBrowser->getSelectedTextureFile(); }

private:
	CTextureBrowser *m_TextureBrowser;

private:
	CTextureSelectDialog(const CTextureSelectDialog &);
	CTextureSelectDialog &operator=(const CTextureSelectDialog &);
	
}; /* class CTextureSelectDialog */

#endif /* #ifndef NL_TEXTURE_SELECT_DIALOG_H */

/* end of file */
