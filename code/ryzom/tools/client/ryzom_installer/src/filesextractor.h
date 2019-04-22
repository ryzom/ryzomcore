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

#ifndef FILEXTRACTOR_H
#define FILEXTRACTOR_H

class IOperationProgressListener;

/**
 * Files extractor
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CFilesExtractor
{
public:
	CFilesExtractor(IOperationProgressListener *listener);
	virtual ~CFilesExtractor();

	void setSourceFile(const QString &src);
	void setDestinationDirectory(const QString &src);

	bool exec();

protected:

	bool extract7z();
	bool extractZip();
	bool extractBnp();

	bool progress(const std::string &filename, uint32 currentFile, uint32 totalFiles);

	IOperationProgressListener *m_listener;

	QString m_sourceFile;
	QString m_destinationDirectory;
};

#endif
