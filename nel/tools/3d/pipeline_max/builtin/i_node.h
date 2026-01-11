/**
 * \file i_node.h
 * \brief INode
 * \date 2012-08-22 19:45GMT
 * \author Jan Boon (Kaetemi)
 * INode
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

#ifndef PIPELINE_I_NODE_H
#define PIPELINE_I_NODE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief INode
 * \date 2012-08-22 19:45GMT
 * \author Jan Boon (Kaetemi)
 * INode
 */
class INode : public CReferenceTarget
{
public:
	INode(CScene *scene);
	virtual ~INode();

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const char *InternalNameUnknown;
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
	virtual INode *parent();
	virtual void setParent(INode *node);
	virtual void addChild(INode *node);
	virtual void removeChild(INode *node); // does not delete
	virtual const ucstring &userName() const;
	INode *find(const ucstring &userName) const;

	// dump
	void dumpNodes(std::ostream &ostream, const std::string &pad = "") const;

	// read access
	/// The children that are linked to this node by the parent tag
	inline const std::set<NLMISC::CRefPtr<INode> > &children() const { return m_Children; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

protected:
	std::set<NLMISC::CRefPtr<INode> > m_Children;

}; /* class INode */

typedef CSceneClassDesc<INode> CNodeClassDesc;
extern const CNodeClassDesc NodeClassDesc;
typedef CSuperClassDesc<INode> CNodeSuperClassDesc;
extern const CNodeSuperClassDesc NodeSuperClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_I_NODE_H */

/* end of file */
