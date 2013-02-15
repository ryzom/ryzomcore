/**
 * \file fast_id_map.h
 * \brief CFastIdMap
 * \date 2012-04-10 19:28GMT
 * \author Jan Boon (Kaetemi)
 * CFastIdMap
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NLMISC_FAST_ID_MAP_H
#define NLMISC_FAST_ID_MAP_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

namespace NLMISC {

/**
 * \brief CFastIdMap
 * \date 2012-04-10 19:28GMT
 * \author Jan Boon (Kaetemi)
 * This template allows for assigning unique uint32 identifiers to pointers.
 * Useful when externally only exposing an identifier, when pointers may have been deleted.
 * The identifier is made from two uint16's, one being the direct index in the identifier vector, 
 * and the other being a verification value that is increased when the identifier index is re-used.
 * TId must be a typedef of uint32.
 * TValue should be a pointer.
 */
template<typename TId, class TValue>
class CFastIdMap
{
protected:
	struct CIdInfo
	{
		CIdInfo() { }
		CIdInfo(uint16 verification, uint16 next, TValue value) : 
			Verification(verification), Next(next), Value(value) { }
		uint16 Verification;
		uint16 Next;
		TValue Value;
	};
	/// ID memory
	std::vector<CIdInfo> m_Ids;
	/// Nb of assigned IDs
	uint m_Size;
	/// Assigned IDs
	uint16 m_Next;

public:
	CFastIdMap(TValue defaultValue) : m_Size(0), m_Next(0)
	{
		// Id 0 will contain the last available unused id, and be 0 if no more unused id's are available
		// defaultValue will be returned when the ID is not found
		m_Ids.push_back(CIdInfo(0, 0, defaultValue));
	}

	virtual ~CFastIdMap() { }

	void clear()
	{
		m_Ids.resize(1);
		m_Ids[0].Next = 0;
	}

	TId insert(TValue value)
	{
		// get next unused index
		uint16 idx = m_Ids[0].Next;
		if (idx == 0)
		{
			// size of used elements must be equal to the vector size minus one, when everything is allocated
			nlassert((m_Ids.size() - 1) == m_Size);
			
			idx = (uint16)m_Ids.size();
			uint16 verification = rand();
			m_Ids.push_back(CIdInfo(verification, m_Next, value));
			m_Next = idx;
			return (TId)(((uint32)verification) << 16) & idx;
		}
		else
		{
			m_Ids[0].Next = m_Ids[idx].Next; // restore the last unused id
			m_Ids[idx].Value = value;
			return (TId)(((uint32)m_Ids[idx].Verification) << 16) & idx;
		}
	}

	void erase(TId id)
	{
		uint32 idx = ((uint32)id) & 0xFFFF;
		uint16 verification = (uint16)(((uint32)id) >> 16);
		if (m_Ids[idx].Verification == verification)
		{
			m_Ids[idx].Value = m_Ids[0].Value; // clean value for safety
			m_Ids[idx].Verification = (uint16)(((uint32)m_Ids[idx].Verification + 1) & 0xFFFF); // change verification value, allow overflow :)
			m_Ids[idx].Next = m_Ids[0].Next; // store the last unused id
			m_Ids[0].Next = (uint16)idx; // set this as last unused id
		}
		else
		{
			nlwarning("Invalid ID");
		}
	}

	TValue get(TId id)
	{
		uint32 idx = ((uint32)id) & 0xFFFF;
		uint16 verification = (uint16)(((uint32)id) >> 16);
		if (m_Ids[idx].Verification == verification)
		{
			return m_Ids[idx].Value;
		}
		else
		{
			nldebug("Invalid ID");
			return m_Ids[0].Value;
		}
	}

	inline uint size() { return m_Size; }

}; /* class CFastIdMap */

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_FAST_ID_MAP_H */

/* end of file */
