/**
 * \file metadata_storage.h
 * \brief CMetadataStorage
 * \date 2012-07-30 14:31GMT
 * \author Jan Boon (Kaetemi)
 * CMetadataStorage
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

#ifndef PIPELINE_METADATA_STORAGE_H
#define PIPELINE_METADATA_STORAGE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "workspace_storage.h"

namespace PIPELINE {

/// Suffix for metafiles that contain the CRC32 etc
#define PIPELINE_DATABASE_STATUS_SUFFIX ".status"
/// Suffix for metafiles that contain error info on database files
#define PIPELINE_DATABASE_ERRORS_SUFFIX ".errors"
/// Suffix for metafiles that contain dependencies for a file
#define PIPELINE_DATABASE_DEPEND_SUFFIX ".depend"
/// Suffix for metafiles that refer to a previously known file that no longer exists
#define PIPELINE_DATABASE_REMOVE_SUFFIX ".remove"

/**
 * \brief CMetadataStorage
 * \date 2012-07-30 14:31GMT
 * \author Jan Boon (Kaetemi)
 * CMetadataStorage
 */
class CMetadataStorage
{
protected:
	// pointers
	// ...
	
	// instances
	// ...
public:
	CMetadataStorage();
	virtual ~CMetadataStorage();
}; /* class CMetadataStorage */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_METADATA_STORAGE_H */

/* end of file */
