/**
 * \file storage_value.cpp
 * \brief CStorageValue
 * \date 2012-08-18 15:00GMT
 * \author Jan Boon (Kaetemi)
 * CStorageValue
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

#include <nel/misc/types_nl.h>
#include "storage_value.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

template <>
void CStorageValue<std::string>::serial(NLMISC::IStream &stream)
{
	stream.serialBuffer(static_cast<uint8 *>(static_cast<void *>(&Value[0])), Value.size());
}

template <>
void CStorageValue<ucstring>::serial(NLMISC::IStream &stream)
{
	stream.serialBuffer(static_cast<uint8 *>(static_cast<void *>(&Value[0])), Value.size() * 2);
}

template <>
void CStorageValue<ucstring>::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { " << Value.toUtf8() << " } ";
}

template <>
void CStorageValue<std::string>::setSize(sint32 size)
{
	Value.resize(size);
}

template <>
void CStorageValue<ucstring>::setSize(sint32 size)
{
	Value.resize(size / 2);
}

template <>
bool CStorageValue<std::string>::getSize(sint32 &size) const
{
	return Value.size();
}

template <>
bool CStorageValue<ucstring>::getSize(sint32 &size) const
{
	return Value.size() * 2;
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
