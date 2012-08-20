/**
 * \file dll_directory.h
 * \brief CDllDirectory
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
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

#ifndef PIPELINE_DLL_DIRECTORY_H
#define PIPELINE_DLL_DIRECTORY_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <iomanip>

// Project includes
#include "storage_object.h"
#include "storage_value.h"

namespace PIPELINE {
namespace MAX {

class CDllEntry;

/**
 * \brief CDllDirectory
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
 */
class CDllDirectory : public CStorageContainer
{
public:
	CDllDirectory();
	virtual ~CDllDirectory();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

	// public
	const CDllEntry *get(std::vector<CDllEntry *>::size_type idx) const;

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	TStorageObjectContainer m_ChunkCache;
	std::vector<CDllEntry *> m_Entries;

}; /* class CDllDirectory */

/**
 * \brief CDllEntry
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
 */
class CDllEntry : public CStorageContainer
{
public:
	CDllEntry();
	virtual ~CDllEntry();

	// inherited
	virtual std::string getClassName();
	virtual void toString(std::ostream &ostream, const std::string &pad = "");
	virtual void parse(uint16 version, TParseLevel level);
	virtual void clean();
	virtual void build(uint16 version);
	virtual void disown();

	const ucstring &dllDescription() { return m_DllDescription->Value; }
	const ucstring &dllFilename() { return m_DllFilename->Value; }

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	CStorageValue<ucstring> *m_DllDescription;
	CStorageValue<ucstring> *m_DllFilename;

}; /* class CDllDirectory */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_DLL_DIRECTORY_H */

/* end of file */
