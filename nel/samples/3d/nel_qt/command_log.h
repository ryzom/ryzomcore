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

#ifndef NLQT_COMMAND_LOG_H
#define NLQT_COMMAND_LOG_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>

// NeL includes
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

// Project includes

namespace NLQT {

/**
 * CCommandLog
 * \brief CCommandLog
 * \date 2010-02-05 20:27GMT
 * \author Jan Boon (Kaetemi)
 */
class CCommandLog : public QWidget, public NLMISC::IDisplayer
{
	Q_OBJECT
	
public:
	CCommandLog(QWidget *parent);
	virtual ~CCommandLog();

protected:
	virtual void doDisplay(const NLMISC::CLog::TDisplayInfo& args, const char *message);

private slots:
	void returnPressed();

private:
	QTextEdit *m_DisplayerOutput;
	QLineEdit *m_CommandInput;

private:
	CCommandLog(const CCommandLog &);
	CCommandLog &operator=(const CCommandLog &);
	
}; /* class CCommandLog */

} /* namespace NLQT */

#endif /* #ifndef NLQT_COMMAND_LOG_H */

/* end of file */
