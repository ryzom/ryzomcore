/**
 * \file material_sidecar.cpp
 * \brief Materials sidecar loader — see material_sidecar.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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
#include "material_sidecar.h"

#include <set>

#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/pipeline/tool_logger.h>

#include "../nel_gltf/json_value.h"
#include "../nel_gltf/gltf_material.h"

using namespace NLMISC;
using namespace NL3D;
using namespace NLGLTF;

bool loadMaterialSidecars(const std::vector<std::string> &paths, TMaterialMap &out,
                          NLPIPELINE::CToolLogger &logger, const std::string &sourceFilePath)
{
	for (uint pi = 0; pi < paths.size(); ++pi)
	{
		const std::string &path = paths[pi];

		std::string content;
		{
			CIFile f;
			if (!f.open(path))
			{
				tlerror(logger, sourceFilePath.c_str(),
					"Cannot open materials sidecar '%s'", path.c_str());
				return false;
			}
			content.resize(f.getFileSize());
			if (!content.empty())
				f.serialBuffer((uint8 *)&content[0], (uint)content.size());
		}

		CJsonValue json;
		std::string err;
		if (!json.parse(content, &err))
		{
			tlerror(logger, sourceFilePath.c_str(),
				"Materials sidecar '%s' is not valid JSON: %s", path.c_str(), err.c_str());
			return false;
		}

		const CJsonValue *mats = json.get("materials");
		if (!mats || !mats->isArray() || !mats->size())
		{
			tlerror(logger, sourceFilePath.c_str(),
				"Materials sidecar '%s' has no materials array", path.c_str());
			return false;
		}

		std::set<std::string> seenInFile;
		for (size_t i = 0; i < mats->size(); ++i)
		{
			const CJsonValue *m = mats->at(i);
			std::string name = m->getString("name", "");
			if (name.empty())
			{
				tlwarning(logger, sourceFilePath.c_str(),
					"Materials sidecar '%s' entry %u has no name; entry ignored (binding is by name, never positional)",
					path.c_str(), (uint)i);
				continue;
			}
			if (!seenInFile.insert(name).second)
			{
				tlerror(logger, sourceFilePath.c_str(),
					"Materials sidecar '%s' has duplicate material name '%s'", path.c_str(), name.c_str());
				return false;
			}
			const CJsonValue *ex = m->get("extras");
			if (!ex || !ex->get("nel_flags"))
			{
				tlwarning(logger, sourceFilePath.c_str(),
					"Materials sidecar '%s' material '%s' has no nel_* extras; entry ignored",
					path.c_str(), name.c_str());
				continue;
			}
			CSmartPtr<CMaterial> mat = new CMaterial();
			if (!materialFromExtras(*ex, *mat, &err))
			{
				tlerror(logger, sourceFilePath.c_str(),
					"Materials sidecar '%s' material '%s' failed to reconstruct: %s",
					path.c_str(), name.c_str(), err.c_str());
				return false;
			}
			TMaterialMap::iterator it = out.find(name);
			if (it != out.end())
			{
				nlinfo("Materials sidecar '%s' overrides earlier sidecar entry for material '%s'",
					path.c_str(), name.c_str());
				it->second = mat;
			}
			else
			{
				out[name] = mat;
			}
		}
	}
	return true;
}

/* end of file */
