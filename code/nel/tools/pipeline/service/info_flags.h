/**
 * \file info_flags.h
 * \brief CInfoFlags
 * \date 2012-03-04 10:46GMT
 * \author Jan Boon (Kaetemi)
 * CInfoFlags
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_INFO_FLAGS_H
#define PIPELINE_INFO_FLAGS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/singleton.h>
#include <nel/misc/variable.h>

// Project includes

namespace PIPELINE {

/**
 * \brief CInfoFlags
 * \date 2012-03-04 10:46GMT
 * \author Jan Boon (Kaetemi)
 * CInfoFlags
 */
	class CInfoFlags : public NLMISC::CManualSingleton<CInfoFlags>
{
protected:
	std::map<std::string, uint> m_FlagMap;
public:
	CInfoFlags();
	virtual ~CInfoFlags();

	void addFlag(const std::string &flagName);
	void removeFlag(const std::string &flagName);

private:
	void updateInfoFlags();

}; /* class CInfoFlags */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_INFO_FLAGS_H */

/* end of file */
