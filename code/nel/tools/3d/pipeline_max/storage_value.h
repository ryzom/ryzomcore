/**
 * \file storage_value.h
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

#ifndef PIPELINE_STORAGE_VALUE_H
#define PIPELINE_STORAGE_VALUE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/ucstring.h>
#include <nel/misc/string_common.h>

// Project includes
#include "storage_object.h"

namespace PIPELINE {
namespace MAX {

template<typename T>
class CStorageValue : public IStorageObject
{
public:
	// public data
	typedef T TType;
	TType Value;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

public: // should be protected but that doesn't compile, nice c++!
	// Sets size when reading
	virtual void setSize(sint32 size);
	// Gets the size when writing, return false if unknown
	virtual bool getSize(sint32 &size) const;
};

template <typename T>
std::string CStorageValue<T>::className() const
{
	return "CStorageValue";
}

template <typename T>
void CStorageValue<T>::serial(NLMISC::IStream &stream)
{
	stream.serial(Value);
}

template <>
void CStorageValue<std::string>::serial(NLMISC::IStream &stream);

template <>
void CStorageValue<ucstring>::serial(NLMISC::IStream &stream);

template <typename T>
void CStorageValue<T>::toString(std::ostream &ostream, const std::string &pad) const
{
	std::string s = NLMISC::toString(Value);
	ostream << "(" << className() << ") { " << s << " } ";
}

template <>
void CStorageValue<ucstring>::toString(std::ostream &ostream, const std::string &pad) const;

template <typename T>
void CStorageValue<T>::setSize(sint32 size)
{
	if (size != sizeof(Value))
		nlerror("Size does not match value type");
	IStorageObject::setSize(size);
}

template <>
void CStorageValue<std::string>::setSize(sint32 size);

template <>
void CStorageValue<ucstring>::setSize(sint32 size);

template <typename T>
bool CStorageValue<T>::getSize(sint32 &size) const
{
	size = sizeof(Value);
	return true;
}

template <>
bool CStorageValue<std::string>::getSize(sint32 &size) const;

template <>
bool CStorageValue<ucstring>::getSize(sint32 &size) const;

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_VALUE_H */

/* end of file */
