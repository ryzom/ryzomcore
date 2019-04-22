/**
 * \file typedefs.h
 * \brief CTypeDefs
 * \date 2012-08-21 12:14GMT
 * \author Jan Boon (Kaetemi)
 * CTypeDefs
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

#ifndef PIPELINE_TYPEDEFS_H
#define PIPELINE_TYPEDEFS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace PIPELINE {
namespace MAX {

// Don't really care about superclass IDs right now, but we have to.
typedef uint32 TSClassId;

// Application versions
const uint16 VersionUnknown = 0x0000;
const uint16 Version3 = 0x2004;
const uint16 Version4 = 0x2006;
const uint16 Version5 = 0x2008;
const uint16 Version6 = 0x2009;
const uint16 Version7 = 0x200A;
const uint16 Version8 = 0x200B;
const uint16 Version9 = 0x200E;
const uint16 Version2008 = 0x200F;
const uint16 Version2010 = 0x2012;

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_TYPEDEFS_H */

/* end of file */
