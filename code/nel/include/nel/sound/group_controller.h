/**
 * \file group_controller.h
 * \brief CGroupController
 * \date 2012-04-10 09:29GMT
 * \author Jan Boon (Kaetemi)
 * CGroupController
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

#ifndef NLSOUND_GROUP_CONTROLLER_H
#define NLSOUND_GROUP_CONTROLLER_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <map>

// NeL includes
#include <nel/misc/common.h>
#include <nel/sound/u_group_controller.h>
#include <nel/sound/containers.h>

// Project includes

namespace NLSOUND {
	class CGroupControllerRoot;

/**
 * \brief CGroupController
 * \date 2012-04-10 09:29GMT
 * \author Jan Boon (Kaetemi)
 * CGroupController
 */
class CGroupController : public UGroupController
{
public:
	friend class CGroupControllerRoot;

private:
	CGroupController *m_Parent;
	std::map<std::string, CGroupController *> m_Children;
	
	/// Gain as set by the interface
	float m_Gain;
	
	/// Gain including parent gain
	float m_FinalGain;

	int m_NbSourcesInclChild;
	TSourceContainer m_Sources;

public:
	CGroupController(CGroupController *parent);

	/// \name UGroupController
	//@{
	virtual void setGain(float gain) { NLMISC::clamp(gain, 0.0f, 1.0f); if (m_Gain != gain) { m_Gain = gain; updateSourceGain(); } }
	virtual float getGain() { return m_Gain; }
	//@}
	
	inline float getFinalGain() const { return m_FinalGain; }
	
	void addSource(CSourceCommon *source);
	void removeSource(CSourceCommon *source);
	
	virtual std::string getPath();
	
protected:
	virtual ~CGroupController(); // subnodes can only be deleted by the root

private:
	inline float calculateTotalGain() { return m_Gain; }
	virtual void calculateFinalGain();
	virtual void increaseSources();
	virtual void decreaseSources();
	void updateSourceGain();

}; /* class CGroupController */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_GROUP_CONTROLLER_H */

/* end of file */
