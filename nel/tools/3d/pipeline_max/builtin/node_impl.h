/**
 * \file node_impl.h
 * \brief CNodeImpl
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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
	virtual ~CNodeImpl() NL_OVERRIDE;

	// class desc
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;

	// inherited
	virtual void parse(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void clean() NL_OVERRIDE;
	virtual void build(uint16 version, uint filter = 0) NL_OVERRIDE;
	virtual void disown() NL_OVERRIDE;
	virtual void init() NL_OVERRIDE;
	virtual bool inherits(const NLMISC::CClassId classId) const NL_OVERRIDE;
	virtual const ISceneClassDesc *classDesc() const NL_OVERRIDE;
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const NL_OVERRIDE;

	// node interface
	virtual INode *parent() NL_OVERRIDE;
	virtual void setParent(INode *node) NL_OVERRIDE;
	// virtual void addChild(INode *node);
	// virtual void removeChild(INode *node); // does not delete
	virtual const ucstring &userName() const NL_OVERRIDE;

	// read access
	inline uint32 nodeVersion() const { return m_NodeVersion; }

	//! \name Typed overlay over the node's optional state chunks (valid between parse and
	//! clean/disown). These decode from the orphaned chunks WITHOUT claiming them (the raw
	//! chunks stay authoritative and re-emit verbatim in position), the CParamBlock/CParamBlock2
	//! discipline — CNodeImpl's own claimed trio (version/parent/name) is rebuilt from typed
	//! state on build, but the optional chunks sit among other orphans (AppData, material refs)
	//! whose relative order a claim-and-re-put would disturb.
	//@{
	/// Node state flags (chunk 0x0963, first dword; observed 8 bytes = 2 dwords). Bit 0x40 =
	/// node hidden (byte-validated on max_top.max, swt export).
	bool nodeFlags(uint32 &flags) const;
	/// True when the node is hidden (flags bit 0x40); false when the flags chunk is absent.
	bool isHidden() const;
	/// Rendering-control flags (chunk 0x099c, first dword; e.g. bit 0x200 = cast shadows).
	bool renderFlags(uint32 &flags) const;
	/// Object-offset PRS (chunks 0x096a pos CVector / 0x096b rot Quat x,y,z,w as stored /
	/// 0x096c ScaleValue: CVector scale + Quat axis system) — the GetObjOffset{Pos,Rot,Scale}
	/// the reference exporter composes into objectTM = offsetTM * nodeTM. Each returns false
	/// when its chunk is absent (identity applies).
	bool objectOffsetPos(float pos[3]) const;
	bool objectOffsetRot(float rot[4]) const;
	bool objectOffsetScale(float s[3], float q[4]) const;
	/// The CVector head of 0x096c alone (biped bone dimensions ride this chunk on biped rigs;
	/// requires only the first 12 bytes where the full ScaleValue read requires 28).
	bool objectOffsetScaleVec(float s[3]) const;
	//@}

protected:
	// inherited
	virtual IStorageObject *createChunkById(uint16 id, bool container) NL_OVERRIDE;

private:
	void decodeStateChunks();

	uint32 m_NodeVersion;
	NLMISC::CRefPtr<INode> m_Parent;
	uint32 m_ParentFlags;
	ucstring m_UserName;

	// typed overlay over the optional node state chunks (presence + first-match values)
	bool m_HasNodeFlags;
	uint32 m_NodeFlags;
	bool m_HasRenderFlags;
	uint32 m_RenderFlags;
	bool m_HasObjOffsetPos;
	float m_ObjOffsetPos[3];
	bool m_HasObjOffsetRot;
	float m_ObjOffsetRot[4];
	bool m_HasObjOffsetScaleVec; // 0x096c first 12 bytes
	bool m_HasObjOffsetScale;    // 0x096c full 28 bytes
	float m_ObjOffsetScaleS[3];
	float m_ObjOffsetScaleQ[4];

}; /* class CNodeImpl */

typedef CSceneClassDesc<CNodeImpl> CNodeImplClassDesc;
extern const CNodeImplClassDesc NodeImplClassDesc;

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_NODE_IMPL_H */

/* end of file */
