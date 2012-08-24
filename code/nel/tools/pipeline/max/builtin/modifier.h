/**
 * \file modifier.h
 * \brief CModifier
 * \date 2012-08-22 08:59GMT
 * \author Jan Boon (Kaetemi)
 * CModifier
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

#ifndef PIPELINE_MODIFIER_H
#define PIPELINE_MODIFIER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "base_object.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CModifier
 * \date 2012-08-22 08:59GMT
 * \author Jan Boon (Kaetemi)
 * CModifier
 */
class CModifier : public CBaseObject
{
public:
	CModifier(CScene *scene);
	virtual ~CModifier();

}; /* class CModifier */

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_MODIFIER_H */

/* end of file */
