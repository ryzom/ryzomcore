/**
 * \file old_param_block.cpp
 * \brief See old_param_block.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Sonnet 5
 */

/*
 * Copyright (C) 2026  by authors
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

#include <nel/misc/types_nl.h>
#include "old_param_block.h"

#include <cstring>

#include "../pipeline_max/storage_object.h"

using namespace PIPELINE::MAX;

namespace OLDPBLOCK {

void readOldParamBlock(CSceneClass *pblock, std::map<sint32, SParam> &out)
{
	const CStorageContainer::TStorageObjectContainer &po = pblock->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = po.begin(); it != po.end(); ++it)
	{
		if (it->first != 0x0002) continue;
		CStorageContainer *pc = dynamic_cast<CStorageContainer *>(it->second);
		if (!pc) continue;
		sint32 idx = -1;
		for (CStorageContainer::TStorageObjectConstIt cit = pc->chunks().begin(); cit != pc->chunks().end(); ++cit)
		{
			CStorageRaw *cr = dynamic_cast<CStorageRaw *>(cit->second);
			if (!cr) continue;
			if (cit->first == 0x0003 && cr->Value.size() == 4)
				memcpy(&idx, nlVectorData(cr->Value), 4);
			else if (cit->first == 0x0102 && cr->Value.size() == 12 && idx >= 0)
			{
				SParam p;
				p.IsPoint3 = true;
				p.IsInt = false;
				p.I = 0;
				memcpy(p.V, nlVectorData(cr->Value), 12);
				out[idx] = p;
			}
			else if (cit->first != 0x0004 && cr->Value.size() == 4 && idx >= 0)
			{
				SParam p;
				p.IsPoint3 = false;
				p.IsInt = (cit->first == 0x0101);
				p.V[1] = p.V[2] = 0.0f;
				memcpy(p.V, nlVectorData(cr->Value), 4);
				memcpy(&p.I, nlVectorData(cr->Value), 4);
				out[idx] = p;
			}
		}
	}
}

} /* namespace OLDPBLOCK */

/* end of file */
