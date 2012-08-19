/**
 * \file class_directory_3.h
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectory3
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

#ifndef PIPELINE_CLASS_DIRECTORY_3_H
#define PIPELINE_CLASS_DIRECTORY_3_H
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
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectory3
 */
class CClassDirectory3 : public CStorageContainer
{
public:
	CClassDirectory3();
	virtual ~CClassDirectory3();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void build(uint16 version);

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CClassDirectory3 */

/**
 * \brief CClassEntry
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassEntry
 */
class CClassEntry : public CStorageContainer
{
public:
	CClassEntry();
	virtual ~CClassEntry();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void build(uint16 version);

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CClassEntry */

/**
 * \brief CClassDirectoryHeader
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectoryHeader
 */
class CClassDirectoryHeader : public IStorageObject
{
public:
	CClassDirectoryHeader();
	virtual ~CClassDirectoryHeader();

	// public data
	sint32 DllIndex;
	NLMISC::CClassId ClassID;
	uint32 SuperClassID;

	// inherited
	virtual std::string getClassName();
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "");

}; /* class CClassDirectoryHeader */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CLASS_DIRECTORY_3_H */

/* end of file */
