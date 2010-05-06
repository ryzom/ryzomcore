/**
 * CCommandLog
 * $Id: command_log.h 2222 2010-02-06 19:16:59Z kaetemi $
 * \file command_log.h
 * \brief CCommandLog
 * \date 2010-02-05 20:27GMT
 * \author Jan Boon (Kaetemi)
 */

/* 
 * Copyright (C) 2010  by authors
 * 
 * This file is part of NEL QT.
 * NEL QT is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * NEL QT is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEL QT; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
