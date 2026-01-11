/**
 * \file track_view_node.h
 * \brief CTrackViewNode
 * \date 2012-08-24 09:44GMT
 * \author Jan Boon (Kaetemi)
 * CTrackViewNode
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

#ifndef PIPELINE_TRACK_VIEW_NODE_H
#define PIPELINE_TRACK_VIEW_NODE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes
#include "reference_target.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

/**
 * \brief CTrackViewNode
 * \date 2012-08-24 09:44GMT
 * \author Jan Boon (Kaetemi)
 * TVNode
 */
class CTrackViewNode : public CReferenceTarget
{
public:
	struct TChild
	{
		TChild() : IsNotAnotherNode(0) { }
		NLMISC::CRefPtr<CReferenceMaker> Reference;
		ucstring DisplayName;
		NLMISC::CClassId Identifier;
		sint32 IsNotAnotherNode;
	};

	CTrackViewNode(CScene *scene);
	virtual ~CTrackViewNode();

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

	// reference maker
	virtual CReferenceMaker *getReference(uint index) const;
	virtual void setReference(uint index, CReferenceMaker *reference);
	virtual uint nbReferences() const;

	// read access
	inline const std::vector<TChild> &children() const { return m_Children; }

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	CStorageRaw *m_Empty0140;
	CStorageRaw *m_Empty0150;

	std::vector<TChild> m_Children;

}; /* class CTrackViewNode */

typedef CSceneClassDesc<CTrackViewNode> CTrackViewNodeClassDesc;
extern const CTrackViewNodeClassDesc TrackViewNodeClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_TRACK_VIEW_NODE_H */

/* end of file */
