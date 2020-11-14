/**
 * \file epoly.h
 * \brief CEPoly
 * \date 2012-08-26 12:03GMT
 * \author Jan Boon (Kaetemi)
 * CEPoly
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

#ifndef PIPELINE_EPOLY_H
#define PIPELINE_EPOLY_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "../dll_plugin_desc.h"

namespace PIPELINE {
namespace MAX {

class CSceneClassRegistry;

namespace EPOLY {

/**
 * \brief CEPoly
 * \date 2012-08-26 12:03GMT
 * \author Jan Boon (Kaetemi)
 * CEPoly
 */
class CEPoly
{
public:
	static void registerClasses(CSceneClassRegistry *registry);

public:
	CEPoly();
	virtual ~CEPoly();

}; /* class CEPoly */

/**
 * \brief CDllPluginDescEPoly
 * \date 2012-08-26 12:03GMT
 * \author Jan Boon (Kaetemi)
 * CDllPluginDescEPoly
 */
class CDllPluginDescEPoly : public IDllPluginDescInternal
{
public:
	virtual const ucchar *displayName() const;
	virtual const ucchar *internalName() const;
}; /* class CDllPluginDescScript */

extern const CDllPluginDescEPoly DllPluginDescEPoly;

} /* namespace EPOLY */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_EPOLY_H */

/* end of file */
