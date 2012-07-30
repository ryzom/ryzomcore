/**
 * \file workspace_storage.h
 * \brief CWorkspaceStorage
 * \date 2012-07-30 14:34GMT
 * \author Jan Boon (Kaetemi)
 * CWorkspaceStorage
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_WORKSPACE_STORAGE_H
#define PIPELINE_WORKSPACE_STORAGE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace PIPELINE {

/// Suffix for directories under the workspace containing the metafiles
#define PIPELINE_DATABASE_META_SUFFIX ".meta"

/**
 * \brief CWorkspaceStorage
 * \date 2012-07-30 14:34GMT
 * \author Jan Boon (Kaetemi)
 * CWorkspaceStorage
 */
class CWorkspaceStorage
{
public:
	/// Get the path for a metadata file, based on a filepath it represents, and the suffix of it's contents
	static std::string getMetaFilePath(const std::string &path, const std::string &dotSuffix);

	/// Get the directory where metadata files are stored, based on the filepath containing the files which the containing metadata represents
	static std::string getMetaDirectoryPath(const std::string &path);

}; /* class CWorkspaceStorage */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_WORKSPACE_STORAGE_H */

/* end of file */
