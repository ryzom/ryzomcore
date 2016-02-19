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

#ifndef NL_TEXTURE_BROWSER_H
#define NL_TEXTURE_BROWSER_H
#include <nel/misc/types_nl.h>

// STL includes
#include <functional>

// Qt includes
#include <QListWidget>

// NeL includes
// ...

class CEventLoop;
typedef std::function<void()> CStdFunctionVoid;

/**
 * CTextureBrowser
 * \brief CTextureBrowser
 * \date 2016-02-18 14:06GMT
 * \author Jan Boon <jan.boon@kaetemi.be>
 */
class CTextureBrowser : public QListWidget
{
	Q_OBJECT

public:
	CTextureBrowser(QWidget *parent = NULL);
	virtual ~CTextureBrowser();


private:
	void setDirectory(const QString &dir);

	// STD INVOKE ->
public:
	void invokeStdFunction(CStdFunctionVoid f);
private slots:
	void callStdFunction(CStdFunctionVoid f);
	// <- STD INVOKE

private:
	CEventLoop *m_Thread;
	QString m_CurrentDirectory;

private:
	CTextureBrowser(const CTextureBrowser &);
	CTextureBrowser &operator=(const CTextureBrowser &);
	
}; /* class CTextureBrowser */

#endif /* #ifndef NL_TEXTURE_BROWSER_H */

/* end of file */
