/**
 * \file root_node.h
 * \brief CRootNode
 * \date 2012-08-22 19:45GMT
 * \author Jan Boon (Kaetemi)
 * CRootNode
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

#ifndef PIPELINE_ROOT_NODE_H
#define PIPELINE_ROOT_NODE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "i_node.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CRootNode
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
 * CRootNode
 */
class CRootNode : public INode
{
public:
	CRootNode(CScene *scene);
	virtual ~CRootNode();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();
	virtual void init();
	virtual bool inherits(const NLMISC::CClassId classId) const;
	virtual const ISceneClassDesc *classDesc() const;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;

	// node interface
	virtual const ucstring &userName() const;

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

}; /* class CRootNode */

typedef CSceneClassDesc<CRootNode> CRootNodeClassDesc;
extern const CRootNodeClassDesc RootNodeClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_ROOT_NODE_H */

/* end of file */
