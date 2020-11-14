/**
 * \file param_block.h
 * \brief CParamBlock
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * CParamBlock
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

#ifndef PIPELINE_PARAM_BLOCK_H
#define PIPELINE_PARAM_BLOCK_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CParamBlock
 * \date 2012-08-22 08:57GMT
 * \author Jan Boon (Kaetemi)
 * CParamBlock
 */
class CParamBlock : public CReferenceTarget
{
public:
	CParamBlock(CScene *scene);
	virtual ~CParamBlock();

}; /* class CParamBlock */

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_PARAM_BLOCK_H */

/* end of file */
