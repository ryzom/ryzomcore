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

#ifndef NLTOOLS_MAIN_WINDOW_H
#define NLTOOLS_MAIN_WINDOW_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QMainWindow>

// NeL includes
#include <nel/misc/rgba.h>
#include <nel/misc/ucstring.h>
#include <nel/misc/time_nl.h>
#include <nel/3d/animation_time.h>
#include <nel/net/login_cookie.h>

// Project includes
// ...

class QTreeView;
class QDirModel;
class QUndoStack;
class QScrollArea;

namespace NLQT {
	class CCommandLogDisplayer;
}

namespace NLTOOLS {
	class CPanoplyPreview;

/**
 * CMainWindow
 * \brief CMainWindow
 * \date 2014-09-19 09:38GMT
 * \author Jan BOON (jan.boon@kaetemi.be)
 */
class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~CMainWindow();

	inline QMenu *widgetsMenu() { return m_WidgetsMenu; }

private slots:
	void about();

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createDockWindows();

private:
	CMainWindow(const CMainWindow &);
	CMainWindow &operator=(const CMainWindow &);

private:
	CPanoplyPreview *m_PanoplyPreview;

	NLQT::CCommandLogDisplayer *m_CommandLog;
	QDockWidget *m_CommandLogDock;
	
	QMenu *m_WidgetsMenu;
	QMenu *m_HelpMenu;

	QAction *m_AboutAct;

}; /* class CMainWindow */

} /* namespace NLTOOLS */

#endif /* #ifndef NLTOOLS_MAIN_WINDOW_H */

/* end of file */
