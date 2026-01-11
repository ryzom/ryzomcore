/**
 * \file config.h
 * \brief CConfig
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
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

#ifndef PIPELINE_CONFIG_H
#define PIPELINE_CONFIG_H
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
 * \brief CConfig
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
 */
class CConfig : public CStorageContainer
{
public:
	CConfig();
	virtual ~CConfig();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfig */

/**
 * \brief CConfig20a0
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
 */
class CConfig20a0 : public CStorageContainer
{
public:
	CConfig20a0();
	virtual ~CConfig20a0();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfig20a0 */

/**
 * \brief CConfig20a0Entry
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig20a0Entry
 */
class CConfig20a0Entry : public CStorageContainer
{
public:
	CConfig20a0Entry();
	virtual ~CConfig20a0Entry();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfig20a0Entry */

/**
 * \brief CConfigScript
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
 */
class CConfigScript : public CStorageContainer
{
public:
	CConfigScript();
	virtual ~CConfigScript();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfigScript */

/**
 * \brief CConfigScriptEntry
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
 */
class CConfigScriptEntry : public CStorageContainer
{
public:
	CConfigScriptEntry();
	virtual ~CConfigScriptEntry();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfigScriptEntry */

/**
 * \brief CConfigScriptHeader
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfigScriptHeader
 */
class CConfigScriptHeader : public IStorageObject
{
public:
	CConfigScriptHeader();
	virtual ~CConfigScriptHeader();

	// public data
	uint32 SuperClassID;
	NLMISC::CClassId ClassID;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CConfigScriptHeader */

/**
 * \brief CConfigScriptMetaContainer
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfigScriptMetaContainer
 * This is totally hilarious.
 */
class CConfigScriptMetaContainer : public CStorageContainer
{
public:
	CConfigScriptMetaContainer();
	virtual ~CConfigScriptMetaContainer();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CConfigScriptMetaContainer */

/**
 * \brief CConfigScriptMetaString
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfigScriptMetaString
 */
class CConfigScriptMetaString : public IStorageObject
{
public:
	CConfigScriptMetaString();
	virtual ~CConfigScriptMetaString();

	// public data
	std::string Value;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CConfigScriptMetaString */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CONFIG_H */

/* end of file */
