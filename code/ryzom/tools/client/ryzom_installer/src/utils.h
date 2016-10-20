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

#ifndef UTILS_H
#define UTILS_H

#include <nel/misc/ucstring.h>

#include <string>

/**
 * Utils functions
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */

// convert a size in bytes to a QString with larger unit (KiB, MiB, etc...)
QString qBytesToHumanReadable(qint64 bytes);

// return true is the specified directory is empty (has no file inside) (and all its subdirectories if recursize is true)
bool isDirectoryEmpty(const QString &directory, bool recursize);

// check if specified directory is writable
bool isDirectoryWritable(const QString &directory);

// return the total size in bytes of specified directtory (and all its subdirectories if recursize is true)
qint64 getDirectorySize(const QString &directory, bool recursize);

// convert a UTF-8 string to QString
QString qFromUtf8(const std::string &str);

// convert a QString to UTF-8 string
std::string qToUtf8(const QString &str);

// convert an UTF-16 string to QString
QString qFromUtf16(const ucstring &str);

// convert a QString to UTF-16 string
ucstring qToUtf16(const QString &str);

// convert an wchar_t* to QString
QString qFromWide(const wchar_t *str);

// convert an QString to wchar_t*
wchar_t* qToWide(const QString &str);

#define Q2C(x) qToUtf8(x).c_str()

// check if a shortcut already exists (the extension will be added)
bool shortcutExists(const QString &shortcut);

// create a shortcut with the native format of the current platform
bool createShortcut(const QString &shortcut, const QString &name, const QString &executable, const QString &arguments, const QString &icon, const QString &workingDir);

// remove a shortcut (the extension will be added)
bool removeShortcut(const QString &shortcut);

// return the real path of shortcut
bool resolveShortcut(const QWidget &window, const QString &shortcut, QString &pathObj);

// append the shortcut of current platform to specified path
QString appendShortcutExtension(const QString &shortcut);

// launch an executable with --version parameter and parse version string
QString getVersionFromExecutable(const QString &path);

// write a resource in QRC to disk
bool writeResource(const QString &resource, const QString &path);

// write a resource in QRC to disk and replace all variables by specified values
bool writeResourceWithTemplates(const QString &resource, const QString &path, const QMap<QString, QString> &strings);

// a little helper class to unintialize COM after using it
class CCOMHelper
{
	bool m_mustUninit;

public:
	CCOMHelper();
	~CCOMHelper();
};

// a little helper class to init/uninit log
class CLogHelper
{
public:
	CLogHelper(const QString &logPath);
	~CLogHelper();
};

#endif
