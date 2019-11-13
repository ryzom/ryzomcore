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

#ifndef FILESCLEANER_H
#define FILESCLEANER_H

class IOperationProgressListener;

/**
 * Files cleaner
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CFilesCleaner
{
public:
	CFilesCleaner(IOperationProgressListener *listener);
	virtual ~CFilesCleaner();

	void setDirectory(const QString &src);

	bool exec();

protected:

	IOperationProgressListener *m_listener;

	QString m_directory;
};

#endif
