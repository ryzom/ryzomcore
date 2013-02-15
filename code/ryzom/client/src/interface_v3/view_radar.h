// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef RY_VIEW_RADAR_H
#define RY_VIEW_RADAR_H

#include "nel/gui/view_base.h"
#include "nel/3d/u_texture.h"
#include "nel/gui/view_renderer.h"

/**
 * class implementing a radar view
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003 September
 */
class CViewRadar : public CViewBase
{
public:

	// Update CViewRadar::parse() if you change this enum
	enum TRadarSpotId
	{
		Std = 0,
		MissionList,
		MissionAuto,
		MissionStep,
		
		NbRadarSpotIds
	};

	/// Constructor
	CViewRadar(const TCtorParam &param);

	bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);

	virtual void updateCoords ();

	/// Draw the view
	virtual void draw ();

	REFLECT_EXPORT_START(CViewRadar, CViewBase)
		REFLECT_FLOAT ("world_size", getWorldSize, setWorldSize);
	REFLECT_EXPORT_END


	void setWorldSize(float f) { _WorldSize = f; }
	float getWorldSize() const { return (float)_WorldSize; }

protected:

	double _WorldSize;

	struct CRadarSpotDesc
	{
		CViewRenderer::CTextureId TextureId;
		CViewRenderer::CTextureId MiniTextureId;
		bool isMissionSpot;
		sint32 TxW;
		sint32 TxH;
		sint32 MTxW;
		sint32 MTxH;
	};

private:
	CRadarSpotDesc		_SpotDescriptions[NbRadarSpotIds];


	class CDBMissionIconqObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update( NLMISC::ICDBNode *node);
		bool _displayMissionSpots;
	};
	CDBMissionIconqObs _MissionIconsObs;
	
	class CDBMiniMissionSpotsObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update( NLMISC::ICDBNode *node);
		bool _displayMiniMissionSpots;

	};
	CDBMiniMissionSpotsObs _MiniMissionSpotsObs;

};

#endif // RY_VIEW_RADAR_H

/* End of view_radar.h */
