/**
 * \file geom_buffers.h
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * CGeomBuffers
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

#ifndef PIPELINE_GEOM_BUFFERS_H
#define PIPELINE_GEOM_BUFFERS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../../storage_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

/**
 * \brief CGeomBuffers
 * \date 2012-08-25 07:55GMT
 * \author Jan Boon (Kaetemi)
 * CGeomBuffers
 */
class CGeomBuffers : public CStorageContainer
{
public:
	CGeomBuffers();
	virtual ~CGeomBuffers();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CGeomBuffers */

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_GEOM_BUFFERS_H */

/* end of file */
