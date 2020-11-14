/**
 * \file mtl_base.h
 * \brief CMtlBase
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CMtlBase
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

#ifndef PIPELINE_MTL_BASE_H
#define PIPELINE_MTL_BASE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CMtlBase
 * \date 2012-08-22 08:53GMT
 * \author Jan Boon (Kaetemi)
 * CMtlBase
 */
class CMtlBase : public CReferenceTarget
{
public:
	CMtlBase(CScene *scene);
	virtual ~CMtlBase();

}; /* class CMtlBase */

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MTL_BASE_H */

/* end of file */
