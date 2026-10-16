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

#include "../pipeline_max/builtin/param_block.h"

using namespace PIPELINE::MAX;

namespace OLDPBLOCK {

void readOldParamBlock(CSceneClass *pblock, std::map<sint32, SParam> &out)
{
	// Every superclass-0x8 object parses through the typed BUILTIN::CParamBlock (one decode
	// path, in the library); this is a thin copy onto the legacy map shape. Entry order is
	// preserved (later duplicate indices overwrite, as the inline decode did), and both value
	// views are bit-copies of the stored dword — IsInt marks only the 0x0101 int kind (a bool
	// param, value chunk 0x0104, kept the legacy float-bits view).
	BUILTIN::CParamBlock *pb = dynamic_cast<BUILTIN::CParamBlock *>(pblock);
	if (!pb) return;
	const std::vector<BUILTIN::CParamBlock::SParam> &params = pb->params();
	for (std::vector<BUILTIN::CParamBlock::SParam>::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it->Index < 0 || !it->HasConstant) continue;
		SParam p;
		p.IsPoint3 = it->Kind == BUILTIN::CParamBlock::KindPoint3;
		p.IsInt = it->Kind == BUILTIN::CParamBlock::KindInt;
		p.I = p.IsPoint3 ? 0 : it->I;
		p.V[0] = it->F[0];
		p.V[1] = p.IsPoint3 ? it->F[1] : 0.0f;
		p.V[2] = p.IsPoint3 ? it->F[2] : 0.0f;
		out[it->Index] = p;
	}
}

} /* namespace OLDPBLOCK */

/* end of file */
