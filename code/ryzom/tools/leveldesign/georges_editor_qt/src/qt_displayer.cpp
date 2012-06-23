/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// Nel includes

#include "qt_displayer.h"
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>

namespace NLQT 
{

	CQtDisplayer::CQtDisplayer(QPlainTextEdit *dlgDebug, bool eraseLastLog, const char *displayerName, bool raw)
		: NLMISC::IDisplayer (displayerName), _NeedHeader(true), _LastLogSizeChecked(0), _Raw(raw) 
	{
		setParam(dlgDebug,eraseLastLog);
	}

	CQtDisplayer::CQtDisplayer() 
		: IDisplayer (""), _NeedHeader(true), _LastLogSizeChecked(0), _Raw(false) 
	{
		;
	}

	CQtDisplayer::~CQtDisplayer() {
		;
	}

	void CQtDisplayer::setParam (QPlainTextEdit *dlgDebug, bool eraseLastLog)
	{
		m_DlgDebug=dlgDebug;
		//dlgDebug->dlgDbgText->WriteText("test");
	}

	void CQtDisplayer::doDisplay ( const NLMISC::CLog::TDisplayInfo& args, const char *message )
	{
		bool needSpace = false;
		std::string str;

		if(m_DlgDebug==NULL)
			return;

		QTextCharFormat format;

		if (args.Date != 0 && !_Raw) {
			str += dateToHumanString(args.Date);
			needSpace = true;
		}

		if (args.LogType != NLMISC::CLog::LOG_NO && !_Raw) 
		{
			if (needSpace) { str += " "; needSpace = false; }
			str += logTypeToString(args.LogType);
			if (args.LogType == NLMISC::CLog::LOG_WARNING)
				format.setForeground(QBrush(QColor("red")));
			else
				format.setForeground(QBrush(QColor("black")));
			needSpace = true;
		}

		// Write thread identifier
		/*if ( args.ThreadId != 0 && !_Raw) {
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString(args.ThreadId);
		needSpace = true;
		}*/
		/*if (!args.ProcessName.empty() && !_Raw) {
		if (needSpace) { str += " "; needSpace = false; }
		str += args.ProcessName;
		needSpace = true;
		}*/

		//if (args.FileName != NULL && !_Raw) {
		//	if (needSpace) { str += " "; needSpace = false; }
		//	str += NLMISC::CFile::getFilename(args.FileName);
		//	needSpace = true;
		//}

		/*if (args.Line != -1 && !_Raw) {
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString(args.Line);
		needSpace = true;
		}*/

		if (args.FuncName != NULL && !_Raw) 
		{
			if (needSpace) 
			{ 
				str += " "; needSpace = false; 
			}
			str += args.FuncName;
			needSpace = true;
		}

		if (needSpace)
		{ 
			str += " : "; needSpace = false; 
		}
		str += message;



		m_DlgDebug->textCursor().insertText(str.c_str(), format);
		//m_DlgDebug->setCenterOnScroll(true);
		m_DlgDebug->centerCursor();
		//m_DlgDebug->ensureCursorVisible();

	}

}