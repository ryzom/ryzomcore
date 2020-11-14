/**
 * \file class_data.h
 * \brief CClassData
 * \date 2012-08-18 19:24GMT
 * \author Jan Boon (Kaetemi)
 * CClassData
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

#ifndef PIPELINE_CLASS_DATA_H
#define PIPELINE_CLASS_DATA_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>

// Project includes
#include "storage_object.h"
#include "storage_value.h"

namespace PIPELINE {
namespace MAX {

/**
 * \brief CClassData
 * \date 2012-08-18 19:24GMT
 * \author Jan Boon (Kaetemi)
 * CClassData
 */
class CClassData : public CStorageContainer
{
public:
	CClassData();
	virtual ~CClassData();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CClassData */

/**
 * \brief CClassDataEntry
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDataEntry
 */
class CClassDataEntry : public CStorageContainer
{
public:
	CClassDataEntry();
	virtual ~CClassDataEntry();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CClassDataEntry */

/**
 * \brief CClassDataHeader
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDataHeader
 */
class CClassDataHeader : public IStorageObject
{
public:
	CClassDataHeader();
	virtual ~CClassDataHeader();

	// public data
	NLMISC::CClassId ClassID;
	uint32 SuperClassID;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CClassDataHeader */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CLASS_DATA_H */

/* end of file */
