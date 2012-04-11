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
	friend CGroupControllerRoot;

private:
	CGroupController *m_Parent;
	std::map<std::string, CGroupController *> m_Children;

	float m_DevGain;
	float m_UserGain;
	float m_FinalGain;

	int m_NbSourcesInclChild;
	TSourceContainer m_Sources;

public:
	CGroupController(CGroupController *parent);

	/// \name UGroupController
	//@{
	virtual void setDevGain(float gain) { NLMISC::clamp(gain, 0.0f, 1.0f); m_DevGain = gain; updateSourceGain(); }
	virtual float getDevGain() { return m_DevGain; }

	virtual void setUserGain(float gain) { NLMISC::clamp(gain, 0.0f, 1.0f); m_UserGain = gain; updateSourceGain(); }
	virtual float getUserGain() { return m_UserGain; }

	virtual void setGain(float devGain, float userGain) { NLMISC::clamp(devGain, 0.0f, 1.0f); NLMISC::clamp(userGain, 0.0f, 1.0f); m_DevGain = devGain; m_UserGain = userGain; updateSourceGain(); }
	//@}

	inline float getFinalGain() const { return m_FinalGain; }

	void addSource(CSourceCommon *source);
	void removeSource(CSourceCommon *source);

	virtual std::string getPath();

protected:
	virtual ~CGroupController(); // subnodes can only be deleted by the root

private:
	inline float calculateTotalGain() { return m_DevGain * m_UserGain; }
	virtual void calculateFinalGain();
	virtual void increaseSources();
	virtual void decreaseSources();
	void updateSourceGain();

}; /* class CGroupController */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_GROUP_CONTROLLER_H */

/* end of file */
