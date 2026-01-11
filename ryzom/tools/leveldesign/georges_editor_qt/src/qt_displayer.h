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


#ifndef QTDISPLAYER_H
#define QTDISPLAYER_H

// NeL includes
#include "modules.h"
#include <nel/misc/displayer.h>

// Qt includes
#include <QPlainTextEdit>

namespace NLQT
{

	class CQtDisplayer : virtual public NLMISC::IDisplayer
	{

	public:
		CQtDisplayer(QPlainTextEdit *dlgDebug, bool eraseLastLog = false, const char *displayerName = "", bool raw = false);
		CQtDisplayer();
		~CQtDisplayer ();
		void setParam (QPlainTextEdit *dlgDebug, bool eraseLastLog = false);

	protected:
		virtual void doDisplay ( const NLMISC::CLog::TDisplayInfo& args, const char *message );

	private:
		QPlainTextEdit  *m_DlgDebug;
		bool            _NeedHeader;
		uint            _LastLogSizeChecked;
		bool            _Raw;
	};/* class CQtDisplayer */

} /* namespace NLQT */

#endif //QTDISPLAYER_H