/**
 * \file node_impl.h
 * \brief CNodeImpl
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
 * CNodeImpl
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

#ifndef PIPELINE_NODE_IMPL_H
#define PIPELINE_NODE_IMPL_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "i_node.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CNodeImpl
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
 * CNodeImpl
 */
class CNodeImpl : public INode
{
public:
	CNodeImpl(CScene *scene);
	virtual ~CNodeImpl();

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
	virtual INode *parent();
	virtual void setParent(INode *node);
	// virtual void addChild(INode *node);
	// virtual void removeChild(INode *node); // does not delete
	virtual const ucstring &userName() const;

	// read access
	inline uint32 nodeVersion() const { return m_NodeVersion; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	uint32 m_NodeVersion;
	NLMISC::CRefPtr<INode> m_Parent;
	uint32 m_ParentFlags;
	ucstring m_UserName;

}; /* class CNodeImpl */

typedef CSceneClassDesc<CNodeImpl> CNodeImplClassDesc;
extern const CNodeImplClassDesc NodeImplClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_NODE_IMPL_H */

/* end of file */
