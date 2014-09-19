// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

#ifndef NLTOOLS_PANOPLY_PREVIEW_H
#define NLTOOLS_PANOPLY_PREVIEW_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

// Project includes

namespace NLTOOLS {

/**
 * CPanoplyPreview
 * \brief CPanoplyPreview
 * \date 2014-09-19 09:38GMT
 * \author Jan BOON (jan.boon@kaetemi.be)
 */
class CPanoplyPreview : public QWidget
{
	Q_OBJECT
	
public:
	CPanoplyPreview(QWidget *parent);
	virtual ~CPanoplyPreview();

//private slots:
	// ...

private:
	QTextEdit *m_DisplayerOutput;
	QLineEdit *m_CommandInput;

private:
	CPanoplyPreview(const CPanoplyPreview &);
	CPanoplyPreview &operator=(const CPanoplyPreview &);
	
}; /* class CPanoplyPreview */

} /* namespace NLTOOLS */

#endif /* #ifndef NLTOOLS_PANOPLY_PREVIEW_H */

/* end of file */
